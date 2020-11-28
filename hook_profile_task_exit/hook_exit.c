#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/kallsyms.h>
#include <linux/uaccess.h>

typedef void (*profile_task_exit_fn)(struct task_struct *task);

profile_task_exit_fn ori_profile_task_exit = NULL;
unsigned long do_exit_addr = 0;

void my_profile_task_exit(struct task_struct *task)
{
    printk("%s current pid = %d, process name = %s\n", __func__, task->pid, task->comm);


    return ori_profile_task_exit(task);
}



int replace_kernel_func(unsigned long func_handler, unsigned long ori_func, unsigned long my_func)
{
    unsigned char *temp_addr = (unsigned char *)func_handler + 5;

    int i = 0;

    do {

        if (*temp_addr == 0xe8) {
            unsigned int *relation_offset = (unsigned int *)(temp_addr + 1);

            if ((unsigned long)temp_addr + 5 + *relation_offset == ori_func) {

                // 这里就是对比一下 看找到的地址是不是和传入的原地址相同，保险

                __asm__("cli;");
                write_cr0(read_cr0() & (~0x10000));

                *relation_offset = my_func - (unsigned long)temp_addr - 5;

                write_cr0(read_cr0() | 0x10000);
                __asm__("sti;");

                printk("%s replace_kernel_func ok\n", __func__);

                return 0;
            }
            printk("%s replace_kernel_func addr = %d\n", __func__, temp_addr);
        }

        temp_addr++;
    }while(i++ < 100);


    printk("%s replace_kernel_func failed\n", __func__);

    return -1;
}




static int __init hook_module_init(void)
{


    ori_profile_task_exit = kallsyms_lookup_name("profile_task_exit");

    do_exit_addr = kallsyms_lookup_name("do_exit");


    printk("%s profile_task_exit = %p, do_exit = %p\n", __func__, ori_profile_task_exit, do_exit_addr);


    replace_kernel_func(do_exit_addr, (unsigned long)ori_profile_task_exit, (unsigned long)my_profile_task_exit);

    return 0;
}



static void __exit hook_module_exit(void)
{
    replace_kernel_func(do_exit_addr, (unsigned long)my_profile_task_exit, (unsigned long)ori_profile_task_exit);
}



module_init(hook_module_init);
module_exit(hook_module_exit);

MODULE_LICENSE("GPL");
