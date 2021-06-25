//
// Created by CHM on 2021/3/4.
//
#include <linux/kernel.h>
#include <linux/input.h>
#include <linux/kprobes.h>
#include <linux/kthread.h>
#include <linux/err.h>
#include <linux/kallsyms.h>
#include <linux/delay.h>
#include <linux/time.h>

#include "common.h"

extern bool key_record_status;
extern spinlock_t record_index_lock;
extern struct completion g_auto_sendkey_completion;
extern struct dev_and_code d;

bool key_caps_status = false;   // 打开为true，关闭为false
bool key_shift_status = false;  // 按下为true，弹起为false
bool left_key_shift_status = false;  // 按下为true，弹起为false
bool right_key_shift_status = false;  // 按下为true，弹起为false
bool key_ctrl_status = false;   // ctrl 按下为true，弹起为false
bool left_key_ctrl_status = false;   // ctrl 按下为true，弹起为false
bool right_key_ctrl_status = false;   // ctrl 按下为true，弹起为false


struct timespec64 pre_key_time = {0};
struct timespec64 cur_key_time = {0};

unsigned long time_per = 60;

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



/* 对于每个探测，用户需要分配一个kprobe对象*/
static struct kprobe kp = {
        0//.symbol_name    = "_do_fork",
};

// 		ARM64
//			参数1~参数8 分别保存到 X0~X7 寄存器中 ，剩下的参数从右往左依次入栈，被调用者实现栈平衡，返回值存放在 X0 中。
/* 在被探测指令执行前，将调用预处理例程 pre_handler，用户需要定义该例程的操作*/
static int handler_pre(struct kprobe *p, struct pt_regs *regs)
{
    unsigned long dev, type, code, value;
    /** printk("%s dev = [%p], type = [%ld], code = [%ld], value =[%ld]\n", */
    /**        __func__, regs->regs[0], regs->regs[1], regs->regs[2], regs->regs[3]); */

    dev = regs->regs[0];
    type = regs->regs[1];
    code = regs->regs[2];
    value = regs->regs[3];

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


        /** if (key_shift_status && key_caps_status) { */
        /**     // shift 和 cpas同时激活 */
        /**     printk("%s current press key = [%c], status = [%s]\n", __func__, usb_kbd_special_keycode1[code], value ? "PRESSED":"RELEASED"); */
        /** } else if (!key_shift_status && key_caps_status) { */
        /**     // 只激活了caps */
        /**     printk("%s current press key = [%c], status = [%s]\n", __func__, usb_kbd_keycode1[code], value ? "PRESSED":"RELEASED"); */
        /** } else if (key_shift_status && !key_caps_status) { */
        /**     // 只激活了shift */
        /**     printk("%s current press key = [%c], status = [%s]\n", __func__, usb_kbd_special_keycode[code], value ? "PRESSED":"RELEASED"); */
        /** } else { */
        /**     // 最正常的扫描码，小写无特殊字符 */
        /**     printk("%s current press key = [%c], status = [%s]\n", __func__, usb_kbd_keycode[code], value ? "PRESSED":"RELEASED"); */
        /** } */


        // 可见字符记录及修改
        // 但当按下ctrl后，则不改，因为可能是快捷键
        if ( ((1 < code && code <= 14) || (15 < code && code < 28) ||
              (29 < code && code < 42) || (42 < code && code < 54) ||
              (70 < code && code < 84)) && !key_ctrl_status) {

			// 超过time_per 时间后，就关闭功能
			if (pre_key_time.tv_sec == 0) {
				ktime_get_ts64(&pre_key_time);
			}

			ktime_get_ts64(&cur_key_time);

			if (! ((cur_key_time.tv_sec - pre_key_time.tv_sec >= 0) && (cur_key_time.tv_sec - pre_key_time.tv_sec <= time_per)) ) {
				printk("%s timeout! time_per = [%d]\n", __func__, time_per);
				key_store_clear();
				key_record_status = false;
			}

			pre_key_time = cur_key_time;


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

				d.dev = (struct input_dev *)dev;
				d.code = code;

				complete(&g_auto_sendkey_completion);
            } else {
                modify_current_key_method1(&code, 0);
            }
        }
    }

    regs->regs[2] = code;

    return 0;
}

/* 在被探测指令执行后，kprobe调用后处理例程post_handler */
static void handler_post(struct kprobe *p, struct pt_regs *regs,
                         unsigned long flags)
{
    // printk("%s\n", __func__);
}

/*在pre-handler或post-handler中的任何指令或者kprobe单步执行的被探测指令产生了例外时，会调用fault_handler*/
static int handler_fault(struct kprobe *p, struct pt_regs *regs, int trapnr)
{
    // printk(KERN_DEBUG "fault_handler: p->addr = 0x%p, trap #%dn",
    //         p->addr, trapnr);
    /* 不处理错误时应该返回*/
    return 0;
}

/**
 */
void install_hook_input_handle_event(void)
{
    int ret=0;
    kp.pre_handler = handler_pre;
    kp.post_handler = handler_post;
    kp.fault_handler = handler_fault;

	// 为防止重入，改hook input_event
    unsigned long addr = kallsyms_lookup_name("input_event");
    printk("input_handle_event addr = %016lx\n", addr);
    kp.addr = addr;
    ret = register_kprobe(&kp);  /*注册kprobe*/
    if (ret < 0) {
        printk("register_kprobe failed, returned %d\n", ret);
        return;
    }
    printk("Planted kprobe at %016lx\n", kp.addr);
}

void uninstall_hook_input_handle_event(void)
{
    unregister_kprobe(&kp);
    printk("kprobe at %016lx unregistered\n", kp.addr);
}
