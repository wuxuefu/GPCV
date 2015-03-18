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
 * @file tp_ft5302.c
 * @brief ft5302 touch screen driver
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
#include <linux/kthread.h>

/*********************************
macro define
***********************************/


#define TI2C_ADDR 0x8C //for IT7256+
#define TI2C_CLK (200)

//#define DEBUG printk
#define DEBUG

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
 #define MULTI_TP_POINTS 3
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
/*********************************************
gloab
***********************************************/
static gp_tp_t ts;
static unsigned int debounce = 27000; /*1ms*/
static int tpd_flag = 0;
static int tpd_halt=0;
static DECLARE_WAIT_QUEUE_HEAD(waiter);
static struct task_struct *thread = NULL;
/********************************************
ti2c read or write
*********************************************/
static int tpd_i2c_write(struct gp_tp_s *ts, char reg, char *buf, int len)
{
	int ret=0;
	ts->i2c_handle->transmitMode = TI2C_NORMAL_WRITE_MODE;
	ts->i2c_handle->slaveAddrMode =TI2C_NORMAL_SLAVEADDR_8BITS;
	ts->i2c_handle->slaveAddr = (unsigned short)TI2C_ADDR;
	ts->i2c_handle->subAddrMode = TI2C_NORMAL_SUBADDR_8BITS;
	ts->i2c_handle->pSubAddr = (short *)&reg;
	ts->i2c_handle->pBuf = buf;
	ts->i2c_handle->dataCnt = len;
	ret=gp_ti2c_bus_xfer(ts->i2c_handle);
	return ret;
}
static int tpd_i2c_read(struct gp_tp_s *ts, char reg, char *buf, int len)
{
	int ret;
	ts->i2c_handle->transmitMode = TI2C_BURST_READ_NOSTOPBIT_MODE;
	ts->i2c_handle->slaveAddrMode =TI2C_NORMAL_SLAVEADDR_8BITS;
	ts->i2c_handle->slaveAddr = (unsigned short)TI2C_ADDR;
	ts->i2c_handle->subAddrMode = TI2C_NORMAL_SUBADDR_8BITS;
	ts->i2c_handle->pSubAddr =(short *) &reg;
	ts->i2c_handle->pBuf = buf;
	ts->i2c_handle->dataCnt = len;
	ret=gp_ti2c_bus_xfer(ts->i2c_handle);
	return ret;
	
}
/****************************************************
TP command 
****************************************************/
static int wait_tp_idle(struct gp_tp_s *ts)
{
	int count=0,ret =0;
	char buffer[1];
	do{ 
    	ret = tpd_i2c_read(ts, 0x80,&buffer[0], 1);
		printk("hi buffer[0]=0x%x\n",buffer[0]);
		msleep(5);
		if(count++>5)
		{
			printk("Can not wait to tp idle\n");
			ret =-1;
			break;
		}
	}while( buffer[0] & 0x01 );
	return ret;
}
static void tpd_print_version(struct gp_tp_s *ts) {
    
	char buffer[9];
	int ret;
	
	wait_tp_idle(ts);
    buffer[0] = 0x1;
    buffer[1] = 0x0;
    ret = tpd_i2c_write(ts, 0x20,buffer,2);
    if(ret<0){
        printk("[mtk-tpd] i2c write communcate error in getting FW version : 0x%x\n", ret);
    }
    msleep(10);
    
	wait_tp_idle(ts);
	
    ret = tpd_i2c_read(ts,0xA0, &buffer[0], 9);
    if (ret <0){
    	printk("[mtk-tpd] i2c read communcate error in getting FW version : 0x%x\n", ret);
    }else{
        printk("[mtk-tpd] ITE7256 Touch Panel Firmware Version %x %x %x %x %x %x %x %x %x\n", 
                buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7], buffer[8]);  
    }
}
static void
report_data(
	unsigned short x,
	unsigned short y,
	unsigned char pressure,
	unsigned char id
)
{
	
	printk("x=%d, y=%d, id=%d\n", x, y, id);
	input_report_abs(ts.dev, ABS_MT_TRACKING_ID, id);
	input_report_abs(ts.dev, ABS_MT_POSITION_X, x);
	input_report_abs(ts.dev, ABS_MT_POSITION_Y, y);
//	input_report_abs(ts.dev, ABS_MT_TRACKING_ID, id);
//	input_report_abs(ts.dev, ABS_MT_POSITION_X, y);
//	input_report_abs(ts.dev, ABS_MT_POSITION_Y, 480-x);
	input_report_abs(ts.dev, ABS_MT_TOUCH_MAJOR, pressure);
	//input_report_abs(ts.dev, ABS_MT_TOUCH_MAJOR, pressure);
	//input_report_abs(ts.dev, ABS_MT_WIDTH_MAJOR, 70);
	input_mt_sync(ts.dev);
}

