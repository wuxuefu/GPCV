/**************************************************************************
 *                                                                        *
 *         Copyright (c) 2010 by Generalplus Inc.                         *
 *                                                                        *
 *  This software is copyrighted by and is the property of Generalplus    *
 *  Inc. All rights are reserved by Generalplus Inc.                      *
 *  This software may only be used in accordance with the                 *
 *  corresponding license agreement. Any unauthorized use, duplication,   *
 *  distribution, or disclosure of this software is expressly forbidden.  *
 *                                                                        *
 *  This Copyright notice MUST not be removed or modified without prior   *
 *  written consent of Generalplus Technology Co., Ltd.                   *
 *                                                                        *
 *  Generalplus Inc. reserves the right to modify this software           *
 *  without notice.                                                       *
 *                                                                        *
 *  Generalplus Inc.                                                      *
 *  3F, No.8, Dusing Rd., Hsinchu Science Park,                           *
 *  Hsinchu City 30078, Taiwan, R.O.C.                                    *
 *                                                                        *
 **************************************************************************/

/**
 * @file touch_ft5302.c
 * @brief ft5302 touch screen driver
 * @author deyueli
 */
#include <linux/earlysuspend.h>

#include <mach/kernel.h>
#include <mach/module.h>
#include <mach/cdev.h>
#include <mach/diag.h>
#include <mach/typedef.h>
#include <mach/gp_ti2c_bus.h>
#include <mach/gp_gpio.h>
#include <mach/hal/hal_gpio.h>
#include <linux/jiffies.h>
#include <mach/clock_mgr/gp_clock.h>
#include <mach/gp_board.h>

//#include "sysconfig.h"

#define MULTI_TP_POINTS 5
#define VIRTUAL_KEYS

#define TI2C_ADDR (0x70 )
#define TI2C_CLK (200)

#define TOUCH_DEVICE_VERSION 0x13
#define TOUCH_DEVICE_RESET_REG 0x18
#define TOUCH_DEVICE_VERSION_REG 0x19
#define	TOUCH_DEVICE_SLEEP_REG 0x18

#define	I2C_RETRY		0x5

//#define ADJUST_CPU_FREQ	1 //mark by deyue

/* For debug, print sample/xsec */
#define DEBUG_SAMPLE_RATE
#define DEBUG_SAMPLE_RATE_TIME 10*1000
//#undef SYSCONFIG_SCREEN_ROTATE
#ifndef SYSCONFIG_SCREEN_ROTATE
#define SYSCONFIG_SCREEN_ROTATE 0
#endif

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
typedef struct gp_tp_s {
	struct input_dev *dev;
	ti2c_set_value_t *i2c_handle;
	int client;
	int touch_reset;
	int prev_touched;
	struct work_struct mt_work;
	struct work_struct mt_set_nice_work;
	struct workqueue_struct *touch_wq;
	int intIoIndex;
}gp_tp_t;

typedef struct multi_touch_data_s {
	gp_point_t points[MULTI_TP_POINTS];
	char id[MULTI_TP_POINTS];
	char status;
	char mode;
	char version;
} multi_touch_data_t;

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/
static void gp_touch_early_suspend(struct early_suspend *h);
static void gp_touch_late_resume(struct early_suspend *h);

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static gp_tp_t ts;

static struct early_suspend touch_early_suspend_desc = {
	.level = EARLY_SUSPEND_LEVEL_DISABLE_FB,
	.suspend = gp_touch_early_suspend,
	.resume = gp_touch_late_resume,
};

#ifdef DEBUG_SAMPLE_RATE
static int sample_count;
static int last_calulate_time;
#endif

static unsigned int debounce = 27000; /*1ms*/
/**************************************************************************
 *             F U N C T I O N    I M P L E M E N T A T I O N S           *
 **************************************************************************/
