#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/delay.h>

struct task_struct *kthread;

/*
 * kthread_should_stop
 * 当有调用kthread_stop kthread_should_stop就会  返回 true    线程 返回
 */

int thread_func(void * data)
{
//    while(true){
//        if(kthread_should_stop())
//            break;
//        printk(KERN_INFO "kthread running\n");
////        mdelay(1000);
//    }

    while(!kthread_should_stop()){
        printk(KERN_INFO "kthread running!\n");
        set_current_state(TASK_INTERRUPTIBLE);
        //   需要 学习下 时钟
        schedule_timeout(100 * HZ);
    }

    return 0;
}

static int __init module_kthread_init(void)
{
    printk(KERN_INFO "module_kthread_init\n");

//    kthread = kthread_create(thread_func, NULL, "my_kthread");
//    wake_up_process(kthread);
    kthread = kthread_run(thread_func, NULL, "my_kthread");

    return 0;
}

static void __exit module_kthread_exit(void)
{
    printk(KERN_INFO "module_kthread_exit\n");
}

module_init(module_kthread_init);
module_exit(module_kthread_exit);
MODULE_LICENSE("GPL");
