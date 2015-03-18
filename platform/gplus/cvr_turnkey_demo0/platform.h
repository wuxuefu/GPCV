#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#define VTAC_HW_VERSION_1 1
#define VTAC_HW_VERSION_3 3

#define VTAC_HW_VERSION VTAC_HW_VERSION_3

//#define MK_GPIO_INDEX(channel,func,gid,pin) (((channel)<<24)|((func)<<16)|((gid)<<8)|(pin))

/**************************************************************************
 * Panel power & Backlight
 **************************************************************************/
//deyue for 1024x600
#define	panel_set_AVDD_EN_channel		0xff//GPIO_CHANNEL_0
#define	panel_set_AVDD_EN_func		1
#define	panel_set_AVDD_EN_gid			42
#define	panel_set_AVDD_EN_pin			25
#define	panel_set_AVDD_EN_level		1


#define	panel_set_backlight_ADJ_channel		0xff//GPIO_CHANNEL_0
#define	panel_set_backlight_ADJ_func		1
#define	panel_set_backlight_ADJ_gid			43
#define	panel_set_backlight_ADJ_pin			26
#define	panel_set_backlight_ADJ_level		1

#define	panel_set_backlight_channel		0x02//IOC7
#define	panel_set_backlight_func		2
#define	panel_set_backlight_gid			5
#define	panel_set_backlight_pin			7
#define	panel_set_backlight_level		1


#define	panel_brightness_backlight_id		0xff		/* No Used */
#define	panel_brightness_backlight_freq		80000	
#define	panel_brightness_backlight_init_duty	15

#define	panel_set_powerOn0_start_delay	0	/*mS*/

#define	panel_set_powerOn0_0_channel	0xff
#define	panel_set_powerOn0_0_func		2
#define	panel_set_powerOn0_0_gid		23
#define	panel_set_powerOn0_0_pin		12
#define	panel_set_powerOn0_0_level		0

#define	panel_set_powerOn0_0to1_delay	30	/*mS*/

#define	panel_set_powerOn0_1_channel	0xff
#define	panel_set_powerOn0_1_func		2
#define	panel_set_powerOn0_1_gid		23
#define	panel_set_powerOn0_1_pin		12
#define	panel_set_powerOn0_1_level		0

#define	panel_set_powerOn0_1to2_delay	10	/*mS*/

#define	panel_set_powerOn0_2_channel	0xff
#define	panel_set_powerOn0_2_func		2
#define	panel_set_powerOn0_2_gid		23
#define	panel_set_powerOn0_2_pin		12
#define	panel_set_powerOn0_2_level		0

#define	panel_set_powerOn0_2to3_delay	10	/*mS*/

#define	panel_set_powerOn0_3_channel	0xff
#define	panel_set_powerOn0_3_func		2
#define	panel_set_powerOn0_3_gid		23
#define	panel_set_powerOn0_3_pin		12
#define	panel_set_powerOn0_3_level		0

#define	panel_set_powerOn0_end_delay	10	/*mS*/

#define	panel_set_powerOn1_start_delay	0	/*mS*/

#define	panel_set_powerOn1_0_channel	0xff
#define	panel_set_powerOn1_0_func		2
#define	panel_set_powerOn1_0_gid		23
#define	panel_set_powerOn1_0_pin		12
#define	panel_set_powerOn1_0_level		0

#define	panel_set_powerOn1_0to1_delay	10	/*mS*/

#define	panel_set_powerOn1_1_channel	0xff
#define	panel_set_powerOn1_1_func		2
#define	panel_set_powerOn1_1_gid		23
#define	panel_set_powerOn1_1_pin		12
#define	panel_set_powerOn1_1_level		0

#define	panel_set_powerOn1_1to2_delay	10	/*mS*/

#define	panel_set_powerOn1_2_channel	0xff
#define	panel_set_powerOn1_2_func		2
#define	panel_set_powerOn1_2_gid		23
#define	panel_set_powerOn1_2_pin		12
#define	panel_set_powerOn1_2_level		0

#define	panel_set_powerOn1_2to3_delay	10	/*mS*/

#define	panel_set_powerOn1_3_channel	0xff
#define	panel_set_powerOn1_3_func		2
#define	panel_set_powerOn1_3_gid		23
#define	panel_set_powerOn1_3_pin		12
#define	panel_set_powerOn1_3_level		0

#define	panel_set_powerOn1_end_delay	10	/*mS*/

#define	panel_set_powerOff0_start_delay	0	/*mS*/

#define	panel_set_powerOff0_0_channel	0xff
#define	panel_set_powerOff0_0_func		2
#define	panel_set_powerOff0_0_gid		23
#define	panel_set_powerOff0_0_pin		12
#define	panel_set_powerOff0_0_level		0

