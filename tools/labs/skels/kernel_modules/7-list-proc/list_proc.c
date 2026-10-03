#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>

MODULE_DESCRIPTION("List current processes");
MODULE_AUTHOR("Kernel Hacker");
MODULE_LICENSE("GPL");

static int my_proc_init(void)
{
    struct task_struct *p;
    p = current;
    pr_info("PID is %d, exec name is %s\n", p->pid, p->comm);

    return 0;
}

static void my_proc_exit(void)
{
    struct task_struct *p;
    p = current;
    pr_info("PID is %d, exec name is %s\n", p->pid, p->comm);
}

module_init(my_proc_init);
module_exit(my_proc_exit);
