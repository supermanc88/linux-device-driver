#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>

static struct kprobe kp = {
        .symbol_name = "input_handle_event",
};

int handle_pre (struct kprobe * p, struct pt_regs *regs)
{
#ifdef CONFIG_X86
    pr_info("<%s> pre_handler: p->addr = %pF, ip = %lx, flags = 0x%lx\n",
            p->symbol_name, p->addr, regs->rip, regs->eflags);
#else
    pr_info("<%s> pre_handler: p->addr = %pF, pc = 0x%lx,"
            " pstate = 0x%lx\n",
        p->symbol_name, p->addr, (long)regs->pc, (long)regs->pstate);
#endif
    return 0;
}

void handle_post (struct kprobe *p, struct pt_regs *regs, unsigned long flags)
{
#ifdef CONFIG_X86
    pr_info("<%s> post_handler: p->addr = %pF, flags = 0x%lx\n",
            p->symbol_name, p->addr, regs->eflags);
#else
    pr_info("<%s> post_handler: p->addr = %pF, pstate = 0x%lx\n",
        p->symbol_name, p->addr, (long)regs->pstate);
#endif
}

int handle_fault (struct kprobe *p, struct pt_regs *regs, int trapnr)
{
    pr_info("fault_handler: p->addr = %pF, trap #%dn", p->addr, trapnr);
    /* Return 0 because we don't handle the fault. */
    return 0;
}


static int __init kprobe_init(void)
{
    kp.pre_handler = handle_pre;
    kp.post_handler = handle_post;
    kp.fault_handler = handle_fault;

    int ret = register_kprobe(&kp);
    if (ret < 0) {
        pr_err("register_kprobe failed, returned %d\n", ret);
        return ret;
    }
    pr_info("Planted kprobe at %pF\n", kp.addr);
    return 0;
}

static void __exit kprobe_exit(void)
{
    unregister_kprobe(&kp);
    pr_info("kprobe at %pF unregistered\n", kp.addr);
}

module_init(kprobe_init);
moduel_exit(kprobe_exit);
MODULE_LICENSE("GPL");
