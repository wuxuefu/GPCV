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
 * @file gslx680_ctp.c type:
 * @brief used IIC driver interface 
 * @author antion
 */

#include <mach/kernel.h>
#include <mach/module.h>
#include <mach/cdev.h>
#include <mach/diag.h>
#include <mach/hardware.h>

#include <mach/hal/hal_i2c_bus.h>
#include <mach/gp_i2c_bus.h>
#include <mach/gp_board.h>

//----------------------------------
#include <linux/init.h>
#include <linux/fs.h> 
#include <mach/gp_display.h>
#include <mach/general.h>

#include <mach/gp_gpio.h>
#include <linux/delay.h> 
#include <mach/hal/hal_gpio.h>
#include <linux/input.h>
#include "gp_gslX680.h"

#if (defined CONFIG_ARCH_GPL32900B)
#include <mach/gp_ti2c_bus.h>
#endif
//*************************************************************************


#define I2C_SLAVE_ADDR		0x80
#define I2C_CLOCK		150
/*
//IOA6
#define INT_IRQ_CHANNEL		0
#define INT_IRQ_FUNC		0
#define INT_IRQ_GID			23
#define INT_IRQ_PIN			6
#define INT_IRQ_LEVEL		1

//IOA2
#define RST_CHANNEL		0
#define RST_FUNC		0
#define RST_GID			20
#define RST_PIN			2
#define RST_LEVEL		1
*/

//IOC25
#define INT_IRQ_CHANNEL		2
#define INT_IRQ_FUNC		1
#define INT_IRQ_GID		35
#define INT_IRQ_PIN		25
#define INT_IRQ_LEVEL		1

//IOC24
#define RST_CHANNEL		2
#define RST_FUNC		1
#define RST_GID			35
#define RST_PIN			24
#define RST_LEVEL		1

#ifndef DBG_ENABLE
	#define DBG_PRINT	if(0)printk	
#else
	#define DBG_PRINT	if(1)printk	
#endif


typedef struct gp_tp_s {
	struct input_dev *dev;
	//ti2c_set_value_t *i2c_handle;
	int client;
	int touch_reset;
	int prev_touched;
	struct work_struct mt_work;
	struct work_struct mt_set_nice_work;
	struct workqueue_struct *touch_wq;
	int intIoIndex;
}gp_tp_t;

/******************************************************************************/

#define TI2C_ADDR 0x80
#define TI2C_CLK 300
#define SCREEN_MAX_X 800
#define SCREEN_MAX_Y 480
#define MAX_CONTACTS 10
#define MULTI_TP_POINTS 5

#define ADJUST_CPU_FREQ	0// 1
#define DMA_TRANS_LEN		0x20
//#define VIRTUAL_KEYS
#define GSL_DEBUG