#ifdef VIRTUAL_KEYS
static ssize_t
virtual_keys_show(
	struct kobject *kobj,
	struct kobj_attribute *attr, char *buf
)
{
	gp_board_touch_t *touch_config = NULL;
	
	touch_config = gp_board_get_config("touch", gp_board_touch_t);
	if ( touch_config != NULL && touch_config->get_touch_virtualkeyshow != NULL) {
		return touch_config->get_touch_virtualkeyshow((char*) buf);
	}
	else{
		printk("[%s][%d]ERROR!!!No Virtual Key is Defined in board_config.c\n", __FUNCTION__, __LINE__);
		return 0;
	}
	/*
	return sprintf(buf,
		__stringify(EV_KEY) ":" __stringify(KEY_BACK) ":360:860:40:83"
		":" __stringify(EV_KEY) ":" __stringify(KEY_HOME) ":240:860:40:83"
		":" __stringify(EV_KEY) ":" __stringify(KEY_MENU) ":120:860:40:83"
		"\n");
		*/
}

static struct kobj_attribute virtual_keys_attr = {
	.attr = {
		.name = "virtualkeys.gp_ts",
		.mode = S_IRUGO,
	},
	.show = &virtual_keys_show,
};

static struct attribute *properties_attrs[] = {
	&virtual_keys_attr.attr,
	NULL
};

static struct attribute_group properties_attr_group = {
	.attrs = properties_attrs,
};

static void
virtual_keys_init(
	void
)
{
	int ret = 0;
	struct kobject *properties_kobj;
	
	properties_kobj = kobject_create_and_add("board_properties", NULL);
	if (properties_kobj)
		ret = sysfs_create_group(properties_kobj, &properties_attr_group);
	if (!properties_kobj || ret)
		printk("failed to create board_properties\n");    
}

#endif

/**
 * @brief touch panel request fucntion
 * @return : handle of the requested touch panel(<0 invalid handle)
 */
int gp_tp_request(void)
{
	return (int)(&ts);
}

/**
 * @brief touch panel free fucntion
 * @param handle[in] : handle of touch panel to release
 * @return : SP_OK(0)/ERROR_ID
 */
int gp_tp_release(int handle)
{
	return 0;
}


static void
multi_touch_parse_data(
	char* raw_data,
	multi_touch_data_t* multi_touch_data
)
{
    //printk("touch parse data!!!\n");
	/* x, y position */
	multi_touch_data->points[0].x = ((raw_data[3]&0x0f) << 8) | raw_data[4];
	multi_touch_data->points[0].y = ((raw_data[5] &0x0f)<< 8) | raw_data[6];
	multi_touch_data->points[1].x = ((raw_data[9] &0x0f)<< 8) | raw_data[10];
	multi_touch_data->points[1].y = ((raw_data[11]&0x0f) << 8) | raw_data[12];
	multi_touch_data->points[2].x = ((raw_data[0x0f] &0x0f)<< 8) | raw_data[0x10];
	multi_touch_data->points[2].y = ((raw_data[0x11] &0x0f)<< 8) | raw_data[0x12];
	multi_touch_data->points[3].x = ((raw_data[0x15] &0x0f)<< 8) | raw_data[0x16];
	multi_touch_data->points[3].y = ((raw_data[0x17] &0x0f)<< 8) | raw_data[0x18];
	multi_touch_data->points[4].x = ((raw_data[0x1b]&0x0f) << 8) | raw_data[0x1c];
	multi_touch_data->points[4].y = ((raw_data[0x1d] &0x0f)<< 8) | raw_data[0x1e];

	/* id */
	multi_touch_data->id[0] = (raw_data[5]&0xf0) >> 4;
	multi_touch_data->id[1] = (raw_data[11]&0xf0)>>4;
	multi_touch_data->id[2] = (raw_data[0x11]&0xf0) >> 4;
	multi_touch_data->id[3] = (raw_data[0x17]&0xf0) >> 4;
	multi_touch_data->id[4] = (raw_data[0x1d]&0xf0) >> 4;

	//multi_touch_data->status = ((raw_data[3]&0xc0)>>6);
	multi_touch_data->status = (raw_data[2] & 0x07);
	//multi_touch_data->mode = raw_data[0xa5];
	//multi_touch_data->version = raw_data[0xa6];
}

