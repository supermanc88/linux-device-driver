#include <linux/kernel.h>
#include <linux/input.h>
#include <linux/random.h>

#include "common.h"


// 全局变量用来存储按键信息
unsigned char key_store[256] = {0};
// 最后一个key的位置
int key_store_index = -1;
// 记录释放到哪个按键了
int key_released_index = 0;

bool key_record_status = false;

extern spinlock_t record_index_lock;

// 改键的扫描码
unsigned char kbd_keycode_map[256] = {0};

void random_kbd_keycode_map(void)
{
	int i = 0;
	for ( ; i < 256; i++) {
		kbd_keycode_map[i] = i;
	}

	int t = 1000;
	while(t--) {
		int j = get_random_int() % 54;
		int k = get_random_int() % 54;
        if ( ((1 < kbd_keycode_map[j] && kbd_keycode_map[j] < 14) || (15 < kbd_keycode_map[j] && kbd_keycode_map[j] < 28) ||
              (29 < kbd_keycode_map[j] && kbd_keycode_map[j] < 42) || (42 < kbd_keycode_map[j] && kbd_keycode_map[j] < 54) ) &&
			 ((1 < kbd_keycode_map[k] && kbd_keycode_map[k] < 14) || (15 < kbd_keycode_map[k] && kbd_keycode_map[k] < 28) ||
              (29 < kbd_keycode_map[k] && kbd_keycode_map[k] < 42) || (42 < kbd_keycode_map[k] && kbd_keycode_map[k] < 54) )
			) {
			int tmp = kbd_keycode_map[j];
			kbd_keycode_map[j] = kbd_keycode_map[k];
			kbd_keycode_map[k] = tmp;

		}

		/** // 小键盘单独键盘内混淆 */
		/** j = get_random_int() % 13 + 71; */
		/** k = get_random_int() % 13 + 71; */
        /**  */
		/** int tmp = kbd_keycode_map[j]; */
		/** kbd_keycode_map[j] = kbd_keycode_map[k]; */
		/** kbd_keycode_map[k] = tmp; */
	}


	// 洗牌算法
	int array_len = 12;
	while (array_len) {
		if (array_len == 1)
			break;
		int ran = get_random_int() % (array_len--) + 2;
		int tmp = kbd_keycode_map[array_len + 2];
		kbd_keycode_map[array_len + 2] = kbd_keycode_map[ran];
		kbd_keycode_map[ran] = tmp;
	}

	array_len = 12;
	while (array_len) {
		if (array_len == 1)
			break;
		int ran = get_random_int() % (array_len--) + 16;
		int tmp = kbd_keycode_map[array_len + 16];
		kbd_keycode_map[array_len + 16] = kbd_keycode_map[ran];
		kbd_keycode_map[ran] = tmp;
	}

	array_len = 12;
	while (array_len) {
		if (array_len == 1)
			break;
		int ran = get_random_int() % (array_len--) + 30;
		int tmp = kbd_keycode_map[array_len + 30];
		kbd_keycode_map[array_len + 30] = kbd_keycode_map[ran];
		kbd_keycode_map[ran] = tmp;
	}

	array_len = 11;
	while (array_len) {
		if (array_len == 1)
			break;
		int ran = get_random_int() % (array_len--) + 43;
		int tmp = kbd_keycode_map[array_len + 43];
		kbd_keycode_map[array_len + 43] = kbd_keycode_map[ran];
		kbd_keycode_map[ran] = tmp;
	}

	array_len = 13;
	while (array_len) {
		if (array_len == 1)
			break;
		int ran = get_random_int() % (array_len--) + 71;
		/** int ran = 0; */
		/** get_random_bytes(&ran, sizeof(int)); */
		/** ran = ran % array_len + 71; */
		int tmp = kbd_keycode_map[array_len + 71];
		kbd_keycode_map[array_len + 71] = kbd_keycode_map[ran];
		kbd_keycode_map[ran] = tmp;
	}
}

/**
 * @brief 设置键盘记录状态
 * @param status
 * @return  返回当前的状态
 */
bool set_key_record_status(bool status)
{
    printk("%s [%d]\n", __func__, status);
    key_record_status = status;
	if (status) {
		random_kbd_keycode_map();
	}
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
void modify_current_key_method1(unsigned long * key, unsigned long is_pressed)
{
    printk("%s key = [%d], status = [%d]\n", __func__, *key, key_record_status);
    if (key_record_status) {
        // 全部修改为按键 0
        // 11是0键的扫描码
        if (*key == 14 /*KEY_BACKSPACE*/) {
            return;
        }
        // if (is_pressed) {
        //     *key = key_store_index % 3 + 2;
        // } else {
        //     *key = key_released_index % 3 + 2;
        //     key_released_index++;
        // }
		*key = kbd_keycode_map[*key];

		printk("%s modified key = [%d]\n", __func__, *key);
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
    key_released_index = 0;
}