#define	panel_set_powerOff0_0to1_delay	10	/*mS*/

#define	panel_set_powerOff0_1_channel	0xff
#define	panel_set_powerOff0_1_func		2
#define	panel_set_powerOff0_1_gid		23
#define	panel_set_powerOff0_1_pin		12
#define	panel_set_powerOff0_1_level		0

#define	panel_set_powerOff0_1to2_delay	10	/*mS*/

#define	panel_set_powerOff0_2_channel	0xff
#define	panel_set_powerOff0_2_func		2
#define	panel_set_powerOff0_2_gid		23
#define	panel_set_powerOff0_2_pin		12
#define	panel_set_powerOff0_2_level		0

#define	panel_set_powerOff0_2to3_delay	10	/*mS*/

#define	panel_set_powerOff0_3_channel	0xff
#define	panel_set_powerOff0_3_func		2
#define	panel_set_powerOff0_3_gid		23
#define	panel_set_powerOff0_3_pin		12
#define	panel_set_powerOff0_3_level		0

#define	panel_set_powerOff0_end_delay	10	/*mS*/

#define	panel_set_powerOff1_start_delay	0	/*mS*/

#define	panel_set_powerOff1_0_channel	0xff
#define	panel_set_powerOff1_0_func		2
#define	panel_set_powerOff1_0_gid		23
#define	panel_set_powerOff1_0_pin		12
#define	panel_set_powerOff1_0_level		0

#define	panel_set_powerOff1_0to1_delay	10	/*mS*/

#define	panel_set_powerOff1_1_channel	0xff
#define	panel_set_powerOff1_1_func		2
#define	panel_set_powerOff1_1_gid		23
#define	panel_set_powerOff1_1_pin		12
#define	panel_set_powerOff1_1_level		0

#define	panel_set_powerOff1_1to2_delay	10	/*mS*/

#define	panel_set_powerOff1_2_channel	0xff
#define	panel_set_powerOff1_2_func		2
#define	panel_set_powerOff1_2_gid		23
#define	panel_set_powerOff1_2_pin		12
#define	panel_set_powerOff1_2_level		0

#define	panel_set_powerOff1_2to3_delay	10	/*mS*/

#define	panel_set_powerOff1_3_channel	0xff
#define	panel_set_powerOff1_3_func		2
#define	panel_set_powerOff1_3_gid		23
#define	panel_set_powerOff1_3_pin		12
#define	panel_set_powerOff1_3_level		0

#define	panel_set_powerOff1_end_delay	10	/*mS*/

#define	panel_set_spi_cs_channel	0xff
#define	panel_set_spi_cs_func		2
#define	panel_set_spi_cs_gid		23
#define	panel_set_spi_cs_pin		12

#define	panel_set_spi_scl_channel	0xff
#define	panel_set_spi_scl_func		2
#define	panel_set_spi_scl_gid		23
#define	panel_set_spi_scl_pin		12

#define	panel_set_spi_sda_channel	0xff
#define	panel_set_spi_sda_func		2
#define	panel_set_spi_sda_gid		23
#define	panel_set_spi_sda_pin		12

#define	panel_set_mirror0_channel	0xff
#define	panel_set_mirror0_func		2
#define	panel_set_mirror0_gid		23
#define	panel_set_mirror0_pin		12

#define	panel_set_mirror1_channel	0xff
#define	panel_set_mirror1_func		2
#define	panel_set_mirror1_gid		23
#define	panel_set_mirror1_pin		12

#define	panel_set_mirror_normal0	1
#define	panel_set_mirror_normal1	1
#define	panel_set_mirror_mirror0	0
#define	panel_set_mirror_mirror1	0

/**************************************************************************
 * SD
 **************************************************************************/

#define	sd0_clk_channel			0x0
#define	sd0_clk_func			1
#define	sd0_clk_gid				28
#define	sd0_clk_pin				23
#define	sd0_clk_level			0
#define	sd0_clk_pull_level		GPIO_PULL_HIGH
#define	sd0_clk_driving			1

#define	sd0_cmd_channel			0x0
#define	sd0_cmd_func			1
#define	sd0_cmd_gid				59
#define	sd0_cmd_pin				24
#define	sd0_cmd_level			0
#define	sd0_cmd_pull_level		GPIO_PULL_HIGH
#define	sd0_cmd_driving		1

#define	sd0_data0_channel		0x0
#define	sd0_data0_func			1
#define	sd0_data0_gid			28
#define	sd0_data0_pin			25
#define	sd0_data0_level			0
#define	sd0_data0_pull_level	GPIO_PULL_HIGH
#define	sd0_data0_driving		1

#define	sd0_data1_channel		0x0
#define	sd0_data1_func			1
#define	sd0_data1_gid			29
#define	sd0_data1_pin			26
#define	sd0_data1_level			0
#define	sd0_data1_pull_level	GPIO_PULL_HIGH
#define	sd0_data1_driving		1

