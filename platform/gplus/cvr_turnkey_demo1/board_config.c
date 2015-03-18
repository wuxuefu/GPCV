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

/*!
 * @file board_config.c
 * @brief The configuration of board
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h> /* everything... */
#include <mach/cdev.h>

#include <mach/gp_board.h>
#include <mach/gp_display.h>
#include <mach/general.h>
#include "sysconfig.h"
#include "platform.h"
#include <mach/gp_pwm.h>
#include <mach/gp_gpio.h>
#include <linux/delay.h> 	/* udelay/mdelay */
#include <mach/gp_adc.h>
#include <mach/gp_usb.h>
#include <mach/hal/hal_gpio.h>
#include <mach/hal/hal_usb.h>
#include <mach/hal/hal_clock.h>
#include <mach/gp_sd.h>
#include <mach/app_dev.h>

MODULE_LICENSE("GPL");
#define SYSCONFIG_INTERNAL_ADC 1
int gp_pwm_device_register(void);

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
typedef struct gp_board_info_s
{
	struct semaphore pin_func_mutex;
	int handle_sd0_detect;
	int handle_sd1_detect;
	int handle_sd2_detect;
/**************************************************************************
 * Sytem Power status
 **************************************************************************/
	int handle_battery_voltage;
	int handle_core_1v2_voltage;
} gp_board_info_t;

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static gp_board_info_t *gp_board_info = NULL;

/**************************************************************************
 * Board
 **************************************************************************/

static int
gpiocfgOut(
	int channel,
	int func,
	int gid,
	int pin,
	int level
)
{
	int handle;
	gpio_content_t ctx;

	if ( channel == 0xff ) {
		return -1;	/* No Function */
	}

	ctx.pin_index = MK_GPIO_INDEX( channel, func, gid, pin );
	handle = gp_gpio_request( ctx.pin_index, NULL );

	gp_gpio_set_function( handle, GPIO_FUNC_GPIO );//set io as GPIO mode
	gp_gpio_set_direction( handle, GPIO_DIR_OUTPUT );
	gp_gpio_set_output( handle, level, 0 );
	
	gp_gpio_release( handle );
	return	0;
}

static int
gpiocfgInput(
	int channel,
	int func,
	int gid,
	int pin
)
{
	int handle;
	gpio_content_t ctx;

	if ( channel == 0xff ) {
		return -1;	/* No Function */
	}

	ctx.pin_index = MK_GPIO_INDEX( channel, func, gid, pin );
	handle = gp_gpio_request( ctx.pin_index, NULL );

	gp_gpio_set_function( handle, GPIO_FUNC_GPIO );//set io as GPIO mode
	gp_gpio_set_direction( handle, GPIO_DIR_INPUT );

	gp_gpio_release( handle );
	return	0;
}
 
/* New Function */
#if 0
static int
gpiocfgOutDrv(
	int channel,
	int func,
	int gid,
	int pin,
	int level,
	unsigned int driving_current
)
{
	int handle;
	gpio_content_t ctx;

	if ( channel == 0xff ) {
		return -1;	/* No Function */
	}

	ctx.pin_index = MK_GPIO_INDEX( channel, func, gid, pin );
	handle = gp_gpio_request( ctx.pin_index, NULL );

	gp_gpio_set_function( handle, GPIO_FUNC_GPIO );//set io as GPIO mode
	gp_gpio_set_direction( handle, GPIO_DIR_OUTPUT );
	gp_gpio_set_output( handle, level, 0 );
	gp_gpio_set_driving_current( handle, driving_current );

	gp_gpio_release( handle );
	return	0;
}
#endif

static int
gpiocfgInputPull(
	int channel,
	int func,
	int gid,
	int pin,
	unsigned int pull_value
)
{
	int handle;
	gpio_content_t ctx;

	if ( channel == 0xff ) {
		return -1;	/* No Function */
	}

	ctx.pin_index = MK_GPIO_INDEX( channel, func, gid, pin );
	handle = gp_gpio_request( ctx.pin_index, NULL );

	gp_gpio_set_function( handle, GPIO_FUNC_GPIO );//set io as GPIO mode
	gp_gpio_set_direction( handle, GPIO_DIR_INPUT );
	gp_gpio_set_pullfunction(handle, pull_value );
	gp_gpio_release( handle );
	return	0;
}

static int
gpiocfgInputPullDrv(
	int channel,
	int func,
	int gid,
	int pin,
	unsigned int pull_value,
	unsigned int driving_current
)
{
	int handle;
	gpio_content_t ctx;

	if ( channel == 0xff ) {
		return -1;	/* No Function */
	}

	ctx.pin_index = MK_GPIO_INDEX( channel, func, gid, pin );
	handle = gp_gpio_request( ctx.pin_index, NULL );

	gp_gpio_set_function( handle, GPIO_FUNC_GPIO );//set io as GPIO mode
	gp_gpio_set_direction( handle, GPIO_DIR_INPUT );
	gp_gpio_set_pullfunction(handle, pull_value );
	gp_gpio_set_driving_current( handle, driving_current );
	gp_gpio_release( handle );
	return	0;
}

/**************************************************************************
 * Touch Control
 **************************************************************************/

int touch_get_resetpin(
	int *index
)
{
	*index = MK_GPIO_INDEX( touch_reset_channel, touch_reset_func, touch_reset_gid, touch_reset_pin );
	return 0;
}

int touch_get_intpin(
	int *index
)
{
	*index = MK_GPIO_INDEX( touch_int_channel, touch_int_func, touch_int_gid, touch_int_pin );
	return 0;
}

#ifndef SYSCONFIG_SCREEN_ROTATE
#define SYSCONFIG_SCREEN_ROTATE 0
#endif
#ifndef SYSCONFIG_SCREEN_ROTATE
#define SYSCONFIG_SCREEN_ROTATE 0
#endif
int touch_get_virtualkeyshow(
	char *buf
)	
{
	printk("[%s][%d][%d]\n", __FUNCTION__, __LINE__, EV_KEY);

#if SYSCONFIG_SCREEN_ROTATE == 0
	return sprintf(buf,
		__stringify(EV_KEY) ":" __stringify(KEY_BACK) ":860:120:83:40"
		":" __stringify(EV_KEY) ":" __stringify(KEY_HOME) ":860:240:83:40"
		":" __stringify(EV_KEY) ":" __stringify(KEY_MENU) ":860:360:83:40"
		"\n");
#elif SYSCONFIG_SCREEN_ROTATE == 90
	return sprintf(buf,
		__stringify(EV_KEY) ":" __stringify(KEY_BACK) ":120:-60:40:83"
		":" __stringify(EV_KEY) ":" __stringify(KEY_HOME) ":240:-60:40:83"
		":" __stringify(EV_KEY) ":" __stringify(KEY_MENU) ":360:-60:40:83"
		"\n");
#elif SYSCONFIG_SCREEN_ROTATE == 180
	return sprintf(buf,
		__stringify(EV_KEY) ":" __stringify(KEY_BACK) ":-60:360:83:40"
		":" __stringify(EV_KEY) ":" __stringify(KEY_HOME) ":-60:240:83:40"
		":" __stringify(EV_KEY) ":" __stringify(KEY_MENU) ":-60:120:83:40"
		"\n");
#elif SYSCONFIG_SCREEN_ROTATE == 270
	return sprintf(buf,
		__stringify(EV_KEY) ":" __stringify(KEY_BACK) ":360:860:40:83"
		":" __stringify(EV_KEY) ":" __stringify(KEY_HOME) ":240:860:40:83"
		":" __stringify(EV_KEY) ":" __stringify(KEY_MENU) ":120:860:40:83"
		"\n");
#else
#error Invalid screen rotation
#endif
}


static const gp_board_touch_t config_touch= {
	.get_touch_resetpin = touch_get_resetpin,
	.get_touch_intpin = touch_get_intpin,
	.get_touch_virtualkeyshow = touch_get_virtualkeyshow,
};



static int
pwm_init(
	int pwm_id,
	int freq,
	int init_duty
)
{
	struct gp_pwm_config_s pwm_config;
	int pwm;
	int	channel =0, func =0, gid =0, pin =0;
	int handle;
	gpio_content_t ctx;

	if ( ( pwm_id == 0xff ) || ( pwm_id > 2 ) || ( pwm_id < 1 ) ) {
		return -1;	/* No Function */
	}

	pwm  = gp_pwm_request(pwm_id);
	if(0 == pwm){
		return -1;
	}

	pwm_config.duty = (UINT32) init_duty;
	pwm_config.freq = (UINT32) freq;
	gp_pwm_set_config(pwm, &pwm_config);
	gp_pwm_enable(pwm);
	gp_pwm_release(pwm);

	switch ( pwm_id ) {
		case 1:
			channel = G32900_pwm1_channel;
			func = G32900_pwm1_func;
			gid = G32900_pwm1_gid;
			pin = G32900_pwm1_pin;
			break;
		case 2:
			channel = G32900_pwm2_channel;
			func = G32900_pwm2_func;
			gid = G32900_pwm2_gid;
			pin = G32900_pwm2_pin;
			break;
	}
	
	ctx.pin_index = MK_GPIO_INDEX( channel, func, gid, pin );
	handle = gp_gpio_request( ctx.pin_index, NULL );
	
	gp_gpio_set_function( handle, GPIO_FUNC_GPIO );//set io as GPIO mode

	gp_gpio_release( handle );
	return	0;
}