static int touch_event_handler( void *arg)
{
	int ret;
	int xraw, yraw;
	struct gp_tp_s *ts = (struct gp_tp_s *)arg;
	unsigned char ucQuery[2] = {0,0};
	unsigned char pucPoint[52];
		
	//sched_setscheduler(current, SCHED_RR, &param); 
    do{
		set_current_state(TASK_INTERRUPTIBLE);
        while (tpd_halt) {tpd_flag = 0; msleep(20);}
        wait_event_interruptible(waiter, tpd_flag != 0);
        tpd_flag = 0;
        set_current_state(TASK_RUNNING); 
			
		 // TPD_DEBUG("[mtk-tpd] Query status= 0x%x\n", pucPoint[0]);

		ret = tpd_i2c_read(ts,0x80, ucQuery, 2);
		if (ret && ((ucQuery[0] & 0x80)==0x80)) 
		{
			tpd_i2c_read(ts,0xE0,pucPoint,ucQuery[1]);
			if (ret) 
			{
				if(pucPoint[0] == 0x41) {
					printk("[%s][%d]button press \n",__FUNCTION__,__LINE__);
				}
				else if(((pucPoint[0] & 0xF0) == 0x00)) 
				{
					int touch_number;
					int report_type;
					int fingerID;
					int pressure;
					int i;
					report_type = (pucPoint[1] & 0X80) >> 7;
					touch_number = ((pucPoint[1] & 0x7F)-2)/4;
					if(touch_number>2)  touch_number=2; //only support 2 touch
					if(touch_number > 0)
					{
						for(i = 0; i < touch_number; i++)
						{
						//get point i's X-coordinate & Y-coordinate
							if(report_type == 0)
							{
								xraw = ((pucPoint[i * 4 + 4] & 0x0F) << 8) + pucPoint[i * 4 + 3];
								yraw = ((pucPoint[i * 4 + 4] & 0xF0) << 4) + pucPoint[i * 4 + 5];
								fingerID = ((pucPoint[i * 4 + 2] & 0xF8)>>3); //0x11111000
								pressure = pucPoint[i * 4 + 2] & 0x07;
								report_data(xraw,yraw,pressure,fingerID);
							}
							else if(report_type == 1)
							{
								xraw = ((pucPoint[i * 5 + 5] & 0x0F) << 8) + pucPoint[i * 5 + 4];
								yraw = ((pucPoint[i * 5 + 5] & 0xF0) << 4) + pucPoint[i * 5 + 6];
								fingerID = ((pucPoint[i * 5 + 2] & 0xF8)>>3); //0x11111000
								pressure = pucPoint[i * 5 + 3] & 0x07;
								report_data(xraw,yraw,pressure,fingerID);
							}
							else
							{
							//customer define
							}
						
						}
						input_sync(ts->dev);
					}
					else
					{
						//printk("no data \n");
						input_mt_sync(ts->dev);
						input_sync(ts->dev);
					}
				}
			}
		}	

			gp_gpio_enable_irq(ts->client, 1);
		//printk("end\n");
    } while (!kthread_should_stop()); 
    return 0;
}




/**
 * interrupt callback
 */
void gp_ts_callback(void* client)
{
	gp_gpio_enable_irq(ts.client, 0);
	//printk("1\n");
	tpd_flag = 1;
	wake_up_interruptible(&waiter);
	
	
}

