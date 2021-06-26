#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/err.h>
#include <linux/completion.h>
#include <linux/spinlock.h>


#include <linux/input.h>

#include "common.h"

unsigned long g_exit_flag = 0;

struct completion g_auto_sendkey_completion;

struct dev_and_code d = {0};

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

	/** while (true) { */
	/**     if (kthread_should_stop()) { */
	/**         printk("%s thread stopped\n", __func__); */
	/**         break; */
	/**     } */
    /**  */
	/**     printk("%s wait_for_completion\n", __func__); */
	/**     wait_for_completion(&g_auto_sendkey_completion); */
    /**  */
	/**     [> msleep(50); <] */
    /**  */
	/**     // 这里发起一个弹起的按键 */
	/**     [> input_report_key(d.dev, d.code, KEY_RELEASED); <] */
	/**     if (d.dev != NULL) { */
	/**         my_input_handle_event(d.dev, EV_KEY, d.code, KEY_RELEASED); */
	/**         input_sync(d.dev); */
    /**  */
	/**         printk("%s auto released code = [%d]\n", __func__, d.code); */
	/**     } */
    /**  */
	/**     printk("%s init_completion\n", __func__); */
	/**     init_completion(&g_auto_sendkey_completion); */
	/** } */

	do {

		printk("%s wait_for_completion\n", __func__);
		wait_for_completion(&g_auto_sendkey_completion);

		/** msleep(50); */

		// 这里发起一个弹起的按键
		/** input_report_key(d.dev, d.code, KEY_RELEASED); */
		if (d.dev != NULL) {
			unsigned long flags;
			spin_lock_irqsave(&d.dev->event_lock, flags);
			my_input_handle_event(d.dev, EV_KEY, d.code, KEY_RELEASED);
			spin_unlock_irqrestore(&d.dev->event_lock, flags);
			input_sync(d.dev);

			printk("%s auto released code = [%d]\n", __func__, d.code);

			memset(&d, 0, sizeof(d));
		}

		printk("%s init_completion\n", __func__);
		/** init_completion(&g_auto_sendkey_completion); */
		reinit_completion(&g_auto_sendkey_completion);
	} while(!g_exit_flag);

	printk("%s thread stopped\n", __func__);
	return 0;
}
