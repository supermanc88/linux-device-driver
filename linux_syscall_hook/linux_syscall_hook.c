#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <asm/errno.h>
#include <linux/unistd.h>

ssize_t syscall_hook_read (struct file * filp, char __user * buf, size_t count, loff_t * offset);
ssize_t syscall_hook_write (struct file * filp, const char __user * buf, size_t count, loff_t * offset);
int syscall_hook_open (struct inode * node, struct file * filp);
int syscall_hook_release (struct inode * node , struct file * filp);
asmlinkage long sys_read_hooked(unsigned int fd, char __user *buf, size_t count);

struct cdev * my_cdev = NULL;
dev_t dev_num;
struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = syscall_hook_read,
    .write = syscall_hook_write,
    .open = syscall_hook_open,
    .release = syscall_hook_release,
};



typedef long (* func_pointer_type)(unsigned int fd, char __user *buf, size_t count);

func_pointer_type * __sys_call_table;

func_pointer_type orignal_sys_read;

asmlinkage long sys_read_hooked(unsigned int fd, char __user *buf, size_t count)
{
    printk(KERN_INFO "enter sys_read_hooked\n");
    return orignal_sys_read(fd, buf, count);
}

uint close_cr0(){

    uint cr0 =0;

    uint ret = 0;

    __asm__ __volatile__("movq  %%cr0, %%rax"
                        :"=a"(cr0)
                        );

    ret = cr0;

    cr0 &= 0xfffeffff;

    __asm__ __volatile__( "movq %%rax, %%cr0"
                        :
                        :"a"(cr0)
                        );

    return ret;

}

void open_cr0(uint oldVar){

        __asm__ __volatile__("movq %%rax, %%cr0"

                :

                :"a"(oldVar)

        );
}



ssize_t syscall_hook_read (struct file * filp, char __user * buf, size_t count, loff_t * offset)
{
    return count;
}
ssize_t syscall_hook_write (struct file * filp, const char __user * buf, size_t count, loff_t * offset)
{
    /*
     *  在这个里面 做  sys_call_table  hook
     *  2.6.32-696.el6.x86_64
     *  ffffffff816005c0 R sys_call_table
     *  在 Module.symvers 中 未发现有导出， 所以此 符号不 导出
     ***************************************************************************************/
    uint cr0_value;

    __sys_call_table = 0xffffffff816005c0;

    __asm__("cli;\n\t");
    cr0_value = close_cr0();

    orignal_sys_read = __sys_call_table[__NR_read];
    __sys_call_table[__NR_read] = &sys_read_hooked;

    open_cr0(cr0_value);
    __asm__("sti;\n\t");

    return count;
}
int syscall_hook_open (struct inode * node, struct file * filp)
{
    filp->private_data = node->i_cdev;
    return 0;
}
int syscall_hook_release (struct inode * node , struct file * filp)
{
    filp->private_data = NULL;
    return 0;
}


static int __init syscall_hook_init(void)
{
    int ret;
    printk(KERN_INFO "syscall_hook_init\n");

    ret = alloc_chrdev_region(&dev_num, 0, 1, "syscall_hook");

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

static void __exit syscall_hook_exit(void)
{
    uint cr0_value;
    kfree(my_cdev);
    unregister_chrdev_region(dev_num, 1);


    __asm__("cli;\n\t");
    cr0_value = close_cr0();

    __sys_call_table[__NR_read] = orignal_sys_read;

    open_cr0(cr0_value);
    __asm__("sti;\n\t");

    printk(KERN_INFO "syscall_hook_exit\n");
}

module_init(syscall_hook_init);
module_exit(syscall_hook_exit);
MODULE_LICENSE("GPL");
