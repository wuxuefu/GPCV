#include <linux/init.h>
#include <linux/configfs.h>
#include <linux/module.h>
#include <linux/kernel.h> /* printk() */
#include <linux/slab.h> /* kmalloc() */
#include <linux/fs.h> /* everything... */
#include <linux/errno.h> /* error codes */
#include <linux/types.h> /* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h> /* O_ACCMODE */
#include <asm/system.h> /* cli(), *_flags */
#include <asm/uaccess.h> /* copy_from/to_user */

#include <mach/panel_cfg.h>
#include <mach/hardware.h>
//#include <mach/regs-scu.h>
//#include <mach/hal/hal_gpio.h>
#include <mach/hal/hal_clock.h>
#include <mach/hal/hal_disp.h>
#include <mach/gp_display.h>
#include <mach/clock_mgr/gp_clock.h>
#include <mach/gp_board.h>
#include <mach/gp_panel.h>
#include <mach/module.h>
#include <linux/delay.h> 	/* udelay/mdelay */
#include <mach/gp_gpio.h>
MODULE_LICENSE_GP;

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/
static int32_t lcd_init(void);
static int32_t lcd_suspend(void);
static int32_t lcd_resume(void);
static void* lcd_get_param(void);

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
#if 1
	#define DEBUG	printk
#else
	#define DEBUG(...)
#endif


/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/



/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static const gp_disp_colormatrix_t gColorMatrix = {
	.a00 = 0x0100,
	.a01 = 0,
	.a02 = 0,
	.a10 = 0,
	.a11 = 0x0100,
	.a12 = 0,
	.a20 = 0,
	.a21 = 0,
	.a22 = 0x0100,
	.b0 = 0,
	.b1 = 0,
	.b2 = 0,
};

static panel_lcdInfo_t gPanelInfo = {
	.name = "panel_lcd_ILI8961_27", // MUST equal module name
	.workFreq    = 24000000,
	.clkPolatiry = 0,
	.resolution = {
		.width  = 320,
		.height = 240,
	},
	.pixelPitch = {
		.width  = 16,
		.height = 9,
	},
	.format      = PANEL_FMT_RGB,
	.type        = PANEL_TYPE_SRGBM888,
	.dataSeqEven = PANEL_SEQUENCE_PRGB888_RGB,
	.dataSeqOdd  = PANEL_SEQUENCE_PRGB888_RGB,
	.vsync = {
		.polarity = 1,
		.fPorch   = 1,
		.bPorch   = 21,
		.width    = 1,
	},
	.hsync = {
		.polarity = 1,
		.fPorch   = 39,
		.bPorch   = 241,
		.width    = 1,
	},
	.pColorMatrix = (gp_disp_colormatrix_t *) &gColorMatrix,
	.pGammaTable = {
		NULL,
		NULL,
		NULL,
	}
};

/* access functions */
static gp_disp_panel_ops_t lcd_fops = {
	.init = lcd_init,
	.suspend = lcd_suspend,
	.resume = lcd_resume,
	.get_param = lcd_get_param,
};

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/
extern int32_t panel_common_init(uint32_t mode);
extern int32_t panel_common_suspend(uint32_t mode);
extern int32_t panel_common_resume(uint32_t mode);

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/
#define MK_GPIO_INDEX(channel,func,gid,pin) ((channel<<24)|(func<<16)|(gid<<8)|(pin))
#define delay_time 2


int cs_pin, data_pin, clk_pin;

static void spi_delay(int count)
{
	udelay(count);
}
static void gpio_write_io(int handle,unsigned int value)
{
	gp_gpio_set_output(handle,value,0);
}
static int gpio_read_io(int handle)
{
	unsigned int value;

	gp_gpio_set_input(handle,2);
	
	gp_gpio_get_value(handle,&value);
	//printk("read io 0x%x \r\n",value);
	return value;
}

static int sent_command(unsigned char adr,unsigned char value)
{
	int i=16;
	
	gpio_write_io(cs_pin,1);//CS=1
	gpio_write_io(data_pin,0);//SCL=0
	gpio_write_io(clk_pin,0);//SDA
	
	adr = adr << 1;
	// set csn low
	gpio_write_io(cs_pin,0);//CS=0
	spi_delay(delay_time);
	for(i=0;i<16;i++)
	{
		if(i<8)
		{
			if(i==1)
				gpio_write_io(data_pin,0); // write flag
			else
			{
				if(adr&0x80)
					gpio_write_io(data_pin,1);
				else
					gpio_write_io(data_pin,0);
				
				adr = adr << 1;
			}
			
			spi_delay(delay_time);
			gpio_write_io(clk_pin,1);
			spi_delay(delay_time);
			gpio_write_io(clk_pin,0);
			spi_delay(delay_time);
		}
		else
		{
			if(value&0x80)
				gpio_write_io(data_pin,1);
			else
				gpio_write_io(data_pin,0);

			value = value <<1;

			spi_delay(delay_time);
			gpio_write_io(clk_pin,1);
			spi_delay(delay_time);
			gpio_write_io(clk_pin,0);
			spi_delay(delay_time);
		}
		
	}
	gpio_write_io(cs_pin,1);
	return 0;
}