#define	sd0_data2_channel		0x0
#define	sd0_data2_func			1
#define	sd0_data2_gid			29
#define	sd0_data2_pin			27
#define	sd0_data2_level			0
#define	sd0_data2_pull_level	GPIO_PULL_HIGH
#define	sd0_data2_driving		1

#define	sd0_data3_channel		0x0
#define	sd0_data3_func			1
#define	sd0_data3_gid			29
#define	sd0_data3_pin			28
#define	sd0_data3_level			0
#define	sd0_data3_pull_level	GPIO_PULL_HIGH
#define	sd0_data3_driving		1
 
#define	sd0_set_power_channel		0xff //IOA21 GID:27 /* No Used  hardware Set 1*/ 
#define	sd0_set_power_func			3
#define	sd0_set_power_gid			27
#define	sd0_set_power_pin			21
#define	sd0_set_power_level			1
#define	sd0_set_power_driving		0

#define	sd0_detect_channel			0x0	//IOA2
#define	sd0_detect_func				0
#define	sd0_detect_gid				22
#define	sd0_detect_pin				2
#define	sd0_detect_level			0
#define	sd0_detect_pull_level		GPIO_PULL_HIGH
#define	sd0_detect_driving			0

#define	sd0_is_write_protected_channel		0xff
#define	sd0_is_write_protected_func			2
#define	sd0_is_write_protected_gid			23
#define	sd0_is_write_protected_pin			12
#define	sd0_is_write_protected_level		0
#define	sd0_is_write_protected_pull_level	GPIO_PULL_HIGH
#define	sd0_is_write_protected_driving		0

#define	sdio0_set_standby_channel	0xff		/* No Used */
#define	sdio0_set_standby_func		2
#define	sdio0_set_standby_gid		30
#define	sdio0_set_standby_pin		24
#define	sdio0_set_standby_level		0

#define	sd1_clk_channel			0xff
#define	sd1_clk_func			2
#define	sd1_clk_gid				54
#define	sd1_clk_pin				5
#define	sd1_clk_level			0
#define	sd1_clk_pull_level		GPIO_PULL_HIGH
#define	sd1_clk_driving			0

#define	sd1_cmd_channel			0xff
#define	sd1_cmd_func			2
#define	sd1_cmd_gid				23
#define	sd1_cmd_pin				4
#define	sd1_cmd_level			0
#define	sd1_cmd_pull_level		GPIO_PULL_HIGH
#define	sd1_cmd_driving			0

#define	sd1_data0_channel		0xff
#define	sd1_data0_func			2
#define	sd1_data0_gid			55
#define	sd1_data0_pin			6
#define	sd1_data0_level			0
#define	sd1_data0_pull_level	GPIO_PULL_HIGH
#define	sd1_data0_driving		0

#define	sd1_data1_channel		0xff
#define	sd1_data1_func			2
#define	sd1_data1_gid			56
#define	sd1_data1_pin			7
#define	sd1_data1_level			0
#define	sd1_data1_pull_level	GPIO_PULL_HIGH
#define	sd1_data1_driving		0

#define	sd1_data2_channel		0xff
#define	sd1_data2_func			2
#define	sd1_data2_gid			22
#define	sd1_data2_pin			2
#define	sd1_data2_level			0
#define	sd1_data2_pull_level	GPIO_PULL_HIGH
#define	sd1_data2_driving		0

#define	sd1_data3_channel		0xff
#define	sd1_data3_func			2
#define	sd1_data3_gid			22
#define	sd1_data3_pin			3
#define	sd1_data3_level			0
#define	sd1_data3_pull_level	GPIO_PULL_HIGH
#define	sd1_data3_driving		0

#define	sd1_set_power_channel	0xff //IOA21 GID:27 /* No Used  hardware Set 1*/ 
#define	sd1_set_power_func		3
#define	sd1_set_power_gid		27
#define	sd1_set_power_pin		21
#define	sd1_set_power_level		0
#define	sd1_set_power_driving	0

#define	sd1_detect_channel		0xff	//IOA30
#define	sd1_detect_func			1
#define	sd1_detect_gid			31
#define	sd1_detect_pin			30
#define	sd1_detect_level		0
#define	sd1_detect_pull_level	GPIO_PULL_HIGH
#define	sd1_detect_driving		0

#define	sd1_is_write_protected_channel		0xff
#define	sd1_is_write_protected_func			2
#define	sd1_is_write_protected_gid			23
#define	sd1_is_write_protected_pin			12
#define	sd1_is_write_protected_level		0
#define	sd1_is_write_protected_pull_level	GPIO_PULL_HIGH
#define	sd1_is_write_protected_driving		0