static int
board_init(
	void
)
{
	printk("board_init!\n");
#ifndef SYSCONFIG_INTERNAL_RTC
	//disable internal RTC clock
	gpHalScuClkEnable((SCU_B_PERI_RTC), SCU_B, 0);
#endif /* !SYSCONFIG_INTERNAL_RTC */

	gp_pwm_device_register();

/**************************************************************************
 * Pin MUX work-around
 **************************************************************************/
	WRITE32(0xfc005144, READ32(0xfc005144) | 0x01); /* Use I2C */

/**************************************************************************
 * Panel power & Backlight
 **************************************************************************/
	//panel_set_backlight
	//gpiocfgOut( panel_set_backlight_channel, panel_set_backlight_func, panel_set_backlight_gid, panel_set_backlight_pin, panel_set_backlight_level );
	//gpiocfgOut( panel_set_AVDD_EN_channel, panel_set_AVDD_EN_func, panel_set_AVDD_EN_gid, panel_set_AVDD_EN_pin, panel_set_AVDD_EN_level );

	//panel_brightness_backlight
	pwm_init( panel_brightness_backlight_id, panel_brightness_backlight_freq, panel_brightness_backlight_init_duty );

	//panel_set_power0
	gpiocfgOut( panel_set_powerOn0_0_channel, panel_set_powerOn0_0_func, panel_set_powerOn0_0_gid, panel_set_powerOn0_0_pin, ~panel_set_powerOn0_0_level );
	gpiocfgOut( panel_set_powerOn0_1_channel, panel_set_powerOn0_1_func, panel_set_powerOn0_1_gid, panel_set_powerOn0_1_pin, ~panel_set_powerOn0_1_level );
	gpiocfgOut( panel_set_powerOn0_2_channel, panel_set_powerOn0_2_func, panel_set_powerOn0_2_gid, panel_set_powerOn0_2_pin, ~panel_set_powerOn0_2_level );
	gpiocfgOut( panel_set_powerOn0_3_channel, panel_set_powerOn0_3_func, panel_set_powerOn0_3_gid, panel_set_powerOn0_3_pin, ~panel_set_powerOn0_3_level );

	//panel_set_power1
	gpiocfgOut( panel_set_powerOn1_0_channel, panel_set_powerOn1_0_func, panel_set_powerOn1_0_gid, panel_set_powerOn1_0_pin, ~panel_set_powerOn1_0_level );
	gpiocfgOut( panel_set_powerOn1_1_channel, panel_set_powerOn1_1_func, panel_set_powerOn1_1_gid, panel_set_powerOn1_1_pin, ~panel_set_powerOn1_1_level );
	gpiocfgOut( panel_set_powerOn1_2_channel, panel_set_powerOn1_2_func, panel_set_powerOn1_2_gid, panel_set_powerOn1_2_pin, ~panel_set_powerOn1_2_level );
	gpiocfgOut( panel_set_powerOn1_3_channel, panel_set_powerOn1_3_func, panel_set_powerOn1_3_gid, panel_set_powerOn1_3_pin, ~panel_set_powerOn1_3_level );

	//panel_set_spi
	gpiocfgOut( panel_set_spi_cs_channel, panel_set_spi_cs_func, panel_set_spi_cs_gid, panel_set_spi_cs_pin, 1 );
	gpiocfgOut( panel_set_spi_scl_channel, panel_set_spi_scl_func, panel_set_spi_scl_gid, panel_set_spi_scl_pin, 1 );
	gpiocfgOut( panel_set_spi_sda_channel, panel_set_spi_sda_func, panel_set_spi_sda_gid, panel_set_spi_sda_pin, 1 );

	//panel_set_mirror
	gpiocfgInput( panel_set_mirror0_channel, panel_set_mirror0_func, panel_set_mirror0_gid, panel_set_mirror0_pin );
	gpiocfgInput( panel_set_mirror1_channel, panel_set_mirror1_func, panel_set_mirror1_gid, panel_set_mirror1_pin );

/**************************************************************************
 * SD
 **************************************************************************/
 	//sd0 pin setting
 	gpiocfgInputPullDrv( sd0_clk_channel, sd0_clk_func, sd0_clk_gid, sd0_clk_pin, sd0_clk_pull_level, sd0_clk_driving );
 	gpiocfgInputPullDrv( sd0_cmd_channel, sd0_cmd_func, sd0_cmd_gid, sd0_cmd_pin, sd0_cmd_pull_level, sd0_cmd_driving );
 	gpiocfgInputPullDrv( sd0_data0_channel, sd0_data0_func, sd0_data0_gid, sd0_data0_pin, sd0_data0_pull_level, sd0_data0_driving );
 	gpiocfgInputPullDrv( sd0_data1_channel, sd0_data1_func, sd0_data1_gid, sd0_data1_pin, sd0_data1_pull_level, sd0_data1_driving );
 	gpiocfgInputPullDrv( sd0_data2_channel, sd0_data2_func, sd0_data2_gid, sd0_data2_pin, sd0_data2_pull_level, sd0_data2_driving );
 	gpiocfgInputPullDrv( sd0_data3_channel, sd0_data3_func, sd0_data3_gid, sd0_data3_pin, sd0_data3_pull_level, sd0_data3_driving );
 	
	//sd0_set_power
	gpiocfgOut( sd0_set_power_channel, sd0_set_power_func, sd0_set_power_gid, sd0_set_power_pin, ~sd0_set_power_level );

	//sd0_detect
	gpiocfgInputPull( sd0_detect_channel, sd0_detect_func, sd0_detect_gid, sd0_detect_pin, sd0_detect_pull_level );
	if(sd0_detect_channel !=0xff)
		gp_board_info->handle_sd0_detect = gp_gpio_request(MK_GPIO_INDEX( sd0_detect_channel, sd0_detect_func, sd0_detect_gid, sd0_detect_pin ), "SD0 detect pin");

	//sd0_is_write_protected
	gpiocfgInputPull( sd0_is_write_protected_channel, sd0_is_write_protected_func, sd0_is_write_protected_gid, sd0_is_write_protected_pin, sd0_is_write_protected_pull_level );

	//sdio0_set_standby
	gpiocfgOut( sdio0_set_standby_channel, sdio0_set_standby_func, sdio0_set_standby_gid, sdio0_set_standby_pin, ~sdio0_set_standby_level );

	//sd1 pin setting
 	gpiocfgInputPullDrv( sd1_clk_channel, sd1_clk_func, sd1_clk_gid, sd1_clk_pin, sd1_clk_pull_level, sd1_clk_driving );
 	gpiocfgInputPullDrv( sd1_cmd_channel, sd1_cmd_func, sd1_cmd_gid, sd1_cmd_pin, sd1_cmd_pull_level, sd1_cmd_driving );
 	gpiocfgInputPullDrv( sd1_data0_channel, sd1_data0_func, sd1_data0_gid, sd1_data0_pin, sd1_data0_pull_level, sd1_data0_driving );
 	gpiocfgInputPullDrv( sd1_data1_channel, sd1_data1_func, sd1_data1_gid, sd1_data1_pin, sd1_data1_pull_level, sd1_data1_driving );
 	gpiocfgInputPullDrv( sd1_data2_channel, sd1_data2_func, sd1_data2_gid, sd1_data2_pin, sd1_data2_pull_level, sd1_data2_driving );
 	gpiocfgInputPullDrv( sd1_data3_channel, sd1_data3_func, sd1_data3_gid, sd1_data3_pin, sd1_data3_pull_level, sd1_data3_driving );
	
	//sd1_set_power
	gpiocfgOut( sd1_set_power_channel, sd1_set_power_func, sd1_set_power_gid, sd1_set_power_pin, ~sd1_set_power_level );

	//sd1_detect
	gpiocfgInputPull( sd1_detect_channel, sd1_detect_func, sd1_detect_gid, sd1_detect_pin, sd1_detect_pull_level );
	if(sd1_detect_channel !=0xff)
		gp_board_info->handle_sd1_detect = gp_gpio_request(MK_GPIO_INDEX( sd1_detect_channel, sd1_detect_func, sd1_detect_gid, sd1_detect_pin ), "SD1 detect pin");

	//sd1_is_write_protected
	gpiocfgInputPull( sd1_is_write_protected_channel, sd1_is_write_protected_func, sd1_is_write_protected_gid, sd1_is_write_protected_pin, sd1_is_write_protected_pull_level);

	//sdio1_set_standby
	gpiocfgOut( sdio1_set_standby_channel, sdio1_set_standby_func, sdio1_set_standby_gid, sdio1_set_standby_pin, ~sdio1_set_standby_level );

	//sd2 pin setting
 	gpiocfgInputPullDrv( sd2_clk_channel, sd2_clk_func, sd2_clk_gid, sd2_clk_pin, sd2_clk_pull_level, sd2_clk_driving );
 	gpiocfgInputPullDrv( sd2_cmd_channel, sd2_cmd_func, sd2_cmd_gid, sd2_cmd_pin, sd2_cmd_pull_level, sd2_cmd_driving );
 	gpiocfgInputPullDrv( sd2_data0_channel, sd2_data0_func, sd2_data0_gid, sd2_data0_pin, sd2_data0_pull_level, sd2_data0_driving );
 	gpiocfgInputPullDrv( sd2_data1_channel, sd2_data1_func, sd2_data1_gid, sd2_data1_pin, sd2_data1_pull_level, sd2_data1_driving );
 	gpiocfgInputPullDrv( sd2_data2_channel, sd2_data2_func, sd2_data2_gid, sd2_data2_pin, sd2_data2_pull_level, sd2_data2_driving );
 	gpiocfgInputPullDrv( sd2_data3_channel, sd2_data3_func, sd2_data3_gid, sd2_data3_pin, sd2_data3_pull_level, sd2_data3_driving );
 	
	//sd2_set_power
	gpiocfgOut( sd2_set_power_channel, sd2_set_power_func, sd2_set_power_gid, sd2_set_power_pin, ~sd2_set_power_level );

	//sd2_detect
	gpiocfgInputPull( sd2_detect_channel, sd2_detect_func, sd2_detect_gid, sd2_detect_pin, sd2_detect_pull_level );
	if(sd2_detect_channel !=0xff)
		gp_board_info->handle_sd2_detect = gp_gpio_request(MK_GPIO_INDEX( sd2_detect_channel, sd2_detect_func, sd2_detect_gid, sd2_detect_pin ), "SD2 detect pin");

	//sd2_is_write_protected
	gpiocfgInputPull( sd2_is_write_protected_channel, sd2_is_write_protected_func, sd2_is_write_protected_gid, sd2_is_write_protected_pin, sd2_is_write_protected_pull_level );

	//sdio2_set_standby
	gpiocfgOut( sdio2_set_standby_channel, sdio2_set_standby_func, sdio2_set_standby_gid, sdio2_set_standby_pin, ~sdio2_set_standby_level );

/**************************************************************************
 * NAND - CSn & WP
 **************************************************************************/
//nand_set_cs


	//nand_set_wp
	gpiocfgOut( nand_set_wp_channel, nand_set_wp_func, nand_set_wp_gid, nand_set_wp_pin, nand_set_wp_active );

/**************************************************************************
 * USB HOST / SLAVE
 **************************************************************************/ 
	//usb_host_en_power
	gpiocfgOut( usb_host_en_power_channel, usb_host_en_power_func, usb_host_en_power_gid, usb_host_en_power_pin, ~usb_host_en_power_active );
	#if 0
	//usb_slave_detect_power

	/*Set GPIO as input. And request GPIO in middleware/usb.*/
	gpiocfgInput( usb_slave_detect_power_channel, usb_slave_detect_power_func, usb_slave_detect_power_gid, usb_slave_detect_power_pin );
	#endif
	//usb_switch_bus
	gpiocfgOut( usb_switch_bus_channel, usb_switch_bus_func, usb_switch_bus_gid, usb_switch_bus_pin, ~usb_switch_bus_active );

	//usb_host_power_good
	gpiocfgInput( usb_power_good_channel, usb_power_good_func, usb_power_good_gid, usb_power_good_pin );

/**************************************************************************
 * Audio out
 **************************************************************************/
	//headphone_detect
	gpiocfgInput( headphone_detect_channel, headphone_detect_func, headphone_detect_gid, headphone_detect_pin );

	//speaker_set_power
	gpiocfgOut( speaker_set_power_channel, speaker_set_power_func, speaker_set_power_gid, speaker_set_power_pin, ~speaker_set_power_active );

/**************************************************************************
 * Sensor
 **************************************************************************/
	//sensor port0 setting
	gpiocfgInput( sensor_port0_mclk_channel, sensor_port0_mclk_func, sensor_port0_mclk_gid, sensor_port0_mclk_pin );
	gpiocfgInput( sensor_port0_pclk_channel, sensor_port0_pclk_func, sensor_port0_pclk_gid, sensor_port0_pclk_pin );
	gpiocfgInput( sensor_port0_vsync_channel, sensor_port0_vsync_func, sensor_port0_vsync_gid, sensor_port0_vsync_pin );
	gpiocfgInput( sensor_port0_hsync_channel, sensor_port0_hsync_func, sensor_port0_hsync_gid, sensor_port0_hsync_pin );
	gpiocfgInput( sensor_port0_data_channel, sensor_port0_data_func, sensor_port0_data_gid_03, sensor_port0_data_startpin+0 );
	gpiocfgInput( sensor_port0_data_channel, sensor_port0_data_func, sensor_port0_data_gid_03, sensor_port0_data_startpin+1 );
	gpiocfgInput( sensor_port0_data_channel, sensor_port0_data_func, sensor_port0_data_gid_03, sensor_port0_data_startpin+2 );
	gpiocfgInput( sensor_port0_data_channel, sensor_port0_data_func, sensor_port0_data_gid_03, sensor_port0_data_startpin+3 );
	gpiocfgInput( sensor_port0_data_channel, sensor_port0_data_func, sensor_port0_data_gid_47, sensor_port0_data_startpin+4 );
	gpiocfgInput( sensor_port0_data_channel, sensor_port0_data_func, sensor_port0_data_gid_47, sensor_port0_data_startpin+5 );
	gpiocfgInput( sensor_port0_data_channel, sensor_port0_data_func, sensor_port0_data_gid_47, sensor_port0_data_startpin+6 );
	gpiocfgInput( sensor_port0_data_channel, sensor_port0_data_func, sensor_port0_data_gid_47, sensor_port0_data_startpin+7 );

	//sensor port1 setting
	gpiocfgInput( sensor_port1_mclk_channel, sensor_port1_mclk_func, sensor_port1_mclk_gid, sensor_port1_mclk_pin );
	gpiocfgInput( sensor_port1_pclk_channel, sensor_port1_pclk_func, sensor_port1_pclk_gid, sensor_port1_pclk_pin );
	gpiocfgInput( sensor_port1_vsync_channel, sensor_port1_vsync_func, sensor_port1_vsync_gid, sensor_port1_vsync_pin );
	gpiocfgInput( sensor_port1_hsync_channel, sensor_port1_hsync_func, sensor_port1_hsync_gid, sensor_port1_hsync_pin );
	gpiocfgInput( sensor_port1_data_channel, sensor_port1_data_func, sensor_port1_data_gid, sensor_port1_data_startpin );
	gpiocfgInput( sensor_port1_data_channel, sensor_port1_data_func, sensor_port1_data_gid, sensor_port1_data_startpin+1 );
	gpiocfgInput( sensor_port1_data_channel, sensor_port1_data_func, sensor_port1_data_gid, sensor_port1_data_startpin+2 );
	gpiocfgInput( sensor_port1_data_channel, sensor_port1_data_func, sensor_port1_data_gid, sensor_port1_data_startpin+3 );
	gpiocfgInput( sensor_port1_data_channel, sensor_port1_data_func, sensor_port1_data_gid, sensor_port1_data_startpin+4 );
	gpiocfgInput( sensor_port1_data_channel, sensor_port1_data_func, sensor_port1_data_gid, sensor_port1_data_startpin+5 );
	gpiocfgInput( sensor_port1_data_channel, sensor_port1_data_func, sensor_port1_data_gid, sensor_port1_data_startpin+6 );
	gpiocfgInput( sensor_port1_data_channel, sensor_port1_data_func, sensor_port1_data_gid, sensor_port1_data_startpin+7 );
	
	//sensor0 reset/standby/power setting
	gpiocfgOut( sensor0_set_reset_channel, sensor0_set_reset_func, sensor0_set_reset_gid, sensor0_set_reset_pin, ~sensor0_set_reset_active );
	gpiocfgOut( sensor0_set_standby_channel, sensor0_set_standby_func, sensor0_set_standby_gid, sensor0_set_standby_pin, ~sensor0_set_standby_active );
	gpiocfgOut( sensor0_set_power_channel, sensor0_set_power_func, sensor0_set_power_gid, sensor0_set_power_pin, ~sensor0_set_power_active );

	//sensor1 reset/standby/power setting
	gpiocfgOut( sensor1_set_reset_channel, sensor1_set_reset_func, sensor1_set_reset_gid, sensor1_set_reset_pin, ~sensor1_set_reset_active );
	gpiocfgOut( sensor1_set_standby_channel, sensor1_set_standby_func, sensor1_set_standby_gid, sensor1_set_standby_pin, ~sensor1_set_standby_active );
	gpiocfgOut( sensor1_set_power_channel, sensor1_set_power_func, sensor1_set_power_gid, sensor1_set_power_pin, ~sensor1_set_power_active );

	//sensor2 reset/standby/power setting
	gpiocfgOut( sensor2_set_power_channel, sensor2_set_power_func, sensor2_set_power_gid, sensor2_set_power_pin, sensor2_set_power_active );
	gpiocfgOut( sensor2_set_reset_channel, sensor2_set_reset_func, sensor2_set_reset_gid, sensor2_set_reset_pin, ~sensor2_set_reset_active );
	gpiocfgOut( sensor2_set_reset_channel, sensor2_set_reset_func, sensor2_set_reset_gid, sensor2_set_reset_pin, sensor2_set_reset_active );
	gpiocfgOut( sensor2_set_reset_channel, sensor2_set_reset_func, sensor2_set_reset_gid, sensor2_set_reset_pin, ~sensor2_set_reset_active );
	gpiocfgOut( sensor2_set_standby_channel, sensor2_set_standby_func, sensor2_set_standby_gid, sensor2_set_standby_pin, ~sensor2_set_standby_active );
	gpiocfgOut( sensor2_set_power_channel, sensor2_set_power_func, sensor2_set_power_gid, sensor2_set_power_pin, sensor2_set_power_active );

/**************************************************************************
 * mipi0 / mipi1
 **************************************************************************/
	//mclk
	gpiocfgInput(mipi_mclk_channel, mipi_mclk_func, mipi_mclk_gid, mipi_mclk_pin);

 	//mipi0
	gpiocfgInput(mipi0_clkn_channel, mipi0_clkn_func, mipi0_clkn_gid, mipi0_clkn_pin);
	gpiocfgInput(mipi0_clkp_channel, mipi0_clkp_func, mipi0_clkp_gid, mipi0_clkp_pin);
	gpiocfgInput(mipi0_data0n_channel, mipi0_data0n_func, mipi0_data0n_gid, mipi0_data0n_pin);
	gpiocfgInput(mipi0_data0p_channel, mipi0_data0p_func, mipi0_data0p_gid, mipi0_data0p_pin);

	//mipi1
	gpiocfgInput(mipi1_clkn_channel, mipi1_clkn_func, mipi1_clkn_gid, mipi1_clkn_pin);
	gpiocfgInput(mipi1_clkp_channel, mipi1_clkp_func, mipi1_clkp_gid, mipi1_clkp_pin);
	gpiocfgInput(mipi1_data0n_channel, mipi1_data0n_func, mipi1_data0n_gid, mipi1_data0n_pin);
	gpiocfgInput(mipi1_data0p_channel, mipi1_data0p_func, mipi1_data0p_gid, mipi1_data0p_pin);
	
/**************************************************************************
 * LED Control
 **************************************************************************/
	//led_set_light(
	gpiocfgOut( led_set_light0_channel, led_set_light0_func, led_set_light0_gid, led_set_light0_pin, ~led_set_light0_active );
	gpiocfgOut( led_set_light1_channel, led_set_light1_func, led_set_light1_gid, led_set_light1_pin, ~led_set_light1_active );
	//gpiocfgOut( led_set_light2_channel, led_set_light2_func, led_set_light2_gid, led_set_light2_pin, ~led_set_light2_active );
	gpiocfgOut( led_set_light3_channel, led_set_light3_func, led_set_light3_gid, led_set_light3_pin, ~led_set_light3_active );
	gpiocfgOut( led_set_light4_channel, led_set_light4_func, led_set_light4_gid, led_set_light4_pin, ~led_set_light4_active );
	gpiocfgOut( led_set_light5_channel, led_set_light5_func, led_set_light5_gid, led_set_light5_pin, ~led_set_light5_active );
	gpiocfgOut( led_set_light6_channel, led_set_light6_func, led_set_light6_gid, led_set_light6_pin, ~led_set_light6_active );
	gpiocfgOut( led_set_light7_channel, led_set_light7_func, led_set_light7_gid, led_set_light7_pin, ~led_set_light7_active );
	gpiocfgOut( led_set_light8_channel, led_set_light8_func, led_set_light8_gid, led_set_light8_pin, ~led_set_light8_active );
	gpiocfgOut( led_set_light9_channel, led_set_light9_func, led_set_light9_gid, led_set_light9_pin, ~led_set_light9_active );

//led_set_brightness
	pwm_init( led_set_brightness_id1, led_set_brightness_freq1, led_set_brightness_init_duty1 );
	pwm_init( led_set_brightness_id2, led_set_brightness_freq2, led_set_brightness_init_duty2 );

/**************************************************************************
 * I2C
 **************************************************************************/	
	gpiocfgInputPull(i2c_sda_channel, i2c_sda_func, i2c_sda_gid, i2c_sda_pin, i2c_sda_pull_level);
	gpiocfgInputPull(i2c_scl_channel, i2c_scl_func, i2c_scl_gid, i2c_scl_pin, i2c_scl_pull_level);

/**************************************************************************
 * TI2C
 **************************************************************************/
	gpiocfgInputPull(ti2c_sda_channel, ti2c_sda_func, ti2c_sda_gid, ti2c_sda_pin, ti2c_sda_pull_level);
	gpiocfgInputPull(ti2c_scl_channel, ti2c_scl_func, ti2c_scl_gid, ti2c_scl_pin, ti2c_scl_pull_level);

/**************************************************************************
 * angle switch
 **************************************************************************/
//angle_sw_detect
	gpiocfgInput( angle_sw_detect_channel, angle_sw_detect_func, angle_sw_detect_gid, angle_sw_detect_pin );

/**************************************************************************
 * PCB Version Detection
 **************************************************************************/
//pcb_version_detect

/**************************************************************************
 * Sytem Power status
 **************************************************************************/
//dc_in_detect
	gpiocfgInput( dc_in_detect_channel, dc_in_detect_func, dc_in_detect_gid, dc_in_detect_pin );

#ifdef SYSCONFIG_INTERNAL_ADC
	gp_board_info->handle_battery_voltage = gp_adc_request(0,0);
	if( !(IS_ERR_VALUE(gp_board_info->handle_battery_voltage) ) )
		gp_adc_start(gp_board_info->handle_battery_voltage, battery_voltage_detect_adc_ch);

	gp_board_info->handle_core_1v2_voltage = gp_adc_request(0,0);
	if( !(IS_ERR_VALUE(gp_board_info->handle_core_1v2_voltage) ) )
		gp_adc_start(gp_board_info->handle_core_1v2_voltage, 8);

	//battery_charger_status
	//gpiocfgInput( battery_charger_status_channel, battery_charger_status_func, battery_charger_status_gid, battery_charger_status_pin );
#else
    gp_board_info->handle_battery_voltage = 0;
#endif
//battery_charger_status
	gpiocfgInput( battery_charger_status_channel, battery_charger_status_func, battery_charger_status_gid, battery_charger_status_pin );

/********************************************************************************
*Hdmi
*****************************************************************************/
	gpiocfgInput(hdmi_detect_channel,hdmi_detect_func,hdmi_detect_gid,hdmi_detect_pin);

/**************************************************************************
 * Pwm
 **************************************************************************/
 	//gpiocfgOut( pwm1_set_channel, pwm1_set_func, pwm1_set_gid, pwm1_set_pin, pwm1_set_level); 
	//gpiocfgOut( pwm2_set_channel, pwm2_set_func, pwm2_set_gid, pwm2_set_pin, pwm2_set_level);
/*****************************************************************************
*ASENSOR
********************************************************************************/
	gpiocfgInput(G_SENSOR_int_channel,G_SENSOR_int_func,G_SENSOR_int_gid,G_SENSOR_int_pin);
	return 0;
}
#if 0
static void
board_exit(
	void
)
{
	gp_gpio_release(gp_board_info->handle_sd0_detect);
	gp_gpio_release(gp_board_info->handle_sd1_detect);
	gp_gpio_release(gp_board_info->handle_sd2_detect);
#ifdef SYSCONFIG_INTERNAL_ADC
	gp_adc_release(gp_board_info->handle_battery_voltage);
	gp_adc_release(gp_board_info->handle_core_1v2_voltage);
#endif
}
#endif

static void
board_power_off(
	void
)
{
	
	printk("board_power_off!\n");
}

static const gp_board_t config_board = {
	.name = "evm32900",
	.init = board_init,
	.power_off = board_power_off,
};

/**************************************************************************
 * Panel power & Backlight
 **************************************************************************/
int 
panel_set_backlight(
	int enable
)
{
#if 1
	UINT32	value = 0, active = panel_set_backlight_level;
	unsigned int channel = panel_set_backlight_channel;
	int handle;
	gpio_content_t ctx;

	if ( channel == 0xff ) {
		return -1;	/* No Function */
	}

	if ( enable ) {	/* setting active polar */
		if ( active != 0 ) {
			value = 1;
		}
		else {
			value = 0;
		}
	}
	else {
		if ( active != 0 ) {
			value = 0;
		}
		else {
			value = 1;
		}
	}

	ctx.pin_index = MK_GPIO_INDEX( panel_set_backlight_channel, panel_set_backlight_func, panel_set_backlight_gid, panel_set_backlight_pin );
	handle = gp_gpio_request(ctx.pin_index, NULL);
	gp_gpio_set_output(handle, value, 0);
	gp_gpio_release(handle);
#else

#endif
	return 0;
}