static int gp_tp_get_data(multi_touch_data_t* tp_data)
{

	int ret = 0;
	char raw_data[0x20];
	//uint16_t data = 0x00;
	unsigned char data[0];
    data[0] = 0x00;

#if 1
	//Read data from 0 to 0x19
	ts.i2c_handle->subAddrMode = TI2C_NORMAL_SUBADDR_8BITS;
	ts.i2c_handle->pSubAddr = data;
	ts.i2c_handle->transmitMode = TI2C_BURST_READ_STOP_MODE;
	//ts.i2c_handle->transmitMode = TI2C_NORMAL_READ_MODE;
	ts.i2c_handle->pBuf = raw_data;
	ts.i2c_handle->dataCnt = sizeof(raw_data);
	//ts.i2c_handle->dataCnt = 1;
	
	ret = gp_ti2c_bus_xfer(ts.i2c_handle);
	if (ret < 0) {
		printk("[%s],[%d] error, ret=%d\n", __FUNCTION__, __LINE__,  ret);
		ret = gp_ti2c_bus_xfer(ts.i2c_handle);
		if(ret<0)	
        {
		    printk("[%s],[%d] error, ret=%d\n", __FUNCTION__, __LINE__,  ret);
            return -1;
        }
	}
#endif
#if 0
    //printk("begin write-->\n");
    //raw_data[0] = 0;
    unsigned char addr[0];
    //addr[0] = 0x18+0x80;
    addr[0] = 0x0;
	ts.i2c_handle->pSubAddr = 0;
    ts.i2c_handle->transmitMode = TI2C_NORMAL_WRITE_MODE;
    ts.i2c_handle->pBuf = addr;
    ts.i2c_handle->dataCnt = 1;
    ret = gp_ti2c_bus_xfer(ts.i2c_handle);
    if(ret <= 0) {
        printk("%d: write reg error!\n", __LINE__);
        return -1;
    }
    //printk("begin read-->\n");

    ts.i2c_handle->transmitMode = TI2C_NORMAL_READ_MODE;
    ts.i2c_handle->pBuf = raw_data;
    ts.i2c_handle->dataCnt = 0x20;
    ret = gp_ti2c_bus_xfer(ts.i2c_handle);
    if(ret <= 0) {
        printk("%d: read reg error!\n", __LINE__);
        return -1;
    }
    //printk("%d: read reg ok!\n");
#endif

	multi_touch_parse_data(raw_data, tp_data);

	return 0;
}

#if 0
static void
gp_ts_print_data(
	multi_touch_data_t multi_touch_data
)
{
	printk("%d, [%d:%d:%d] [%d:%d:%d] [%d:%d:%d] [%d:%d:%d] [%d:%d:%d], 0x%x, 0x%x\n",
		multi_touch_data.status,
		multi_touch_data.points[0].x, multi_touch_data.points[0].y, multi_touch_data.id[0],
		multi_touch_data.points[1].x, multi_touch_data.points[1].y, multi_touch_data.id[1],
		multi_touch_data.points[2].x, multi_touch_data.points[2].y, multi_touch_data.id[2],
		multi_touch_data.points[3].x, multi_touch_data.points[3].y, multi_touch_data.id[3],
		multi_touch_data.points[4].x, multi_touch_data.points[4].y, multi_touch_data.id[4],
		multi_touch_data.mode,
		multi_touch_data.version
	);
}
#endif

static void
report_data(
	unsigned short x,
	unsigned short y,
	unsigned char pressure,
	unsigned char id
)
{
	//printk("x=%d, y=%d, id=%d\n", x, y, id);
#if (defined SYSCONFIG_ANDROID_SDK_VERSION) && (SYSCONFIG_ANDROID_SDK_VERSION == 10)
	input_report_abs(ts.dev, ABS_MT_TRACKING_ID, id);
	input_report_abs(ts.dev, ABS_MT_POSITION_X, x);
	input_report_abs(ts.dev, ABS_MT_POSITION_Y, y);
//	input_report_abs(ts.dev, ABS_MT_TRACKING_ID, id);
//	input_report_abs(ts.dev, ABS_MT_POSITION_X, y);
//	input_report_abs(ts.dev, ABS_MT_POSITION_Y, 480-x);
	input_report_abs(ts.dev, ABS_MT_PRESSURE, 1);
	//input_report_abs(ts.dev, ABS_MT_TOUCH_MAJOR, pressure);
	//input_report_abs(ts.dev, ABS_MT_WIDTH_MAJOR, 70);
#else
	input_report_abs(ts.dev, ABS_MT_TRACKING_ID, id);
	input_report_abs(ts.dev, ABS_MT_POSITION_X, x);
	input_report_abs(ts.dev, ABS_MT_POSITION_Y, y);
	input_report_abs(ts.dev, ABS_MT_TOUCH_MAJOR, pressure);
#endif    
	input_mt_sync(ts.dev);
}

