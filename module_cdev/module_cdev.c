#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <asm/errno.h>

ssize_t module_cdev_read (struct file * filp, char __user * buf, size_t count, loff_t * offset);
ssize_t module_cdev_write (struct file * filp, const char __user * buf, size_t count, loff_t * offset);
int module_cdev_open (struct inode * node, struct file * filp);
int module_cdev_release (struct inode * node , struct file * filp);


struct cdev * my_cdev = NULL;
dev_t dev_num;
struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = module_cdev_read,
    .write = module_cdev_write,
    .open = module_cdev_open,
    .release = module_cdev_release,
};


ssize_t module_cdev_read (struct file * filp, char __user * buf, size_t count, loff_t * offset)
{
    return count;
}
ssize_t module_cdev_write (struct file * filp, const char __user * buf, size_t count, loff_t * offset)
{
    return count;
}
int module_cdev_open (struct inode * node, struct file * filp)
{
    filp->private_data = node->i_cdev;
    return 0;
}
int module_cdev_release (struct inode * node , struct file * filp)
{
    filp->private_data = NULL;
    return 0;
}


static int __init module_cdev_init(void)
{
    int ret;
    printk(KERN_INFO "module cdev init\n");

    ret = alloc_chrdev_region(&dev_num, 0, 1, "module_cdev");

    if(ret){
        printk(KERN_INFO "alloc chrdev region failed\n");
        goto error;
    }

    my_cdev = cdev_alloc();

    if(my_cdev == NULL){
        printk(KERN_INFO "cdev alloc failde\n");
        ret = -ENOMEM;
        goto error1;
    }

    cdev_init(my_cdev, &fops);

    ret = cdev_add(my_cdev, dev_num, 1);

    if(ret){
        printk(KERN_INFO "cdev add failed\n");
        goto error1;
    }


    return 0;

    error1:
    kfree(my_cdev);
    unregister_chrdev_region(dev_num, 1);

    error:
    return ret;
}

static void __exit module_cdev_exit(void)
{
    kfree(my_cdev);
    unregister_chrdev_region(dev_num, 1);
    printk(KERN_INFO "module cdev exit\n");
}

module_init(module_cdev_init);
module_exit(module_cdev_exit);
MODULE_LICENSE("GPL");
