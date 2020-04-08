#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <asm/desc.h>
#include <asm/desc_defs.h>
#include <asm/pgtable.h>
#include <linux/unistd.h>
#include <linux/fs.h>
#include <linux/cdev.h>

typedef long (*sys_call_ptr_t)(void);
typedef asmlinkage long (* orig_read_t)(unsigned int fd, char __user *buf, size_t count);

orig_read_t orig_read = 0;

// memory protection shinanigans
unsigned int level;
pte_t *pte;

unsigned long * _sys_call_table;

void get_idt_table(gate_desc ** idt_table_pointer)
{
    struct desc_ptr idt;
    gate_desc * each_vector;
    unsigned long i;
    int idt_index = 0;

    __asm__("sidt %0":"=m"(idt));

    printk(KERN_INFO "IDT starting at %p, idt size = %x, single ldt_desc size = %d\n",
           idt.address, idt.size, sizeof(ldt_desc));

    for(i = idt.address; i < idt.address + idt.size; i+=sizeof (ldt_desc)){
        each_vector = (gate_desc *)i;

        printk(KERN_INFO "idt index = %d, segment = %x, offset = 0x%08x 0x%04x 0x%04x\n",
               idt_index, each_vector->segment,
               each_vector->offset_high,
               each_vector->offset_middle,
               each_vector->offset_low);
        idt_index++;
    }

    *idt_table_pointer = idt.address;

}


asmlinkage long hooked_read(unsigned int fd, char __user *buf, size_t count)
{
    printk(KERN_INFO "enter my hooked read\n");

    return orig_read(fd, buf, count);
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
    return count;
}
int syscall_hook_open (struct inode * node, struct file * filp)
{
    filp->private_data = node->i_cdev;
}
int syscall_hook_release (struct inode * node, struct file * filp)
{
    return 0;
}

dev_t dev_num;

struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = syscall_hook_read,
    .write = syscall_hook_write,
    .open = syscall_hook_open,
    .release = syscall_hook_release,
};

struct cdev * my_cdev = NULL;

static int __init syscall_hook_init(void)
{
//    gate_desc * idt_table;
//    gate_desc * system_call_gate;           // 0x80  中断 描述 地址

//    unsigned long system_call_off;
//    unsigned char * system_call_addr;       // 中断例程地址
//    int lowoff;
    int oldval;
    int ret;

//    printk(KERN_INFO "syscall hook init\n");

//    get_idt_table(&idt_table);

//    system_call_gate = &idt_table[0x80];

////    system_call_off = (system_call_gate->offset_high << 32 ) |
////            (system_call_gate->offset_middle << 16) |
////            (system_call_gate->offset_low);
//    system_call_off = system_call_gate;

//    system_call_off = system_call_off << 32;

//    lowoff = system_call_gate->offset_middle;

//    lowoff = (lowoff << 16) + system_call_gate->offset_low;

//    system_call_off = system_call_off | lowoff;


//    system_call_addr = (unsigned char *)system_call_off;

//    printk(KERN_INFO "0x80 interrupt addr = %p, sizeof(unsigned long) = %d\n",
//           system_call_addr,
//           sizeof (unsigned long));


      // sys_call_table的 查找还不确定

//    _sys_call_table = 0xffffffff815925e0;

//    orig_read = (orig_read_t)(_sys_call_table[__NR_read]);

//    oldval = close_cr0();

////    pte = lookup_address((unsigned long)_sys_call_table, &level);

////    // change PTE to allow writing
////    set_pte_atomic(pte, pte_mkwrite(*pte));

//    _sys_call_table[__NR_read] = hooked_read;

    open_cr0(oldval);

    printk(KERN_INFO "hooked install\n");

    //  添加一 段  字符设备 代码  用来 控制

    ret = alloc_chrdev_region(&dev_num, 0, 1, "syscall_hook");

    if(ret){
        printk(KERN_INFO "alloc chrdev region failed\n");
        return ret;
    }

    my_cdev = cdev_alloc();

    if(my_cdev == NULL){
        printk(KERN_INFO "cdev alloc failed\n");
        return -ENOMEM;
    }

    cdev_init(my_cdev, &fops);

    ret = cdev_add(my_cdev, dev_num, 1);

    if(ret){
        printk(KERN_INFO "cdev add failed\n");
        return ret;
    }

    return 0;
}

static void __exit syscall_hook_exit(void)
{
    int oldval;
    printk(KERN_INFO "syscall hook exit\n");


    cdev_del(my_cdev);
    unregister_chrdev_region(dev_num, 1);


//    if(orig_read != NULL){

//        oldval = close_cr0();

//        _sys_call_table[__NR_read] = orig_read;

////        set_pte_atomic(pte, pte_clear_flags(*pte, _PAGE_RW));

//        open_cr0(oldval);
//    }
}


module_init(syscall_hook_init);
module_exit(syscall_hook_exit);
MODULE_LICENSE("GPL");