/**
 * interrupt callback
 */
void gp_ts_callback(void* client)
{
	gp_gpio_enable_irq(ts.client, 0);
	//printk("[%s:%d]---->>>>>>>\n", __FUNCTION__, __LINE__);
	queue_work(ts.touch_wq, &ts.mt_work);
}

static void
gp_mt_set_nice_work(
	struct work_struct *work
)
{
	printk("[%s:%d]\n", __FUNCTION__, __LINE__);
	set_user_nice(current, -20);
}

static void
gp_multi_touch_work(
	struct work_struct *work
)
{
	//printk("[%s:%d]+++++++++++++++++++++\n", __FUNCTION__, __LINE__);
	multi_touch_data_t multi_touch_data;
	int i,ret;
	int touched;
	unsigned int pending;
	int irq_state = 1;

	//gp_gpio_get_value(ts.client, &irq_state);
	//gp_gpio_get_value(ts.client, &irq_state);
	gp_gpio_get_value(ts.client, &irq_state);
	if (irq_state != 0) {
		/* Filter INT pin noise. */
		//printk("Error. Touch INT pin should be low.\n");
		//goto __error_check_touch_int; //mark deyue
	}
#if ADJUST_CPU_FREQ
	clockstatus_configure(CLOCK_STATUS_TOUCH,1);
#endif

	memset(&multi_touch_data, 0, sizeof(multi_touch_data_t));
#if 0
	i = 0;
	while (1) {
		if (gp_tp_get_data(&multi_touch_data) == 0)
			break;

		i ++;
		if (i > 5) {
			printk("Get mt data fail.\n");
			//gp_gpio_set_output(ts.touch_reset, 0, 0);
			//mdelay(100);
			//gp_gpio_set_output(ts.touch_reset, 1, 0);
			gp_gpio_enable_irq(ts.client, 1);
			return;
		}
	}
#else
	ret = gp_tp_get_data(&multi_touch_data);
	if( ret < 0) {
	    printk("gp_tp_get_data fail---------------,return %d\n",ret);
		gp_gpio_enable_irq(ts.client, 1);
		return;
	}
#endif
    //printk("tp get data ok!!!!!!\n");

	touched = multi_touch_data.status & 0xf;
	for (i = 0; i < touched; i++) {
		int x, y;
#if SYSCONFIG_SCREEN_ROTATE == 0
		x = multi_touch_data.points[i].x;
		y = multi_touch_data.points[i].y;
#elif SYSCONFIG_SCREEN_ROTATE == 90
		x = multi_touch_data.points[i].y;
		y = 800 - multi_touch_data.points[i].x;
#elif SYSCONFIG_SCREEN_ROTATE == 180
		x = 800 - multi_touch_data.points[i].x;
		y = 480 - multi_touch_data.points[i].y;
#elif SYSCONFIG_SCREEN_ROTATE == 270
		x = 480 - multi_touch_data.points[i].y;
		y = multi_touch_data.points[i].x;
#else
#error Invalid screen rotation
#endif                
		report_data(x, y, 70, multi_touch_data.id[i]);
	}

#if (defined SYSCONFIG_ANDROID_SDK_VERSION) && (SYSCONFIG_ANDROID_SDK_VERSION == 10)
	for (i = 0; i < ts.prev_touched - touched; i++) {
		input_report_abs(ts.dev, ABS_MT_PRESSURE, 0);
		//input_report_abs(ts.dev, ABS_MT_TOUCH_MAJOR, 0);
		//input_report_abs(ts.dev, ABS_MT_WIDTH_MAJOR, 0);
		input_mt_sync(ts.dev);
	}
#else
	if (ts.prev_touched && touched == 0) {
		input_mt_sync(ts.dev);
	}
#endif

#if ADJUST_CPU_FREQ
	if(touched == 0){
		clockstatus_configure(CLOCK_STATUS_TOUCH,0);
	}
#endif
	ts.prev_touched = touched;
	input_sync(ts.dev);

#ifdef DEBUG_SAMPLE_RATE
	sample_count ++;
	if ((jiffies - last_calulate_time) >= msecs_to_jiffies(DEBUG_SAMPLE_RATE_TIME)) {
		printk("[touch], count=%d, time=%d\n", sample_count, jiffies_to_msecs(jiffies - last_calulate_time));
		sample_count = 0;
		last_calulate_time = jiffies;
	}
#endif

__error_check_touch_int:
__error_get_mt_data:
	/* Clear interrupt flag */
	pending = (1 << GPIO_PIN_NUMBER(ts.intIoIndex));
	gpHalGpioSetIntPending(ts.intIoIndex, pending);

	gp_gpio_enable_irq(ts.client, 1);
}

