#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>


static int __init module_template_init(void)
{
    printk(KERN_INFO "module template init\n");
    return 0;
}

static void __exit module_template_exit(void)
{
    printk(KERN_INFO "module template exit\n");
}

module_init(module_template_init);
module_exit(module_template_exit);
MODULE_LICENSE("GPL");