/** device driver probe*/
static int __init gp_tp_probe(struct platform_device *pdev)
{
	int rc;
	int ret = 0;
	int err = 0;
	int intidx, slaveAddr;
	gp_board_touch_t *touch_config = NULL;
	
	DIAG_DEBUG("Entering gp_tp_probe\n");
	memset(&ts, 0, sizeof(gp_tp_t));
	
	ts.dev = input_allocate_device();
	if ( NULL==ts.dev ){
		DIAG_ERROR("Unable to alloc input device\n");
		ret = -ENOMEM;
		goto __err_alloc;
	}
	
	//get i2c
	if ( touch_config != NULL && touch_config->get_i2c_slaveaddr != NULL) {
		touch_config->get_i2c_slaveaddr(&slaveAddr);
	} else {
		slaveAddr = TI2C_ADDR;
	}
	ts.i2c_handle = (ti2c_set_value_t *)kmalloc(sizeof(ti2c_set_value_t), GFP_KERNEL);
	if (NULL == ts.i2c_handle) {
		ret = -ENOMEM; goto __err_i2c_allocate;
	}

	ts.i2c_handle->pDeviceString = "Touch_IT7256";
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
	input_set_abs_params(ts.dev, ABS_MT_POSITION_X, 0, 480 - 1, 0, 0);
	input_set_abs_params(ts.dev, ABS_MT_POSITION_Y, 0, 272 - 1, 0, 0);
	input_set_abs_params(ts.dev, ABS_MT_TRACKING_ID, 0, 1, 0, 0);
	input_set_abs_params(ts.dev, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);
	
	ts.dev->name = "gp_ts";
	ts.dev->phys = "gp_ts";
	ts.dev->id.bustype = BUS_I2C;

	/* All went ok, so register to the input system */
	rc = input_register_device(ts.dev);
	if (rc) {
		ret = -EIO;
		goto __err_reg_input;
	}
	
	thread = kthread_run(touch_event_handler, &ts, "i7260");
    if (IS_ERR(thread)) { 
        err = PTR_ERR(thread);
        printk("i7260  failed to create kernel thread: %d\n", err);
		goto __err_kthread;
    }

    printk("register device ok!");
	tpd_print_version(&ts);
	//get int pin and register pin int
	touch_config = gp_board_get_config("touch", gp_board_touch_t);
	if ( touch_config != NULL && touch_config->get_touch_intpin != NULL) {
		touch_config->get_touch_intpin(&intidx);
        printk("get intpin intidx 0x%x", intidx);
	} else {
		intidx = MK_GPIO_INDEX(0,0,0,0);
	}
	ts.client = gp_gpio_request(intidx, "touch_int"); /* GPIO1[7] ---- */
	if(IS_ERR((void*)ts.client)) {
		DIAG_ERROR("%s unable to register client\n", __func__);
		ret = -ENOMEM;
		goto __err_register;
	}
	gp_gpio_set_function(ts.client, GPIO_FUNC_GPIO);
	gp_gpio_set_input(ts.client, 0);
	printk("test set gpio hight\n");
	gp_gpio_register_isr(ts.client, gp_ts_callback, (void *)ts.client);
	gp_gpio_irq_property(ts.client, GPIO_IRQ_LEVEL_TRIGGER|GPIO_IRQ_LEVEL_LOW, &debounce);
	printk("GPIO_IRQ_ACTIVE_FALING=%d,GPIO_IRQ_EDGE_TRIGGER=%d\n",GPIO_IRQ_ACTIVE_FALING,GPIO_IRQ_EDGE_TRIGGER);
    printk("register isr ok! enable le \n");
	//register_early_suspend(&touch_early_suspend_desc);

	
	DIAG_DEBUG("gp_tp driver loaded(%d)\n", ret);
	return 0;


__err_register:
	kthread_stop(thread);
	thread=NULL;
__err_kthread:
	input_unregister_device(ts.dev);
__err_reg_input:
	gp_ti2c_bus_release(ts.i2c_handle);
__err_i2c:
	kfree(ts.i2c_handle);	
__err_i2c_allocate:
	input_free_device(ts.dev);
__err_alloc:
	return ret;
}

/** device driver remove*/
static int gp_tp_remove(struct platform_device *pdev)
{
	//unregister_early_suspend(&touch_early_suspend_desc);
	//gp_gpio_release(ts.touch_reset);
	gp_gpio_unregister_isr(ts.client);
	gp_gpio_release(ts.client);
	gp_ti2c_bus_release(ts.i2c_handle);
	kfree(ts.i2c_handle);
	input_unregister_device(ts.dev);
	input_free_device(ts.dev);	
	destroy_workqueue(ts.touch_wq);
	kthread_stop(thread);
	thread=NULL;
	return 0;
}

static int gp_tp_suspend(struct platform_device *pdev, pm_message_t state)
{
	int ret = 0;
	unsigned char Wrbuf[3] = { 0x04, 0x00, 0x02 };
    printk("IT7256 call suspend\n");    
	
  
    tpd_halt = 1;
	gp_gpio_unregister_isr(ts.client);
	gp_gpio_release(ts.client);
	msleep(100);
    ret = tpd_i2c_write(&ts, 0x20,Wrbuf, 3);
    if(ret <0){
        printk("[mtk-tpd] i2c write communcate error during suspend: 0x%x\n", ret);
    }
	return 0;
}

static int gp_tp_resume(struct platform_device *pdev)
{
	int ret = 0;
	int i = 0;
	char cmdbuf[] = {0xF8, 0x00};
	char buffer;
	gp_board_touch_t *touch_config = NULL;
	int intidx;
	printk("IT7256 call resume\n");
	ret = tpd_i2c_write(&ts, 0x20,cmdbuf, 2);
	if(ret<0){
		  printk("[mtk-tpd] i2c write communcate error during resume: 0x%x\n", ret);
	}

	msleep(100);
	tpd_halt = 0;
	touch_config = gp_board_get_config("touch", gp_board_touch_t);
	if ( touch_config != NULL && touch_config->get_touch_intpin != NULL) {
		touch_config->get_touch_intpin(&intidx);
		printk("get intpin intidx 0x%x", intidx);
	} else {
		 intidx= MK_GPIO_INDEX(0,0,0,0);
	}
	ts.client = gp_gpio_request(intidx, "touch_int"); /* GPIO1[7] ---- */
	if(IS_ERR((void*)ts.client)) {
		DIAG_ERROR("%s unable to register client\n", __func__);
		ret = -ENOMEM;
		return ret;

	}
	gp_gpio_set_function(ts.client, GPIO_FUNC_GPIO);
	gp_gpio_set_input(ts.client, 0);
	gp_gpio_register_isr(ts.client, gp_ts_callback, (void *)ts.client);
	gp_gpio_irq_property(ts.client, GPIO_IRQ_LEVEL_TRIGGER|GPIO_IRQ_LEVEL_LOW, &debounce);
	return ret;
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