#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
//#include <asm/errno.h>
#include <linux/device.h>               //class_create

// 相关参考： https://www.cnblogs.com/skywang12345/archive/2013/05/15/driver_class.html

/*
    在linux3.16基础上开发
*/

#define BUFF_SIZE 4096
#define MAGIC_NUM           'K'
#define CLEAR_DATA          _IO(MAGIC_NUM, 0)

ssize_t module_cdev_read (struct file * filp, char __user * buf, size_t count, loff_t * offset);
ssize_t module_cdev_write (struct file * filp, const char __user * buf, size_t count, loff_t * offset);
int module_cdev_open (struct inode * node, struct file * filp);
int module_cdev_release (struct inode * node , struct file * filp);
loff_t module_cdev_llseek (struct file *filp, loff_t offset, int whence);
int module_cdev_ioctl (struct file *filp, unsigned int cmd, unsigned long arg);

struct cdev * my_cdev = NULL;
dev_t dev_num;

char dev_buf[BUFF_SIZE] = {0};


struct class *module_class;
struct device *module_class_device;

struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = module_cdev_read,
    .write = module_cdev_write,
    .open = module_cdev_open,
    .release = module_cdev_release,
    .llseek = module_cdev_llseek,
    .unlocked_ioctl = module_cdev_ioctl,
};


int module_cdev_ioctl (struct file *filp, unsigned int cmd, unsigned long arg)
{
    char *my_buf = filp->private_data;

    switch (cmd) {
    case CLEAR_DATA:
        memset(my_buf, 0, BUFF_SIZE);
        break;
    default:
        return -EINVAL;
    }

    return 0;
}


loff_t module_cdev_llseek (struct file *filp, loff_t offset, int whence)
{
    loff_t new_pos;
    switch (whence) {
    case SEEK_SET:
        new_pos = offset;
        break;
    case SEEK_CUR:
        new_pos = filp->f_pos + offset;
        break;
    case SEEK_END:
        new_pos = BUFF_SIZE - 1 + offset;
        break;
    }

    if (new_pos < 0 || new_pos > BUFF_SIZE)
        return -EINVAL;
    filp->f_pos = new_pos;

    return new_pos;

}

ssize_t module_cdev_read (struct file * filp, char __user * buf, size_t count, loff_t * offset)
{
    unsigned long pos = *offset;
    int rc = 0;
    unsigned int size = count;
    char *my_buf = filp->private_data;

    if (copy_to_user(buf, my_buf+pos, size)) {
        return -EFAULT;
    } else {
        *offset += size;
        rc = size;
    }

    return rc;
}
ssize_t module_cdev_write (struct file * filp, const char __user * buf, size_t count, loff_t * offset)
{
    unsigned long pos = *offset;
    int rc = 0;
    unsigned int size = count;
    char *my_buf = filp->private_data;


    if (copy_from_user(my_buf+pos, buf, size)) {
        return -EFAULT;
    } else {
       *offset += size;
        rc = size;
    }



    return rc;
}
int module_cdev_open (struct inode * node, struct file * filp)
{
    filp->private_data = dev_buf;
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
    printk(KERN_INFO "module_cdev_init\n");

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


    // 在这里自动创建设备节点
    // 内核中定义了struct class结构体，一个class结构体类型对应一个类，内核同时提供了class_create函数，可以用它来创建一个类，
    // 这个类存放于sysfs下面，一旦创建好了这个类，再调用device_create函数在/dev目录下创建相应的设备节点。
    // 这样，加载模块的时候，用户空间中的udev会自动响应device_create函数，去/sysfs下寻找对应的类，从而创建设备节点。
    module_class = class_create(THIS_MODULE, "module_cdev_class");
    module_class_device = device_create(module_class, NULL, dev_num, NULL, "module_cdev_name");


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

    device_destroy(module_class, dev_num);
    class_destroy(module_class);

    printk(KERN_INFO "module_cdev_exit\n");
}

module_init(module_cdev_init);
module_exit(module_cdev_exit);
MODULE_LICENSE("GPL");
