/*
 * Driver for keys on GPIO lines. no interrupt needed.
 */

#include <linux/module.h>

#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <mach/gp_gpio_key.h>
#include <mach/gp_gpio.h>
#include "platform.h"
//////////////////////////////////////////////////////////////////////////////////////
#define __MODULE__          "Key_driver" //display for L%d[__MODULE__] //Car_recorde_disp1
#define __DBGLVL__          2 // 0=OFF, 1=ERROR, 2=WAENING 3=MSG 4=ALL
#define __DBGNAME__         printk
#include "../../../sdk/include/dbgs.h"
/////////////////////////////////////////////////////////////////////////////////////
//#define MK_GPIO_INDEX(channel,func,gid,pin) ((channel<<24)|(func<<16)|(gid<<8)|(pin))

struct gp_gpio_button_data {
	struct gp_gpio_keys_button *button;
	struct input_dev *input;
	struct timer_list timer;
	struct work_struct work;
	int handle;
	int old_state;
	int debounce_cnt;
};

struct gp_gpio_keys_drvdata {
	struct input_dev *input;
	struct gp_gpio_button_data data[0];
};

static void gp_gpio_keys_report_event(struct work_struct *work)
{
	struct gp_gpio_button_data *bdata =
		container_of(work, struct gp_gpio_button_data, work);
	struct gp_gpio_keys_button *button = bdata->button;
	struct input_dev *input = bdata->input;
	int state;

	gp_gpio_get_value(bdata->handle, &state);
	state = (state ? 1 : 0) ^ (button->active_low);
	if (state != bdata->old_state) {
		if( bdata->debounce_cnt ) {
			/*debounce end,report key event*/
			bdata->old_state = state;
			bdata->debounce_cnt = 0;
			__inf("report key %d %d %d\n", button->code, state, !!state);
			input_event(input, EV_KEY, button->code, !!state);
			input_sync(input);
		} else {
			/*start debounce*/
			bdata->debounce_cnt ++;
		}
	}
	/*next scan, 20ms*/
	mod_timer( &bdata->timer, jiffies+HZ/50);
}

static void gp_gpio_keys_timer(unsigned long _data)
{
	struct gp_gpio_button_data *data = (struct gp_gpio_button_data *)_data;

	schedule_work(&data->work);
}

static int __devinit gp_gpio_keys_probe(struct platform_device *pdev)
{
	struct gp_gpio_keys_platform_data *pdata = pdev->dev.platform_data;
	struct gp_gpio_keys_drvdata *ddata;
	struct input_dev *input;
	int i, error = 0;
	int handle = -1;


	ddata = kzalloc(sizeof(struct gp_gpio_keys_drvdata) +
			pdata->nr_buttons * sizeof(struct gp_gpio_button_data),
			GFP_KERNEL);
	input = input_allocate_device();
	if (!ddata || !input) {
		error = -ENOMEM;
		goto fail1;
	}

	platform_set_drvdata(pdev, ddata);

	input->name = pdev->name;
	input->phys = "gpio-keys/input0";
	input->dev.parent = &pdev->dev;

	input->id.bustype = BUS_HOST;
	input->id.vendor = 0x0001;
	input->id.product = 0x0001;
	input->id.version = 0x0100;

	ddata->input = input;

	for (i = 0; i < pdata->nr_buttons; i++) {
		struct gp_gpio_keys_button *button = &pdata->buttons[i];
		struct gp_gpio_button_data *bdata = &ddata->data[i];

		bdata->input = input;
		bdata->button = button;
		setup_timer(&bdata->timer, gp_gpio_keys_timer, (unsigned long)bdata);
		INIT_WORK(&bdata->work, gp_gpio_keys_report_event);

		handle = gp_gpio_request(button->gpio, button->desc ?: "gpio_keys");
		if (IS_ERR((void*)handle)) {
			pr_err("gpio-keys: failed to request GPIO %d,"
				" error %d\n", button->gpio, handle);
			goto fail2;
		}

		bdata->handle = handle;
		error = gp_gpio_set_input(handle, (button->active_low)? GPIO_PULL_HIGH: GPIO_PULL_LOW);
		if (error < 0) {
			pr_err("gpio-keys: failed to configure input"
				" direction for GPIO %d, error %d\n",
				button->gpio, error);
			gp_gpio_release(handle);
			goto fail2;
		}

		input_set_capability(input, EV_KEY, button->code);
		bdata->old_state = 0;
		mod_timer(&bdata->timer, jiffies+HZ/2);	//start scan after 500ms
	}

	error = input_register_device(input);
	if (error) {
		pr_err("gpio-keys: Unable to register input device, "
			"error: %d\n", error);
		goto fail2;
	}

	//device_init_wakeup(&pdev->dev, wakeup);

	return 0;

 fail2:
 	while(--i>=0) {
		del_timer_sync(&ddata->data[i].timer);
		cancel_work_sync(&ddata->data[i].work);
		gp_gpio_release(ddata->data[i].handle);
 	}
 fail1:
	input_free_device(input);
	kfree(ddata);

	return error;
}