#ifdef GSL_DEBUG 
#define print_info(fmt, args...)   \
        do{                              \
                printk(fmt, ##args);     \
        }while(0)
#else
#define print_info(fmt, args...)   //
#endif


#define HW_I2C			0x01
#define HW_TI2C			0x02

#if (defined CONFIG_ARCH_GPL32900)
#define I2C_MODE		HW_I2C
#elif (defined CONFIG_ARCH_GPL32900B)
#define I2C_MODE		HW_TI2C
#endif

static gp_tp_t ts;

static u32 id_sign[MAX_CONTACTS+1] = {0};
static u8 id_state_flag[MAX_CONTACTS+1] = {0};
static u8 id_state_old_flag[MAX_CONTACTS+1] = {0};
static u16 x_old[MAX_CONTACTS+1] = {0};
static u16 y_old[MAX_CONTACTS+1] = {0};
static u16 x_new = 0;
static u16 y_new = 0;

#ifdef GSLX680_COMPATIBLE
static char chip_type = 0x88;
#endif

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/

int touch_reset;
#if(I2C_MODE == HW_I2C)
static int gp_i2c_handle;
#elif (I2C_MODE == HW_TI2C) 
static ti2c_set_value_t g_ti2c_handle;
#endif

//#define dynamic_i2c
/****************************************************************************************/

#if (I2C_MODE == HW_I2C)
static int gslx680_i2c_request(void)
{
	int i2c_handle;

	i2c_handle = gp_i2c_bus_request(I2C_SLAVE_ADDR, I2C_CLOCK);
	if (i2c_handle == -ENOMEM) {
		DIAG_ERROR("i2c request fail\n",i2c_handle);
		return -1;
	}
	return i2c_handle;	
}


static int gsl_ts_read(unsigned char reg,unsigned char *data,int length)
{
	int ret;
	int i2c_handle = 0;

	gp_i2c_bus_write(gp_i2c_handle,&reg,1);
	ret = gp_i2c_bus_read(gp_i2c_handle,(unsigned char *)data,length);

#ifdef dynamic_i2c
	gp_i2c_bus_release(gp_i2c_handle);
	gp_i2c_handle = 0;
#endif
	return ret;
}

static int gsl_ts_write(unsigned char reg,unsigned char *data,int length)
{
	int ret;

	gp_i2c_bus_write(gp_i2c_handle,(unsigned char *)&reg,1);
	gp_i2c_bus_write(gp_i2c_handle,(unsigned char *)data,length);

	//gp_i2c_bus_release(gp_i2c_handle);
}
static int gsl_ts_continue_write(unsigned char *data,int lenth)
{
	gp_i2c_bus_write(i2c_handle,data,lenth);
}
static void gsl_ts_free_i2c()
{
	gp_i2c_bus_release(gp_i2c_handle);
	gp_i2c_handle = 0;
}
#elif (I2C_MODE == HW_TI2C)
static int gslx680_i2c_request(void)
{
	int ret=0;
	memset(&g_ti2c_handle,0,sizeof(ti2c_set_value_t));
	g_ti2c_handle.pDeviceString = "Touch_IT7260";
	g_ti2c_handle.slaveAddrMode = TI2C_NORMAL_SLAVEADDR_8BITS;
	g_ti2c_handle.slaveAddr = (unsigned short)TI2C_ADDR;
	g_ti2c_handle.clockRate = I2C_CLOCK;
	g_ti2c_handle.apbdmaEn = 0;
	/* open ti2c */
	ret = gp_ti2c_bus_request(&g_ti2c_handle);
	if(ret != 0) {
		DIAG_ERROR("[%s], Open TI2C device fail.\n", __FUNCTION__);
		ret = -EIO;
	}
    printk("ti2c bus request ok !\n");
	return 1;
}
static int gsl_ts_read(unsigned char reg,unsigned char *data,int length)
{
	int ret=0;
	g_ti2c_handle.transmitMode = TI2C_BURST_READ_NOSTOPBIT_MODE;
	g_ti2c_handle.slaveAddrMode =TI2C_NORMAL_SLAVEADDR_8BITS;
	g_ti2c_handle.slaveAddr = (unsigned short)TI2C_ADDR;
	g_ti2c_handle.subAddrMode = TI2C_NORMAL_SUBADDR_8BITS;
	g_ti2c_handle.pSubAddr =(unsigned short *) &reg;
	g_ti2c_handle.pBuf = data;
	g_ti2c_handle.dataCnt = length;
	g_ti2c_handle.apbdmaEn =0;
	ret=gp_ti2c_bus_xfer(&g_ti2c_handle);
	
	#ifdef dynamic_i2c
	gp_ti2c_bus_release(&g_ti2c_handle);
	#endif
	return ret;
}
static int gsl_ts_write(unsigned char reg,unsigned char *data,int length)
{
	int ret=0;

	g_ti2c_handle.transmitMode = TI2C_NORMAL_WRITE_MODE;
	g_ti2c_handle.slaveAddrMode =TI2C_NORMAL_SLAVEADDR_8BITS;
	g_ti2c_handle.slaveAddr = (unsigned short)TI2C_ADDR;
	g_ti2c_handle.subAddrMode = TI2C_NORMAL_SUBADDR_8BITS;
	g_ti2c_handle.pSubAddr = (unsigned short *)&reg;
	g_ti2c_handle.pBuf = data;
	g_ti2c_handle.dataCnt = length;
	ret=gp_ti2c_bus_xfer(&g_ti2c_handle);

	return ret;
}

static int gsl_ts_continue_write(unsigned char *data,int length)
{
	int ret= 0;
	unsigned char reg =*data;
	unsigned char *buf_write =data+1;	
	ret=gsl_ts_write(reg,buf_write,length-1);
	return ret;
}


static void gsl_ts_free_i2c()
{
	gp_ti2c_bus_release(&g_ti2c_handle);
	
}
#endif
static void test_i2c(void)
{
	unsigned char read_buf[4] = {0x0,0x0,0x0,0x0};
	unsigned char write_buf[4] = {0x12,0x00,0x00,0x00};

	gsl_ts_read(0xf0, read_buf, sizeof(read_buf));
	printk("gslX680 test_i2c read 0xf0: %x_%x_%x_%x\n",read_buf[3],read_buf[2],read_buf[1],read_buf[0]);
	gsl_ts_write(0xf0, write_buf, sizeof(write_buf));
	printk("gslX680 test_i2c write 0xf0: %x_%x_%x_%x\n",write_buf[3],write_buf[2],write_buf[1],write_buf[0]);
	gsl_ts_read(0xf0, read_buf, sizeof(read_buf));
	printk("gslX680 test_i2c read 0xf0: %x_%x_%x_%x\n",read_buf[3],read_buf[2],read_buf[1],read_buf[0]);

}

static void 
record_point(u16 x, u16 y , u8 id)
{
	u16 x_err =0;
	u16 y_err =0;

	id_sign[id]=id_sign[id]+1;
	
	if(id_sign[id]==1){
		x_old[id]=x;
		y_old[id]=y;
	}

	x = (x_old[id] + x)/2;
	y = (y_old[id] + y)/2;
		
	if(x>x_old[id]){
		x_err=x -x_old[id];
	}
	else{
		x_err=x_old[id]-x;
	}

	if(y>y_old[id]){
		y_err=y -y_old[id];
	}
	else{
		y_err=y_old[id]-y;
	}

	if( (x_err > 3 && y_err > 1) || (x_err > 1 && y_err > 3) ){
		x_new = x;     x_old[id] = x;
		y_new = y;     y_old[id] = y;
	}
	else{
		if(x_err > 3){
			x_new = x;     x_old[id] = x;
		}
		else
			x_new = x_old[id];
		if(y_err> 3){
			y_new = y;     y_old[id] = y;
		}
		else
			y_new = y_old[id];
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
	unsigned short temp;
	temp = x;
	x = y;
	y = temp;	
	if(x >= SCREEN_MAX_X||y >= SCREEN_MAX_Y)
		return;
	//print_info("x=%d, y=%d, id=%d\n", x, y, id);

	input_report_abs(ts.dev, ABS_MT_TRACKING_ID, id-1);
	input_report_abs(ts.dev, ABS_MT_POSITION_X, x);
	input_report_abs(ts.dev, ABS_MT_POSITION_Y, y);
	input_report_abs(ts.dev, ABS_MT_TOUCH_MAJOR, pressure);
	input_mt_sync(ts.dev);
}

/**
 * interrupt callback
 */
void gp_ts_callback(void* client)
{
	//gp_gpio_enable_irq(ts.client, 0);
#ifdef dynamic_i2c
	gp_i2c_handle = gslx680_i2c_request();
	if (!gp_i2c_handle) {
		printk("[%s] request i2c fail\n",__FUNCTION__);
		gp_i2c_handle = gslx680_i2c_request();
		if (!gp_i2c_handle)
			printk("[%s] request2 i2c fail\n",__FUNCTION__);
	}
#endif	
	queue_work(ts.touch_wq, &ts.mt_work);
}


static void
gp_mt_set_nice_work(
	struct work_struct *work
)
{
	print_info("[%s:%d]\n", __FUNCTION__, __LINE__);
	set_user_nice(current, -20);
}

static inline unsigned int  join_bytes(unsigned char a, unsigned char b)
{
	unsigned int ab = 0;
	ab = ab | a;
	ab = ab << 8 | b;
	return ab;
}

static void
gp_multi_touch_work(
	struct work_struct *work
)
{

	int i,ret;
	char touched, id;
	unsigned short x, y;
	unsigned int pending;
 	unsigned char tp_data[(MULTI_TP_POINTS + 1)*4 ];

#ifdef GSL_NOID_VERSION
	unsigned int  tmp1;
	unsigned char buf[4] = {0};
	struct gsl_touch_info cinfo = {0};
#endif
	//printk("WQ  gp_multi_touch_work.\n");

#if ADJUST_CPU_FREQ
	clockstatus_configure(CLOCK_STATUS_TOUCH,1);
#endif

	ret = gsl_ts_read(0x80, tp_data, sizeof(tp_data));
	if( ret < 0) {
		print_info("gp_tp_get_data fail,return %d\n",ret);
		gp_gpio_enable_irq(ts.client, 1);
		return;
	}

	touched = (tp_data[0]< MULTI_TP_POINTS ? tp_data[0] : MULTI_TP_POINTS);
	
#ifdef GSL_NOID_VERSION
	if(chip_type == 0x82)
	{
		cinfo.finger_num = touched;
		print_info("tp-gsl  finger_num = %d\n",cinfo.finger_num);
		for(i = 0; i < (touched < MAX_CONTACTS ? touched : MAX_CONTACTS); i ++)
		{		
			cinfo.x[i] = join_bytes(tp_data[4 *( i + 1) + 3] & 0xf,tp_data[4 *( i + 1) + 2]);
			cinfo.y[i] = join_bytes(tp_data[4 *( i + 1) + 1],tp_data[4 *( i + 1) + 0]);	
			print_info("tp-gsl  x = %d y = %d \n",cinfo.x[i],cinfo.y[i]);
		}
		cinfo.finger_num=(tp_data[3]<<24)|(tp_data[2]<<16)
			|(tp_data[1]<<8)|(tp_data[0]);
		gsl_alg_id_main(&cinfo);
		tmp1=gsl_mask_tiaoping();
		print_info("[tp-gsl] tmp1=%x\n",tmp1);
		if(tmp1>0&&tmp1<0xffffffff)
		{
			buf[0] = 0xf0; buf[1]=0xa;
			gsl_ts_continue_write((unsigned char *)&buf[0],2);
			buf[0]=0x8;
			buf[1]=(u8)(tmp1 & 0xff);
			buf[2]=(u8)((tmp1>>8) & 0xff);
			buf[3]=(u8)((tmp1>>16) & 0xff);
			buf[4]=(u8)((tmp1>>24) & 0xff);
			print_info("tmp1=%08x,buf[0]=%02x,buf[1]=%02x,buf[2]=%02x,buf[3]=%02x\n",
				tmp1,buf[0],buf[1],buf[2],buf[3]);
			gsl_ts_continue_write((unsigned char *)&buf[0],5);	
		}
		touched = cinfo.finger_num;
	}
#endif	
	for (i=1;i<=MAX_CONTACTS;i++) {
		if(touched == 0)
			id_sign[i] = 0;	
		id_state_flag[i] = 0;		
	}
	//printk("point = %d  ",touched);
	for (i = 0; i < touched; i++) {
#ifdef GSL_NOID_VERSION
	if(chip_type == 0x82)
	{
		id = cinfo.id[i];
		x =  cinfo.x[i];
		y =  cinfo.y[i];	
	}
	else
#endif	
	{
		id = tp_data[4 *( i + 1) + 3] >> 4;
		x = join_bytes(tp_data[4 *( i + 1) + 3] & 0xf,tp_data[4 *( i + 1) + 2]);
		y = join_bytes(tp_data[4 *( i + 1) + 1],tp_data[4 *( i + 1) + 0]);		
	}
		if(1 <= id && id <= MAX_CONTACTS){
			record_point(x, y, id);
			report_data(x_new, y_new, 10, id);
			id_state_flag[(u8)id] = 1;
		}
	}
	if (touched == 0) {
		input_mt_sync(ts.dev);
	}
	for(i=1;i<=MAX_CONTACTS;i++)
	{	
		if( (0 == touched) || ((0 != id_state_old_flag[i]) && (0 == id_state_flag[i])) )
		{
			id_sign[i]=0;
		}
		id_state_old_flag[i] = id_state_flag[i];		
	}

#if ADJUST_CPU_FREQ
	if(touched == 0){
		clockstatus_configure(CLOCK_STATUS_TOUCH,0);
	}
#endif
	ts.prev_touched = touched;
	input_sync(ts.dev);


	/* Clear interrupt flag */
	pending = (1 << GPIO_PIN_NUMBER(ts.intIoIndex));
	gpHalGpioSetIntPending(ts.intIoIndex, pending);
	gp_gpio_enable_irq(ts.client, 1);
}


static void startup_chip(void)
{
	unsigned char buf[5];		
#ifdef GSL_NOID_VERSION
	if(chip_type == 0x82)
		gsl_DataInit(gsl_config_data_id);
#endif
	memset((unsigned char *)buf,0,sizeof(buf));
	buf[0] = 0xe0;
	gsl_ts_continue_write((unsigned char *)&buf[0],5);
	msleep(5);
}

static void reset_chip(void)
{
	unsigned char buf[5];
	memset((unsigned char *)buf,0,sizeof(buf));
	buf[0] = 0xe0;
	buf[1] = 0x88;
	gsl_ts_continue_write((unsigned char *)&buf[0],2);
	buf[0] = 0xe4;
	buf[1] = 0x04;
	gsl_ts_continue_write((unsigned char *)&buf[0],2);
	msleep(5);
	buf[0] = 0xbc;
	buf[1] = 0x00;	
	buf[2] = 0x00;	
	buf[3] = 0x00;
	buf[4] = 0x00;		
	gsl_ts_continue_write((unsigned char *)&buf[0],5);
	msleep(5);
}

static void clr_reg(void)
{
	unsigned char  write_buf[5]	= {0};

	write_buf[0] = 0xe0;//reg data
	write_buf[1] = 0x88;
	gsl_ts_continue_write((unsigned char *)&write_buf[0],2);
	msleep(5);
	write_buf[0] = 0x80;
	write_buf[1] = 0x03;
	gsl_ts_continue_write((unsigned char *)&write_buf[0],2);	
	msleep(5);
	write_buf[0] = 0xe4;
	write_buf[1] = 0x04;
	gsl_ts_continue_write((unsigned char *)&write_buf[0],2);
	msleep(5);
	write_buf[0] = 0xe0;
	write_buf[1] = 0x00;
	gsl_ts_continue_write((unsigned char *)&write_buf[0],2);	
	msleep(5);
}
#define muti_write 0x20
static void gp_load_ctp_fw(void)
{
	unsigned char addr[2];
	unsigned char buf[1+muti_write*4];
	int i = 0,j=0;
	printk("gsl1680 load firmware\n");
	while(1) {
		addr[0] = (unsigned char)(GSLX680_FW[i].offset&0xff);
		if (addr[0] == 0xff) {
			break;
		} else if (addr[0] == 0xf0) {
			addr[1] = (unsigned char)(GSLX680_FW[i].val&0xff);
			gsl_ts_continue_write((unsigned char *)&addr[0],2);
			j=0;
		} else {
			if(j==0)
				buf[j] = addr[0];
			buf[++j] = (unsigned char)(GSLX680_FW[i].val&0xff);
			buf[++j] = (unsigned char)((GSLX680_FW[i].val>>8)&0xff);
			buf[++j] = (unsigned char)((GSLX680_FW[i].val>>16)&0xff);
			buf[++j] = (unsigned char)((GSLX680_FW[i].val>>24)&0xff);
			if(j==(muti_write*4))
			{
				gsl_ts_continue_write((unsigned char *)&buf[0],j+1);
				j=0;
			}	
		}
		//printk("addr = 0x%x,data = 0x%x\n",addr[0],GSLX680_FW[i].val);
		i += 1;
	}
	printk("gsl1680 load firmware[%d] done!\n",i);

	return ;
}
static gp_load_ctp_fw_1680E() //for 1680e
{
	
}
static void init_chip()
{	
	//test_i2c();
	clr_reg();
	reset_chip();	
#ifdef GSLX680_COMPATIBLE
	if(0x88 == chip_type)
	{
		gp_load_ctp_fw();
	}
	else
#endif
	{
		gp_load_ctp_fw_1680E();		
	}	
	startup_chip();
	//reset_chip();
	//startup_chip();
	
}

static void check_mem_data()
{
	unsigned char  read_buf[4]  = {0};
	
	msleep(10);
	gsl_ts_read(0xb0, read_buf, sizeof(read_buf));
	
	if (read_buf[3] != 0x5a || read_buf[2] != 0x5a || read_buf[1] != 0x5a || read_buf[0] != 0x5a)
	{
		printk("#########check mem read 0xb0 = %x %x %x %x #########\n", read_buf[3], read_buf[2], read_buf[1], read_buf[0]);
		
		gp_gpio_set_output(touch_reset, 1,0);

		DBG_PRINT("=======gslx680 ctp test=====\n");
		msleep(5);
		gp_gpio_set_output(touch_reset, 0,0);
		msleep(5);
		gp_gpio_set_output(touch_reset, 1,0);
		msleep(5);
		init_chip();
	}
}


#ifdef GSLX680_COMPATIBLE
static void judge_chip_type()
{
	unsigned char read_buf[4]  = {0};

	msleep(10);
	gsl_ts_read(0xfc, read_buf, sizeof(read_buf));
	
	if(read_buf[2] != 0x82 && read_buf[2] != 0x88)
	{
		msleep(20);
		gsl_ts_read(0xfc, read_buf, sizeof(read_buf));
	}
	
	if(0x88 == read_buf[2])
	{
		chip_type = 0x88;
		//is_noid_version = 0;
	}
	else
	{
		chip_type = 0x82;
		//is_noid_version = 1;
	}
}
#endif




/** device driver probe*/
static int __init gp_tp_probe(struct platform_device *pdev)
{
	int rc;
	int ret = 0;
	int intidx;

	unsigned int debounce = 1;//27000; /*1ms*/
	//gp_board_touch_t *touch_config = NULL;
	gp_board_touch_t *touch_config = NULL;
	
	print_info("Entering gp_tp_probe\n");

#ifdef VIRTUAL_KEYS
	virtual_keys_init();
#endif

	memset(&ts, 0, sizeof(gp_tp_t));

	/* Create single thread work queue */
	ts.touch_wq = create_singlethread_workqueue("touch_wq");
	if (!ts.touch_wq)
	{
		print_info("%s unable to create single thread work queue\n", __func__);
		ret = -ENOMEM;
		goto __err_work_queue;
	}
	INIT_WORK(&ts.mt_set_nice_work, gp_mt_set_nice_work);
	queue_work(ts.touch_wq, &ts.mt_set_nice_work);

	ts.dev = input_allocate_device();
	if ( NULL==ts.dev ){
		print_info("Unable to alloc input device\n");
		ret = -ENOMEM;
		goto __err_alloc;
	}

I2C_REQUEST:
	if (gslx680_i2c_request() == -1) {
		DIAG_ERROR("cap touch panel iic request error!\n");
		goto I2C_REQUEST;
		return -ENOMEM;
	}

	__set_bit(EV_ABS, ts.dev->evbit);

	input_set_abs_params(ts.dev, ABS_MT_POSITION_X, 0, SCREEN_MAX_X - 1, 0, 0);
	input_set_abs_params(ts.dev, ABS_MT_POSITION_Y, 0, SCREEN_MAX_Y - 1, 0, 0);
	input_set_abs_params(ts.dev, ABS_MT_TRACKING_ID, 0, (MAX_CONTACTS + 1), 0, 0);
	input_set_abs_params(ts.dev, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);

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
	/* request cpt reset pin*/
	touch_config = gp_board_get_config("touch", gp_board_touch_t);
	if ( touch_config != NULL && touch_config->get_touch_resetpin != NULL) {
		touch_config->get_touch_resetpin(&intidx);
        printk("get intpin intidx 0x%x", intidx);
	} else {
		intidx = MK_GPIO_INDEX( RST_CHANNEL, RST_FUNC, RST_GID, RST_PIN );
	}
	touch_reset = gp_gpio_request(intidx, "touch_reset"); /* GPIO1[7] ---- */
	if(IS_ERR((void*)touch_reset)) {
		print_info("%s unable to register client\n", __func__);
		ret = -ENOMEM;
		goto __err_register;
	}   	
	gp_gpio_set_output(touch_reset, 1,0);

	DBG_PRINT("=======gslx680 ctp test=====\n");
	msleep(5);
 	gp_gpio_set_output(touch_reset, 0,0);
	msleep(5);
	gp_gpio_set_output(touch_reset, 1,0);
	msleep(5);
#ifdef GSLX680_COMPATIBLE
	judge_chip_type();
#endif	
	init_chip();
	check_mem_data();
	
#ifdef dynamic_i2c
	gsl_ts_free_i2c();
#endif
	INIT_WORK(&ts.mt_work, gp_multi_touch_work);
	
	touch_config = gp_board_get_config("touch", gp_board_touch_t);
	if ( touch_config != NULL && touch_config->get_touch_intpin != NULL) {
		touch_config->get_touch_intpin(&intidx);
        printk("get intpin intidx 0x%x", intidx);
	} else {
		intidx = MK_GPIO_INDEX( INT_IRQ_CHANNEL, INT_IRQ_FUNC, INT_IRQ_GID, INT_IRQ_PIN );
	}
	ts.client = gp_gpio_request(intidx, "touch_int"); /* GPIO1[7] ---- */
	if(IS_ERR((void*)ts.client)) {
		print_info("%s unable to register client\n", __func__);
		ret = -ENOMEM;
		goto __err_register;
	}
	gp_gpio_set_input(ts.client, GPIO_PULL_LOW);
#if (defined CONFIG_ARCH_GPL32900B)
	gp_gpio_irq_property(ts.client, GPIO_IRQ_LEVEL_TRIGGER|GPIO_IRQ_LEVEL_LOW, &debounce);
#else
	gp_gpio_irq_property(ts.client, GPIO_IRQ_EDGE_TRIGGER|GPIO_IRQ_ACTIVE_RISING, &debounce);
#endif
	gp_gpio_register_isr(ts.client, gp_ts_callback, (void *)ts.client);

	print_info("End gp_tp_probe\n");

	return 0;


__err_register:
	input_unregister_device(ts.dev);
__err_reg_input:
	//gp_ti2c_bus_release(ts.i2c_handle);
//	kfree(ts.i2c_handle);	
	//gp_gpio_release(ts.touch_reset);
	input_free_device(ts.dev);
__err_alloc:
	destroy_workqueue(ts.touch_wq);
__err_work_queue:
	return ret;
}


/** device driver remove*/
static int gp_tp_remove(struct platform_device *pdev)
{
	gp_gpio_unregister_isr(ts.client);
	gp_gpio_release(touch_reset);
	gsl_ts_free_i2c();
	input_unregister_device(ts.dev);
	input_free_device(ts.dev);	
	destroy_workqueue(ts.touch_wq);

	return 0;
}

static int gp_tp_suspend(struct platform_device *pdev, pm_message_t state)
{
	printk("Enter gp_tp_suspend.\n");

	gp_gpio_enable_irq(ts.client, 0);
 	gp_gpio_set_output(touch_reset, 0,0);
	msleep(5);
	return 0;
}

static int gp_tp_resume(struct platform_device *pdev)
{
	printk("Enter gp_tp_resume.\n");

	msleep(10);
	gp_gpio_set_output(touch_reset, 1,0);
	msleep(20); 	
	reset_chip();
	startup_chip();	
	check_mem_data();
	gp_gpio_enable_irq(ts.client, 1);
		
	return 0;
}


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
	printk("gp_tp_module_init \n");
	platform_device_register(&gp_tp_device);
	rc = platform_driver_register(&gp_tp_driver);
	printk("gp_tp_module_init  end\n");
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