#define	sdio1_set_standby_channel	0xff		/* B_KSD_CARD1, WIFI_DISABLE_L */
#define	sdio1_set_standby_func		2
#define	sdio1_set_standby_gid		30
#define	sdio1_set_standby_pin		24
#define	sdio1_set_standby_level		0

#define	sd2_clk_channel			0xff
#define	sd2_clk_func			2
#define	sd2_clk_gid				26
#define	sd2_clk_pin				20
#define	sd2_clk_level			0
#define	sd2_clk_pull_level		GPIO_PULL_HIGH
#define	sd2_clk_driving			0

#define	sd2_cmd_channel			0xff
#define	sd2_cmd_func			2
#define	sd2_cmd_gid				27
#define	sd2_cmd_pin				21
#define	sd2_cmd_level			0
#define	sd2_cmd_pull_level		GPIO_PULL_HIGH
#define	sd2_cmd_driving			0

#define	sd2_data0_channel		0xff
#define	sd2_data0_func			2
#define	sd2_data0_gid			25
#define	sd2_data0_pin			20
#define	sd2_data0_level			0
#define	sd2_data0_pull_level	GPIO_PULL_HIGH
#define	sd2_data0_driving		0

#define	sd2_data1_channel		0xff
#define	sd2_data1_func			2
#define	sd2_data1_gid			25
#define	sd2_data1_pin			19
#define	sd2_data1_level			0
#define	sd2_data1_pull_level	GPIO_PULL_HIGH
#define	sd2_data1_driving		0

#define	sd2_data2_channel		0xff
#define	sd2_data2_func			2
#define	sd2_data2_gid			25
#define	sd2_data2_pin			22
#define	sd2_data2_level			0
#define	sd2_data2_pull_level	GPIO_PULL_HIGH
#define	sd2_data2_driving		0

#define	sd2_data3_channel		0xff
#define	sd2_data3_func			2
#define	sd2_data3_gid			25
#define	sd2_data3_pin			21
#define	sd2_data3_level			0
#define	sd2_data3_pull_level	GPIO_PULL_HIGH
#define	sd2_data3_driving		0

#define	sd2_set_power_channel	0xff //IOA21 GID:27 /* No Used  hardware Set 1*/ 
#define	sd2_set_power_func		3
#define	sd2_set_power_gid		27
#define	sd2_set_power_pin		21
#define	sd2_set_power_level		0
#define	sd2_set_power_driving	0


#define	sd2_detect_channel			0xff	//IOA29
#define	sd2_detect_func				1
#define	sd2_detect_gid				30
#define	sd2_detect_pin				29
#define	sd2_detect_level			0
#define	sd2_detect_pull_level		GPIO_PULL_HIGH
#define	sd2_detect_driving			0

/*#define	sd2_detect_channel		0xff//0
#define	sd2_detect_func			0
#define	sd2_detect_gid			19
#define	sd2_detect_pin			1
#define	sd2_detect_level		0
#define	sd2_detect_pull_level	GPIO_PULL_HIGH
#define	sd2_detect_driving		0*/

#define	sd2_is_write_protected_channel		0xff
#define	sd2_is_write_protected_func			2
#define	sd2_is_write_protected_gid			23
#define	sd2_is_write_protected_pin			12
#define	sd2_is_write_protected_level		0
#define	sd2_is_write_protected_pull_level	GPIO_PULL_HIGH
#define	sd2_is_write_protected_driving		0

#define	sdio2_set_standby_channel	0xff		/* B_KSD_CARD1, WIFI_DISABLE_L */
#define	sdio2_set_standby_func		2
#define	sdio2_set_standby_gid		30
#define	sdio2_set_standby_pin		24
#define	sdio2_set_standby_level		0

/**************************************************************************
 * NAND - CSn & WP
 **************************************************************************/
#define	nand_set_cs0_channel	0xff
#define	nand_set_cs0_func		2
#define	nand_set_cs0_gid		23
#define	nand_set_cs0_pin		12
#define	nand_set_cs0_active		0

#define	nand_set_cs1_channel	0xff
#define	nand_set_cs1_func		2
#define	nand_set_cs1_gid		23
#define	nand_set_cs1_pin		12
#define	nand_set_cs1_active		0

#define	nand_set_cs2_channel	0xff
#define	nand_set_cs2_func		2
#define	nand_set_cs2_gid		23
#define	nand_set_cs2_pin		12
#define	nand_set_cs2_active		0

#define	nand_set_cs3_channel	0xff
#define	nand_set_cs3_func		2
#define	nand_set_cs3_gid		23
#define	nand_set_cs3_pin		12
#define	nand_set_cs3_active		0

#define	nand_set_wp_channel	0xff
#define	nand_set_wp_func		2
#define	nand_set_wp_gid		23
#define	nand_set_wp_pin		12
#define	nand_set_wp_active		0