static int __devexit gp_gpio_keys_remove(struct platform_device *pdev)
{
	struct gp_gpio_keys_platform_data *pdata = pdev->dev.platform_data;
	struct gp_gpio_keys_drvdata *ddata = platform_get_drvdata(pdev);
	struct input_dev *input = ddata->input;
	int i;

	device_init_wakeup(&pdev->dev, 0);

	for (i = 0; i < pdata->nr_buttons; i++) {
		del_timer_sync(&ddata->data[i].timer);
		cancel_work_sync(&ddata->data[i].work);
		gp_gpio_release(ddata->data[i].handle);
	}

	input_unregister_device(input);
	kfree(ddata);
	return 0;
}

static struct gp_gpio_keys_button gp_gpio_keys[]={
	/*code, index, active low, description*/
	{KEY_UP,KEY_UP_INDEX,0,"up"},		/*B_KEYSCAN0*/ //menu ok
	{KEY_ESC,KEY_ESC_INDEX,0,"esc"},	/*B_KEYSCAN5*/	//IOC1
	{KEY_DOWN,KEY_DOWN_INDEX,0,"down"},	/*B_KEYSCAN1*/ //down ok
	{KEY_LEFT,KEY_LEFT_INDEX,0,"left"},	/*B_KEYSCAN2*/ // mode ok
	{KEY_RIGHT,KEY_RIGHT_INDEX,0,"right"},	/*B_KEYSCAN3*/ // up ok //IOD23
	{KEY_EXIT,KEY_EXIT_INDEX,0,"exit"},	/*B_KEYSCAN3*///enter ok
	
};

static struct gp_gpio_keys_platform_data gp_gpio_keys_data = {
	.buttons = gp_gpio_keys,
	.nr_buttons = ARRAY_SIZE(gp_gpio_keys)
};

static struct platform_device gp_gpio_keys_device = {
	.name	= "gpio-keys",
	.id	= -1,
	.dev	= {
		.platform_data = &gp_gpio_keys_data
	},
};

static struct platform_driver gp_gpio_keys_driver = {
	.probe		= gp_gpio_keys_probe,
	.remove		= __devexit_p(gp_gpio_keys_remove),
	.driver		= {
		.name	= "gpio-keys",
		.owner	= THIS_MODULE,
	}
};

static int __init gp_gpio_keys_init(void)
{
	platform_device_register(&gp_gpio_keys_device);
	return platform_driver_register(&gp_gpio_keys_driver);
}

static void __exit gp_gpio_keys_exit(void)
{
	platform_driver_unregister(&gp_gpio_keys_driver);
}

module_init(gp_gpio_keys_init);
module_exit(gp_gpio_keys_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Keyboard driver for CPU GPIOs");
MODULE_ALIAS("platform:gpio-keys");