/** device driver probe*/
static int __init gp_tp_probe(struct platform_device *pdev)
{
	int rc;
	int ret = 0;
	int intidx, slaveAddr;
	gp_board_touch_t *touch_config = NULL;
	
	DIAG_DEBUG("Entering gp_tp_probe\n");
	printk("Entering gp_tp_probe\n");

#ifdef VIRTUAL_KEYS
	virtual_keys_init();
#endif

	memset(&ts, 0, sizeof(gp_tp_t));

	/* Create single thread work queue */
	ts.touch_wq = create_singlethread_workqueue("touch_wq");
	if (!ts.touch_wq)
	{
		DIAG_ERROR("%s unable to create single thread work queue\n", __func__);
		ret = -ENOMEM;
		goto __err_work_queue;
	}
	INIT_WORK(&ts.mt_set_nice_work, gp_mt_set_nice_work);
	queue_work(ts.touch_wq, &ts.mt_set_nice_work);
	touch_config = gp_board_get_config("touch", gp_board_touch_t);
	if ( touch_config != NULL && touch_config->get_touch_intpin != NULL) {
		touch_config->get_touch_intpin(&intidx);
        //printk("get intpin intidx 0x%x", intidx);
	} else {
		intidx = MK_GPIO_INDEX(0,0,0,0);
	}
	if ( touch_config != NULL && touch_config->get_i2c_slaveaddr != NULL) {
		touch_config->get_i2c_slaveaddr(&slaveAddr);
	} else {
		slaveAddr = TI2C_ADDR;
	}
	
	ts.dev = input_allocate_device();
	if ( NULL==ts.dev ){
		DIAG_ERROR("Unable to alloc input device\n");
		ret = -ENOMEM;
		goto __err_alloc;
	}

	ts.i2c_handle = (ti2c_set_value_t *)kmalloc(sizeof(ti2c_set_value_t), GFP_KERNEL);
	if (NULL == ts.i2c_handle) {
		ret = -ENOMEM; goto __err_i2c_allocate;
	}

	ts.i2c_handle->pDeviceString = "Touch_ft5302";
	ts.i2c_handle->slaveAddrMode = TI2C_NORMAL_SLAVEADDR_8BITS;
	ts.i2c_handle->slaveAddr = (unsigned short)slaveAddr;
	ts.i2c_handle->clockRate = TI2C_CLK;
	ts.i2c_handle->apbdmaEn = true;
	/* open ti2c */
	ret = gp_ti2c_bus_request(ts.i2c_handle);
	if(ret != 0) {
		DIAG_ERROR("[%s], Open TI2C device fail.\n", __FUNCTION__);
		ret = -EIO;
		goto __err_i2c;
	}

    printk("ti2c bus request ok !\n");
	__set_bit(EV_ABS, ts.dev->evbit);

#if SYSCONFIG_SCREEN_ROTATE == 0
    printk("set abs params 1024x600 rotate 0");
	input_set_abs_params(ts.dev, ABS_MT_POSITION_X, 0, 1024 - 1, 0, 0);
	//input_set_abs_params(ts.dev, ABS_MT_POSITION_X, 0, 800 - 1, 0, 0);
	input_set_abs_params(ts.dev, ABS_MT_POSITION_Y, 0, 600 - 1, 0, 0);
	//input_set_abs_params(ts.dev, ABS_MT_POSITION_Y, 0, 480 - 1, 0, 0);
#elif SYSCONFIG_SCREEN_ROTATE == 90
    printk("set abs params 480x800");
	input_set_abs_params(ts.dev, ABS_MT_POSITION_X, 0, 480 - 1, 0, 0);
	input_set_abs_params(ts.dev, ABS_MT_POSITION_Y, 0, 800 - 1, 0, 0);
#elif SYSCONFIG_SCREEN_ROTATE == 180
    printk("set abs params 800x480 rotate 180");
	input_set_abs_params(ts.dev, ABS_MT_POSITION_X, 0, 800 - 1, 0, 0);
	input_set_abs_params(ts.dev, ABS_MT_POSITION_Y, 0, 480 - 1, 0, 0);
#elif SYSCONFIG_SCREEN_ROTATE == 270
    printk("set abs params 480x800 rotate 270");
	input_set_abs_params(ts.dev, ABS_MT_POSITION_X, 0, 480 - 1, 0, 0);
	input_set_abs_params(ts.dev, ABS_MT_POSITION_Y, 0, 800 - 1, 0, 0);
#else
#error Invalid screen rotation
#endif
    
	//input_set_abs_params(ts.dev, ABS_MT_POSITION_X, 0, 800 - 1, 0, 0);
	//input_set_abs_params(ts.dev, ABS_MT_POSITION_Y, 0, 480 - 1, 0, 0);
#if (defined SYSCONFIG_ANDROID_SDK_VERSION) && (SYSCONFIG_ANDROID_SDK_VERSION == 10)
	//input_set_abs_params(ts.dev, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);
	input_set_abs_params(ts.dev, ABS_MT_PRESSURE, 0, 1, 0, 0);
	//input_set_abs_params(ts.dev, ABS_MT_WIDTH_MAJOR, 0, 255, 0, 0);
	//input_set_abs_params(ts.dev, ABS_MT_TRACKING_ID, 1, 255, 0, 0);
#else
	input_set_abs_params(ts.dev, ABS_MT_TRACKING_ID, 0, 1, 0, 0);
	input_set_abs_params(ts.dev, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);
#endif

	input_set_capability(ts.dev, EV_KEY, KEY_BACK);
	input_set_capability(ts.dev, EV_KEY, KEY_HOME);
	input_set_capability(ts.dev, EV_KEY, KEY_MENU);

	ts.dev->name = "gp_ts";
	ts.dev->phys = "gp_ts";
	ts.dev->id.bustype = BUS_I2C;

	/* All went ok, so register to the input system */
	rc = input_register_device(ts.dev);
	if (rc) {
		ret = -EIO;
		goto __err_reg_input;
	}

	INIT_WORK(&ts.mt_work, gp_multi_touch_work);
    printk("register device ok!");

	ts.client = gp_gpio_request(intidx, "touch_int"); /* GPIO1[7] ---- */
	if(IS_ERR((void*)ts.client)) {
		DIAG_ERROR("%s unable to register client\n", __func__);
		ret = -ENOMEM;
		goto __err_register;
	}
    printk("request gpio int ok !\n");

	//gp_gpio_set_input(ts.client, GPIO_PULL_HIGH);
	gp_gpio_set_input(ts.client, 0);
	gp_gpio_irq_property(ts.client, GPIO_IRQ_EDGE_TRIGGER|GPIO_IRQ_ACTIVE_FALING, &debounce);
	gp_gpio_register_isr(ts.client, gp_ts_callback, (void *)ts.client);
    printk("register isr ok!\n");

	register_early_suspend(&touch_early_suspend_desc);

#ifdef DEBUG_SAMPLE_RATE
	sample_count = 0;
	last_calulate_time = jiffies;
#endif


#if 0
	multi_touch_data_t multi_touch_data;
    int i = 0;
    int iret;
    for(i=0; i< 100; i++) {

	    iret = gp_tp_get_data(&multi_touch_data);
        printk("get data ret %d\n", iret);
    }
#endif

	DIAG_DEBUG("gp_tp driver loaded(%d)\n", ret);
	return 0;


__err_register:
	input_unregister_device(ts.dev);
__err_reg_input:
	gp_ti2c_bus_release(ts.i2c_handle);
__err_i2c:
	kfree(ts.i2c_handle);	
__err_i2c_allocate:
	//gp_gpio_release(ts.touch_reset);
__err_pin_request:
	input_free_device(ts.dev);
__err_alloc:
	destroy_workqueue(ts.touch_wq);
__err_work_queue:
	return ret;
}