int 
panel_brightness_backlight(
	int duty
)
{
	struct gp_pwm_config_s pwm_config;
	int pwm;
	int pwm_id;

	pwm_id = panel_brightness_backlight_id;
	if ( pwm_id == 0xff ) {
		return -1;
	}
	pwm  = gp_pwm_request(pwm_id);
	
	pwm_config.duty = (UINT32) duty;
	pwm_config.freq = panel_brightness_backlight_freq;
	gp_pwm_set_config(pwm, &pwm_config);
	gp_pwm_enable(pwm);
	gp_pwm_release(pwm);

	return 0;
}

int 
panel_set_power0(
	int enable
)
{
	UINT32	pinIndex0, pinIndex1, pinIndex2, pinIndex3;
	UINT32	value0, value1, value2, value3;
	unsigned int channel0, channel1, channel2, channel3;
	unsigned int func0, func1, func2, func3;
	unsigned int gid0, gid1, gid2, gid3;
	unsigned int pin0, pin1, pin2, pin3;
	unsigned int active0, active1, active2, active3;
	int handle;
	gpio_content_t ctx;
    int ret = 0;

	if ( enable != 0 ) {
		channel0 = panel_set_AVDD_EN_channel;
		channel1 = panel_set_powerOn0_1_channel;
		channel2 = panel_set_powerOn0_2_channel;
		channel3 = panel_set_powerOn0_3_channel;
		func0 = panel_set_AVDD_EN_func;
		func1 = panel_set_powerOn0_1_func;
		func2 = panel_set_powerOn0_2_func;
		func3 = panel_set_powerOn0_3_func;
		gid0 = panel_set_AVDD_EN_gid;
		gid1 = panel_set_powerOn0_1_gid;
		gid2 = panel_set_powerOn0_2_gid;
		gid3 = panel_set_powerOn0_3_gid;
		pin0 = panel_set_AVDD_EN_pin;
		pin1 = panel_set_powerOn0_1_pin;
		pin2 = panel_set_powerOn0_2_pin;
		pin3 = panel_set_powerOn0_3_pin;
		active0 = panel_set_AVDD_EN_level;
		active1 = panel_set_powerOn0_1_level;
		active2 = panel_set_powerOn0_2_level;
		active3 = panel_set_powerOn0_3_level;

		if ( active0 != 0 ) {
			value0 = 1;
		}
		else {
			value0 = 0;
		}

		if ( active1 != 0 ) {
			value1 = 1;
		}
		else {
			value1 = 0;
		}

		if ( active2 != 0 ) {
			value2 = 1;
		}
		else {
			value2 = 0;
		}

		if ( active3 != 0 ) {
			value3 = 1;
		}
		else {
			value3 = 0;
		}

		pinIndex0 = MK_GPIO_INDEX( channel0, func0, gid0, pin0 );
		pinIndex1 = MK_GPIO_INDEX( channel1, func1, gid1, pin1 );
		pinIndex2 = MK_GPIO_INDEX( channel2, func2, gid2, pin2 );
		pinIndex3 = MK_GPIO_INDEX( channel3, func3, gid3, pin3 );

		if ( panel_set_powerOn1_start_delay != 0 ) {
			msleep(panel_set_powerOn0_start_delay);
		}
		if ( channel0 == 0xff ) {
			return -1;	/* No Function */
		}
		
		ctx.pin_index = pinIndex0;
		handle = gp_gpio_request(ctx.pin_index, NULL);
		ret = gp_gpio_set_output( handle, value0, 0 );
	    gp_gpio_release(handle);
		
		if ( channel1 == 0xff ) {
			return 0;	/* No Function */
		}
		
		ctx.pin_index = pinIndex1;
		handle = gp_gpio_request(ctx.pin_index, NULL);
		gp_gpio_set_output( handle, value1, 0 );
		gp_gpio_release(handle);

		if ( panel_set_powerOn0_1to2_delay != 0 ) {
			msleep(panel_set_powerOn0_1to2_delay);
		}
		if ( channel2 == 0xff ) {
			return 0;	/* No Function */
		}

		ctx.pin_index = pinIndex2;
		handle = gp_gpio_request(ctx.pin_index, NULL);
		gp_gpio_set_output( handle, value2, 0 );
		gp_gpio_release(handle);
		if ( panel_set_powerOn0_2to3_delay != 0 ) {
			msleep(panel_set_powerOn0_2to3_delay);
		}
		if ( channel3 == 0xff ) {
			return 0;	/* No Function */
		}
		
		ctx.pin_index = pinIndex3;
		handle = gp_gpio_request(ctx.pin_index, NULL);
		gp_gpio_set_output( handle, value3, 0 );
		gp_gpio_release(handle);
		if ( panel_set_powerOn0_end_delay != 0 ) {
			msleep(panel_set_powerOn0_end_delay);
		}
		return 0;
	}
	else {
		channel0 = panel_set_AVDD_EN_channel;
		channel1 = panel_set_powerOff0_1_channel;
		channel2 = panel_set_powerOff0_2_channel;
		channel3 = panel_set_powerOff0_3_channel;
		func0 = panel_set_AVDD_EN_func;
		func1 = panel_set_powerOff0_1_func;
		func2 = panel_set_powerOff0_2_func;
		func3 = panel_set_powerOff0_3_func;
		gid0 = panel_set_AVDD_EN_gid;
		gid1 = panel_set_powerOff0_1_gid;
		gid2 = panel_set_powerOff0_2_gid;
		gid3 = panel_set_powerOff0_3_gid;
		pin0 = panel_set_AVDD_EN_pin;
		pin1 = panel_set_powerOff0_1_pin;
		pin2 = panel_set_powerOff0_2_pin;
		pin3 = panel_set_powerOff0_3_pin;
		active0 = panel_set_AVDD_EN_level;
		active1 = panel_set_powerOff0_1_level;
		active2 = panel_set_powerOff0_2_level;
		active3 = panel_set_powerOff0_3_level;

		if ( channel0 == 0xff ) {
			return -1;	/* No Function */
		}

		if ( active0 != 0 ) {
			value0 = 0;
		}
		else {
			value0 = 1;
		}

		if ( active1 != 0 ) {
			value1 = 0;
		}
		else {
			value1 = 1;
		}

		if ( active2 != 0 ) {
			value2 = 0;
		}
		else {
			value2 = 1;
		}

		if ( active3 != 0 ) {
			value3 = 0;
		}
		else {
			value3 = 1;
		}

		pinIndex0 = MK_GPIO_INDEX( channel0, func0, gid0, pin0 );
		pinIndex1 = MK_GPIO_INDEX( channel1, func1, gid1, pin1 );
		pinIndex2 = MK_GPIO_INDEX( channel2, func2, gid2, pin2 );
		pinIndex3 = MK_GPIO_INDEX( channel3, func3, gid3, pin3 );

		if ( panel_set_powerOff0_start_delay != 0 ) {
			msleep(panel_set_powerOff0_start_delay);
		}
		
		ctx.pin_index = pinIndex0;
		handle = gp_gpio_request(ctx.pin_index, NULL);
		gp_gpio_set_output( handle, value0, 0 );
		gp_gpio_release(handle);
		if ( panel_set_powerOff0_0to1_delay != 0 ) {
			msleep(panel_set_powerOff0_0to1_delay);
		}
		if ( channel1 == 0xff ) {
			return 0;	/* No Function */
		}
		
		ctx.pin_index = pinIndex1;
		handle = gp_gpio_request(ctx.pin_index, NULL);
		gp_gpio_set_output( handle, value1, 0 );
		gp_gpio_release(handle);
		if ( panel_set_powerOff0_0to1_delay != 0 ) {
			msleep(panel_set_powerOff0_1to2_delay);
		}
		if ( channel2 == 0xff ) {
			return 0;	/* No Function */
		}

		ctx.pin_index = pinIndex2;
		handle = gp_gpio_request(ctx.pin_index, NULL);
		gp_gpio_set_output( handle, value2, 0 );
		gp_gpio_release(handle);
		if ( panel_set_powerOff0_0to1_delay != 0 ) {
			msleep(panel_set_powerOff0_2to3_delay);

		}
		if ( channel3 == 0xff ) {
			return 0;	/* No Function */
		}
		
		ctx.pin_index = pinIndex3;
		handle = gp_gpio_request(ctx.pin_index, NULL);
		gp_gpio_set_output( handle, value3, 0 );
		gp_gpio_release(handle);
		if ( panel_set_powerOff0_0to1_delay != 0 ) {
			msleep(panel_set_powerOff0_end_delay);
		}
		return 0;
	}

	return 0;
}

int 
panel_set_power1(
	int enable
)
{
	UINT32	pinIndex0, pinIndex1, pinIndex2, pinIndex3;
	UINT32	value0, value1, value2, value3;
	unsigned int channel0, channel1, channel2, channel3;
	unsigned int func0, func1, func2, func3;
	unsigned int gid0, gid1, gid2, gid3;
	unsigned int pin0, pin1, pin2, pin3;
	unsigned int active0, active1, active2, active3;
	int handle;
	gpio_content_t ctx;

	if ( enable != 0 ) {
		channel0 = panel_set_powerOn1_0_channel;
		channel1 = panel_set_powerOn1_1_channel;
		channel2 = panel_set_powerOn1_2_channel;
		channel3 = panel_set_powerOn1_3_channel;
		func0 = panel_set_powerOn1_0_func;
		func1 = panel_set_powerOn1_1_func;
		func2 = panel_set_powerOn1_2_func;
		func3 = panel_set_powerOn1_3_func;
		gid0 = panel_set_powerOn1_0_gid;
		gid1 = panel_set_powerOn1_1_gid;
		gid2 = panel_set_powerOn1_2_gid;
		gid3 = panel_set_powerOn1_3_gid;
		pin0 = panel_set_powerOn1_0_pin;
		pin1 = panel_set_powerOn1_1_pin;
		pin2 = panel_set_powerOn1_2_pin;
		pin3 = panel_set_powerOn1_3_pin;
		active0 = panel_set_powerOn1_0_level;
		active1 = panel_set_powerOn1_1_level;
		active2 = panel_set_powerOn1_2_level;
		active3 = panel_set_powerOn1_3_level;

		if ( channel0 == 0xff ) {
			return -1;	/* No Function */
		}

		if ( active0 != 0 ) {
			value0 = 1;
		}
		else {
			value0 = 0;
		}

		if ( active1 != 0 ) {
			value1 = 1;
		}
		else {
			value1 = 0;
		}

		if ( active2 != 0 ) {
			value2 = 1;
		}
		else {
			value2 = 0;
		}

		if ( active3 != 0 ) {
			value3 = 1;
		}
		else {
			value3 = 0;
		}

		pinIndex0 = MK_GPIO_INDEX( channel0, func0, gid0, pin0 );
		pinIndex1 = MK_GPIO_INDEX( channel1, func1, gid1, pin1 );
		pinIndex2 = MK_GPIO_INDEX( channel2, func2, gid2, pin2 );
		pinIndex3 = MK_GPIO_INDEX( channel3, func3, gid3, pin3 );

		if ( panel_set_powerOn1_start_delay != 0 ) {
			msleep(panel_set_powerOn1_start_delay);
		}
		
		ctx.pin_index = pinIndex0;
		handle = gp_gpio_request(ctx.pin_index, NULL);
		gp_gpio_set_output( handle, value0, 0 );
		gp_gpio_release(handle);
		if ( panel_set_powerOn1_0to1_delay != 0 ) {
			msleep(panel_set_powerOn1_0to1_delay);
		}
		if ( channel1 == 0xff ) {
			return 0;	/* No Function */
		}
		
		ctx.pin_index = pinIndex1;
		handle = gp_gpio_request(ctx.pin_index, NULL);
		gp_gpio_set_output( handle, value1, 0 );
		gp_gpio_release(handle);
		if ( panel_set_powerOn1_1to2_delay != 0 ) {
			msleep(panel_set_powerOn1_1to2_delay);
		}
		if ( channel2 == 0xff ) {
			return 0;	/* No Function */
		}

		ctx.pin_index = pinIndex2;
		handle = gp_gpio_request(ctx.pin_index, NULL);
		gp_gpio_set_output( handle, value2, 0 );
		gp_gpio_release(handle);
		if ( panel_set_powerOn1_2to3_delay != 0 ) {
			msleep(panel_set_powerOn1_2to3_delay);
		}
		if ( channel3 == 0xff ) {
			return 0;	/* No Function */
		}
		
		ctx.pin_index = pinIndex3;
		handle = gp_gpio_request(ctx.pin_index, NULL);
		gp_gpio_set_output( handle, value3, 0 );
		gp_gpio_release(handle);
		if ( panel_set_powerOn1_end_delay != 0 ) {
			msleep(panel_set_powerOn1_end_delay);
		}
		return 0;
	}
	else {
		channel0 = panel_set_powerOff1_0_channel;
		channel1 = panel_set_powerOff1_1_channel;
		channel2 = panel_set_powerOff1_2_channel;
		channel3 = panel_set_powerOff1_3_channel;
		func0 = panel_set_powerOff1_0_func;
		func1 = panel_set_powerOff1_1_func;
		func2 = panel_set_powerOff1_2_func;
		func3 = panel_set_powerOff1_3_func;
		gid0 = panel_set_powerOff1_0_gid;
		gid1 = panel_set_powerOff1_1_gid;
		gid2 = panel_set_powerOff1_2_gid;
		gid3 = panel_set_powerOff1_3_gid;
		pin0 = panel_set_powerOff1_0_pin;
		pin1 = panel_set_powerOff1_1_pin;
		pin2 = panel_set_powerOff1_2_pin;
		pin3 = panel_set_powerOff1_3_pin;
		active0 = panel_set_powerOff1_0_level;
		active1 = panel_set_powerOff1_1_level;
		active2 = panel_set_powerOff1_2_level;
		active3 = panel_set_powerOff1_3_level;

		if ( channel0 == 0xff ) {
			return -1;	/* No Function */
		}

		if ( active0 != 0 ) {
			value0 = 0;
		}
		else {
			value0 = 1;
		}

		if ( active1 != 0 ) {
			value1 = 0;
		}
		else {
			value1 = 1;
		}

		if ( active2 != 0 ) {
			value2 = 0;
		}
		else {
			value2 = 1;
		}

		if ( active3 != 0 ) {
			value3 = 0;
		}
		else {
			value3 = 1;
		}

		pinIndex0 = MK_GPIO_INDEX( channel0, func0, gid0, pin0 );
		pinIndex1 = MK_GPIO_INDEX( channel1, func1, gid1, pin1 );
		pinIndex2 = MK_GPIO_INDEX( channel2, func2, gid2, pin2 );
		pinIndex3 = MK_GPIO_INDEX( channel3, func3, gid3, pin3 );

		if ( panel_set_powerOff1_start_delay != 0 ) {
			msleep(panel_set_powerOff1_start_delay);
		}
		
		ctx.pin_index = pinIndex0;
		handle = gp_gpio_request(ctx.pin_index, NULL);
		gp_gpio_set_output( handle, value0, 0 );
		gp_gpio_release(handle);
		if ( panel_set_powerOff1_0to1_delay != 0 ) {
			msleep(panel_set_powerOff1_0to1_delay);
		}
		if ( channel1 == 0xff ) {
			return 0;	/* No Function */
		}
		
		ctx.pin_index = pinIndex1;
		handle = gp_gpio_request(ctx.pin_index, NULL);
		gp_gpio_set_output( handle, value1, 0 );
		gp_gpio_release(handle);
		if ( panel_set_powerOff1_0to1_delay != 0 ) {
			msleep(panel_set_powerOff1_1to2_delay);
		}
		if ( channel2 == 0xff ) {
			return 0;	/* No Function */
		}

		ctx.pin_index = pinIndex2;
		handle = gp_gpio_request(ctx.pin_index, NULL);
		gp_gpio_set_output( handle, value2, 0 );
		gp_gpio_release(handle);
		if ( panel_set_powerOff1_0to1_delay != 0 ) {
			msleep(panel_set_powerOff1_2to3_delay);
		}
		if ( channel3 == 0xff ) {
			return 0;	/* No Function */
		}
		
		ctx.pin_index = pinIndex3;
		handle = gp_gpio_request(ctx.pin_index, NULL);
		gp_gpio_set_output( handle, value3, 0 );
		gp_gpio_release(handle);
		if ( panel_set_powerOff1_0to1_delay != 0 ) {
			msleep(panel_set_powerOff1_end_delay);
		}
		return 0;
	}

	return 0;
}

int 
panel_spi_cs(
	UINT32 val
)
{	
	unsigned int channel = panel_set_spi_cs_channel;
	int handle;
	gpio_content_t ctx;

	if ( channel == 0xff ) {
		return -1;	/* No Function */
	}

	ctx.pin_index = MK_GPIO_INDEX( panel_set_spi_cs_channel, panel_set_spi_cs_func, panel_set_spi_cs_gid, panel_set_spi_cs_pin );
	handle = gp_gpio_request(ctx.pin_index, NULL);
	gp_gpio_set_output( handle, val, 0 );
	gp_gpio_release(handle);

	return 0;
}

int 
panel_spi_scl(
	UINT32 val
)
{	
	unsigned int channel = panel_set_spi_scl_channel;
	int handle;
	gpio_content_t ctx;

	if ( channel == 0xff ) {
		return -1;	/* No Function */
	}

	ctx.pin_index = MK_GPIO_INDEX( panel_set_spi_scl_channel, panel_set_spi_scl_func, panel_set_spi_scl_gid, panel_set_spi_scl_pin );
	handle = gp_gpio_request(ctx.pin_index, NULL);
	gp_gpio_set_output( handle, val, 0 );
	gp_gpio_release(handle);

	return 0;
}