/**************************************************************************
 * USB HOST / SLAVE
 **************************************************************************/ 
#define	usb_host_en_power_channel		0xff
#define	usb_host_en_power_func		2
#define	usb_host_en_power_gid			23
#define	usb_host_en_power_pin			12
#define	usb_host_en_power_active		1

#define	usb_switch_bus_channel	0xff
#define	usb_switch_bus_func		2
#define	usb_switch_bus_gid		23
#define	usb_switch_bus_pin		12
#define	usb_switch_bus_active		1

#define	usb_power_good_channel	0xff	/* No Used */
#define	usb_power_good_func		2
#define	usb_power_good_gid		23
#define	usb_power_good_pin		12
#define	usb_power_good_level		1

#define	usb_slave_detect_config			USB_SLAVE_VBUS_GPIO	//USB_SLAVE_VBUS_POWERON1//USB_SLAVE_VBUS_NONE /*0x0 POWERON1, 0x1 GPIO, 0xff NONE*/
#define	usb_slave_detect_power_channel		0x0//GPIO_CHANNEL_0
#define	usb_slave_detect_power_func			0
#define	usb_slave_detect_power_gid			57 //31
#define	usb_slave_detect_power_pin			22 //30
#define usb_slave_detect_power_level		GPIO_PULL_FLOATING

/**************************************************************************
 * Audio out
 **************************************************************************/
#define	headphone_detect_channel	0xff
#define	headphone_detect_func		2
#define	headphone_detect_gid		23
#define	headphone_detect_pin		12
#define	headphone_detect_level		0

#define	speaker_set_power_channel	0x02  //IOC15
#define	speaker_set_power_func		0
#define	speaker_set_power_gid		16
#define	speaker_set_power_pin		15
#define	speaker_set_power_active	1

/**************************************************************************
 * Sensor port
 **************************************************************************/ 
#define sensor_port0_mclk_channel	0xff //IOA10
#define sensor_port0_mclk_func		0
#define sensor_port0_mclk_gid		0
#define sensor_port0_mclk_pin		10
#define sensor_port0_mclk_level		0

#define sensor_port0_pclk_channel	0xff //IOA11
#define sensor_port0_pclk_func		0
#define sensor_port0_pclk_gid		0
#define sensor_port0_pclk_pin		11
#define sensor_port0_pclk_level		0

#define sensor_port0_vsync_channel	0xff //IOA12
#define sensor_port0_vsync_func		0
#define sensor_port0_vsync_gid		0
#define sensor_port0_vsync_pin		12
#define sensor_port0_vsync_level	0

#define sensor_port0_hsync_channel	0xff	//IOA13
#define sensor_port0_hsync_func		0
#define sensor_port0_hsync_gid		0
#define sensor_port0_hsync_pin		13
#define sensor_port0_hsync_level	0

#define sensor_port0_data_channel	0xff //IOC0~IOC7
#define sensor_port0_data_func		0
#define sensor_port0_data_gid_03	4
#define sensor_port0_data_gid_47	5
#define sensor_port0_data_startpin	0
#define sensor_port0_data_level		0

#define sensor_port1_mclk_channel	0xff
#define sensor_port1_mclk_func		1
#define sensor_port1_mclk_gid		0
#define sensor_port1_mclk_pin		10
#define sensor_port1_mclk_level		0

#define sensor_port1_pclk_channel	0xff
#define sensor_port1_pclk_func		1
#define sensor_port1_pclk_gid		0
#define sensor_port1_pclk_pin		11
#define sensor_port1_pclk_level		0

#define sensor_port1_vsync_channel	0xff
#define sensor_port1_vsync_func		1
#define sensor_port1_vsync_gid		1
#define sensor_port1_vsync_pin		12
#define sensor_port1_vsync_level	0

#define sensor_port1_hsync_channel	0xff
#define sensor_port1_hsync_func		1
#define sensor_port1_hsync_gid		2
#define sensor_port1_hsync_pin		13
#define sensor_port1_hsync_level	0

#define sensor_port1_data_channel	0xff
#define sensor_port1_data_func		0
#define sensor_port1_data_gid		4
#define sensor_port1_data_startpin	0
#define sensor_port1_data_level		0

/**************************************************************************
 * Sensor reset/standby/power
 **************************************************************************/
#define	sensor0_set_reset_channel	0xff // IOC2 //Reset
#define	sensor0_set_reset_func		2
#define	sensor0_set_reset_gid		23
#define	sensor0_set_reset_pin		12
#define	sensor0_set_reset_active	0

#define	sensor0_set_standby_channel	0xff //IOA16 //SN18_EN GID:36
#define	sensor0_set_standby_func	2                                                           
#define	sensor0_set_standby_gid		36
#define	sensor0_set_standby_pin		16
#define	sensor0_set_standby_active	1

