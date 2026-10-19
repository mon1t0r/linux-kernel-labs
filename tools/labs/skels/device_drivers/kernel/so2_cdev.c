/*
 * Character device drivers lab
 *
 * All tasks
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/wait.h>

#include "../include/so2_cdev.h"

MODULE_DESCRIPTION("SO2 character device");
MODULE_AUTHOR("SO2");
MODULE_LICENSE("GPL");

#define LOG_LEVEL	KERN_INFO

/* Device or resource busy error if MAJOR version is occupied */
#define MY_MAJOR		42
#define MY_MINOR		0
#define NUM_MINORS		1
#define MODULE_NAME		"so2_cdev"
#define MESSAGE			"hello\n"
#define IOCTL_MESSAGE		"Hello ioctl"

#ifndef BUFSIZ
#define BUFSIZ		4096
#endif


struct so2_device_data {
    struct cdev cdev;
    atomic_t is_busy;
    char buf[BUFSIZ];
    size_t buf_len;
	/* TODO 7: extra members for home */
};

struct so2_device_data devs[NUM_MINORS];

static int so2_cdev_open(struct inode *inode, struct file *file)
{
	struct so2_device_data *data;
    int was_busy;

    data = container_of(inode->i_cdev, struct so2_device_data, cdev);

    was_busy = atomic_cmpxchg(&data->is_busy, 0, 1);
    if(was_busy) {
        return -EBUSY;
    }

	file->private_data = data;

    pr_info("Device opened\n");

#if 0
	set_current_state(TASK_INTERRUPTIBLE);
	schedule_timeout(10 * HZ);
#endif

	return 0;
}

static int
so2_cdev_release(struct inode *inode, struct file *file)
{
    struct so2_device_data *data;

    data = (struct so2_device_data *) file->private_data;

    atomic_set(&data->is_busy, 0);

    pr_info("Device closed\n");

	return 0;
}

static ssize_t
so2_cdev_read(struct file *file,
		char __user *user_buffer,
		size_t size, loff_t *offset)
{
	struct so2_device_data *data;
	size_t to_read;
    unsigned long copy_res;

    data = (struct so2_device_data *) file->private_data;

    if((data->buf_len - *offset) > size) {
        to_read = size;
    } else {
        to_read = data->buf_len - *offset;
    }

    copy_res = copy_to_user(user_buffer, data->buf, to_read);
    if(copy_res != 0) {
        return -EFAULT;
    }

    (*offset) += to_read;

	return to_read;
}

static ssize_t
so2_cdev_write(struct file *file,
		const char __user *user_buffer,
		size_t size, loff_t *offset)
{
	struct so2_device_data *data;
    size_t to_write;
    unsigned long copy_res;

    data = (struct so2_device_data *) file->private_data;

    if(*offset + size > BUFSIZ) {
        to_write = BUFSIZ - *offset;
    } else {
        to_write = size;
    }

    copy_res = copy_from_user(data->buf + *offset, user_buffer, to_write);
    if(copy_res != 0) {
        return -EFAULT;
    }

    (*offset) += to_write;

    data->buf_len = *offset;

	/* TODO 7: extra tasks for home */

	return size;
}

static long
so2_cdev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct so2_device_data *data =
		(struct so2_device_data *) file->private_data;
	int ret = 0;
	int remains;

	switch (cmd) {
    case MY_IOCTL_PRINT:
        printk(LOG_LEVEL "%s\n", IOCTL_MESSAGE);
        break;

	/* TODO 7: extra tasks, for home */
	default:
		ret = -EINVAL;
	}

	return ret;
}

static const struct file_operations so2_fops = {
	.owner = THIS_MODULE,
    .open = so2_cdev_open,
    .release = so2_cdev_release,
    .read = so2_cdev_read,
    .write = so2_cdev_write,
    .write = so2_cdev_write,
    .unlocked_ioctl = so2_cdev_ioctl
};

static int so2_cdev_init(void)
{
	int err;
	int i;

    err = register_chrdev_region(MKDEV(MY_MAJOR, MY_MINOR), NUM_MINORS, MODULE_NAME);
    if(err != 0) {
        return err;
    }
    pr_info("Register successfull\n");

    for (i = 0; i < NUM_MINORS; i++) {
        atomic_set(&devs[i].is_busy, 0);
        strcpy(devs[i].buf, MESSAGE);
        devs[i].buf_len = sizeof(MESSAGE) - 1;

        cdev_init(&devs[i].cdev, &so2_fops);
        cdev_add(&devs[i].cdev, MKDEV(MY_MAJOR, i), 1);
    }

	return 0;
}

static void so2_cdev_exit(void)
{
	int i;

	for (i = 0; i < NUM_MINORS; i++) {
        cdev_del(&devs[i].cdev);
    }

    unregister_chrdev_region(MKDEV(MY_MAJOR, MY_MINOR), NUM_MINORS);
    pr_info("Unregister successfull\n");
}

module_init(so2_cdev_init);
module_exit(so2_cdev_exit);