int 
panel_spi_sda(
	UINT32 val
)
{	
	unsigned int channel = panel_set_spi_sda_channel;
	int handle;
	gpio_content_t ctx;

	if ( channel == 0xff ) {
		return -1;	/* No Function */
	}

	ctx.pin_index = MK_GPIO_INDEX( panel_set_spi_sda_channel, panel_set_spi_sda_func, panel_set_spi_sda_gid, panel_set_spi_sda_pin );
	handle = gp_gpio_request(ctx.pin_index, NULL);
	gp_gpio_set_output( handle, val, 0 );
	gp_gpio_release(handle);

	return 0;
}

int 
panel_set_mirror(
	int enable
)
{
	UINT32	pinIndex0, pinIndex1, value0 = 0, value1 = 0;
	unsigned int channel0 = panel_set_mirror0_channel, channel1 = panel_set_mirror1_channel;
	int handle;
	gpio_content_t ctx;

	if ( ( channel0 == 0xff ) || ( channel1 == 0xff )  ) {
		return -1;	/* No Function */
	}

	if ( enable ) {	/* setting active polar */
		value0 = panel_set_mirror_mirror0;
		value1 = panel_set_mirror_mirror1;
	}
	else {
		value0 = panel_set_mirror_normal0;
		value1 = panel_set_mirror_normal1;
	}

	pinIndex0 = MK_GPIO_INDEX( panel_set_mirror0_channel, panel_set_mirror0_func, panel_set_mirror0_gid, panel_set_mirror0_pin );
	pinIndex1 = MK_GPIO_INDEX( panel_set_mirror1_channel, panel_set_mirror1_func, panel_set_mirror1_gid, panel_set_mirror1_pin );

	ctx.pin_index = pinIndex0;
	handle = gp_gpio_request(ctx.pin_index, NULL);
	gp_gpio_set_output( handle, value0, 0 );
	gp_gpio_release(handle);

	ctx.pin_index = pinIndex1;
	handle = gp_gpio_request(ctx.pin_index, NULL);	
	gp_gpio_set_output( handle, value1, 0 );
	gp_gpio_release(handle);

	return 0;
}

static const gp_board_panel_t config_panel = {
	.set_backlight = panel_set_backlight,
	.set_brightness = panel_brightness_backlight,
	.set_panelpowerOn0 = panel_set_power0,
	.set_panelpowerOn1 = panel_set_power1,
	.set_panel_spi_cs = panel_spi_cs,
	.set_panel_spi_scl = panel_spi_scl,
	.set_panel_spi_sda = panel_spi_sda,
	.set_panel_mirror = panel_set_mirror,
};

/**************************************************************************
 * SD
 **************************************************************************/
int 
sd0_set_power(
	int enable
)
{
	UINT32	value = 0, active = sd0_set_power_level;
	unsigned int channel = sd0_set_power_channel;
	int handle;
	gpio_content_t ctx;

	if ( channel == 0xff ) {
		return -1;	/* No Function */
	}

	if ( enable ) {	/* setting active polar */
		if ( active != 0 ) {
			value = 1;
		}
		else {
			value = 0;
		}
	}
	else {
		if ( active != 0 ) {
			value = 0;
		}
		else {
			value = 1;
		}
	}

	ctx.pin_index = MK_GPIO_INDEX( sd0_set_power_channel, sd0_set_power_func, sd0_set_power_gid, sd0_set_power_pin );
	handle = gp_gpio_request(ctx.pin_index, NULL);
	gp_gpio_set_output( handle, value, 0 );
	gp_gpio_release(handle);
	return 0;
}

int
sd0_detect(
	void
)
{
	UINT32	value = 0, active = sd0_detect_level;
	unsigned int channel = sd0_detect_channel;

	if ( channel == 0xff ) {
		return -1;	/* No Function */
	}

	gp_gpio_get_value(gp_board_info->handle_sd0_detect, &value);

	if ( value ) {	/* check active polar */
		if ( active != 0 ) {
			return 1;
		}
		else {
			return 0;
		}
	}
	else {
		if ( active != 0 ) {
			return 0;
		}
		else {
			return 1;
		}
	}

	return 0;
}

int
sd0_is_write_protected(
	void
)
{
	UINT32	value = 0, active = sd0_is_write_protected_level;
	unsigned int channel = sd0_is_write_protected_channel;
	int handle;
	gpio_content_t ctx;

	if ( channel == 0xff ) {
		return -1;	/* No Function */
	}

	ctx.pin_index = MK_GPIO_INDEX( sd0_is_write_protected_channel, sd0_is_write_protected_func, sd0_is_write_protected_gid, sd0_is_write_protected_pin );
	handle = gp_gpio_request(ctx.pin_index, NULL);
	gp_gpio_get_value(handle, &value);
	gp_gpio_release(handle);

	if ( value ) {	/* check active polar */
		if ( active != 0 ) {
			return 1;
		}
		else {
			return 0;
		}
	}
	else {
		if ( active != 0 ) {
			return 0;
		}
		else {
			return 1;
		}
	}

	return 0;
}

int 
sdio0_set_standby(
	int enable
)
{
	UINT32	value = 0, active = sdio0_set_standby_level;
	unsigned int channel = sdio0_set_standby_channel;
	int handle;
	gpio_content_t ctx;

	if ( channel == 0xff ) {
		return -1;	/* No Function */
	}

	if ( enable ) {	/* setting active polar */
		if ( active != 0 ) {
			value = 1;
		}
		else {
			value = 0;
		}
	}
	else {
		if ( active != 0 ) {
			value = 0;
		}
		else {
			value = 1;
		}
	}

	ctx.pin_index = MK_GPIO_INDEX( sdio0_set_standby_channel, sdio0_set_standby_func, sdio0_set_standby_gid, sdio0_set_standby_pin );
	handle = gp_gpio_request(ctx.pin_index, NULL);
	gp_gpio_set_output( handle, value, 0 );
	gp_gpio_release(handle);

	return 0;
}

int 
sd0_function_get(
	void
)
{
/* 0: SD, 1: SDIO, 0xff: no used */

	return 0xff;
}

int 
sd0_io_set(
	int	mode,
	int	out_value
)
{
/* 0: output, 1: input, 2: sd/sdio */	
	return 0;
}


int 
sd1_set_power(
	int enable
)
{
	UINT32	value = 0, active = sd1_set_power_level;
	unsigned int channel = sd1_set_power_channel;
	int handle;
	gpio_content_t ctx;

	if ( channel == 0xff ) {
		return -1;	/* No Function */
	}

	if ( enable ) {	/* setting active polar */
		if ( active != 0 ) {
			value = 1;
		}
		else {
			value = 0;
		}
	}
	else {
		if ( active != 0 ) {
			value = 0;
		}
		else {
			value = 1;
		}
	}

	ctx.pin_index = MK_GPIO_INDEX( sd1_set_power_channel, sd1_set_power_func, sd1_set_power_gid, sd1_set_power_pin );
	handle = gp_gpio_request(ctx.pin_index, NULL);
	gp_gpio_set_output( handle, value, 0 );
	gp_gpio_release(handle);

	return 0;
}

int
sd1_detect(
	void
)
{
	UINT32	value = 0, active = sd1_detect_level;
	unsigned int channel = sd1_detect_channel;

	if ( channel == 0xff ) {
		return -1;	/* No Function */
	}

	gp_gpio_get_value(gp_board_info->handle_sd1_detect, &value);

	if ( value ) {	/* check active polar */
		if ( active != 0 ) {
			return 1;
		}
		else {
			return 0;
		}
	}
	else {
		if ( active != 0 ) {
			return 0;
		}
		else {
			return 1;
		}
	}

	return 0;
}

int
sd1_is_write_protected(
	void
)
{
	UINT32	value = 0, active = sd1_is_write_protected_level;
	unsigned int channel = sd1_is_write_protected_channel;
	int handle;
	gpio_content_t ctx;

	if ( channel == 0xff ) {
		return -1;	/* No Function */
	}

	ctx.pin_index = MK_GPIO_INDEX( sd1_is_write_protected_func, sd1_is_write_protected_func, sd1_is_write_protected_gid, sd1_is_write_protected_pin );
	handle = gp_gpio_request(ctx.pin_index, NULL);
	gp_gpio_get_value(handle, &value);
	gp_gpio_release(handle);

	if ( value ) {	/* check active polar */
		if ( active != 0 ) {
			return 1;
		}
		else {
			return 0;
		}
	}
	else {
		if ( active != 0 ) {
			return 0;
		}
		else {
			return 1;
		}
	}

	return 0;
}

int 
sdio1_set_standby(
	int enable
)
{
	UINT32	value = 0, active = sdio1_set_standby_level;
	unsigned int channel = sdio1_set_standby_channel;
	int handle;
	gpio_content_t ctx;

	if ( channel == 0xff ) {
		return -1;	/* No Function */
	}

	if ( enable ) {	/* setting active polar */
		if ( active != 0 ) {
			value = 1;
		}
		else {
			value = 0;
		}
	}
	else {
		if ( active != 0 ) {
			value = 0;
		}
		else {
			value = 1;
		}
	}

	ctx.pin_index = MK_GPIO_INDEX( sdio1_set_standby_channel, sdio1_set_standby_func, sdio1_set_standby_gid, sdio1_set_standby_pin );
	handle = gp_gpio_request(ctx.pin_index, NULL);
	gp_gpio_set_output( handle, value, 0 );
	gp_gpio_release(handle);

	return 0;
}

int 
sd1_function_get(
	void
)
{
/* 0: SD, 1: SDIO, 0xff: no used */

	return 0xff;
}

int 
sd1_io_set(
	int	mode,
	int	out_value
)
{
/* 0: output, 1: input, 2: sd/sdio */	
	return 0;
}

int 
sd2_set_power(
	int enable
)
{
	UINT32	value = 0, active = sd2_set_power_level;
	unsigned int channel = sd2_set_power_channel;
	int handle;
	gpio_content_t ctx;

	if ( channel == 0xff ) {
		return -1;	/* No Function */
	}

	if ( enable ) {	/* setting active polar */
		if ( active != 0 ) {
			value = 1;
		}
		else {
			value = 0;
		}
	}
	else {
		if ( active != 0 ) {
			value = 0;
		}
		else {
			value = 1;
		}
	}

	ctx.pin_index = MK_GPIO_INDEX( sd2_set_power_channel, sd2_set_power_func, sd2_set_power_gid, sd2_set_power_pin );
	handle = gp_gpio_request(ctx.pin_index, NULL);
	gp_gpio_set_output( handle, value, 0 );
	gp_gpio_release(handle);
	return 0;
}

int
sd2_detect(
	void
)
{
	UINT32	value = 0, active = sd2_detect_level;
	unsigned int channel = sd2_detect_channel;

	//return 1;
	if ( channel == 0xff ) {
		return -1;	/* No Function */
	}

	gp_gpio_get_value(gp_board_info->handle_sd2_detect, &value);

	if ( value ) {	/* check active polar */
		if ( active != 0 ) {
			return 1;
		}
		else {
			return 0;
		}
	}
	else {
		if ( active != 0 ) {
			return 0;
		}
		else {
			return 1;
		}
	}

	return 0;
}

int
sd2_is_write_protected(
	void
)
{
	UINT32	value = 0, active = sd2_is_write_protected_level;
	unsigned int channel = sd2_is_write_protected_channel;
	int handle;
	gpio_content_t ctx;

	if ( channel == 0xff ) {
		return -1;	/* No Function */
	}

	ctx.pin_index = MK_GPIO_INDEX( sd2_is_write_protected_channel, sd2_is_write_protected_func, sd2_is_write_protected_gid, sd2_is_write_protected_pin );
	handle = gp_gpio_request(ctx.pin_index, NULL);
	gp_gpio_get_value(handle, &value);
	gp_gpio_release(handle);

	if ( value ) {	/* check active polar */
		if ( active != 0 ) {
			return 1;
		}
		else {
			return 0;
		}
	}
	else {
		if ( active != 0 ) {
			return 0;
		}
		else {
			return 1;
		}
	}

	return 0;
}

int 
sdio2_set_standby(
	int enable
)
{
	UINT32	value = 0, active = sdio2_set_standby_level;
	unsigned int channel = sdio2_set_standby_channel;
	int handle;
	gpio_content_t ctx;

	if ( channel == 0xff ) {
		return -1;	/* No Function */
	}

	if ( enable ) {	/* setting active polar */
		if ( active != 0 ) {
			value = 1;
		}
		else {
			value = 0;
		}
	}
	else {
		if ( active != 0 ) {
			value = 0;
		}
		else {
			value = 1;
		}
	}

	ctx.pin_index = MK_GPIO_INDEX( sdio2_set_standby_channel, sdio2_set_standby_func, sdio2_set_standby_gid, sdio2_set_standby_pin );
	handle = gp_gpio_request(ctx.pin_index, NULL);
	gp_gpio_set_output( handle, value, 0 );
	gp_gpio_release(handle);

	return 0;
}

int 
sd2_function_get(
	void
)
{
/* 0: SD, 1: SDIO, 0xff: no used */

	return 0xff;
}

int 
sd2_io_set(
	int	mode,
	int	out_value
)
{
/* 0: output, 1: input, 2: sd/sdio */	
	return 0;
}

static const gp_board_sd_t config_sd0 = {
	.set_power = sd0_set_power,
	.detect = sd0_detect,
	.is_write_protected = sd0_is_write_protected,
	.set_standby = sdio0_set_standby,
	.get_func = sd0_function_get,
	.set_io = sd0_io_set,
};

static const gp_board_sd_t config_sd1 = {
	.set_power = sd1_set_power,
	.detect = sd1_detect,
	.is_write_protected = sd1_is_write_protected,
	.set_standby = sdio1_set_standby,
	.get_func = sd1_function_get,
	.set_io = sd1_io_set,
};

static const gp_board_sd_t config_sd2 = {
	.set_power = sd2_set_power,
	.detect = sd2_detect,
	.is_write_protected = sd2_is_write_protected,
	.set_standby = sdio2_set_standby,
	.get_func = sd2_function_get,
	.set_io = sd2_io_set,
};

/**************************************************************************
 * NAND - CSn & WP
 **************************************************************************/
int 
nand_set_cs(
	int csn,
	int status
)
{
	UINT32	value = 0;
	unsigned int channel = nand_set_cs0_channel;
	unsigned int func = nand_set_cs0_func;
	unsigned int gid = nand_set_cs0_gid;
	unsigned int pin = nand_set_cs0_pin;
	unsigned int active = nand_set_cs0_active;
	int handle;
	gpio_content_t ctx;

	if ( csn > 4 ) {
		return -1;	/* out of define */
	}

	switch( csn ) {
		case 0:
			channel = nand_set_cs0_channel;
			func = nand_set_cs0_func;
			gid = nand_set_cs0_gid;
			pin = nand_set_cs0_pin;
			active = nand_set_cs0_active;
			break;
		case 1:
			channel = nand_set_cs1_channel;
			func = nand_set_cs1_func;
			gid = nand_set_cs1_gid;
			pin = nand_set_cs1_pin;
			active = nand_set_cs1_active;
			break;
		case 2:
			channel = nand_set_cs2_channel;
			func = nand_set_cs2_func;
			gid = nand_set_cs2_gid;
			pin = nand_set_cs2_pin;
			active = nand_set_cs2_active;
			break;
		case 3:
			channel = nand_set_cs3_channel;
			func = nand_set_cs3_func;
			gid = nand_set_cs3_gid;
			pin = nand_set_cs3_pin;
			active = nand_set_cs3_active;
			break;
	}

	if ( channel == 0xff ) {
		return -1;	/* No Function */
	}

	if ( status ) {	/* setting active polar */
		if ( active != 0 ) {
			value = 1;
		}
		else {
			value = 0;
		}
	}
	else {
		if ( active != 0 ) {
			value = 0;
		}
		else {
			value = 1;
		}
	}

	ctx.pin_index = MK_GPIO_INDEX( channel, func, gid, pin );
	handle = gp_gpio_request(ctx.pin_index, NULL);
	gp_gpio_set_output( handle, value, 0 );
	gp_gpio_release(handle);

	return 0;
}

int
nand_set_wp(
	int enable
)
{
	UINT32	value = 0, active = nand_set_wp_active;
	unsigned int channel = nand_set_wp_channel;
	int handle;
	gpio_content_t ctx;

	if ( channel == 0xff ) {
		return -1;	/* No Function */
	}

	if ( enable ) {	/* setting active polar */
		if ( active != 0 ) {
			value = 1;
		}
		else {
			value = 0;
		}
	}
	else {
		if ( active != 0 ) {
			value = 0;
		}
		else {
			value = 1;
		}
	}

	ctx.pin_index = MK_GPIO_INDEX( nand_set_wp_channel, nand_set_wp_func, nand_set_wp_gid, nand_set_wp_pin );
	handle = gp_gpio_request(ctx.pin_index, NULL);
	gp_gpio_set_output( handle, value, 0 );
	gp_gpio_release(handle);

	return 0;
}

static const gp_board_nand_t config_nand = {
	.set_cs = nand_set_cs,
	.set_wp = nand_set_wp,
};

/**************************************************************************
 * USB HOST / SLAVE
 **************************************************************************/ 
/**
* @brief Get enable configuration of PHY 0
* @return 1 or 0
*/
 int usb_phy0_func_en_get (void) {
	// printk("[%s][%d]\n", __FUNCTION__, __LINE__);
    //return SYSCONFIG_USB_PHY0_EN;
}

/**
* @brief Get enable configuration of PHY 1
* @return 0 HOST, 1 for SLAVE, 2 for HOST/SLAVE, 3 for None. 
*/
int usb_phy1_func_sel_get (void) {
	//printk("[%s][%d]\n", __FUNCTION__, __LINE__);
	return SYSCONFIG_USB_PHY1_SEL;
}

