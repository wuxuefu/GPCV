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
#include <mach/gp_adc.h>

#define MK_GPIO_INDEX(channel,func,gid,pin) ((channel<<24)|(func<<16)|(gid<<8)|(pin))

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


static int adc_handle;
static int old_key_value=0;
static int debounce_num=0;
//static int key_table[]={60,180,310,600};
#if 0
static int key_table[]={60,180,310,600};
#else
static int key_table[]={146,210,300,410};
#endif
static void gp_gpio_keys_report_event(struct work_struct *work)
{
	struct gp_gpio_button_data *bdata =
		container_of(work, struct gp_gpio_button_data, work);
	struct gp_gpio_keys_button *button = bdata->button;
	struct input_dev *input = bdata->input;
	int state;
	int value;
	
	int key_value;
	
	if(button->code ==KEY_EXIT ||button->code == KEY_ESC)
	{
		gp_gpio_get_value(bdata->handle, &state);
		state = (state ? 1 : 0) ^ (button->active_low);
		if (state != bdata->old_state) {
			if( bdata->debounce_cnt ) {
				/*debounce end,report key event*/
				bdata->old_state = state;
				bdata->debounce_cnt = 0;
				input_event(input, EV_KEY, button->code, !!state);
				input_sync(input);
			} else {
				/*start debounce*/
				bdata->debounce_cnt ++;
			}
		}
	}
	value = gp_adc_read(adc_handle);
	//printk("value = %d\n",value);
	if(value<key_table[0])
	{
		key_value = 1;
		if(key_value != old_key_value)
		{
			if(debounce_num == 1)
			{
				old_key_value = 1;
				debounce_num = 0;
				printk("KEY_DOWN press\n");
				input_event(input, EV_KEY, KEY_LEFT, 1);
				input_sync(input);
			}
			else
			{
				debounce_num = 1;
			}
		}
	}
	else if(value<key_table[1])
	{
		key_value = 2;
		if(key_value != old_key_value)
		{
			if(debounce_num == 2)
			{
				old_key_value = 2;
				debounce_num = 0;
				printk("KEY_LEFT press\n");
				input_event(input, EV_KEY, KEY_RIGHT, 1);
				input_sync(input);
			}
			else
			{
				debounce_num = 2;
			}
		}
	}
	else if(value<key_table[2])
	{
		key_value = 3;
		if(key_value != old_key_value)
		{
			if(debounce_num == 3)
			{
				old_key_value = 3;
				debounce_num = 0;
				printk("KEY_RIGHT press\n");
				input_event(input, EV_KEY, KEY_UP, 1);
				input_sync(input);
			}
			else
			{
				debounce_num = 3;
			}
		}
	}
	else if(value<key_table[3])
	{
		key_value = 4;
		if(key_value != old_key_value)
		{
			if(debounce_num == 4)
			{
				old_key_value = 4;
				debounce_num = 0;
				printk("KEY_EXIT press\n");
				input_event(input, EV_KEY, KEY_DOWN, 1);
				input_sync(input);
			}
			else
			{
				debounce_num = 4;
			}
		}
	}
	else
	{
		if(old_key_value == 1)
		{
			old_key_value = 0;
			printk("KEY_DOWN release\n");
			input_event(input, EV_KEY, KEY_LEFT, 0);
			input_sync(input);			
		}
		else if(old_key_value == 2)
		{
			old_key_value = 0;
			printk("KEY_LEFT release\n");
			input_event(input, EV_KEY, KEY_RIGHT, 0);
			input_sync(input);			
		}
		else if(old_key_value == 3)
		{
			old_key_value = 0;
			printk("KEY_RIGHT release\n");
			input_event(input, EV_KEY, KEY_UP, 0);
			input_sync(input);			
		}
		else if(old_key_value == 4)
		{
			old_key_value = 0;
			printk("KEY_EXIT release\n");
			input_event(input, EV_KEY, KEY_DOWN, 0);
			input_sync(input);			
		}
		else
		{
			
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

		if(i<2)
		{
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
	adc_handle = gp_adc_request(0, NULL);
	gp_adc_start(adc_handle, 2); //0 means channel 0
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
	{KEY_EXIT,MK_GPIO_INDEX(2,0,16,12),1,"exit"},		/*B_KEYSCAN0*/   //IOC12
	{KEY_ESC,MK_GPIO_INDEX(2,2,4,1),0,"esc"},	/*B_KEYSCAN5*/	//IOC1
	{KEY_DOWN,MK_GPIO_INDEX(2,0,15,10),0,"down"},	/*B_KEYSCAN1*/
	{KEY_LEFT,MK_GPIO_INDEX(2,0,15,10),0,"left"},	/*B_KEYSCAN2*/
	{KEY_RIGHT,MK_GPIO_INDEX(2,0,15,10),0,"right"},	/*B_KEYSCAN3*/
	{KEY_UP,MK_GPIO_INDEX(2,0,15,10),0,"up"},	/*B_KEYSCAN4*/ 

	
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


