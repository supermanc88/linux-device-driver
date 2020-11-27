#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kallsyms.h>


static int __init get_sys_call_table_init(void)
{

    // 在使用kallsyms_lookup_name 函数时，先查看此函数是否导出

    void *sys_call_table_addr = kallsyms_lookup_name("sys_call_table");
    void *vfs_read_addr = kallsyms_lookup_name("vfs_read");


    printk("%s (sys_call_table = %p, vfs_read = %p\n", __func__, sys_call_table_addr, vfs_read_addr);

    return 0;
}


static void __exit get_sys_call_table_exit(void)
{

}



module_init(get_sys_call_table_init);
module_exit(get_sys_call_table_exit);

MODULE_LICENSE("GPL");