/** 
     * @brief Get speed configuration of HOST
     * @param none
     * @return 1 (High Speed 2.0) or 0 (Full Speed 1.1)
*/
int usb_get_host_speed(void) {
	//printk("[%s][%d]\n", __FUNCTION__, __LINE__);
	#ifdef SYSCONFIG_USB_HOST_HIGHSPEED_MODE
		return 1;
	#else
		return 0;
	#endif
}
 
/**
* @brief Enable power of VBUS of USB HOST with GPIO
* @param [In]enable: 1 or 0
     * @return SUCCESS or FAIL
     */
int 
usb_host_en_power(
	int enable
)
{
	UINT32	value = 0, active = usb_host_en_power_active;
	unsigned int channel = usb_host_en_power_channel;
	int handle;
	gpio_content_t ctx;

	if ( channel == 0xff ) {
		return -1;	/* No Function */
	}

	if ( enable ) {	/* setting active polar */
		if ( active != 0 ) {
			value = 1;
		}
		else {
			value = 0;
		}
	}
	else {
		if ( active != 0 ) {
			value = 0;
		}
		else {
			value = 1;
		}
	}

	ctx.pin_index = MK_GPIO_INDEX( usb_host_en_power_channel, usb_host_en_power_func, usb_host_en_power_gid, usb_host_en_power_pin );
	handle = gp_gpio_request(ctx.pin_index, NULL);
	gp_gpio_set_output( handle, value, 0 );
	gp_gpio_release(handle);

	return 0;
}

/**
     * @brief Get USB SLAVE VBUS detect status from POWON1/GPIO
     * @param none
     * @return 1 (CONNECT) or 0 (DISCONNECT)
     */
int 
usb_slave_detect_power(
	void
)
{
	int configIdx = 0;
	unsigned int value = 0;
	//printk("[%s][%d]\n", __FUNCTION__, __LINE__);
	if (usb_slave_detect_config == USB_SLAVE_VBUS_POWERON1) {
		/*From POWERON1*/
		value = gpHalUsbVbusDetect();
	}
	else if( usb_slave_detect_config == USB_SLAVE_VBUS_GPIO ) {
		configIdx = MK_GPIO_INDEX( usb_slave_detect_power_channel, 
															  usb_slave_detect_power_func, 
															  usb_slave_detect_power_gid, 
															  usb_slave_detect_power_pin );
		gpHalGpioGetValue(configIdx, &value);
	}
	else{
		value = 0;
	}

	return value;
}

/**
     * @brief Switch USB HOST and SLAVE for PHY1
     * @param [In]mode: 1 or 0
     * @return SUCCESS or FAIL
     */
int 
usb_switch_bus(
	int mode
)
{
	UINT32	value = 0, active = usb_switch_bus_active;
	unsigned int channel = usb_switch_bus_channel;
	int handle;
	gpio_content_t ctx;

	if ( channel == 0xff ) {
		return -1;	/* No Function */
	}

	if ( mode ) {	/* setting active polar */
		if ( active != 0 ) {
			value = 1;
		}
		else {
			value = 0;
		}
	}
	else {
		if ( active != 0 ) {
			value = 0;
		}
		else {
			value = 1;
		}
	}

	ctx.pin_index = MK_GPIO_INDEX( usb_switch_bus_channel, usb_switch_bus_func, usb_switch_bus_gid, usb_switch_bus_pin );
	handle = gp_gpio_request(ctx.pin_index, NULL);
	gp_gpio_set_output( handle, value, 0 );
	gp_gpio_release(handle);

	return 0;
}


/**
     * @brief Get VBUS detect config 
     * @param none
     * @return -1 (POWERON1) or configuration (GPIO) or 
     *  			0xff(none)
*/

int usb_slave_get_vbus_config(void) {
	int config = usb_slave_detect_config;

	if ( usb_slave_detect_config == USB_SLAVE_VBUS_POWERON1 ) {
		printk("U->POWERON1\n");
	}
	else if( usb_slave_detect_config == USB_SLAVE_VBUS_GPIO ) {
		printk("U->GPIO\n");
		config = MK_GPIO_INDEX( usb_slave_detect_power_channel, 
															  usb_slave_detect_power_func, 
															  usb_slave_detect_power_gid, 
															  usb_slave_detect_power_pin );
		return config;
	}
	else if( usb_slave_detect_config == USB_SLAVE_VBUS_NONE) {
		printk("U->NONE\n");
	}
	return usb_slave_detect_config;
}

/**
     * @brief Get GPIO power level
     * @param none
     * @return GPIO_PULL_LOW/GPIO_PULL_HIGH/GPIO_PULL_FLOATING
     *			
*/

int usb_slave_get_gpio_power_level(void) {
	
	return usb_slave_detect_power_level;
}

/**
     * @brief Get Hub config(By platform.cfg)
     * @param none
     * @return 0~5
*/

int usb_host_get_hub_config(void) {
	#ifdef SYSCONFIG_USB_HOST_HUB_CONFIG
	return SYSCONFIG_USB_HOST_HUB_CONFIG;
	#else
	return 5;
	#endif
}

/**
     * @brief Get usb PHY0 voltage up configuration(By platform.cfg)
     * @param none
     * @return 0~5
*/

int usb_get_phy0_voltage_up_config(void) {
	#ifdef SYSCONFIG_USB_PHY0_VOLTAGE_UP
	return SYSCONFIG_USB_PHY0_VOLTAGE_UP;
	#else
	return 0;
	#endif
}

/**
     * @brief Get usb PHY1 voltage up configuration(By platform.cfg)
     * @param none
     * @return 0~5
*/

int usb_get_phy1_voltage_up_config(void) {
	#ifdef SYSCONFIG_USB_PHY1_VOLTAGE_UP
	return SYSCONFIG_USB_PHY1_VOLTAGE_UP;
	#else
	return 0;
	#endif
}

/**
	* @brief Get VBUS Power switch status
	* @param none
	* @return 1 (power good) or 0 (power no good) or -1 (No used)
*/
int usb_host_powergood(void) {
	UINT32	value = 0, active = usb_power_good_level;
	unsigned int channel = usb_power_good_channel;
	int handle;
	gpio_content_t ctx;

	if ( channel == 0xff ) {
		return -1;	/* No Function */
	}

	ctx.pin_index = MK_GPIO_INDEX( usb_power_good_channel, usb_power_good_func, usb_power_good_gid, usb_power_good_pin );
	handle = gp_gpio_request(ctx.pin_index, NULL);
	gp_gpio_get_value(handle, &value);
	gp_gpio_release(handle);

	if ( value ) {	/* check active polar */
		if ( active != 0 ) {
			return 1;
		}
		else {
			return 0;
		}
	}
	else {
		if ( active != 0 ) {
			return 0;
		}
		else {
			return 1;
		}
	}

	return 0;
}

#if 0
/**
     * @brief Get exit from safe remove config.
     * @param none
     * @return 1 USB driver breaks out after detecting save 
     *  	   remove singal. 
*/
int usb_exit_from_safe_remove_config(void) {
	return 1;
}
#endif

#if 0
/**
     * @brief Get usb serial number by main storage app header.
     * @param none
     * @return serial number. 
*/
static unsigned int usb_app_serial_get(void) {

	printk("main: %s\n",SYSCONFIG_MAINSTORAGE);
	if (!strcmp(SYSCONFIG_MAINSTORAGE, "gp_nand_module") && config_nand.get_sn)
		return config_nand.get_sn();
	else if (!strcmp(SYSCONFIG_MAINSTORAGE, "gp_sd") && config_sd0.get_sn)
		return config_sd0.get_sn();
	else
		return 0;

}
#endif

#if 0
/**
	* @brief Set USB device information
	* @param none
	* @return 0;
*/
/*===========User Configuration===========*/
#define USB_VID 0x1B3F
#define USB_PID 0x6000
#define USB_SERIAL_NUMBER 2 /*2: APP header, 1: Ramdom, 0: Fixed*/
/*Max 64 charaters.*/
#define USB_VENDOR_NAME "GENERALPLUS"
/*Max 16 charaters.*/
#define USB_PRODUCT_NAME "Usb Storage"
/*Max 8 charaters. It's the device information in PC.*/
#define USB_MSDC_MANUFACTURER_NAME_0 "GENERAL"
/*Max 16 charaters.*/
#define USB_MSDC_PRODUCT_NAME_0 "PLUS Usb Disk-0 "
/*Max 4 charaters.*/
#define USB_MSDC_REVISION_NAME_0 "1.00"
/*Max 8 charaters. It's the device information for lun = 1 in PC.*/
#define USB_MSDC_MANUFACTURER_NAME_1 "GENERAL"
/*Max 16 charaters.*/
#define USB_MSDC_PRODUCT_NAME_1 "PLUS Usb Disk-1 "
/*Max 4 charaters.*/
#define USB_MSDC_REVISION_NAME_1 "2.00"
/*===========User Configuration===========End*/

/*===========DO NOT MODIFY BELOW SETTING===========*/
#define USB_VID_PID_SIZE 4
#define DESC_STR_VENDOR_MAX_SIZE   64
#define DESC_STR_PRODUCT_MAX_SIZE  16
#define USBSCSI_INQ_VENDOR_TXT_LEN   8
#define USBSCSI_INQ_PRODUCT_TXT_LEN  16
#define USBSCSI_INQ_REVISION_TXT_LEN 4

int usb_slave_infor_get( int index, int lun, char* buffer ) {
	int length = 0;
	unsigned short value = 0;
	char *s_ptr = 0;
	if( index!= USB_INDEX_SERIAL && buffer == NULL ) {
		printk("[%s][%d] PTR NULL?\n", __FUNCTION__, __LINE__);
		return -1;
	}
	switch( index ) {
		case USB_INDEX_VID:
		value = USB_VID;
		memcpy(buffer, (char*)&value, USB_VID_PID_SIZE);
		break;
		case USB_INDEX_PID:
		value = USB_PID;
		memcpy(buffer, (char*)&value, USB_VID_PID_SIZE);
		break;
		case USB_INDEX_SERIAL:
		return USB_SERIAL_NUMBER;
		break;
		case USB_INDEX_VENDOR_NAME:
		length = (strlen(USB_VENDOR_NAME) > DESC_STR_VENDOR_MAX_SIZE)? DESC_STR_VENDOR_MAX_SIZE: strlen(USB_VENDOR_NAME);
		strncpy( buffer, USB_VENDOR_NAME, length);	
		break;
		case USB_INDEX_PRODUCT_NAME:
		length = (strlen(USB_PRODUCT_NAME) > DESC_STR_PRODUCT_MAX_SIZE)? DESC_STR_PRODUCT_MAX_SIZE: strlen(USB_PRODUCT_NAME);
		strncpy( buffer, USB_PRODUCT_NAME, length);		
		break;
		case USB_INDEX_MSDC_MANUFACTURER_NAME:
		switch( lun ){
			case 0:	
				s_ptr = USB_MSDC_MANUFACTURER_NAME_0;
				break;
			case 1:
				s_ptr = USB_MSDC_MANUFACTURER_NAME_1;
				break;
		}
		length = (strlen(s_ptr) > USBSCSI_INQ_VENDOR_TXT_LEN)? USBSCSI_INQ_VENDOR_TXT_LEN: strlen(s_ptr);
		strncpy( buffer, s_ptr, length);	
		//strncpy( strName, "USB DISK", strlen("USB DISK"));	
		break;
		case USB_INDEX_MSDC_PRODUCT_NAME:
		switch( lun ){
			case 0:	
				s_ptr = USB_MSDC_PRODUCT_NAME_0;
				break;
			case 1:
				s_ptr = USB_MSDC_PRODUCT_NAME_1;
				break;
		}
		length = (strlen(s_ptr) > USBSCSI_INQ_PRODUCT_TXT_LEN)? USBSCSI_INQ_PRODUCT_TXT_LEN: strlen(s_ptr);
		strncpy( buffer, s_ptr, length);
		//memset(strName, 0, 16);
		//strncpy(strName, "MASS STORAGE", strlen("MASS STORAGE"));		
		break;
		case USB_INDEX_MSDC_REVISION_ID:
		switch( lun ){
			case 0:	
				s_ptr = USB_MSDC_REVISION_NAME_0;
				break;
			case 1:
				s_ptr = USB_MSDC_REVISION_NAME_1;
				break;
		}
		length = (strlen(s_ptr) > USBSCSI_INQ_REVISION_TXT_LEN)? USBSCSI_INQ_REVISION_TXT_LEN: strlen(s_ptr);
		strncpy( buffer, s_ptr, length);	
		//memset(strName, 0, 4);
		break;
		default:
		return -1;
	}
	return 0;
}
/*===========DO NOT MODIFY BELOW SETTING===========END*/
#endif

static const gp_board_usb_t config_usb = {
	.phy0_func_en_get = usb_phy0_func_en_get,
	.phy1_func_sel_get = usb_phy1_func_sel_get,
	.get_host_speed = usb_get_host_speed,
	.set_power = usb_host_en_power,
	.slave_detect = usb_slave_detect_power,
	.otg_switch = usb_switch_bus,
	.get_host_pg = usb_host_powergood,
	.get_slave_vbus_config = usb_slave_get_vbus_config,
	.get_host_hub_config = usb_host_get_hub_config,
	.get_phy0_voltage_up_config = usb_get_phy0_voltage_up_config,
	.get_phy1_voltage_up_config = usb_get_phy1_voltage_up_config,
	.get_slave_gpio_power_level = usb_slave_get_gpio_power_level,
	//.get_exit_safe_remove_config = usb_exit_from_safe_remove_config,
	//.get_usb_slave_infor = usb_slave_infor_get,
	//.get_usb_app_serial = usb_app_serial_get,
};

/**************************************************************************
 * Audio out
 **************************************************************************/
int 
headphone_detect(
	void
)
{
	UINT32	value = 0, active = headphone_detect_level;
	unsigned int channel = headphone_detect_channel;
	int handle;
	gpio_content_t ctx;

	if ( channel == 0xff ) {
		return -1;	/* No Function */
	}

	ctx.pin_index = MK_GPIO_INDEX( headphone_detect_channel, headphone_detect_func, headphone_detect_gid, headphone_detect_pin );
	handle = gp_gpio_request(ctx.pin_index, NULL);
	gp_gpio_get_value(handle, &value);
	gp_gpio_release(handle);

	if ( value ) {	/* check active polar */
		if ( active != 0 ) {
			return 1;
		}
		else {
			return 0;
		}
	}
	else {
		if ( active != 0 ) {
			return 0;
		}
		else {
			return 1;
		}
	}

	return 0;
}

int 
speaker_set_power(
	int enable
)
{
	UINT32	value = 0, active = speaker_set_power_active;
	unsigned int channel = speaker_set_power_channel;
	int handle;
	gpio_content_t ctx;

	if ( channel == 0xff ) {
		return -1;	/* No Function */
	}

	if ( enable ) {	/* setting active polar */
		if ( active != 0 ) {
			value = 1;
		}
		else {
			value = 0;
		}
	}
	else {
		if ( active != 0 ) {
			value = 0;
		}
		else {
			value = 1;
		}
	}

	ctx.pin_index = MK_GPIO_INDEX( speaker_set_power_channel, speaker_set_power_func, speaker_set_power_gid, speaker_set_power_pin );
	handle = gp_gpio_request(ctx.pin_index, NULL);
	gp_gpio_set_output( handle, value, 0 );
	gp_gpio_release(handle);

	return 0;
}

static const gp_board_audio_t config_audio_out = {
	.get_headphone_detect = headphone_detect,
	.set_speaker_power = speaker_set_power,
};

/**************************************************************************
 * PS2_UART_mouse
 **************************************************************************/
unsigned int board_config_get_ps2_uart_mouse_clk_gpio(void)
{
	return ((ps2mouse_set_clk_channel<<24)|(ps2mouse_set_clk_func<<16)|(ps2mouse_set_clk_gid<<8)|ps2mouse_set_clk_pin);
} 

unsigned int board_config_get_ps2_uart_mouse_data_gpio(void)
{
	return ((ps2mouse_set_dat_channel<<24)|(ps2mouse_set_dat_func<<16)|(ps2mouse_set_dat_gid<<8)|ps2mouse_set_dat_pin);
} 

static const gp_board_ps2_uart_mouse_t		config_ps2_uart_mouse = {
	.get_clk_gpio = board_config_get_ps2_uart_mouse_clk_gpio,
	.get_data_gpio = board_config_get_ps2_uart_mouse_data_gpio,
}; 

/**************************************************************************
 * Sensor
 **************************************************************************/
