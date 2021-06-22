#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/err.h>
#include <linux/completion.h>


#include <linux/input.h>

#include "common.h"


struct completion g_auto_sendkey_completion;

struct dev_and_code d;

typedef void (* input_handle_event_fn)(struct input_dev *dev,
			       unsigned int type, unsigned int code, int value);

/**
* \brief auto_sendkey_thread 自动发送弹起按键的线程
* \param data
* \return
*/
int auto_sendkey_thread(void *data)
{
	printk("%s\n", __func__);

	input_handle_event_fn my_input_handle_event = lookup_symbol_by_name("input_handle_event");

	while (true) {
		if (kthread_should_stop()) {
			break;
		}

		wait_for_completion(&g_auto_sendkey_completion);

		/** msleep(50); */

		// 这里发起一个弹起的按键
		/** input_report_key(d.dev, d.code, KEY_RELEASED); */
		my_input_handle_event(d.dev, EV_KEY, d.code, KEY_RELEASED);
		input_sync(d.dev);

		printk("%s auto released code = [%d]\n", __func__, d.code);

		init_completion(&g_auto_sendkey_completion);
	}

	return 0;
}