#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/kallsyms.h>
#include <linux/input.h>


/* 对于每个探测，用户需要分配一个kprobe对象*/
static struct kprobe kp = {
    0//.symbol_name    = "_do_fork",
};

// 		ARM64
//			参数1~参数8 分别保存到 X0~X7 寄存器中 ，剩下的参数从右往左依次入栈，被调用者实现栈平衡，返回值存放在 X0 中。
/* 在被探测指令执行前，将调用预处理例程 pre_handler，用户需要定义该例程的操作*/
static int handler_pre(struct kprobe *p, struct pt_regs *regs)
{
    printk("%s dev = [%p], type = [%ld], code = [%ld], value =[%ld]\n",
           __func__, regs->regs[0], regs->regs[1], regs->regs[2], regs->regs[3]);
    printk("%s regs->regs[0] = [%016lx]\n", __func__, regs->regs[0]);
    printk("%s regs->regs[1] = [%016lx]\n", __func__, regs->regs[1]);
    printk("%s regs->regs[2] = [%016lx]\n", __func__, regs->regs[2]);
    printk("%s regs->regs[3] = [%016lx]\n", __func__, regs->regs[3]);
    printk("%s regs->regs[4] = [%016lx]\n", __func__, regs->regs[4]);
    printk("%s regs->regs[5] = [%016lx]\n", __func__, regs->regs[5]);
    printk("%s regs->regs[6] = [%016lx]\n", __func__, regs->regs[6]);
    printk("%s regs->regs[7] = [%016lx]\n", __func__, regs->regs[7]);

    if (regs->regs[1] == EV_KEY) {
        regs->regs[2] = regs->regs[2] + 1;
    }
    /* 在这里可以调用内核接口函数dump_stack打印出栈的内容*/
    //dump_stack();
    return 0;
}
 
/* 在被探测指令执行后，kprobe调用后处理例程post_handler */
static void handler_post(struct kprobe *p, struct pt_regs *regs,
                unsigned long flags)
{
	printk("%s\n", __func__);
}
 
/*在pre-handler或post-handler中的任何指令或者kprobe单步执行的被探测指令产生了例外时，会调用fault_handler*/
static int handler_fault(struct kprobe *p, struct pt_regs *regs, int trapnr)
{
    printk(KERN_DEBUG "fault_handler: p->addr = 0x%p, trap #%dn",
        p->addr, trapnr);
    /* 不处理错误时应该返回*/
    return 0;
}
 
/*初始化内核模块*/
static int __init kprobe_init(void)
{
    int ret=0;
    kp.pre_handler = handler_pre;
    kp.post_handler = handler_post;
    kp.fault_handler = handler_fault;
 
    unsigned long addr = kallsyms_lookup_name("input_handle_event");
    printk("input_handle_event addr = %016lx\n", addr);
    kp.addr = addr;
    ret = register_kprobe(&kp);  /*注册kprobe*/
    if (ret < 0) {
        printk("register_kprobe failed, returned %d\n", ret);
        return ret;
    }
    printk("Planted kprobe at %016lx\n", kp.addr);
    return 0;
}

static void __exit kprobe_exit(void)
{
    unregister_kprobe(&kp);
    printk("kprobe at %016lx unregistered\n", kp.addr);
}

module_init(kprobe_init)
module_exit(kprobe_exit)
MODULE_LICENSE("GPL");