int
sensor_set_port(
	char *port
)
{
	if( strncmp(port, "PORT0", strlen(port)) == 0 ) {
		printk("set port PORT0\n");
		gpiocfgInput( sensor_port0_mclk_channel, sensor_port0_mclk_func, sensor_port0_mclk_gid, sensor_port0_mclk_pin );
		gpiocfgInput( sensor_port0_pclk_channel, sensor_port0_pclk_func, sensor_port0_pclk_gid, sensor_port0_pclk_pin );
		gpiocfgInput( sensor_port0_vsync_channel, sensor_port0_vsync_func, sensor_port0_vsync_gid, sensor_port0_vsync_pin );
		gpiocfgInput( sensor_port0_hsync_channel, sensor_port0_hsync_func, sensor_port0_hsync_gid, sensor_port0_hsync_pin );
		gpiocfgInput( sensor_port0_data_channel, sensor_port0_data_func, sensor_port0_data_gid_03, sensor_port0_data_startpin );
		gpiocfgInput( sensor_port0_data_channel, sensor_port0_data_func, sensor_port0_data_gid_03, sensor_port0_data_startpin+1 );
		gpiocfgInput( sensor_port0_data_channel, sensor_port0_data_func, sensor_port0_data_gid_03, sensor_port0_data_startpin+2 );
		gpiocfgInput( sensor_port0_data_channel, sensor_port0_data_func, sensor_port0_data_gid_03, sensor_port0_data_startpin+3 );
		gpiocfgInput( sensor_port0_data_channel, sensor_port0_data_func, sensor_port0_data_gid_47, sensor_port0_data_startpin+4 );
		gpiocfgInput( sensor_port0_data_channel, sensor_port0_data_func, sensor_port0_data_gid_47, sensor_port0_data_startpin+5 );
		gpiocfgInput( sensor_port0_data_channel, sensor_port0_data_func, sensor_port0_data_gid_47, sensor_port0_data_startpin+6 );
		gpiocfgInput( sensor_port0_data_channel, sensor_port0_data_func, sensor_port0_data_gid_47, sensor_port0_data_startpin+7 );
	}
	
	if( strncmp(port, "MIPI", strlen(port)) == 0 ) {
		printk("set port MIPI\n");
		//mclk
		gpiocfgInput(mipi_mclk_channel, mipi_mclk_func, mipi_mclk_gid, mipi_mclk_pin);
		
		//mipi0
		gpiocfgInput(mipi0_clkn_channel, mipi0_clkn_func, mipi0_clkn_gid, mipi0_clkn_pin);
		gpiocfgInput(mipi0_clkp_channel, mipi0_clkp_func, mipi0_clkp_gid, mipi0_clkp_pin);
		gpiocfgInput(mipi0_data0n_channel, mipi0_data0n_func, mipi0_data0n_gid, mipi0_data0n_pin);
		gpiocfgInput(mipi0_data0p_channel, mipi0_data0p_func, mipi0_data0p_gid, mipi0_data0p_pin);

		//mipi1
		gpiocfgInput(mipi1_clkn_channel, mipi1_clkn_func, mipi1_clkn_gid, mipi1_clkn_pin);
		gpiocfgInput(mipi1_clkp_channel, mipi1_clkp_func, mipi1_clkp_gid, mipi1_clkp_pin);
		gpiocfgInput(mipi1_data0n_channel, mipi1_data0n_func, mipi1_data0n_gid, mipi1_data0n_pin);
		gpiocfgInput(mipi1_data0p_channel, mipi1_data0p_func, mipi1_data0p_gid, mipi1_data0p_pin);
	}
	return 0;
}

int 
sensor0_set_reset(
	int enable
)
{
	UINT32	value = 0, active = sensor0_set_reset_active;
	unsigned int channel = sensor0_set_reset_channel;
	int handle;
	gpio_content_t ctx;

	if ( channel == 0xff ) {
		return -1;	/* No Function */
	}

	if ( enable ) {	/* setting active polar */
		if ( active != 0 ) {
			value = 1;
		}
		else {
			value = 0;
		}
	}
	else {
		if ( active != 0 ) {
			value = 0;
		}
		else {
			value = 1;
		}
	}

	ctx.pin_index = MK_GPIO_INDEX( sensor0_set_reset_channel, sensor0_set_reset_func, sensor0_set_reset_gid, sensor0_set_reset_pin );
	handle = gp_gpio_request(ctx.pin_index, NULL);
	gp_gpio_set_output( handle, value, 0 );
	gp_gpio_release(handle);

	return 0;
}

int 
sensor0_set_standby(
	int enable
)
{
	UINT32	value = 0, active = sensor0_set_standby_active;
	unsigned int channel = sensor0_set_standby_channel;
	int handle;
	gpio_content_t ctx;

	if ( channel == 0xff ) {
		return -1;	/* No Function */
	}

	if ( enable ) {	/* setting active polar */
		if ( active != 0 ) {
			value = 1;
		}
		else {
			value = 0;
		}
	}
	else {
		if ( active != 0 ) {
			value = 0;
		}
		else {
			value = 1;
		}
	}

	ctx.pin_index = MK_GPIO_INDEX( sensor0_set_standby_channel, sensor0_set_standby_func, sensor0_set_standby_gid, sensor0_set_standby_pin );
	handle = gp_gpio_request(ctx.pin_index, NULL);
	gp_gpio_set_output( handle, value, 0 );
	gp_gpio_release(handle);

	return 0;
}

int
sensor0_set_power(
	int enable
)
{
	UINT32	value = 0, active = sensor0_set_power_active;
	unsigned int channel = sensor0_set_power_channel;
	int handle;
	gpio_content_t ctx;

	if ( channel == 0xff ) {
		return -1;	/* No Function */
	}

	if ( enable ) {	/* setting active polar */
		if ( active != 0 ) {
			value = 1;
		}
		else {
			value = 0;
		}
	}
	else {
		if ( active != 0 ) {
			value = 0;
		}
		else {
			value = 1;
		}
	}

	ctx.pin_index = MK_GPIO_INDEX( sensor0_set_power_channel, sensor0_set_power_func, sensor0_set_power_gid, sensor0_set_power_pin );
	handle = gp_gpio_request(ctx.pin_index, NULL);
	gp_gpio_set_output( handle, value, 0 );
	gp_gpio_release(handle);

	return 0;
}

int 
sensor1_set_reset(
	int enable
)
{
	UINT32	value = 0, active = sensor1_set_reset_active;
	unsigned int channel = sensor1_set_reset_channel;
	int handle;
	gpio_content_t ctx;

	if ( channel == 0xff ) {
		return -1;	/* No Function */
	}

	if ( enable ) {	/* setting active polar */
		if ( active != 0 ) {
			value = 1;
		}
		else {
			value = 0;
		}
	}
	else {
		if ( active != 0 ) {
			value = 0;
		}
		else {
			value = 1;
		}
	}

	ctx.pin_index = MK_GPIO_INDEX( sensor1_set_reset_channel, sensor1_set_reset_func, sensor1_set_reset_gid, sensor1_set_reset_pin );
	handle = gp_gpio_request(ctx.pin_index, NULL);
	gp_gpio_set_output( handle, value, 0 );
	gp_gpio_release(handle);

	return 0;
}

int 
sensor1_set_standby(
	int enable
)
{
	UINT32	value = 0, active = sensor1_set_standby_active;
	unsigned int channel = sensor1_set_standby_channel;
	int handle;
	gpio_content_t ctx;

	if ( channel == 0xff ) {
		return -1;	/* No Function */
	}

	if ( enable ) {	/* setting active polar */
		if ( active != 0 ) {
			value = 1;
		}
		else {
			value = 0;
		}
	}
	else {
		if ( active != 0 ) {
			value = 0;
		}
		else {
			value = 1;
		}
	}

	ctx.pin_index = MK_GPIO_INDEX( sensor1_set_standby_channel, sensor1_set_standby_func, sensor1_set_standby_gid, sensor1_set_standby_pin );
	handle = gp_gpio_request(ctx.pin_index, NULL);
	gp_gpio_set_output( handle, value, 0 );
	gp_gpio_release(handle);

	return 0;
}

int
sensor1_set_power(
	int enable
)
{
	UINT32	value = 0, active = sensor1_set_power_active;
	unsigned int channel = sensor1_set_power_channel;
	int handle;
	gpio_content_t ctx;

	if ( channel == 0xff ) {
		return -1;	/* No Function */
	}

	if ( enable ) {	/* setting active polar */
		if ( active != 0 ) {
			value = 1;
		}
		else {
			value = 0;
		}
	}
	else {
		if ( active != 0 ) {
			value = 0;
		}
		else {
			value = 1;
		}
	}

	ctx.pin_index = MK_GPIO_INDEX( sensor1_set_power_channel, sensor1_set_power_func, sensor1_set_power_gid, sensor1_set_power_pin );
	handle = gp_gpio_request(ctx.pin_index, NULL);
	gp_gpio_set_output( handle, value, 0 );
	gp_gpio_release(handle);

	return 0;
}

int 
sensor2_set_reset(
	int enable
)
{
	UINT32	value = 0, active = sensor2_set_reset_active;
	unsigned int channel = sensor2_set_reset_channel;
	int handle;
	gpio_content_t ctx;

	if ( channel == 0xff ) {
		return -1;	/* No Function */
	}

	if ( enable ) {	/* setting active polar */
		if ( active != 0 ) {
			value = 1;
		}
		else {
			value = 0;
		}
	}
	else {
		if ( active != 0 ) {
			value = 0;
		}
		else {
			value = 1;
		}
	}

	ctx.pin_index = MK_GPIO_INDEX(sensor2_set_reset_channel, sensor2_set_reset_func, sensor2_set_reset_gid, sensor2_set_reset_pin);
	handle = gp_gpio_request(ctx.pin_index, NULL);
	gp_gpio_set_output( handle, value, 0 );
	gp_gpio_release(handle);

	return 0;
}

int 
sensor2_set_standby(
	int enable
)
{
	UINT32	value = 0, active = sensor2_set_standby_active;
	unsigned int channel = sensor2_set_standby_channel;
	int handle;
	gpio_content_t ctx;

	if ( channel == 0xff ) {
		return -1;	/* No Function */
	}

	if ( enable ) {	/* setting active polar */
		if ( active != 0 ) {
			value = 1;
		}
		else {
			value = 0;
		}
	}
	else {
		if ( active != 0 ) {
			value = 0;
		}
		else {
			value = 1;
		}
	}

	ctx.pin_index = MK_GPIO_INDEX(sensor2_set_standby_channel, sensor2_set_standby_func, sensor2_set_standby_gid, sensor2_set_standby_pin);
	handle = gp_gpio_request(ctx.pin_index, NULL);
	gp_gpio_set_output( handle, value, 0 );
	gp_gpio_release(handle);

	return 0;
}

int
sensor2_set_power(
	int enable
)
{
	UINT32	value = 0, active = sensor2_set_power_active;
	unsigned int channel = sensor2_set_power_channel;
	int handle;
	gpio_content_t ctx;

	if ( channel == 0xff ) {
		return -1;	/* No Function */
	}

	if ( enable ) {	/* setting active polar */
		if ( active != 0 ) {
			value = 1;
		}
		else {
			value = 0;
		}
	}
	else {
		if ( active != 0 ) {
			value = 0;
		}
		else {
			value = 1;
		}
	}

	ctx.pin_index = MK_GPIO_INDEX(sensor2_set_power_channel, sensor2_set_power_func, sensor2_set_power_gid, sensor2_set_power_pin);
	handle = gp_gpio_request(ctx.pin_index, NULL);
	gp_gpio_set_output( handle, value, 0 );
	gp_gpio_release(handle);

	return 0;
}

static const gp_board_sensor_t config_sensor0 = {
	.set_sensor_reset = sensor0_set_reset,
	.set_sensor_standby = sensor0_set_standby,
	.set_sensor_power = sensor0_set_power,
	.set_sensor_port = sensor_set_port,
};

static const gp_board_sensor_t config_sensor1 = {
	.set_sensor_reset = sensor1_set_reset,
	.set_sensor_standby = sensor1_set_standby,
	.set_sensor_power = sensor1_set_power,
	.set_sensor_port = sensor_set_port,
};

static const gp_board_sensor_t config_sensor2 = {
	.set_sensor_reset = sensor2_set_reset,
	.set_sensor_standby = sensor2_set_standby,
	.set_sensor_power = sensor2_set_power,
	.set_sensor_port = sensor_set_port,
};

/**************************************************************************
 * LED Control
 **************************************************************************/
int 
led_set_light(
	int type,
	int enable
)
{
	UINT32	value = 0;
	unsigned int channel = led_set_light0_channel;
	unsigned int func = led_set_light0_func;
	unsigned int gid = led_set_light0_gid;
	unsigned int pin = led_set_light0_pin;
	unsigned int active = led_set_light0_active;
	int handle;
	gpio_content_t ctx;

	if ( type > 9 ) {
		return -1;	/* out of define */
	}

	switch( type ) {
		case 0:
			channel = led_set_light0_channel;
			func = led_set_light0_func;
			gid = led_set_light0_gid;
			pin = led_set_light0_pin;
			active = led_set_light0_active;
			break;
		case 1:
			channel = led_set_light1_channel;
			func = led_set_light1_func;
			gid = led_set_light1_gid;
			pin = led_set_light1_pin;
			active = led_set_light1_active;
			break;
		case 2:
			channel = led_set_light2_channel;
			func = led_set_light2_func;
			gid = led_set_light2_gid;
			pin = led_set_light2_pin;
			active = led_set_light2_active;
			break;
		case 3:
			channel = led_set_light3_channel;
			func = led_set_light3_func;
			gid = led_set_light3_gid;
			pin = led_set_light3_pin;
			active = led_set_light3_active;
			break;
		case 4:
			channel = led_set_light4_channel;
			func = led_set_light4_func;
			gid = led_set_light4_gid;
			pin = led_set_light4_pin;
			active = led_set_light4_active;
			break;
		case 5:
			channel = led_set_light5_channel;
			func = led_set_light5_func;
			gid = led_set_light5_gid;
			pin = led_set_light5_pin;
			active = led_set_light5_active;
			break;
		case 6:
			channel = led_set_light6_channel;
			func = led_set_light6_func;
			gid = led_set_light6_gid;
			pin = led_set_light6_pin;
			active = led_set_light6_active;
			break;
		case 7:
			channel = led_set_light7_channel;
			func = led_set_light7_func;
			gid = led_set_light7_gid;
			pin = led_set_light7_pin;
			active = led_set_light7_active;
			break;
		case 8:
			channel = led_set_light8_channel;
			func = led_set_light8_func;
			gid = led_set_light8_gid;
			pin = led_set_light8_pin;
			active = led_set_light8_active;
			break;
		case 9:
			channel = led_set_light9_channel;
			func = led_set_light9_func;
			gid = led_set_light9_gid;
			pin = led_set_light9_pin;
			active = led_set_light9_active;
			break;
	}


	if ( channel == 0xff ) {
		return -1;	/* No Function */
	}

	if ( enable ) {	/* setting active polar */
		if ( active != 0 ) {
			value = 1;
		}
		else {
			value = 0;
		}
	}
	else {
		if ( active != 0 ) {
			value = 0;
		}
		else {
			value = 1;
		}
	}

	ctx.pin_index = MK_GPIO_INDEX( channel, func, gid, pin );
	handle = gp_gpio_request(ctx.pin_index, NULL);
	gp_gpio_set_output( handle, value, 0 );
	gp_gpio_release(handle);

	return 0;
}

int 
led_set_brightness(
	int channel,
	int enable,
	int value
)
{
	struct gp_pwm_config_s pwm_config;
	int pwm;
	int pwm_id;
	UINT32	freq;

	pwm_id = 0xff;
	switch ( channel ) {
		case 1:
			pwm_id = led_set_brightness_id1;
			freq = led_set_brightness_freq1;
			break;
		case 2:
			pwm_id = led_set_brightness_id2;
			freq = led_set_brightness_freq2;
			break;
	}

	if ( pwm_id == 0xff ) {
		return -1;
	}
	
	pwm  = gp_pwm_request(pwm_id);
	if(0 == pwm){
		return -1;
	}
	
	pwm_config.duty = (UINT32) value;
	pwm_config.freq = freq;
	gp_pwm_set_config(pwm, &pwm_config);
	gp_pwm_enable(pwm);
	gp_pwm_release(pwm);

	return 0;
}

static const gp_board_led_t config_led_control = {
	.set_led_light = led_set_light,
	.set_led_brightness = led_set_brightness,
};

/**************************************************************************
 * angle switch
 **************************************************************************/
int
angle_sw_detect(
	void
)
{
	UINT32	value = 0, active = angle_sw_detect_normal_level;
	unsigned int channel = angle_sw_detect_channel;
	int handle;
	gpio_content_t ctx;

	if ( channel == 0xff ) {
		return -1;	/* No Function */
	}

	ctx.pin_index = MK_GPIO_INDEX( angle_sw_detect_channel, angle_sw_detect_func, angle_sw_detect_gid, angle_sw_detect_pin );
	handle = gp_gpio_request(ctx.pin_index, NULL);
	gp_gpio_get_value(handle, &value);
	gp_gpio_release(handle);

	if ( value ) {	/* check active polar */
		if ( active != 0 ) {
			return angle_sw_detect_normal;
		}
		else {
			return angle_sw_detect_rot;
		}
	}
	else {
		if ( active != 0 ) {
			return angle_sw_detect_rot;
		}
		else {
			return angle_sw_detect_normal;
		}
	}

	return -1;
}

static const gp_board_angle_sw_t config_angle_sw = {
	.detect_angle_sw = angle_sw_detect,
};

/**************************************************************************
 * pwm config
 **************************************************************************/
static int config_pwm_channel[2] = {1,2};
static const gp_board_pwm_t config_pwm_info = {
	.channel = config_pwm_channel,
	.count = 2,			
};

/**************************************************************************
 * PCB Version Detection
 **************************************************************************/
int
pcb_version_detect(
	void
)
{
	return -1;
}

static const gp_board_pcb_t config_pcb_version = {
	.detect_pcb_version = pcb_version_detect,
};

/**************************************************************************
 * Sytem Power status
 **************************************************************************/