#define	sensor0_set_power_channel	0 //IOA17 SN28_EN  GID:36
#define	sensor0_set_power_func		0
#define	sensor0_set_power_gid		36
#define	sensor0_set_power_pin		16
#define	sensor0_set_power_active	1

#define	sensor1_set_reset_channel	0xff
#define	sensor1_set_reset_func		2
#define	sensor1_set_reset_gid		23
#define	sensor1_set_reset_pin		12
#define	sensor1_set_reset_active	0

#define	sensor1_set_standby_channel	0xff
#define	sensor1_set_standby_func	2
#define	sensor1_set_standby_gid		23
#define	sensor1_set_standby_pin		12
#define	sensor1_set_standby_active	1

#define	sensor1_set_power_channel	0xff
#define	sensor1_set_power_func		2
#define	sensor1_set_power_gid		23
#define	sensor1_set_power_pin		12
#define	sensor1_set_power_active	1

#define	sensor2_set_reset_channel	0x2 //IOC2
#define	sensor2_set_reset_func		2
#define	sensor2_set_reset_gid			4
#define	sensor2_set_reset_pin			2
#define	sensor2_set_reset_active	0

#define	sensor2_set_standby_channel	0xff
#define	sensor2_set_standby_func	2
#define	sensor2_set_standby_gid		23
#define	sensor2_set_standby_pin		12
#define	sensor2_set_standby_active	1

#define	sensor2_set_power_channel	0x0 //IOA17
#define	sensor2_set_power_func		0
#define	sensor2_set_power_gid			36
#define	sensor2_set_power_pin			17
#define	sensor2_set_power_active	1

/**************************************************************************
 * Mipi0 / Mipi1
 **************************************************************************/ 
#define mipi_mclk_channel			0x0	//IOA10
#define mipi_mclk_func				0
#define mipi_mclk_gid				0
#define mipi_mclk_pin				10
#define mipi_mclk_level				GPIO_PULL_LOW

#define mipi0_clkn_channel			0x02 //IOC26
#define mipi0_clkn_func				0
#define mipi0_clkn_gid				40
#define mipi0_clkn_pin				26
#define mipi0_clkn_level			GPIO_PULL_LOW

#define mipi0_clkp_channel			0x02 //IOC27
#define mipi0_clkp_func				0
#define mipi0_clkp_gid				39
#define mipi0_clkp_pin				27
#define mipi0_clkp_level			GPIO_PULL_LOW

#define mipi0_data0n_channel		0x02 //IOC24
#define mipi0_data0n_func			0
#define mipi0_data0n_gid			40
#define mipi0_data0n_pin			24
#define mipi0_data0n_level			GPIO_PULL_LOW

#define mipi0_data0p_channel		0x02 //IOC25
#define mipi0_data0p_func			0
#define mipi0_data0p_gid			41
#define mipi0_data0p_pin			25
#define mipi0_data0p_level			GPIO_PULL_LOW

#define mipi1_clkn_channel			0xff //0x02 //IOC2
#define mipi1_clkn_func				1
#define mipi1_clkn_gid				4
#define mipi1_clkn_pin				2
#define mipi1_clkn_level			GPIO_PULL_LOW

#define mipi1_clkp_channel			0xff //0x02 //IOC3
#define mipi1_clkp_func				1
#define mipi1_clkp_gid				4
#define mipi1_clkp_pin				3
#define mipi1_clkp_level			GPIO_PULL_LOW

#define mipi1_data0n_channel		0xff //0x02 //IOC0
#define mipi1_data0n_func			1
#define mipi1_data0n_gid			4
#define mipi1_data0n_pin			0
#define mipi1_data0n_level			GPIO_PULL_LOW

#define mipi1_data0p_channel		0xff //0x02 //IOC1
#define mipi1_data0p_func			1
#define mipi1_data0p_gid			4
#define mipi1_data0p_pin			1
#define mipi1_data0p_level			GPIO_PULL_LOW

/**************************************************************************
 * LED Control Ap_state_config.h 
 **************************************************************************/
#if (VTAC_HW_VERSION == VTAC_HW_VERSION_3)//IR_LED

#define	led_set_light0_channel	0x0 //IR //IOA18
#define	led_set_light0_func		0
#define	led_set_light0_gid		36
#define	led_set_light0_pin		18
#define	led_set_light0_active	1

#else

#define	led_set_light0_channel	0x04//0x0 //IR IOE1
#define	led_set_light0_func		1//0
#define	led_set_light0_gid		34//36
#define	led_set_light0_pin		1//18
#define	led_set_light0_active	1

#endif

