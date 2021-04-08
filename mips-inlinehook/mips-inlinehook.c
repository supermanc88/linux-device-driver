#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kallsyms.h>
#include <linux/string.h>
#include <linux/input.h>

unsigned long hook_addr;
unsigned long hook_ret_addr;
unsigned long ra_value;

void modify_current_key(void)
{
	unsigned long dev, type, code, value;
	__asm__ volatile(
			"move %0, $a0\n\t"
			:"=r"(dev)
			);
	__asm__ volatile(
			"move %0, $a1\n\t"
			:"=r"(type)
			);
	__asm__ volatile(
			"move %0, $a2\n\t"
			:"=r"(code)
			);
	__asm__ volatile(
			"move %0, $a3\n\t"
			:"=r"(value)
			);
    printk("%s dev = [%p], type = [%ld], code = [%ld], value =[%ld]\n", __func__,
		    dev, type, code, value);
    if (type == EV_KEY)
    	code += 1;
	__asm__ volatile(
			"move $a2, %0\n\t"
			:
			:"r"(code)
			);
}

__attribute__ ((naked)) void my_input_handle_event(void)
{

    // 调用函数之后，把原来的ra改变了，所以需要保存原来的ra
    __asm__ volatile(
            //"sd $ra, ra_value\n\t"
	    "daddiu $sp,$sp,-136\n\t"
	    "sd $v0, 0($sp)\n\t"
	    "sd $v1, 8($sp)\n\t"
	    "sd $a0, 16($sp)\n\t"
	    "sd $a1, 24($sp)\n\t"
	    "sd $a2, 32($sp)\n\t"
	    "sd $a3, 40($sp)\n\t"
	    "sd $s0, 48($sp)\n\t"
	    "sd $s1, 56($sp)\n\t"
	    "sd $s2, 64($sp)\n\t"
	    "sd $s3, 72($sp)\n\t"
	    "sd $s4, 80($sp)\n\t"
	    "sd $s5, 88($sp)\n\t"
	    "sd $s6, 96($sp)\n\t"
	    "sd $s7, 104($sp)\n\t"
	    "sd $gp, 112($sp)\n\t"
	    "sd $fp, 120($sp)\n\t"
	    "sd $ra, 128($sp)\n\t"
            );
    __asm__ volatile(
//            "dla $v0, modify_current_key\n\t"
//            "move $t9,$v0\n\t"
            "bal modify_current_key\n\t"
            );
    __asm__ volatile(
            //"ld $ra, ra_value\n\t"
	    "ld $v0, 0($sp)\n\t"
	    "ld $v1, 8($sp)\n\t"
	    "ld $a0, 16($sp)\n\t"
	    "ld $a1, 24($sp)\n\t"
//	    "ld $a2, 32($sp)\n\t"
	    "ld $a3, 40($sp)\n\t"
	    "ld $s0, 48($sp)\n\t"
	    "ld $s1, 56($sp)\n\t"
	    "ld $s2, 64($sp)\n\t"
	    "ld $s3, 72($sp)\n\t"
	    "ld $s4, 80($sp)\n\t"
	    "ld $s5, 88($sp)\n\t"
	    "ld $s6, 96($sp)\n\t"
	    "ld $s7, 104($sp)\n\t"
	    "ld $gp, 112($sp)\n\t"
	    "ld $fp, 120($sp)\n\t"
	    "ld $ra, 128($sp)\n\t"
	    "daddiu $sp,$sp,136\n\t"
            );
    __asm__ volatile(
            // "daddiu  $sp, -0x50\n\t"
            // "sltiu   $v0, $a1, 0x17\n\t"
            // "sd      $s0, 0x18($sp)\n\t"
            // "swc2    $21, 0x13F($sp)\n\t"
	    "daddiu $sp,$sp,-64\n\t"
	    "sd $ra,56($sp)\n\t"
	    "sd $s5,48($sp)\n\t"
	    "sd $s4,40($sp)\n\t"
            "ld $t0,hook_ret_addr\n\t"
            "jr $t0\n\t"
            );
}

/*
 *  input_handle_event
.text:0000000120000C3C B0 FF BD 67                                   daddiu  $sp, -0x50                 // 从这开始
.text:0000000120000C40 17 00 A2 2C                                   sltiu   $v0, $a1, 0x17
.text:0000000120000C44 18 00 B0 FF                                   sd      $s0, 0x18($sp)
.text:0000000120000C48 3F 01 B5 EB                                   swc2    $21, 0x13F($sp)
.text:0000000120000C4C F4 00 B3 EB                                   swc2    $19, 0xF4($sp)             // 返回在这
.text:0000000120000C50 B2 00 B1 EB                                   swc2    $17, 0xB2($sp)
.text:0000000120000C54 A9 02 40 10                                   beqz    $v0, 0x1200016FC
.text:0000000120000C58 25 80 80 00                                   move    $s0, $a0
.text:0000000120000C5C 03 F8 A2 7C                                   dext    $v0, $a1, 0, 0x20  # ' '
.text:0000000120000C60 F8 18 02 00                                   dsll    $v1, $v0, 3
.text:0000000120000C64 B2 81 02 3C B0 60 42 64                       li      $v0, 0xFFFFFFFF81B260B0
.text:0000000120000C6C 03 18 42 D8                                   ldc2    $2, 0x1803($v0)
.text:0000000120000C70 25 88 A0 00                                   move    $s1, $a1
.text:0000000120000C74 25 98 C0 00                                   move    $s3, $a2
.text:0000000120000C78 08 00 40 00                                   jr      $v0
 * */