int
dc_in_detect(
	void
)
{
	UINT32	value = 0, active = dc_in_detect_level;
	unsigned int channel = dc_in_detect_channel;
	int handle;
	gpio_content_t ctx;

	if ( channel == 0xff ) {
		return -1;	/* No Function */
	}

	ctx.pin_index = MK_GPIO_INDEX( dc_in_detect_channel, dc_in_detect_func, dc_in_detect_gid, dc_in_detect_pin );
	handle = gp_gpio_request(ctx.pin_index, NULL);
	gp_gpio_get_value(handle, &value);
	gp_gpio_release(handle);

	if ( value ) {	/* check active polar */
		if ( active != 0 ) {
			return 1;
		}
		else {
			return 0;
		}
	}
	else {
		if ( active != 0 ) {
			return 0;
		}
		else {
			return 1;
		}
	}

	return 0;
}
#ifdef SYSCONFIG_INTERNAL_ADC
static int bat_low_vol_cnt = 0;
#endif
int
battery_voltage_detect(
	void
)
{
#ifdef SYSCONFIG_INTERNAL_ADC
	int adc_value;
	int cnt;
#endif

	const static unsigned int voltage_table[5][2] = {
		{945, 4},	
		{912, 3},	
		{879, 2},	
		{847, 1},	
		{814, 0},
	};
#ifdef SYSCONFIG_INTERNAL_ADC
#if 0
	if( !IS_ERR_VALUE(gp_board_info->handle_core_1v2_voltage)) {
		adc_value = gp_adc_read_timeout(gp_board_info->handle_core_1v2_voltage, 0);
		//printk("core_lv2_voltage:%d\n\r",adc_value);
		if( !(IS_ERR_VALUE(adc_value)) ) {
			if( adc_value >= 380 ) {	/* battery = 3.3v */
				if( 1 == bat_low_vol_cnt ){
					//printk("battery voltage very low %d\n", adc_value);
					return 0;
				}else{
					bat_low_vol_cnt++;
				}
			}else{
				bat_low_vol_cnt = 0;
			}
		}
	}
#endif
	
	if( !IS_ERR_VALUE(gp_board_info->handle_battery_voltage)) {
		adc_value = gp_adc_read_timeout(gp_board_info->handle_battery_voltage, 0);
		if( !(IS_ERR_VALUE(adc_value)) ) {
			for( cnt=0; cnt<5; cnt++ ) {
				if( adc_value >= voltage_table[cnt][0] ) {
					if(voltage_table[cnt][1]<=20) {
						//printk("battery_voltage_detect %d, %d\n", adc_value, voltage_table[cnt][1]);
					}
					return voltage_table[cnt][1];
				}
			}
		}
	}
#else
	return voltage_table[0][1];
#endif
	return -1;
}
int
battery_charger_status(
	void
)
{
	UINT32	value = 0, active = battery_charger_status_level;
	unsigned int channel = battery_charger_status_channel;
	int handle;
	gpio_content_t ctx;

	if ( channel == 0xff ) {
		return -1;	/* No Function */
	}

	ctx.pin_index = MK_GPIO_INDEX( battery_charger_status_channel, battery_charger_status_func, battery_charger_status_gid, battery_charger_status_pin );
	handle = gp_gpio_request(ctx.pin_index, NULL);
	gp_gpio_get_value(handle, &value);
	gp_gpio_release(handle);

	if ( value ) {	/* check active polar */
		if ( active != 0 ) {
			return 1;
		}
		else {
			return 0;
		}
	}
	else {
		if ( active != 0 ) {
			return 0;
		}
		else {
			return 1;
		}
	}

	return 0;
}


/**
 *	@brief: Prepare the platform for entering the system sleep state indicated
 *	by @begin().
 *	@prepare() is called right after devices have been suspended (ie. the
 *	appropriate .suspend() method has been executed for each device) and
 *	before device drivers' late suspend callbacks are executed.  It returns
 *	0 on success or a negative error code otherwise, in which case the
 *	system cannot enter the desired sleep state (@prepare_late(), @enter(),
 *	@wake(), and @finish() will not be called in that case).
 */
int 
pm_prepare(
	void
)
{
#ifdef SYSCONFIG_GP_FAST_BOOT
	printk("[%s][%d] for FasTBoot\n", __FUNCTION__, __LINE__);
	// config gpio or necessary operation for fast boot
	return 0;
#else

#if 0
	// config gpio or necessary operation for suspend/resume to ram
/**************************************************************************
 * Panel power & Backlight
 **************************************************************************/
	//panel_set_backlight
	pm_gpio( &(gp_board_info->handle_panel_set_backlight), panel_set_backlight_channel, panel_set_backlight_ss, panel_set_backlight_ss_pull );

	//panel_set_spi
	pm_gpio( &(gp_board_info->handle_panel_set_spi_cs), panel_set_spi_cs_channel, panel_set_spi_cs_ss, panel_set_spi_cs_ss_pull );
	pm_gpio( &(gp_board_info->handle_panel_set_spi_scl), panel_set_spi_scl_channel, panel_set_spi_scl_ss, panel_set_spi_scl_ss_pull );
	pm_gpio( &(gp_board_info->handle_panel_set_spi_sda), panel_set_spi_sda_channel, panel_set_spi_sda_ss, panel_set_spi_sda_ss_pull );

	//panel_set_mirror
	pm_gpio( &(gp_board_info->handle_panel_set_mirror0), panel_set_mirror0_channel, panel_set_mirror0_ss, panel_set_mirror0_ss_pull );
	pm_gpio( &(gp_board_info->handle_panel_set_mirror1), panel_set_mirror1_channel, panel_set_mirror1_ss, panel_set_mirror1_ss_pull );

/**************************************************************************
 * SD
 **************************************************************************/
	//sd0_detect
	pm_gpio( &(gp_board_info->handle_sd0_detect), sd0_detect_channel, sd0_detect_ss, sd0_detect_ss_pull );

	//sd0_is_write_protected
	pm_gpio( &(gp_board_info->handle_sd0_is_write_protected), sd0_is_write_protected_channel, sd0_is_write_protected_ss, sd0_is_write_protected_ss_pull );

	//sdio0_set_standby
	pm_gpio( &(gp_board_info->handle_sdio0_set_standby), sdio0_set_standby_channel, sdio0_set_standby_ss, sdio0_set_standby_ss_pull );

	//sd1_detect
	pm_gpio( &(gp_board_info->handle_sd1_detect), sd1_detect_channel, sd1_detect_ss, sd1_detect_ss_pull );

	//sd1_is_write_protected
	pm_gpio( &(gp_board_info->handle_sd1_is_write_protected), sd1_is_write_protected_channel, sd1_is_write_protected_ss, sd1_is_write_protected_ss_pull );

	//sdio1_set_standby
	pm_gpio( &(gp_board_info->handle_sdio1_set_standby), sdio1_set_standby_channel, sdio1_set_standby_ss, sdio1_set_standby_ss_pull );

	//sd2_detect
	pm_gpio( &(gp_board_info->handle_sd2_detect), sd2_detect_channel, sd2_detect_ss, sd2_detect_ss_pull );

	//sd2_is_write_protected
	pm_gpio( &(gp_board_info->handle_sd2_is_write_protected), sd2_is_write_protected_channel, sd2_is_write_protected_ss, sd2_is_write_protected_ss_pull );

	//sdio2_set_standby
	pm_gpio( &(gp_board_info->handle_sdio2_set_standby), sdio2_set_standby_channel, sdio2_set_standby_ss, sdio2_set_standby_ss_pull );

/**************************************************************************
 * NAND - CSn & WP
 **************************************************************************/
	//nand_set_cs

	//nand_set_wp
	pm_gpio( &(gp_board_info->handle_nand_set_wp), nand_set_wp_channel, nand_set_wp_ss, nand_set_wp_ss_pull );

/**************************************************************************
 * USB HOST / SLAVE
 **************************************************************************/ 
	//usb_host_en_power
	pm_gpio( &(gp_board_info->handle_usb_host_en_power), usb_host_en_power_channel, usb_host_en_power_ss, usb_host_en_power_ss_pull );

	//usb_switch_bus
	pm_gpio( &(gp_board_info->handle_usb_switch_bus), usb_switch_bus_channel, usb_switch_bus_ss, usb_switch_bus_ss_pull );

	//usb_host_power_good
	pm_gpio( &(gp_board_info->handle_usb_slave_detect_power), usb_power_good_channel, usb_power_good_ss, usb_power_good_ss_pull );

/**************************************************************************
 * Audio out
 **************************************************************************/
	//headphone_detect
	pm_gpio( &(gp_board_info->handle_headphone_detect), headphone_detect_channel, headphone_detect_ss, headphone_detect_ss_pull );

	//speaker_set_power
	pm_gpio( &(gp_board_info->handle_speaker_set_power), speaker_set_power_channel, speaker_set_power_ss, speaker_set_power_ss_pull );

/**************************************************************************
 * Sensor
 **************************************************************************/
	//sensor_set_reset1
	pm_gpio( &(gp_board_info->handle_sensor_set_reset0), sensor_set_reset0_channel, sensor_set_reset0_ss, sensor_set_reset0_ss_pull );

	//sensor_set_standby1
	pm_gpio( &(gp_board_info->handle_sensor_set_standby0), sensor_set_standby0_channel, sensor_set_standby0_ss, sensor_set_standby0_ss_pull );

	//sensor_set_power1
	pm_gpio( &(gp_board_info->handle_sensor_set_power0), sensor_set_power0_channel, sensor_set_power0_ss, sensor_set_power0_ss_pull );

	//sensor_set_reset2
	pm_gpio( &(gp_board_info->handle_sensor_set_reset1), sensor_set_reset1_channel, sensor_set_reset1_ss, sensor_set_reset1_ss_pull );

	//sensor_set_standby2
	pm_gpio( &(gp_board_info->handle_sensor_set_standby1), sensor_set_standby1_channel, sensor_set_standby1_ss, sensor_set_standby1_ss_pull );

	//sensor_set_power2
	pm_gpio( &(gp_board_info->handle_sensor_set_power1), sensor_set_power1_channel, sensor_set_power1_ss, sensor_set_power1_ss_pull );

/**************************************************************************
 * LED Control
 **************************************************************************/
	//led_set_light(
	pm_gpio( &(gp_board_info->handle_led_set_light0), led_set_light0_channel, led_set_light0_ss, led_set_light0_ss_pull );
	pm_gpio( &(gp_board_info->handle_led_set_light1), led_set_light1_channel, led_set_light1_ss, led_set_light1_ss_pull );
	pm_gpio( &(gp_board_info->handle_led_set_light2), led_set_light2_channel, led_set_light2_ss, led_set_light2_ss_pull );
	pm_gpio( &(gp_board_info->handle_led_set_light3), led_set_light3_channel, led_set_light3_ss, led_set_light3_ss_pull );
	pm_gpio( &(gp_board_info->handle_led_set_light4), led_set_light4_channel, led_set_light4_ss, led_set_light4_ss_pull );
	pm_gpio( &(gp_board_info->handle_led_set_light5), led_set_light5_channel, led_set_light5_ss, led_set_light5_ss_pull );
	pm_gpio( &(gp_board_info->handle_led_set_light6), led_set_light6_channel, led_set_light6_ss, led_set_light6_ss_pull );
	pm_gpio( &(gp_board_info->handle_led_set_light7), led_set_light7_channel, led_set_light7_ss, led_set_light7_ss_pull );
	pm_gpio( &(gp_board_info->handle_led_set_light8), led_set_light8_channel, led_set_light8_ss, led_set_light8_ss_pull );
	pm_gpio( &(gp_board_info->handle_led_set_light9), led_set_light9_channel, led_set_light9_ss, led_set_light9_ss_pull );

/**************************************************************************
 * angle switch
 **************************************************************************/
//angle_sw_detect
	pm_gpio( &(gp_board_info->handle_angle_sw_detect), angle_sw_detect_channel, angle_sw_detect_ss, angle_sw_detect_ss_pull );

/**************************************************************************
 * PCB Version Detection
 **************************************************************************/
//pcb_version_detect


/**************************************************************************
 * Sytem Power status
 **************************************************************************/
//dc_in_detect
	pm_gpio( &(gp_board_info->handle_dc_in_detect), dc_in_detect_channel, dc_in_detect_ss, dc_in_detect_ss_pull );

//battery_voltage_detect


//battery_charger_status
	pm_gpio( &(gp_board_info->handle_battery_charger_status), battery_charger_status_channel, battery_charger_status_ss, battery_charger_status_ss_pull );
#endif

#endif
	return 0;
}

/**
 *	@brief: Finish wake-up of the platform.
 *	@finish is called right prior to calling device drivers' regular suspend
 *	callbacks.
 *	This callback is optional, but should be implemented by the platforms
 *	that implement @prepare().  If implemented, it is always called after
 *	@enter() and @wake(), if implemented, even if any of them fails.
 */
void 
pm_finish(
	void
)
{
#ifdef SYSCONFIG_GP_FAST_BOOT
	printk("[%s][%d] for FasTBoot\n", __FUNCTION__, __LINE__);
	// config gpio or necessary operation for fast boot
	return;
#else
#if 0
/**************************************************************************
 * Panel power & Backlight
 **************************************************************************/
	//panel_set_backlight
	pm_gpio( &(gp_board_info->handle_panel_set_backlight), panel_set_backlight_channel, panel_set_backlight_rs, panel_set_backlight_rs_pull );

	//panel_set_spi
	pm_gpio( &(gp_board_info->handle_panel_set_spi_cs), panel_set_spi_cs_channel, panel_set_spi_cs_rs, panel_set_spi_cs_rs_pull );
	pm_gpio( &(gp_board_info->handle_panel_set_spi_scl), panel_set_spi_scl_channel, panel_set_spi_scl_rs, panel_set_spi_scl_rs_pull );
	pm_gpio( &(gp_board_info->handle_panel_set_spi_sda), panel_set_spi_sda_channel, panel_set_spi_sda_rs, panel_set_spi_sda_rs_pull );

	//panel_set_mirror
	pm_gpio( &(gp_board_info->handle_panel_set_mirror0), panel_set_mirror0_channel, panel_set_mirror0_rs, panel_set_mirror0_rs_pull );
	pm_gpio( &(gp_board_info->handle_panel_set_mirror1), panel_set_mirror1_channel, panel_set_mirror1_rs, panel_set_mirror1_rs_pull );

/**************************************************************************
 * SD
 **************************************************************************/
	//sd0_detect
	pm_gpio( &(gp_board_info->handle_sd0_detect), sd0_detect_channel, sd0_detect_rs, sd0_detect_rs_pull );

	//sd0_is_write_protected
	pm_gpio( &(gp_board_info->handle_sd0_is_write_protected), sd0_is_write_protected_channel, sd0_is_write_protected_rs, sd0_is_write_protected_rs_pull );

	//sdio0_set_standby
	pm_gpio( &(gp_board_info->handle_sdio0_set_standby), sdio0_set_standby_channel, sdio0_set_standby_rs, sdio0_set_standby_rs_pull );

	//sd1_detect
	pm_gpio( &(gp_board_info->handle_sd1_detect), sd1_detect_channel, sd1_detect_rs, sd1_detect_rs_pull );

	//sd1_is_write_protected
	pm_gpio( &(gp_board_info->handle_sd1_is_write_protected), sd1_is_write_protected_channel, sd1_is_write_protected_rs, sd1_is_write_protected_rs_pull );

	//sdio1_set_standby
	pm_gpio( &(gp_board_info->handle_sdio1_set_standby), sdio1_set_standby_channel, sdio1_set_standby_rs, sdio1_set_standby_rs_pull );

	//sd2_detect
	pm_gpio( &(gp_board_info->handle_sd2_detect), sd2_detect_channel, sd2_detect_rs, sd2_detect_rs_pull );

	//sd2_is_write_protected
	pm_gpio( &(gp_board_info->handle_sd2_is_write_protected), sd2_is_write_protected_channel, sd2_is_write_protected_rs, sd2_is_write_protected_rs_pull );

	//sdio2_set_standby
	pm_gpio( &(gp_board_info->handle_sdio2_set_standby), sdio2_set_standby_channel, sdio2_set_standby_rs, sdio2_set_standby_rs_pull );

/**************************************************************************
 * MS
 **************************************************************************/
	//ms_detect
	pm_gpio( &(gp_board_info->handle_ms_detect), ms_detect_channel, ms_detect_rs, ms_detect_rs_pull );

/**************************************************************************
 * NAND - CSn & WP
 **************************************************************************/
	//nand_set_cs

	//nand_set_wp
	pm_gpio( &(gp_board_info->handle_nand_set_wp), nand_set_wp_channel, nand_set_wp_rs, nand_set_wp_rs_pull );

/**************************************************************************
 * USB HOST / SLAVE
 **************************************************************************/ 
	//usb_host_en_power
	pm_gpio( &(gp_board_info->handle_usb_host_en_power), usb_host_en_power_channel, usb_host_en_power_rs, usb_host_en_power_rs_pull );

	//usb_switch_bus
	pm_gpio( &(gp_board_info->handle_usb_switch_bus), usb_switch_bus_channel, usb_switch_bus_rs, usb_switch_bus_rs_pull );

	//usb_host_power_good
	pm_gpio( &(gp_board_info->handle_usb_slave_detect_power), usb_power_good_channel, usb_power_good_rs, usb_power_good_rs_pull );

/**************************************************************************
 * Audio out
 **************************************************************************/
	//headphone_detect
	pm_gpio( &(gp_board_info->handle_headphone_detect), headphone_detect_channel, headphone_detect_rs, headphone_detect_rs_pull );

	//speaker_set_power
	pm_gpio( &(gp_board_info->handle_speaker_set_power), speaker_set_power_channel, speaker_set_power_rs, speaker_set_power_rs_pull );

/**************************************************************************
 * Sensor
 **************************************************************************/
	//sensor_set_reset1
	pm_gpio( &(gp_board_info->handle_sensor_set_reset0), sensor_set_reset0_channel, sensor_set_reset0_rs, sensor_set_reset0_rs_pull );

	//sensor_set_standby1
	pm_gpio( &(gp_board_info->handle_sensor_set_standby0), sensor_set_standby0_channel, sensor_set_standby0_rs, sensor_set_standby0_rs_pull );

	//sensor_set_power1
	pm_gpio( &(gp_board_info->handle_sensor_set_power0), sensor_set_power0_channel, sensor_set_power0_rs, sensor_set_power0_rs_pull );

	//sensor_set_reset2
	pm_gpio( &(gp_board_info->handle_sensor_set_reset1), sensor_set_reset1_channel, sensor_set_reset1_rs, sensor_set_reset1_rs_pull );

	//sensor_set_standby2
	pm_gpio( &(gp_board_info->handle_sensor_set_standby1), sensor_set_standby1_channel, sensor_set_standby1_rs, sensor_set_standby1_rs_pull );

	//sensor_set_power2
	pm_gpio( &(gp_board_info->handle_sensor_set_power1), sensor_set_power1_channel, sensor_set_power1_rs, sensor_set_power1_rs_pull );

/**************************************************************************
 * LED Control
 **************************************************************************/
	//led_set_light(
	pm_gpio( &(gp_board_info->handle_led_set_light0), led_set_light0_channel, led_set_light0_rs, led_set_light0_rs_pull );
	pm_gpio( &(gp_board_info->handle_led_set_light1), led_set_light1_channel, led_set_light1_rs, led_set_light1_rs_pull );
	pm_gpio( &(gp_board_info->handle_led_set_light2), led_set_light2_channel, led_set_light2_rs, led_set_light2_rs_pull );
	pm_gpio( &(gp_board_info->handle_led_set_light3), led_set_light3_channel, led_set_light3_rs, led_set_light3_rs_pull );
	pm_gpio( &(gp_board_info->handle_led_set_light4), led_set_light4_channel, led_set_light4_rs, led_set_light4_rs_pull );
	pm_gpio( &(gp_board_info->handle_led_set_light5), led_set_light5_channel, led_set_light5_rs, led_set_light5_rs_pull );
	pm_gpio( &(gp_board_info->handle_led_set_light6), led_set_light6_channel, led_set_light6_rs, led_set_light6_rs_pull );
	pm_gpio( &(gp_board_info->handle_led_set_light7), led_set_light7_channel, led_set_light7_rs, led_set_light7_rs_pull );
	pm_gpio( &(gp_board_info->handle_led_set_light8), led_set_light8_channel, led_set_light8_rs, led_set_light8_rs_pull );

	pm_gpio( &(gp_board_info->handle_led_set_light9), led_set_light9_channel, led_set_light9_rs, led_set_light9_rs_pull );

/**************************************************************************
 * angle switch
 **************************************************************************/
//angle_sw_detect
	pm_gpio( &(gp_board_info->handle_angle_sw_detect), angle_sw_detect_channel, angle_sw_detect_rs, angle_sw_detect_rs_pull );

/**************************************************************************
 * PCB Version Detection
 **************************************************************************/
//pcb_version_detect


/**************************************************************************
 * Sytem Power status
 **************************************************************************/
//dc_in_detect
	pm_gpio( &(gp_board_info->handle_dc_in_detect), dc_in_detect_channel, dc_in_detect_rs, dc_in_detect_rs_pull );

//battery_voltage_detect


//battery_charger_status
	pm_gpio( &(gp_board_info->handle_battery_charger_status), battery_charger_status_channel, battery_charger_status_rs, battery_charger_status_rs_pull );
#endif
#endif
	return;
}

