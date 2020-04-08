#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>


static int __init get_func_module_init(void)
{
    printk(KERN_INFO "get func module init\n");
    return 0;
}

static void __exit get_func_module_exit(void)
{
    printk(KERN_INFO "get func module exit\n");
}


module_init(get_func_module_init);
module_exit(get_func_module_exit);
MODULE_LICENSE("GPL");
