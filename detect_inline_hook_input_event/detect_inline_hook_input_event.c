#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <asm/errno.h>

#include <linux/input.h>

ssize_t detect_inline_hook_input_event_read (struct file * filp, char __user * buf, size_t count, loff_t * offset);
ssize_t detect_inline_hook_input_event_write (struct file * filp, const char __user * buf, size_t count, loff_t * offset);
int detect_inline_hook_input_event_open (struct inode * node, struct file * filp);
int detect_inline_hook_input_event_release (struct inode * node , struct file * filp);

// 2.6.32-696.el6.x86_64
unsigned char signature_code[145] = {
    0x55, 0x48, 0x89, 0xe5, 0x48, 0x83, 0xec, 0x40, 0x48, 0x89, 0x5d, 0xd8, 0x4c,
    0x89, 0x65, 0xe0, 0x4c, 0x89, 0x6d, 0xe8, 0x4c, 0x89, 0x75, 0xf0, 0x4c, 0x89,
    0x7d, 0xf8, 0x0f, 0x1f, 0x44, 0x00, 0x00, 0x83, 0xfe, 0x1f, 0x49, 0x89, 0xfc,
    0x89, 0xf3, 0x41, 0x89, 0xd5, 0x77, 0x0a, 0x0f, 0xa3, 0x77, 0x20, 0x19, 0xc0,
    0x85, 0xc0, 0x75, 0x18, 0x48, 0x8b, 0x5d, 0xd8, 0x4c, 0x8b, 0x65, 0xe0, 0x4c,
    0x8b, 0x6d, 0xe8, 0x4c, 0x8b, 0x75, 0xf0, 0x4c, 0x8b, 0x7d, 0xf8, 0xc9, 0xc3,
    0x66, 0x90, 0x4c, 0x8d, 0xb7, 0xf8, 0x07, 0x00, 0x00, 0x89, 0x4d, 0xc8, 0x4c,
    0x89, 0xf7, 0xe8, 0x2e, 0xff, 0x13, 0x00, 0x8b, 0x4d, 0xc8, 0x44, 0x89, 0xee,
    0x89, 0xdf, 0x49, 0x89, 0xc7, 0x89, 0xca, 0xe8, 0x5c, 0x4e, 0xf3, 0xff, 0x8b,
    0x4d, 0xc8, 0x89, 0xde, 0x4c, 0x89, 0xe7, 0x44, 0x89, 0xea, 0xe8, 0xbc, 0xf6,
    0xff, 0xff, 0x4c, 0x89, 0xfe, 0x4c, 0x89, 0xf7, 0xe8, 0x71, 0x00, 0x14, 0x00,
    0xeb, 0xa7
};

unsigned char current_input_event_code[145] = {0};

struct cdev * my_cdev = NULL;
dev_t dev_num;
struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = detect_inline_hook_input_event_read,
    .write = detect_inline_hook_input_event_write,
    .open = detect_inline_hook_input_event_open,
    .release = detect_inline_hook_input_event_release,
};


ssize_t detect_inline_hook_input_event_read (struct file * filp, char __user * buf, size_t count, loff_t * offset)
{
    return count;
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

void restore_inline_hook_input_event(void)
{
    uint old_value;
    printk(KERN_INFO "enter restore_inline_hook_input_event\n");

    __asm__("cli;");

    old_value = close_cr0();



    open_cr0(old_value);

    __asm__("sti;");

}

ssize_t detect_inline_hook_input_event_write (struct file * filp, const char __user * buf, size_t count, loff_t * offset)
{
    unsigned char * input_event_addr = &input_event;

    int i;

    for(i=0; i<145; i++){
        printk(KERN_INFO "0x%02x\n", input_event_addr[i]);
        current_input_event_code[i] = input_event_addr[i];
    }


    // compare tow array content
    //  找个 东西 记录

    for(i=0; i<145; i++){
        if(signature_code[i] != current_input_event_code[i]){
            printk(KERN_INFO "have a inline hook at 0x%p\n", (input_event_addr + i));
        }
    }


    restore_inline_hook_input_event();

    return count;
}
int detect_inline_hook_input_event_open (struct inode * node, struct file * filp)
{
    filp->private_data = node->i_cdev;
    return 0;
}
int detect_inline_hook_input_event_release (struct inode * node , struct file * filp)
{
    filp->private_data = NULL;
    return 0;
}


static int __init detect_inline_hook_input_event_init(void)
{
    int ret;
    printk(KERN_INFO "detect_inline_hook_input_event_init\n");

    ret = alloc_chrdev_region(&dev_num, 0, 1, "detect_inline_hook_input_event");

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

static void __exit detect_inline_hook_input_event_exit(void)
{
    kfree(my_cdev);
    unregister_chrdev_region(dev_num, 1);
    printk(KERN_INFO "detect_inline_hook_input_event_exit\n");
}

module_init(detect_inline_hook_input_event_init);
module_exit(detect_inline_hook_input_event_exit);
MODULE_LICENSE("GPL");