static const gp_board_system_t config_system_status = {
	.detect_dc_in = dc_in_detect,
	.detect_battery_voltage = battery_voltage_detect,
	.detect_battery_charger = battery_charger_status,
	.prepare = pm_prepare,
	.finish = pm_finish,
};

/**************************************************************************
 * Pin Function
 **************************************************************************/
static void
gp_board_set_pin_func(
	int32_t type,
	int32_t enable,
	uint32_t timeout
)
{
	switch (type) {
		case GP_PIN_DISP0_LCD:
			if (enable) {
				gpHalGpioSetPadGrp((1 << 16) | (7 << 8));
				gpHalGpioSetPadGrp((1 << 16) | (8 << 8));
				gpHalGpioSetPadGrp((1 << 16) | (9 << 8));
				gpHalGpioSetPadGrp((1 << 16) | (10<< 8));
				gpHalGpioSetPadGrp((1 << 16) | (11<< 8));
				gpHalGpioSetPadGrp((1 << 16) | (12<< 8));
				gpHalGpioSetPadGrp((1 << 16) | (13<< 8));
				gpHalGpioSetPadGrp((1 << 16) | (14<< 8));
				gpHalGpioSetPadGrp((1 << 16) | (15<< 8));
				gpHalGpioSetPadGrp((1 << 16) | (16<< 8));
				gpHalGpioSetPadGrp((1 << 16) | (47<< 8));
				gpHalGpioSetPadGrp((1 << 16) | (48<< 8));
				gpHalGpioSetPadGrp((1 << 16) | (49<< 8));
				gpHalGpioSetPadGrp((1 << 16) | (50<< 8));
				gpHalGpioSetPadGrp((1 << 16) | (51<< 8));				
			}
			else {		
				gpHalGpioSetPadGrp((0 << 16) | (7 << 8));
				gpHalGpioSetPadGrp((0 << 16) | (8 << 8));
				gpHalGpioSetPadGrp((0 << 16) | (9 << 8));
				gpHalGpioSetPadGrp((0 << 16) | (10<< 8));
				gpHalGpioSetPadGrp((0 << 16) | (11<< 8));
				gpHalGpioSetPadGrp((0 << 16) | (12<< 8));
				gpHalGpioSetPadGrp((0 << 16) | (13<< 8));
				gpHalGpioSetPadGrp((0 << 16) | (14<< 8));
				gpHalGpioSetPadGrp((0 << 16) | (15<< 8));
				gpHalGpioSetPadGrp((0 << 16) | (16<< 8));
				gpHalGpioSetPadGrp((0 << 16) | (47<< 8));
				gpHalGpioSetPadGrp((0 << 16) | (48<< 8));
				gpHalGpioSetPadGrp((0 << 16) | (49<< 8));
				gpHalGpioSetPadGrp((0 << 16) | (50<< 8));
				gpHalGpioSetPadGrp((0 << 16) | (51<< 8));				
			}
			break;
			
		case GP_PIN_PWM:
			if (enable) {
				gpHalGpioSetPadGrp((0 << 16) | (28 << 8));
				//gpHalGpioSetPadGrp((3 << 16) | (12 << 8));
			}
			else {
				gpHalGpioSetPadGrp((0 << 16) | (28 << 8));
				//gpHalGpioSetPadGrp((0 << 16) | (12 << 8));
			}
			break;

		case GP_PIN_SPI:
			if (enable) {
				gpHalGpioSetPadGrp((1 << 16) | (23 << 8)); 	//CS, IOA4
				gpHalGpioSetPadGrp((1 << 16) | (54 << 8)); 	//TX, IOA5
				gpHalGpioSetPadGrp((1 << 16) | (55 << 8)); 	//RX, IOA6
				gpHalGpioSetPadGrp((1 << 16) | (56 << 8));	//CLK,IOA7
			}
			else {
				gpHalGpioSetPadGrp((0 << 16) | (23 << 8));
				gpHalGpioSetPadGrp((0 << 16) | (54 << 8));
				gpHalGpioSetPadGrp((0 << 16) | (55 << 8));
				gpHalGpioSetPadGrp((0 << 16) | (56 << 8));
			}
			break;

		case GP_PIN_SPI1:
			if (enable) {
				gpHalGpioSetPadGrp((1 << 16) | (39 << 8));	//CS, IOC27
				gpHalGpioSetPadGrp((1 << 16) | (40 << 8));	//CLK,IOC26
				gpHalGpioSetPadGrp((1 << 16) | (41 << 8));	//RX, IOC25
				gpHalGpioSetPadGrp((1 << 16) | (40 << 8));	//TX, IOC24
			}
			else {
				gpHalGpioSetPadGrp((0 << 16) | (39 << 8));
				gpHalGpioSetPadGrp((0 << 16) | (40 << 8));
				gpHalGpioSetPadGrp((0 << 16) | (41 << 8));
				gpHalGpioSetPadGrp((0 << 16) | (40 << 8));
			}
			break;
			
		case GP_PIN_SD0:
			if (enable) {
				gpHalGpioSetPadGrp((sd0_clk_func << 16) | (sd0_clk_gid << 8));
				gpHalGpioSetPadGrp((sd0_cmd_func << 16) | (sd0_cmd_gid << 8));
				gpHalGpioSetPadGrp((sd0_data0_func << 16) | (sd0_data0_gid << 8));
				gpHalGpioSetPadGrp((sd0_data1_func << 16) | (sd0_data1_gid << 8));
				gpHalGpioSetPadGrp((sd0_data2_func << 16) | (sd0_data2_gid << 8));
				gpHalGpioSetPadGrp((sd0_data3_func << 16) | (sd0_data3_gid << 8));
			}
			else {
				gpHalGpioSetPadGrp((0 << 16) | (sd0_clk_gid << 8));
				gpHalGpioSetPadGrp((0 << 16) | (sd0_cmd_gid << 8));
				gpHalGpioSetPadGrp((0 << 16) | (sd0_data0_gid << 8));
				gpHalGpioSetPadGrp((0 << 16) | (sd0_data1_gid << 8));
				gpHalGpioSetPadGrp((0 << 16) | (sd0_data2_gid << 8));
				gpHalGpioSetPadGrp((0 << 16) | (sd0_data3_gid << 8));
			}
			break;	
		case GP_PIN_SD1:
			if (enable) {
				gpHalGpioSetPadGrp((sd1_clk_func << 16) | (sd1_clk_gid << 8));
				gpHalGpioSetPadGrp((sd1_cmd_func << 16) | (sd1_cmd_gid << 8));
				gpHalGpioSetPadGrp((sd1_data0_func << 16) | (sd1_data0_gid << 8));
				gpHalGpioSetPadGrp((sd1_data1_func << 16) | (sd1_data1_gid << 8));
				gpHalGpioSetPadGrp((sd1_data2_func << 16) | (sd1_data2_gid << 8));
				gpHalGpioSetPadGrp((sd1_data3_func << 16) | (sd1_data3_gid << 8));
			}
			else {
				gpHalGpioSetPadGrp((0 << 16) | (sd1_clk_gid << 8));
				gpHalGpioSetPadGrp((0 << 16) | (sd1_cmd_gid << 8));
				gpHalGpioSetPadGrp((0 << 16) | (sd1_data0_gid << 8));
				gpHalGpioSetPadGrp((0 << 16) | (sd1_data1_gid << 8));
				gpHalGpioSetPadGrp((0 << 16) | (sd1_data2_gid << 8));
				gpHalGpioSetPadGrp((0 << 16) | (sd1_data3_gid << 8));
			}
			break;
		case GP_PIN_SD2:
			if (enable) {
				gpHalGpioSetPadGrp((sd2_clk_func << 16) | (sd2_clk_gid << 8));
				gpHalGpioSetPadGrp((sd2_cmd_func << 16) | (sd2_cmd_gid << 8));
				gpHalGpioSetPadGrp((sd2_data0_func << 16) | (sd2_data0_gid << 8));
				gpHalGpioSetPadGrp((sd2_data1_func << 16) | (sd2_data1_gid << 8));
				gpHalGpioSetPadGrp((sd2_data2_func << 16) | (sd2_data2_gid << 8));
				gpHalGpioSetPadGrp((sd2_data3_func << 16) | (sd2_data3_gid << 8));
			}
			else {
				gpHalGpioSetPadGrp((0 << 16) | (sd2_clk_gid << 8));
				gpHalGpioSetPadGrp((0 << 16) | (sd2_cmd_gid << 8));
				gpHalGpioSetPadGrp((0 << 16) | (sd2_data0_gid << 8));
				gpHalGpioSetPadGrp((0 << 16) | (sd2_data1_gid << 8));
				gpHalGpioSetPadGrp((0 << 16) | (sd2_data2_gid << 8));
				gpHalGpioSetPadGrp((0 << 16) | (sd2_data3_gid << 8));
			}
			break;	
		default:
			break;
	}

	return;
}

int32_t
pin_func_request(
	int32_t type,
	uint32_t timeout
)
{
	int32_t ret;

	/* Mutex lock */
	if (timeout == GP_BOARD_NO_WAIT) {
		if (down_trylock(&gp_board_info->pin_func_mutex) != 0) {
			printk("[%s:%d]\n", __FUNCTION__, __LINE__);
			return -EIO;
		}
	}
	else if (timeout == GP_BOARD_WAIT_FOREVER) {
		if (down_interruptible(&gp_board_info->pin_func_mutex) != 0) {
			printk("[%s:%d]\n", __FUNCTION__, __LINE__);
			return -EIO;
		}
	}
	else if (down_timeout(&gp_board_info->pin_func_mutex, timeout * HZ / 1000) != 0) {
		printk("[%s:%d]\n", __FUNCTION__, __LINE__);
		return -EIO;
	}

	/* malloc pin config entry */
	ret = gp_board_malloc_pin_conf_entry(type);
	if (ret >= 0) {
		gp_board_set_pin_func(type, 1, timeout);
	}

	/* Mutex unlock */
	up(&gp_board_info->pin_func_mutex);

	return ret;
}

void
pin_func_release(
	int32_t handle
)
{
	/* Mutex lock */
	if (down_interruptible(&gp_board_info->pin_func_mutex) != 0) {
		return;
	}

	gp_board_set_pin_func(handle, 0, 0);

	/* free pin config entry */
	gp_board_free_pin_conf_entry(handle);

	/* Mutex unlock */
	up(&gp_board_info->pin_func_mutex);
}

static const gp_board_pin_func_t config_pin_func = {
	.pin_func_request = pin_func_request,
	.pin_func_release = pin_func_release,
};
/************************************************************
HDMI 
***********************************************************/
static int hdmi_decet()
{
	gpio_content_t ctx;
	UINT32	value = 0;
	int handle;	

	if ( hdmi_detect_channel == 0xff ) {
		return -1;	/* No Function */
	}
	
	ctx.pin_index = MK_GPIO_INDEX( hdmi_detect_channel, hdmi_detect_func, hdmi_detect_gid, hdmi_detect_pin );
	handle = gp_gpio_request(ctx.pin_index, NULL);

	gp_gpio_get_value(handle, &value);
	gp_gpio_release(handle);

	return value;
}

static const  gp_board_HDMI_t config_hdmi = {
	.detect_hdmi_in = hdmi_decet,
};

/************************************************************
TV
***********************************************************/
static int tv_decet()
{
	gpio_content_t ctx;
	UINT32	value = 0;
	int handle;	

	if ( tv_detect_channel == 0xff ) {
		return -1;	/* No Function */
	}
	
	ctx.pin_index = MK_GPIO_INDEX( tv_detect_channel, tv_detect_func, tv_detect_gid, tv_detect_pin );
	handle = gp_gpio_request(ctx.pin_index, NULL);

	gp_gpio_get_value(handle, &value);
	gp_gpio_release(handle);
	//printk("@@@@@@@@@@@@[%s:%d] = [%d]\n", __FUNCTION__, __LINE__,value);
	return value;
}

static const  gp_board_TV_t config_tv = {
	.detect_tv_in = tv_decet,
};
/**************************************************************************
 * Pin Function
 **************************************************************************/
static long
board_ioctl(
	struct file *filp,
	uint32_t cmd,
	unsigned long arg
)
{
	switch (cmd) {
		case 0:
			break;

		default:
			break;
	}

	return 0;
}

/**************************************************************************
 * Register
 **************************************************************************/
static const gp_board_item_t config_items[] = {
	GP_BOARD_ITEM("board", &config_board),
	GP_BOARD_ITEM("panel", &config_panel),
	GP_BOARD_ITEM("sd0", &config_sd0),
	GP_BOARD_ITEM("sd1", &config_sd1),
	GP_BOARD_ITEM("sd2", &config_sd2),
	GP_BOARD_ITEM("nand", &config_nand),
	GP_BOARD_ITEM("usb", &config_usb),
	GP_BOARD_ITEM("audio_out", &config_audio_out),
	GP_BOARD_ITEM("ps2_uart_mouse", &config_ps2_uart_mouse),
	GP_BOARD_ITEM("sensor0", &config_sensor0),
	GP_BOARD_ITEM("sensor1", &config_sensor1),
	GP_BOARD_ITEM("sensor2", &config_sensor2),
	GP_BOARD_ITEM("led", &config_led_control),
	GP_BOARD_ITEM("angle_sw", &config_angle_sw),
	GP_BOARD_ITEM("pcb", &config_pcb_version),
	GP_BOARD_ITEM("sys_pwr", &config_system_status),
	GP_BOARD_ITEM("pin_func", &config_pin_func),
	GP_BOARD_ITEM("pwm", &config_pwm_info),
	GP_BOARD_ITEM("HDMI",&config_hdmi),
	GP_BOARD_ITEM("TV",&config_tv),	
	GP_BOARD_ITEM("touch",&config_touch),

};

static const gp_board_config_t config = {
	(gp_board_item_t*)config_items,
	GP_BOARD_COUNT_OF(config_items),
	&board_ioctl,
};

static int __init
board_config_init(
	void
)
{
	int32_t retval;
	gp_board_t *pConfig;

	/* malloc */
	gp_board_info = kmalloc(sizeof(gp_board_info_t), GFP_KERNEL);
	if (!gp_board_info) {
		printk("[%s:%d], Error\n", __FUNCTION__, __LINE__);
		retval = -ENOMEM;
		goto fail_malloc;
	}
	memset(gp_board_info, 0, sizeof(gp_board_info_t));

	/* init mutex */
	init_MUTEX(&gp_board_info->pin_func_mutex);

	/* register board config */
	gp_board_register(&config);

	pConfig = gp_board_get_config("board", gp_board_t);
	if (pConfig == NULL || pConfig->init == NULL) {
		printk("Board init not fount!\n");
	}
	else {
		pConfig->init();
	}

	return 0;

fail_malloc:
	return retval;
}

static void
board_config_exit(
	void
)
{
	kfree(gp_board_info);
	gp_board_info = NULL;
}

module_init(board_config_init);
module_exit(board_config_exit);
