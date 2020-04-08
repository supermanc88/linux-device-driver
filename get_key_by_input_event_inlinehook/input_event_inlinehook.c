#include <linux/init.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/irqflags.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/cdev.h>


/*
0  --->  55
1  --->  48
2  --->  89
3  --->  e5
4  --->  48
5  --->  83
6  --->  ec
7  --->  40
8  --->  48
9  --->  89
10  --->  5d
11  --->  d8
12  --->  4c
13  --->  89
14  --->  65
15  --->  e0
16  --->  4c
17  --->  89
18  --->  6d
19  --->  e8

| 55                       | push rbp                                |
| 48:89E5                  | mov rbp,rsp                             |
| 48:83EC 40               | sub rsp,40                              |
| 48:895D D8               | mov qword ptr ss:[rbp-28],rbx           |
| 4C:8965 E0               | mov qword ptr ss:[rbp-20],r12           |
| 4C:896D E8               | mov qword ptr ss:[rbp-18],r13           |


E9 XXXXXXXX         jmp current_addr + xxxxxxxx

*/

unsigned long g_input_event_addr;
unsigned long g_input_event_ret_addr;

void dispaly_input_event_opcode(void)
{
    int i;
    unsigned char * input_event_addr = (unsigned char *)&input_event;
    for(i=0; i<20; i++){
        printk(KERN_INFO "%d  --->  %x\n", i, input_event_addr[i]);
    }
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


void print_current_key(void)
{

    unsigned long v_rcx, v_rdx, v_rdi, v_rsi;

    __asm__ __volatile("push %rax;\n\t"
                       "push %rbx;\n\t"
                       "push %rcx;\n\t"
                       "push %rdx;\n\t"
                       "push %rdi;\n\t"
                       "push %rsi;\n\t"
                       "push %r8;\n\t"
                       "push %r9;\n\t"
                       "push %r10;\n\t"
                       "push %r11;\n\t"
                       "push %r12;\n\t"
                       "push %r13;\n\t"
                       "push %r14;\n\t"
                       "push %r15;\n\t");

    __asm__ __volatile("movq %%rcx, %%rax;\n\t"
                       :"=a"(v_rcx));
    __asm__ __volatile("movq %%rdx, %%rax;\n\t"
                       :"=a"(v_rdx));
    __asm__ __volatile("movq %%rdi, %%rax;\n\t"
                       :"=a"(v_rdi));
    __asm__ __volatile("movq %%rsi, %%rax;\n\t"
                       :"=a"(v_rsi));

    printk(KERN_INFO "rcx = %ld, rdx = %ld, rdi = %ld, rsi = %ld\n",
           v_rcx, v_rdx, v_rdi, v_rsi);

    __asm__ __volatile("pop %r15;\n\t"
                       "pop %r14;\n\t"
                       "pop %r13;\n\t"
                       "pop %r12;\n\t"
                       "pop %r11;\n\t"
                       "pop %r10;\n\t"
                       "pop %r9;\n\t"
                       "pop %r8;\n\t"
                       "pop %rsi;\n\t"
                       "pop %rdi;\n\t"
                       "pop %rdx;\n\t"
                       "pop %rcx;\n\t"
                       "pop %rbx;\n\t"
                       "pop %rax;\n\t");
}


void __attribute__((naked)) my_input_event(void)
{
    //  这里 堆栈 平衡出 问题了
//    printk(KERN_INFO "enter my_input_event\n");
    // naked函数 好像不 生效，    所以 跳转+9

//    __asm__ __volatile__("call *print_current_key\n");

    __asm__ __volatile__("push %rbp;\n"
                         "call print_current_key;\n"
                         "mov %rsp, %rbp;\n"
                         "sub $0x40, %rsp;");

    __asm__ __volatile__("jmp *g_input_event_ret_addr");
}


void install_hook(void)
{
    unsigned char jmpcode[8] = {0xE9, 0,0,0,0,0x90,0x90,0x90};

    uint oldval;

    unsigned long jmpsize = my_input_event - g_input_event_addr - 5 + 9;

    memcpy(jmpcode+1, &jmpsize, 4);


    //  关 保护
    __asm__("cli;");
//    local_irq_disable();
//    local_irq_save();
    oldval = close_cr0();


    memcpy(&input_event, jmpcode, 8);


    // 开 保护
    open_cr0(oldval);
    __asm__("sti;");

    printk(KERN_INFO "install hook\n");
}

ssize_t inlinehook_read (struct file * filp, char __user * buf, size_t count, loff_t * offset)
{
    return 0;
}
ssize_t inlinehook_write (struct file * filp, const char __user * buf, size_t count, loff_t * offset)
{
    install_hook();
    return count;
}
int inlinehook_open (struct inode * node, struct file * filp)
{
    filp->private_data = node->i_cdev;
    return 0;
}

struct file_operations fops ={
    .owner = THIS_MODULE,
    .read = inlinehook_read,
    .write = inlinehook_write,
    .open = inlinehook_open,
};


//dev_t dev_number;
struct cdev * cdev;

static int __init inlinehook_init(void)
{
    int ret;
    g_input_event_addr = &input_event;
    g_input_event_ret_addr = g_input_event_addr + 8;
    printk(KERN_INFO "inlinehook init %p\n", inlinehook_init);


    //  注册一个 字符设备
//    alloc_chrdev_region(&dev_number, 0, 1, "inlinehook");

//    cdev = cdev_alloc();

//    cdev_init(cdev, &fops);

//    cdev->owner = THIS_MODULE;

//    ret = cdev_add(cdev, dev_number, 1);

//    if(ret){
//        printk(KERN_INFO "cdev add error code %d\n", ret);
//        return ret;
//    }

    ret = register_chrdev(200, "inlinehook", &fops);

    if(ret){
        printk(KERN_INFO "cdev add error code %d\n", ret);
        return ret;
    }


//    install_hook();
    dispaly_input_event_opcode();
    return 0;
}


static void __exit inlinehook_exit(void)
{
//    cdev_del(cdev);
//    //
//    unregister_chrdev_region(dev_number, 1);
    unregister_chrdev(200, "inlinehook");
    printk(KERN_INFO "inlinehook exit\n");
}

module_init(inlinehook_init);
module_exit(inlinehook_exit);

MODULE_LICENSE("GPL");