/** device driver remove*/
static int gp_tp_remove(struct platform_device *pdev)
{
	unregister_early_suspend(&touch_early_suspend_desc);
	//gp_gpio_release(ts.touch_reset);
	gp_gpio_unregister_isr(ts.client);
	gp_gpio_release(ts.client);
	gp_ti2c_bus_release(ts.i2c_handle);
	kfree(ts.i2c_handle);
	input_unregister_device(ts.dev);
	input_free_device(ts.dev);	
	destroy_workqueue(ts.touch_wq);
	return 0;
}

/**
 * \brief   touch driver early suspend function
 */
static void
gp_touch_early_suspend(
	struct early_suspend *h
)
{
	int ret = 0, j;
	unsigned char data;
	uint16_t subAddr = 0;
  
	printk("Enter touch early suspend\n");
    gp_gpio_unregister_isr(ts.client);

    /* Enter sleep mode */
	ts.i2c_handle->transmitMode = TI2C_NORMAL_WRITE_MODE;
	ts.i2c_handle->subAddrMode = TI2C_NORMAL_SUBADDR_8BITS;
	subAddr = TOUCH_DEVICE_SLEEP_REG;
	ts.i2c_handle->pSubAddr = &subAddr;
	data = 0x01;  //From SPEC
	ts.i2c_handle->pBuf = (char*)&data;
	ts.i2c_handle->dataCnt = 1;
	ret = gp_ti2c_bus_xfer(ts.i2c_handle);
	if (ret < 0 ) {
		printk("[%s],[%d] error, ret=%d\n", __FUNCTION__, __LINE__,  ret);
		for(j=0; j<I2C_RETRY; j++) {
			ret = gp_ti2c_bus_xfer(ts.i2c_handle);
			if(ret>=0)
				break;
		}
		if(ret<0) {
			printk("Exit touch early suspend\n");
			return;
		}
	}

	printk("Exit touch early suspend\n");
}

