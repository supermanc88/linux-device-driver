#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <asm/uaccess.h>
#include <asm/processor.h>


#define TEST_FILE_PATH      "/root/test.txt"


char *read_line(char *buf, int buf_len, struct file *filp)
{
    int ret, i = 0;
    mm_segment_t old_fs;

    old_fs = get_fs();
    set_fs (KERNEL_DS);

    ret = filp->f_op->read(filp, buf, buf_len, &(filp->f_pos));

    set_fs (old_fs);

    if (ret <= 0)
        return NULL;

    while(buf[i++] != '\n' && i < ret);


    if (i < ret) {
        // 更新文件位置
        filp->f_pos += i - ret;
    }

    if (i < buf_len) {
        buf[i] = 0;
    }

    return buf;
}



static int __init kern_rw_file_init(void)
{
    struct file *filp;
    int ret, i;
    char buf[512];
    int offset = 0;

    printk(KERN_INFO "%s\n", __func__);


//    filp_open - open file and return file pointer

//    @filename:	path to open
//    @flags:	open flags as per the open(2) second argument
//    @mode:	mode for the new file if O_CREAT is set, else ignored

//    This is the helper to open a file from kernelspace if you really
//    have to.  But in generally you should not do this, so please move
//    along, nothing to see here..

    filp = filp_open(TEST_FILE_PATH, 0, 0);


    if (filp == NULL) {
        printk(KERN_ERR "%s file_open error\n", __func__);
        return -EINVAL;
    }

    while (true) {
       ret = kernel_read(filp, offset, buf, 512);

       printk(KERN_INFO "%s kernel_read\n", __func__);

       if (ret > 0) {
           offset += ret;
           for (i = 0; i < ret; i++)
               printk("%c", buf[i]);
       } else
           break;

    }

    filp_close(filp, NULL);

    return 0;
}



static void __exit kern_rw_file_exit(void)
{
    printk(KERN_INFO "%s\n", __func__);
}




module_init(kern_rw_file_init);
module_exit(kern_rw_file_exit);

MODULE_LICENSE("GPL");
