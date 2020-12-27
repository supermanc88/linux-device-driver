#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <asm/processor.h>
#include <asm/uaccess.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/file.h>


void process_info_print(void)
{
    struct task_struct *task_list;
    size_t process_counter = 0;
    char *pathname, *p = NULL;
    struct mm_struct *mm;


    for_each_process(task_list) {
        mm  = task_list->mm;

        if (mm) {
            down_read(&mm->mmap_sem);

            if (mm->exe_file) {
                pathname = kmalloc(256, GFP_KERNEL);
                if (pathname) {
                    p = d_path(&mm->exe_file->f_path, pathname, 256);
                }
            }

            up_read(&mm->mmap_sem);
        }

        if (p) {
            printk("%s comm = [%s], path = [%s], real parent pid = [%d] parent pid = [%d], pid = [%d]\n", __func__,
                    task_list->comm, p, task_list->real_parent->pid, task_list->parent->pid, task_list->pid);
        } else {
            printk("%s comm = [%s], real parent pid = [%d] parent pid = [%d], pid = [%d]\n", __func__,
                    task_list->comm, task_list->real_parent->pid, task_list->parent->pid, task_list->pid);
        }

        process_counter++;
    }

    printk("%s process_counter = [%d]\n", __func__, process_counter);
}


static int __init list_process_init(void)
{
    int rc = 0;

    printk("%s\n", __func__);

    process_info_print();

    return rc;
}




static void __exit list_process_exit(void)
{
    printk("%s\n", __func__);
}



module_init(list_process_init);
module_exit(list_process_exit);
MODULE_LICENSE("GPL");