/**
 * \brief   touch driver late resume function
 */
static void
gp_touch_late_resume(
	struct early_suspend *h
)
{	
	printk("Enter touch late resume.\n");
	/*
	gp_gpio_set_output(ts.touch_reset, 0, 0);
	msleep(10);
	gp_gpio_set_output(ts.touch_reset, 1, 0);
	*/
	//msleep(100);

	if (ts.prev_touched) {
		printk("touch driver report event, %d.\n", ts.prev_touched);
#if (defined SYSCONFIG_ANDROID_SDK_VERSION) && (SYSCONFIG_ANDROID_SDK_VERSION == 10)
		int i;
		for (i = 0; i < ts.prev_touched; i++) {
			input_report_abs(ts.dev, ABS_MT_PRESSURE, 0);
			//input_report_abs(ts.dev, ABS_MT_TOUCH_MAJOR, 0);
			//input_report_abs(ts.dev, ABS_MT_WIDTH_MAJOR, 0);
			input_mt_sync(ts.dev);
		}
#else
	input_mt_sync(ts.dev);
#endif
		ts.prev_touched = 0;
		input_sync(ts.dev);
	}

	gp_gpio_register_isr(ts.client, gp_ts_callback, (void *)ts.client);
	printk("Exit touch late resume.\n");
}

static int gp_tp_suspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

static int gp_tp_resume(struct platform_device *pdev)
{
	return 0;
}

/**
 * @brief   touch pad device release
 */
static void gp_tp_device_release(struct device *dev)
{
	DIAG_INFO("remove touch pad device ok\n");
}

static struct platform_device gp_tp_device = {
	.name = "gp_tp",
	.id   = -1,
	.dev	= {
		.release = gp_tp_device_release,
	}
};

static struct platform_driver gp_tp_driver = {
       .driver         = {
	       .name   = "gp_tp",
	       .owner  = THIS_MODULE,
       },
       .probe          = gp_tp_probe,
       .remove         = gp_tp_remove,
       .suspend        = gp_tp_suspend,
       .resume         = gp_tp_resume,

};

static int __init gp_tp_module_init(void)
{
	int rc;

	platform_device_register(&gp_tp_device);
	rc = platform_driver_register(&gp_tp_driver);
	return rc;
}

static void __exit gp_tp_module_exit(void)
{
	platform_device_unregister(&gp_tp_device);
	platform_driver_unregister(&gp_tp_driver);
}

module_init(gp_tp_module_init);
module_exit(gp_tp_module_exit);
MODULE_LICENSE_GP;