static int __init mips_inline_hook_init(void)
{
    printk("%s\n", __func__);

    unsigned long func_addr = kallsyms_lookup_name("input_handle_event");

    printk("%s input_handle_event addr = 0x%16lx\n", __func__, func_addr);

    int i = 0;

    unsigned char * opcodes = (char *)func_addr;
    for (; i < 40; i++) {
        printk("%x ", opcodes[i]);
    }
    printk("%s-------原数据打印完毕-----------\n", __func__);
    // 从 input_handle_event +0x0 处进行hook
    hook_addr = func_addr + 0;
    hook_ret_addr = hook_addr + 16;

    unsigned long jmp_addr = &my_input_handle_event;
    jmp_addr += 36;
    printk("%s my_input_handle_event addr = 0x%16lx, jmp_addr = 0x%16lx\n", __func__, &my_input_handle_event, jmp_addr);

    // 经测试不能使用j xxx的结构，这种结构最大跳转只能256M

//    unsigned char hook_opcodes[8] = {0};
//
//    unsigned int addr = (unsigned int)&my_input_handle_event;
//    unsigned int jmp_opcode = ((addr & 0xfffffff) >> 2) | 0x8000000;
//    hook_opcodes[0] = ((unsigned char *)&jmp_opcode)[3];
//    hook_opcodes[1] = ((unsigned char *)&jmp_opcode)[2];
//    hook_opcodes[2] = ((unsigned char *)&jmp_opcode)[1];
//    hook_opcodes[3] = ((unsigned char *)&jmp_opcode)[0];
//
//    printk("%s jmp_opcodes = [%x], hook_opcodes[0] = [%2x], hook_opcodes[1] = [%2x], hook_opcodes[2] = [%2x], hook_opcodes[3] = [%2x]\n",
//           __func__, jmp_opcode, hook_opcodes[0], hook_opcodes[1], hook_opcodes[2], hook_opcodes[3]);
//
//    memcpy(hook_addr, jmp_opcode, 8);



    // 可能需要构造
    // lw rt,offset(base)
    // jr rt
//    unsigned char hook_opcodes[32] = {0x00, 0x00, 0x0D, 0x3C, 0x00, 0x00, 0xAD, 0x35,
//                                      0x38, 0x6C, 0x0D, 0x00, 0x00, 0x00, 0xAD, 0x35,
//                                      0x38, 0x6C, 0x0D, 0x00, 0x00, 0x00, 0xAD, 0x35,
//                                      0x08, 0x00, 0xA0, 0x01, 0x00, 0x00, 0x00, 0x00};
//
//    unsigned short temp;
//    temp = (unsigned short )func_addr;
//    *((unsigned short *)&hook_opcodes[20]) = temp;
//    temp = (unsigned short )((func_addr >> 16) & 0x0000ffffffffffff);
//    *((unsigned short *)&hook_opcodes[12]) = temp;
//    temp = (unsigned short )((func_addr >> 32) & 0x00000000ffffffff);
//    *((unsigned short *)&hook_opcodes[4]) = temp;
//    temp = (unsigned short )((func_addr >> 48) & 0x000000000000ffff);
//    *((unsigned short *)&hook_opcodes[0]) = temp;

    // 找到一个这样的特征
    // 12 C0 02 3C 56 78 42 64                       li      $v0, 0xFFFFFFFFC0127856
    // 08 00 40 00                                   jr      $v0
    unsigned char hook_opcodes[16] = {0x00, 0x00, 0x0c, 0x3c, 0x00, 0x00, 0x8c, 0x65,
                                      0x08, 0x00, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00};
    unsigned short temp;
    temp = (unsigned short )jmp_addr;
    *((unsigned short *)&hook_opcodes[4]) = temp;
    temp = (unsigned short )((jmp_addr >> 16) & 0x0000ffffffffffff);
    *((unsigned short *)&hook_opcodes[0]) = temp;

    for (i = 0; i < 16; i++) {
        printk("%x ", hook_opcodes[i]);
    }
    printk("\n");
    printk("%s--------hook_opcodes打印完毕----------\n", __func__);

    memcpy(hook_addr, hook_opcodes, 16);
    for (i = 0; i < 40; i++) {
        printk("%d\n", opcodes[i]);
    }
    printk("\n");
    printk("%s--------hook_addr打印完毕----------\n", __func__);

    opcodes = (unsigned char *)jmp_addr;
    for (i = 0; i < 52; i++ ) {
        printk("%x ", opcodes[i]);
    }
    printk("%s--------my_input_handle_event打印完毕----------\n", __func__);
    return 0;
}


static void __exit mips_inline_hook_exit(void)
{


    printk("%s\n", __func__);
}

module_init(mips_inline_hook_init)
module_exit(mips_inline_hook_exit)
MODULE_LICENSE("GPL");