static int read_command(unsigned char reg,unsigned char *value)
{
	int i=16;
	int temp=0;
	int pin_value;
	unsigned char adr = reg;
	gpio_write_io(cs_pin,1);//CS=1
	gpio_write_io(data_pin,0);//SCL=0
	gpio_write_io(clk_pin,0);//SDA
	
	adr = adr << 1;
	// set csn low
	gpio_write_io(cs_pin,0);//CS=0
	spi_delay(delay_time);
	for(i=0;i<16;i++)
	{
		if(i<8)
		{
			if(i==1)
				gpio_write_io(data_pin,1); // read flag
			else
			{
				if(adr&0x80)
					gpio_write_io(data_pin,1);
				else
					gpio_write_io(data_pin,0);
				
				adr = adr << 1;
			}
			
			spi_delay(delay_time);
			gpio_write_io(clk_pin,1);
			spi_delay(delay_time);
			gpio_write_io(clk_pin,0);
			spi_delay(delay_time);
		}
		else
		{
			gpio_write_io(clk_pin,1);
			spi_delay(delay_time);
			pin_value=gpio_read_io(data_pin);
			temp = temp<<1;
			temp+=pin_value;
			
			spi_delay(delay_time);
			gpio_write_io(clk_pin,0);
			spi_delay(delay_time);
		}
		
	}
	gpio_write_io(cs_pin,1);
	*value = temp;
	printk("reg 0x%x,value 0x%x\r\n",reg,temp);
	return 0;
}

static void init_disp_comond(void)
{

	unsigned char value;
	//init pin.
	printk("[%s][%d]\r\n",__FUNCTION__,__LINE__);
	
	cs_pin = gp_gpio_request(MK_GPIO_INDEX(0,0,48,15), NULL ); //IOA15
	data_pin = gp_gpio_request(MK_GPIO_INDEX(1,0,12,0), NULL ); //IOB0
	clk_pin = gp_gpio_request(MK_GPIO_INDEX(1,0,13,1), NULL ); //IOB1

	gp_gpio_set_output(cs_pin,1,0);
	gp_gpio_set_output(data_pin,1,0);
	gp_gpio_set_output(clk_pin,1,0);

	sent_command(0x05,0x5f);
	read_command(0x05,&value);
	sent_command(0x05,0x1f);
	read_command(0x05,&value);
	sent_command(0x05,0x5f);
	read_command(0x05,&value);
	sent_command(0x2b,0x01);
	read_command(0x2b,&value);
	sent_command(0x00,0x09);
	read_command(0x00,&value);
	sent_command(0x01,0x9f);
	read_command(0x01,&value);
	
	//m-sent_command(0x03,0x60);
	sent_command(0x03,0x2e);
	read_command(0x03,&value);	
	
	//m-sent_command(0x0d,0x60);
	sent_command(0x0d,0x50);
	read_command(0x0d,&value);	
	
	//m-sent_command(0x04,0x1b);
	sent_command(0x04,0x18);
	read_command(0x04,&value);
	sent_command(0x16,0x04);
	read_command(0x16,&value);
	gp_gpio_release(cs_pin);
	gp_gpio_release(data_pin);
	gp_gpio_release(clk_pin);

}


static int32_t
lcd_init(
	void
)
{
	int ret=0;
	DEBUG("[%s:%d]\n", __FUNCTION__, __LINE__);
	init_disp_comond();
	ret=panel_common_init(GID_SRGBM888);
	DEBUG("[%s:%d]\n", __FUNCTION__, __LINE__);
	return ret;
}

static int32_t
lcd_suspend(
	void
)
{
	DEBUG("[%s:%d]\n", __FUNCTION__, __LINE__);
	return panel_common_suspend(GID_SRGBM888);
}

static int32_t
lcd_resume(
	void
)
{
	DEBUG("[%s:%d]\n", __FUNCTION__, __LINE__);
	return panel_common_resume(GID_SRGBM888);
}

static void*
lcd_get_param(
	void
)
{
	DEBUG("[%s:%d]\n", __FUNCTION__, __LINE__);
	return (void *)&gPanelInfo;
}

static int32_t
panel_init(
	void
)
{
	DEBUG("[%s:%d]\n", __FUNCTION__, __LINE__);
	register_paneldev(SP_DISP_OUTPUT_LCD, gPanelInfo.name, &lcd_fops);
	return 0;
}

static void
panel_exit(
	void
)
{
	DEBUG("[%s:%d]\n", __FUNCTION__, __LINE__);
	unregister_paneldev(SP_DISP_OUTPUT_LCD, gPanelInfo.name);
}

module_init(panel_init);
module_exit(panel_exit);