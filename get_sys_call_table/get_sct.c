#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kallsyms.h>
#include <linux/syscalls.h>
#include <asm/uaccess.h>
#include <linux/sched.h>

unsigned long long *sys_call_table_addr;

asmlinkage long (*orig_exit)(int error_code);
asmlinkage long new_exit(int error_code)
{
    printk("%s enter current pid = %d, comm = %s\n", __func__, current->pid, current->comm);


    return orig_exit(error_code);
}


void install_hook(void)
{
    __asm__("cli;");
    write_cr0(read_cr0() & (~0x10000));
    orig_exit = sys_call_table_addr[__NR_exit];
    sys_call_table_addr[__NR_exit] = new_exit;
    write_cr0(read_cr0() | 0x10000);
    __asm__("sti;");
}


void uninstall_hook(void)
{
    __asm__("cli;");
    write_cr0(read_cr0() & (~0x10000));
    sys_call_table_addr[__NR_exit] = orig_exit;
    write_cr0(read_cr0() | 0x10000);
    __asm__("sti;");
}



static int __init get_sys_call_table_init(void)
{

    // 在使用kallsyms_lookup_name 函数时，先查看此函数是否导出

    sys_call_table_addr = kallsyms_lookup_name("sys_call_table");
    void *vfs_read_addr = kallsyms_lookup_name("vfs_read");

    // 这个函数是被inline优化掉的，测试看看可不可以获取到地址,经测试，不可获取
    void *rcu_read_lock = kallsyms_lookup_name("rcu_read_lock");


    install_hook();

    printk("%s (sys_call_table = %p, vfs_read = %p, rcu_read_lock = %p\n", __func__, sys_call_table_addr, vfs_read_addr, rcu_read_lock);

    return 0;
}


static void __exit get_sys_call_table_exit(void)
{

}



module_init(get_sys_call_table_init);
module_exit(get_sys_call_table_exit);

MODULE_LICENSE("GPL");