#if (VTAC_HW_VERSION == VTAC_HW_VERSION_3) //Work_LED
#define	led_set_light1_channel	0x0	//LED IOA19
#define	led_set_light1_func		0
#define	led_set_light1_gid		36
#define	led_set_light1_pin		19
#define	led_set_light1_active	1
#else
#define	led_set_light1_channel	0x0	//LED IOA18
#define	led_set_light1_func		0
#define	led_set_light1_gid		36
#define	led_set_light1_pin		18
#define	led_set_light1_active	1
#endif

#define	led_set_light2_channel	0x2 //power hold
#define	led_set_light2_func		2
#define	led_set_light2_gid		4
#define	led_set_light2_pin		0
#define	led_set_light2_active	1

#define	led_set_light3_channel	0x2 //mute
#define	led_set_light3_func		0
#define	led_set_light3_gid		16
#define	led_set_light3_pin		15
#define	led_set_light3_active	1

#define	led_set_light4_channel	0xff
#define	led_set_light4_func		2
#define	led_set_light4_gid		23
#define	led_set_light4_pin		12
#define	led_set_light4_active	1

#define	led_set_light5_channel	0xff
#define	led_set_light5_func		2
#define	led_set_light5_gid		23
#define	led_set_light5_pin		12
#define	led_set_light5_active	1

#define	led_set_light6_channel	0xff
#define	led_set_light6_func		2
#define	led_set_light6_gid		23
#define	led_set_light6_pin		12
#define	led_set_light6_active	1

#define	led_set_light7_channel	0xff
#define	led_set_light7_func		2
#define	led_set_light7_gid		23
#define	led_set_light7_pin		12
#define	led_set_light7_active	1

#define	led_set_light8_channel	0xff
#define	led_set_light8_func		2
#define	led_set_light8_gid		23
#define	led_set_light8_pin		12
#define	led_set_light8_active	1

#define	led_set_light9_channel	0xff
#define	led_set_light9_func		2
#define	led_set_light9_gid		23
#define	led_set_light9_pin		12
#define	led_set_light9_active	1

#define	led_set_brightness_id0		0xff	/* No Used */
#define	led_set_brightness_freq0	10000
#define	led_set_brightness_init_duty0	0
#define	led_set_brightness_id1		0xff	/* No Used */
#define	led_set_brightness_freq1	10000
#define	led_set_brightness_init_duty1	0
#define	led_set_brightness_id2		0xff	/* No Used */
#define	led_set_brightness_freq2	10000
#define	led_set_brightness_init_duty2	0

/**************************************************************************
 * I2C
 **************************************************************************/
#define	i2c_sda_channel			0xFF	//0x1	//IOB16
#define	i2c_sda_func			0
#define	i2c_sda_gid				13
#define	i2c_sda_pin				16
#define	i2c_sda_pull_level		GPIO_PULL_HIGH
#define	i2c_scl_channel			0xFF	//0x1	//IOB17
#define	i2c_scl_func			0
#define	i2c_scl_gid				13
#define	i2c_scl_pin				17
#define	i2c_scl_pull_level		GPIO_PULL_HIGH

/**************************************************************************
 * TI2C
 **************************************************************************/
#define	ti2c_sda_channel		0x0		//IOA1
#define	ti2c_sda_func			1
#define	ti2c_sda_gid			21
#define	ti2c_sda_pin			1
#define	ti2c_sda_pull_level		GPIO_PULL_HIGH
#define	ti2c_scl_channel		0x0		//IOA0
#define	ti2c_scl_func			1
#define	ti2c_scl_gid			21
#define	ti2c_scl_pin			0
#define	ti2c_scl_pull_level		GPIO_PULL_HIGH

/**************************************************************************
 * angle switch
 **************************************************************************/
#define	angle_sw_detect_channel		0xff
#define	angle_sw_detect_func			2
#define	angle_sw_detect_gid				23
#define	angle_sw_detect_pin				12
#define	angle_sw_detect_normal_level	0
#define	angle_sw_detect_normal			0		/*degree*/
#define	angle_sw_detect_rot				90		/*degree*/

/**************************************************************************
 * PCB Version Detection
 **************************************************************************/


/**************************************************************************
 * Sytem Power status
 **************************************************************************/
#define	dc_in_detect_channel	0xff
#define	dc_in_detect_func		2
#define	dc_in_detect_gid		23
#define	dc_in_detect_pin		12
#define	dc_in_detect_level		0

#define	battery_voltage_detect_adc_ch	0x0

#define	battery_charger_status_channel	0xff
#define	battery_charger_status_func		2
#define	battery_charger_status_gid		23
#define	battery_charger_status_pin		12
#define	battery_charger_status_level		0

/**************************************************************************
 * PWM channel
 **************************************************************************/
#define	G32900_pwm1_channel		0xff
#define	G32900_pwm1_func		0
#define	G32900_pwm1_gid			42
#define	G32900_pwm1_pin			25

#define	G32900_pwm2_channel		0xff
#define	G32900_pwm2_func		0
#define	G32900_pwm2_gid			43
#define	G32900_pwm2_pin			26

/**************************************************************************
 * ps2_uart_mouse pin sel
 **************************************************************************/
#define	ps2mouse_set_clk_channel		0xff
#define	ps2mouse_set_clk_func		2
#define	ps2mouse_set_clk_gid			31
#define	ps2mouse_set_clk_pin			29

#define	ps2mouse_set_dat_channel		0xff
#define	ps2mouse_set_dat_func		2
#define	ps2mouse_set_dat_gid			32
#define	ps2mouse_set_dat_pin			30

/*****************************************************************************
* HDMI 
*****************************************************************************/
#define hdmi_detect_channel		0x0 //IOA29
#define	hdmi_detect_func		1
#define hdmi_detect_gid			30
#define hdmi_detect_pin			29
/*****************************************************************************
* TV
*****************************************************************************/

#define tv_detect_channel		0x2 //IOC14
#define tv_detect_func			0
#define tv_detect_gid			16
#define tv_detect_pin			14
/*#define tv_detect_channel		0
#define tv_detect_func			1
#define tv_detect_gid			31
#define tv_detect_pin			30*/

/*****************************************************************************
* touch //using IOD1 IOD2
*****************************************************************************/
#define touch_reset_channel     0xff
#define touch_reset_func        0
#define touch_reset_gid         47
#define touch_reset_pin         1

#define touch_int_channel       0xff
#define touch_int_func          0
#define touch_int_gid           47
#define touch_int_pin           2


#define G_SENSOR_int_channel    0x0 //IOA3
#define G_SENSOR_int_func		 0
#define G_SENSOR_int_gid        22
#define G_SENSOR_int_pin        3

#if (VTAC_HW_VERSION == VTAC_HW_VERSION_3)

#define	GPS_TX_channel		0x0 //IOA30
#define	GPS_TX_func			0
#define	GPS_TX_gid			31
#define	GPS_TX_pin			30
#define	GPS_TX_active		1
#define	GPS_TX_level		1

#define	GPS_PWR_channel		0x0 //IOA20
#define	GPS_PWR_func		1
#define	GPS_PWR_gid			26
#define	GPS_PWR_pin			20
#define	GPS_PWR_active		1
#define	GPS_PWR_level		1

#else

#define	GPS_TX_channel	0xff
#define	GPS_TX_func		2
#define	GPS_TX_gid		23
#define	GPS_TX_pin		12
#define	GPS_TX_active	0
#define	GPS_TX_level	0

#define	GPS_PWR_channel	0xff
#define	GPS_PWR_func		2
#define	GPS_PWR_gid		23
#define	GPS_PWR_pin		12
#define	GPS_PWR_activ	0
#define	GPS_PWR_level	0
#endif

#ifndef MK_GPIO_INDEX
#define MK_GPIO_INDEX(channel,func,gid,pin) ((channel<<24)|(func<<16)|(gid<<8)|(pin))
#endif


#if (VTAC_HW_VERSION == VTAC_HW_VERSION_3)

/*
MODE IOC10 GID:15  func:0
RECORD	IOC11 GID:16  func:1
MENU	IOC12 GID:16  func:1
DOWN	IOD24 GID:53  func:1
UP	 IOD23 GID:52  func:1
MK_GPIO_INDEX(channel,func,gid,pin)
*/
#define KEY_UP_INDEX MK_GPIO_INDEX(2,1,16,12) //menu
#define KEY_ESC_INDEX MK_GPIO_INDEX(2,2,4,1) //power
#define KEY_LEFT_INDEX MK_GPIO_INDEX(2,0,15,10) //mode
#define KEY_EXIT_INDEX MK_GPIO_INDEX(2,1,16,11) //enter
#define KEY_DOWN_INDEX MK_GPIO_INDEX(3,1,53,24) //down
#define KEY_RIGHT_INDEX MK_GPIO_INDEX(3,1,52,23) //up

#else

#define KEY_UP_INDEX MK_GPIO_INDEX(2,0,15,10) //menu
#define KEY_ESC_INDEX MK_GPIO_INDEX(2,2,4,1) //power
#define KEY_DOWN_INDEX MK_GPIO_INDEX(0,1,1,12) //down
#define KEY_LEFT_INDEX MK_GPIO_INDEX(2,0,16,11) //mode
#define KEY_RIGHT_INDEX MK_GPIO_INDEX(0,2,46,11) //up
#define KEY_EXIT_INDEX MK_GPIO_INDEX(2,0,16,12) //enter

#endif

#endif // _PLATFORM_H_
