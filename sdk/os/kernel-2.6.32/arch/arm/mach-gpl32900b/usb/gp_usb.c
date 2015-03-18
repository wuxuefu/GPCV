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
 * @file    gp_usb.c
 * @brief   Implement of usb driver.
 * @author  allen.chang
 * @since   2010/11/22
 * @date    2010/11/22
 */

 
#include <linux/miscdevice.h>
#include <linux/interrupt.h>
#include <linux/errno.h>
#include <linux/clk.h>
#include <linux/fs.h>
#include <linux/semaphore.h>
#include <linux/usb.h>
#include <mach/kernel.h>
#include <mach/module.h>
#include <mach/cdev.h>
#include <mach/diag.h>
#include <asm/uaccess.h>
#include <mach/gp_i2c_internal.h>
#include <mach/irqs.h>
#include <mach/gp_usb.h>
#include <mach/hal/hal_usb.h>
#include <mach/module.h>
#include <mach/gp_version.h>
#include <mach/gp_board.h>
#include <mach/gp_gpio.h>
#include <mach/general.h>
#include <mach/clk/gp_clk_core.h>
#include <mach/hal/hal_clock.h>
#include <mach/gp_chunkmem.h>
#include <linux/usb/android_composite.h>
/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/


/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
#define USB_HIGH_SPEED 1
#define USB_FULL_SPEED 0

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
 
 
/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/
extern char* usb_device_connect_state_get ( void );
extern unsigned int spmp_vbus_detect( void );
/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/
static long gp_usb_fops_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
//static int gp_usb_fops_release(struct inode *inode, struct file *file);
typedef struct usb_info_s {
	struct miscdevice dev;      /*!< @brief gpio device */
	struct semaphore sem;       /*!< @brief mutex semaphore for gpio ops */
} usb_info_t;

static struct file_operations usb_device_fops = {
	.owner          = THIS_MODULE,
	.unlocked_ioctl = gp_usb_fops_ioctl,
	//.release = gp_usb_fops_release
};

static gp_uvc_control_t *uvc_ops_handler = NULL;	
/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static uint8_t* pUsbBuf = NULL;
static usb_info_t *usb = NULL;
static int usb_wait_disk_sync = 0;
static int mac_pc_save_remove = 0;
static int usbd_device_type = USB_DEVICE_NONE_TYPE;
static int usbd_connectto = USB_DEVICE_CONNECT_TO_NONE;

void gp_register_uvc_handler(gp_uvc_control_t* control)
{
	uvc_ops_handler = control;
}

EXPORT_SYMBOL(gp_register_uvc_handler);

//static struct semaphore sem; 
/**
 * @brief   USB Clock enable
 * @param   en [IN]: enable
 * @return  none
 * @see
 */
void gp_usb_clock_en(int en)
{
	//printk("[%s][%d]\n", __FUNCTION__, __LINE__);
	gpHalUsbClockEn (en);
}
EXPORT_SYMBOL(gp_usb_clock_en);

/**
 * @brief   USB software connect setting
 * @param   connect [IN]: [1]connect [0]disconnect
 * @return  none
 * @see
 */
void gp_usb_slave_sw_connect(int connect)
{
	//printk("[%s][%d]\n", __FUNCTION__, __LINE__);
	gpHalUsbSlaveSwConnect(connect);
}
EXPORT_SYMBOL(gp_usb_slave_sw_connect);

/**
 * @brief   Host set delay time to wait first first setup
 * @param   d_t : ms unit
 * @return  none
 * @see
 */
void gp_usb_set_delay_time(int d_t)
{
	//printk("[%s][%d][%\n", __FUNCTION__, __LINE__);
	gpHalUsbSetDelayTime(d_t);
}
EXPORT_SYMBOL(gp_usb_set_delay_time);


/**
 * @brief   USB PHY0 enable
 * @param   en [IN]: enable
 * @return  none
 * @see
 */
void gp_usb_phy0_en(int en)
{
	//printk("[%s][%d]\n", __FUNCTION__, __LINE__);
	gpHalUsbPhy0En (en);
}
EXPORT_SYMBOL(gp_usb_phy0_en);

/**
 * @brief   USB PHY1 enable
 * @param   en [IN]: enable
 * @return  none
 * @see
 */
void gp_usb_phy1_en(int en)
{
	//printk("[%s][%d]\n", __FUNCTION__, __LINE__);
	gpHalUsbPhy1En (en);
}
EXPORT_SYMBOL(gp_usb_phy1_en);

/**
 * @brief   USB PHY1 mode config
 * @param   mode [IN]: USB_PHY1_HOST or USB_PHY1_SLAVE
 * @return  none
 * @see
 */
void gp_usb_phy1_config(int mode)
{
	gpHalUsbPhy1Config (mode);
}
EXPORT_SYMBOL(gp_usb_phy1_config);


/**
 * @brief   USB HOST Enable function
 * @param   en [IN]: enable
 * @return  none
 * @see
 */
void gp_usb_host_en(int en)
{
	//printk("[%s][%d]\n", __FUNCTION__, __LINE__);
	gpHalUsbHostEn (0, en);
}
EXPORT_SYMBOL(gp_usb_host_en);

/**
 * @brief   USB HOST config get
 * @param   none
 * @return  0: HOST in PHY0, 1 HOST in PHY1, 2 HOST is disable.
 * @see
 */
int gp_usb_host_config_get(void)
{
	//printk("[%s][%d]\n", __FUNCTION__, __LINE__);
	return gpHalUsbHostConfigGet();
}


EXPORT_SYMBOL(gp_usb_host_config_get);

/**
 * @brief   USB Slave VBUS Detect Get
 * @param   none
 * @return  1 Host Connect, 0 Host Disconnect
 * @see
 */
int gp_usb_vbus_detect_get(void)
{
	//printk("[%s][%d]\n", __FUNCTION__, __LINE__);
	return gpHalUsbVbusDetect();
}
EXPORT_SYMBOL(gp_usb_vbus_detect_get);

/**
 * @brief   USB Slave Host addressed
 * @param   none
 * @return   1 Host addressed, 0 Host does't enumerate device
 * @see
 */
int gp_usb_host_addred(void)
{
	return gpHalUsbHostAddred();
}
EXPORT_SYMBOL(gp_usb_host_addred);

/**
 * @brief   USB Slave Host Configed
 * @param   none
 * @return   1 Host Configed, 0 Host does't enumerate device
 * @see
 */
int gp_usb_host_configed(void)
{
	//printk("[%s][%d]\n", __FUNCTION__, __LINE__);
	return gpHalUsbHostConfiged();
}
EXPORT_SYMBOL(gp_usb_host_configed);

/**
 * @brief   USB HOST safty remove
 * @param   none
 * @return  host , 0 [not safty remove] or 1 [issue fsaty remove]
 * @see
 */
int gp_usb_host_safty_removed(void)
{
	//printk("[%s][%d]\n", __FUNCTION__, __LINE__);
	return gpHalUsbHostSafyRemoved() | mac_pc_save_remove;
}
EXPORT_SYMBOL(gp_usb_host_safty_removed);

/**
 * @brief   MAC safty remove
 * @param   setting: 1 or 0
 * @return  none
 * @see
 */
void gp_usb_mac_pc_remove_set( int setting )
{
	mac_pc_save_remove = setting;
}
EXPORT_SYMBOL(gp_usb_mac_pc_remove_set);

/**
 * @brief   Romove setting get when none usb detect set.
 * @param   None
 * @return  Remove State
 * @see
 */
int gp_usb_none_detect_remove_get( void )
{
	return gpHalUsbGetNoneDetectRemove();
}
EXPORT_SYMBOL(gp_usb_none_detect_remove_get);

/**
 * @brief   Romove setting set when none usb detect set.
 * @param   setting: 1 or 0
 * @return  none
 * @see
 */
void gp_usb_none_detect_remove_set( int setting )
{
	struct gp_board_usb_s *pConfig = gp_board_get_config("usb", gp_board_usb_t);
	int vbus_config = pConfig->get_slave_vbus_config();
	if( vbus_config == USB_SLAVE_VBUS_NONE )
		gpHalUsbSetNoneDetectRemove(setting);
}
EXPORT_SYMBOL(gp_usb_none_detect_remove_set);

/**
 * @brief   Detect PC's first packet 
 * @param   none
 * @return 	0 [not detect in specific duration] 
 * 			1 [detect pc's setup packet in specific duration]
 *  		0xff [ error usage case reply ]
 * @see
 */
static int gp_usb_detect_first_packet(void)
{
	//printk("[%s][%d]\n", __FUNCTION__, __LINE__);
	return gpHalUsbDetectFirstPacket();
}
EXPORT_SYMBOL(gp_usb_detect_first_packet);

/**
 * @brief   Set for waiting disk was sync.
 * @param   setting [IN]: [1] wait disk sync, [0] not wait.
 * @return 	none.
 * @see
 */
static void gp_usb_disk_sync_wait_set( int setting ){
	if( setting ) {
		usb_wait_disk_sync = setting;
	}
	else{
		usb_wait_disk_sync = 0;
	}
}

/**
 * @brief   Get flag for waiting disk was sync.
 * @param   none
 * @return 	return [1] wait disk sync, [0] not wait.
 * @see
 */
int gp_usb_disk_sync_wait_get( void ){
	return usb_wait_disk_sync;
}
EXPORT_SYMBOL(gp_usb_disk_sync_wait_get);

/**
 * @brief   USB HOST TVID Buf Get
 * @param   pBuf [OUT]: the buffer used to stored the TVID.
 * @return  none
 * @see
 */
void gp_usb_tvid_get(uint8_t *pBuf)
{
	printk("[%s][%d]\n", __FUNCTION__, __LINE__);
	if( pBuf != NULL ) {
		printk("[%s][%d], [%x][%x][%x]\n", __FUNCTION__, __LINE__, 
		pUsbBuf[0], pUsbBuf[1], pUsbBuf[2]);
		memcpy(pBuf, pUsbBuf, 512);
		printk("[%s][%d], [%x][%x][%x]\n", __FUNCTION__, __LINE__, 
		pUsbBuf[0], pUsbBuf[1], pUsbBuf[2]);
	}
}
EXPORT_SYMBOL(gp_usb_tvid_get);

/**
 * @brief   USB HOST TVID Buf Set
 * @param   pBuf [IN]: the buffer used to stored the TVID.
 * @return  none
 * @see
 */
void gp_usb_tvid_set( uint8_t *pBuf )
{
	printk("[%s][%d]\n", __FUNCTION__, __LINE__);
	if( pBuf != NULL ) {
		printk("[%s][%d], [%x][%x][%x]\n", __FUNCTION__, __LINE__, 
		pUsbBuf[0], pUsbBuf[1], pUsbBuf[2]);
		memcpy( pUsbBuf, pBuf, 512);
		printk("[%s][%d], [%x][%x][%x]\n", __FUNCTION__, __LINE__, 
		pUsbBuf[0], pUsbBuf[1], pUsbBuf[2]);
	}
}
EXPORT_SYMBOL(gp_usb_tvid_set);

#if 0
static int gp_usb_fops_release(struct inode *inode, struct file *file)
{
	int phyCurrentConfig = 0, phy1SwConnect = 0;
	phyCurrentConfig = gpHalUsbPhyConfigGet();
	
	printk("[%s][%d]\n", __FUNCTION__, __LINE__);
	if( !((phyCurrentConfig & 0x03) == USB_HOST_PHY1_DEV_NULL) ) {
		printk("Disable SW\n");
		phy1SwConnect = gpHalUsbSlaveSwConnectGet();
		gp_usb_slave_sw_connect(0);
	}
	gp_usb_en( phyCurrentConfig, 0);
	return 0;
}
#endif

void gp_usb_set_device_mode(int type)
{
	usbd_device_type = type;
	//printk("%s: type = %d\n", __func__, type);
	
	if(usbd_device_type == USB_DEVICE_DETECTION_TYPE)
	{
		usbd_connectto = USB_DEVICE_NONE_TYPE;
	}	
}

EXPORT_SYMBOL(gp_usb_set_device_mode);

int gp_usb_get_device_mode(void)
{
	return usbd_device_type;
}

EXPORT_SYMBOL(gp_usb_get_device_mode);

void gp_usb_set_device_soft_connect(int connect)
{
	gpHalUsbDeviceSetConnnect(connect);
}	

EXPORT_SYMBOL(gp_usb_set_device_soft_connect);

void gp_usb_set_device_connectto(int connectto)
{
	usbd_connectto = connectto;
	//printk("%s: usbd_connectto = %d\n", __func__, usbd_connectto);
}	

EXPORT_SYMBOL(gp_usb_set_device_connectto);

int gp_usb_get_device_connectto(void)
{
	return usbd_connectto;
}	

EXPORT_SYMBOL(gp_usb_get_device_connectto);

/**
 * @brief   usb device ioctl function
 */
static long gp_usb_fops_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	long ret = 0;
	//int handle;
	//usb_content_t ctx;
	usb_content_t usb_content;
	uint32_t delay_time;
	uint32_t vbus_detect = 0;
	uint32_t host_config = 0;
	uint32_t safty_remove = 0;
	uint32_t first_packet = 0;	
	uint32_t wait_disk_sync = 0;
	char* usbDiskPlugStatus = NULL;
	struct gp_board_usb_s *pConfig = gp_board_get_config("usb", gp_board_usb_t);
	//unsigned int buf_address;
	//unsigned int* pBuf;
	uint32_t uvc_state;
	gp_uvc_parameter_t uvc_param;
	unsigned int physical_addr;
	

	switch (cmd) {
	case USBDEVFS_PHY_SELECT_SET:
		if (copy_from_user(&usb_content, (void __user*)arg, sizeof(usb_content_t))) {
			ret = -EFAULT;
			break;
		}
		printk("[%s][%d] USBPHY_SELECT [%x]\n", __FUNCTION__, __LINE__, usb_content.enable);
		if( usb_content.enable == USB_PHY1_SLAVE) {
			printk("Disable Host Power\n");
			pConfig->set_power(1);
		}
		else if ( usb_content.enable == USB_PHY1_HOST ) {
			printk("Diable SW connect\n");
			gp_usb_slave_sw_connect(0);
		}

		gp_usb_phy1_config(usb_content.enable);

		if( usb_content.enable == USB_PHY1_SLAVE) {
			printk("Enable SW connect\n");
			gp_usb_slave_sw_connect(1);
		}
		else if ( usb_content.enable == USB_PHY1_HOST ) {
			printk("Enable Host Power\n");
			pConfig->set_power(0);
		}
		//printk("USBPHY_SELECT END\n");
		break;

	case USBDEVFS_SW_CONNECT_SET:
//		printk("[%s][%d] USBSW_CONNECT\n", __FUNCTION__, __LINE__);
		if (copy_from_user(&usb_content, (void __user*)arg, sizeof(usb_content))) {
			ret = -EFAULT;
			break;
		}
		//printk("[%s][%d] USBSW_CONNECT [%x]\n", __FUNCTION__, __LINE__, usb_content.enable);
		gp_usb_slave_sw_connect(usb_content.enable);
		//printk("[%s][%d] USBSW_CONNECT\n", __FUNCTION__, __LINE__);
		break;

	case USBDEVFS_TVID_SET:
		printk("[%s][%d] USBTVID_SET\n", __FUNCTION__, __LINE__);
		if (copy_from_user(pUsbBuf, (void __user*)arg, 512)) {
			ret = -EFAULT;
			break;
		}
		printk("[%s][%d] USBTVID_SET Buf[%x], 0x[%x], 0x[%x]\n", __FUNCTION__, __LINE__, (uint32_t) pUsbBuf, pUsbBuf[0], pUsbBuf[1]);
		break;

	case USBDEVFS_TVID_GET:
		printk("[%s][%d] USBTVID_GET\n", __FUNCTION__, __LINE__);
		copy_to_user ((void __user *) arg, (const void *)pUsbBuf, 512);
		printk("[%s][%d] USBTVID_GET \n", __FUNCTION__, __LINE__);
		break;

	case USBDEVFS_DISK_PLUG_STATUS_GET:
		usbDiskPlugStatus = usb_device_connect_state_get();
		copy_to_user ((void __user *) arg, (int *)usbDiskPlugStatus, 16);
		break;

	case USBDEVFS_VBUS_HIGH:
		vbus_detect = pConfig->slave_detect();
		//printk("[%s][%d] USBDEVFS_VBUS_HIGH [%x]\n", __FUNCTION__, __LINE__, vbus_detect);
		copy_to_user ((void __user *) arg, (const void *) &vbus_detect, sizeof(uint32_t));
		break;

	case USBDEVFS_HOST_CONFIGED:
		host_config = gp_usb_host_configed();
		//printk("[%s][%d] USBDEVFS_HOST_CONFIGED [%x]\n", __FUNCTION__, __LINE__, host_config);
		copy_to_user ((void __user *) arg, (const void *) &host_config, sizeof(uint32_t));
		break;

	case USBDEVFS_HOST_EYE_TEST:
		printk("[%s][%d] USB_EYE_TEST\n", __FUNCTION__, __LINE__);
		if (copy_from_user(&usb_content, (void __user*)arg, sizeof(usb_content_t))) {
			ret = -EFAULT;
			break;
		}
		if( usb_content.enable == USB_TESTMODE_EYE20) {
			printk("USB EYE Pattern 2.0\n");
			gpHalUsbPGSet( 0, 1 );
			gpHalUsbPGSet( 1, 1 );
		}
		else if( usb_content.enable == USB_TESTMODE_EYE11 ) {
			printk("USB EYE Pattern 1.1\n");
			gpHalUsbPGSet( 0, 4 );
			gpHalUsbPGSet( 1, 4 );
		}
		printk("[%s][%d] USB_EYE_TEST FINISH\n", __FUNCTION__, __LINE__);
		break;

	case USBDEVFS_HOST_SAFTY_REMOVED:
		safty_remove = gp_usb_host_safty_removed();
		//printk("[%s][%d] USBDEVFS_HOST_SAFTY_REMOVED [%x]\n", __FUNCTION__, __LINE__, safty_remove);
		copy_to_user ((void __user *) arg, (const void *) &safty_remove, sizeof(uint32_t));
		break;

	case USBDEVFS_SET_DELAY_TIME:
		if (copy_from_user(&delay_time, (void __user*)arg, sizeof(uint32_t))) {
			ret = -EFAULT;
			break;
		}
		//printk("USBSW_SET_DELAY_TIME [%d]\n",delay_time);
		gp_usb_set_delay_time(delay_time);
		break;

	case USBDEVFS_DETECT_FIRST_PACKET:
		first_packet = gp_usb_detect_first_packet();
		printk("USBDEVFS_DETECT_FIRST_PACKET [%x]\n",first_packet);
		copy_to_user ((void __user *) arg, (const void *) &first_packet, sizeof(uint32_t));
		break;

	case USBDEVFS_WAIT_DISK_SYNC:
		if (copy_from_user(&wait_disk_sync, (void __user*)arg, sizeof(uint32_t))) {
			ret = -EFAULT;
			break;
		}
		gp_usb_disk_sync_wait_set(wait_disk_sync);
		printk("USBDEVFS_WAIT_DISK_SYNC.[%x]\n",wait_disk_sync);
		break;

    case USBDEVFS_HOST_ADDRED:
        host_config = gp_usb_host_addred();
		//printk("[%s][%d] USBDEVFS_HOST_ADDRED [%x]\n", __FUNCTION__, __LINE__, host_config);
		copy_to_user ((void __user *) arg, (const void *) &host_config, sizeof(uint32_t));
		break;
		
	case USBDEVFS_GET_CONNECT_TYPE:
		if (copy_to_user((void __user*)arg, &usbd_connectto, sizeof(uint32_t))) {
			ret = -EFAULT;
			break;
		}
    	break;
	
	case USBDEVFS_SET_UVC_PARAM:
		if (copy_from_user(&uvc_param, (void __user*)arg, sizeof(gp_uvc_parameter_t))) {
			ret = -EFAULT;
			break;
		}
		if(uvc_param.buf)
		{
			physical_addr = gp_user_va_to_pa((void *)uvc_param.buf);
			//printk("Translate 0x%x to physical address 0x%x, len = 0x%x\n", (int)uvc_param.buf, physical_addr, uvc_param.len);
		}	
		if(uvc_ops_handler != NULL)
				uvc_ops_handler->send_video_frame((void*)physical_addr, uvc_param.len);	
    	break;
    	
    case USBDEVFS_SET_UVC_AUDIO_PARAM:
		if (copy_from_user(&uvc_param, (void __user*)arg, sizeof(gp_uvc_parameter_t))) {
			ret = -EFAULT;
			break;
		}
		if(uvc_param.buf)
		{
			physical_addr = gp_user_va_to_pa((void *)uvc_param.buf);
			//printk("Translate 0x%x to physical address 0x%x, len = 0x%x\n", (int)uvc_param.buf, physical_addr, uvc_param.len);
		}		
		if(uvc_ops_handler != NULL)
				uvc_ops_handler->send_audio_data((void*)physical_addr, uvc_param.len);
    	break;	

	case USBDEVFS_GET_UVC_STATE:
		if(uvc_ops_handler != NULL)
			uvc_state = uvc_ops_handler->get_uvc_state();
		else
			uvc_state = USB_UVC_NOT_READY_STATE;	
			
		if (copy_to_user((void __user*)arg, &uvc_state, sizeof(uint32_t))) {
			ret = -EFAULT;
			break;
		}
    	break;    
	default:
		ret = -ENOTTY; /* Inappropriate ioctl for device */
		break;
	}

	return ret;
}



/**
 * @brief   USB enable 
 * @param   phyConfig [IN]: config USB PHY 
 * 2'b00: PHY0 Host PHY1 Device 2'b01: PHY0 Disable PHY1 Device 
 * 2'b10: PHY0 Device PHY1 Host 2'b11: PHY0 Disable PHY1 Host 
 * @param   en [IN]: enable
 * @return  none
 * @see
 */
void gp_usb_en(int phyConfig, int en)
{
	if ( en ) {
        /*Warning!Don't disable clock for HOST initialize. Allen.Chang@generalplus.com
		gpHalScuUsbPhyClkEnable(0);*/

		if( phyConfig == USB_HOST_PHY0_DEV_PHY1) {
			printk("Enable PHY CLK\n");
			gpHalScuUsbPhyClkEnable(1);
			printk("Resume PHY\n");
			gpHalUsbPhyPowerControlSet(0, 0x0);
			gpHalUsbPhyPowerControlSet(1, 0x0);
		}
		else if( phyConfig == USB_HOST_NULL_DEV_PHY1) {
			printk("Disable USB PHY/Clk\n");
			gpHalScuUsbPhyClkEnable(0);
			gpHalUsbPhyPowerControlSet(1, 0x01);
		}
		else if( phyConfig == USB_HOST_PHY1_DEV_NULL ){
			printk("Enable PHY CLK\n");
			gpHalScuUsbPhyClkEnable(1);
			printk("Resume PHY\n");
			gpHalUsbPhyPowerControlSet(1, 0x0);
		}
		printk("Reset Config\n");
		gpHalUsbPhyConfigSet(phyConfig);
	}
	else {
		/*Force PHY Suspend*/
		if( gpHalUsbPhyConfigGet() != USB_HOST_PHY0_DEV_PHY1) {
			printk("Disable PHY setting\n");
			gpHalUsbPhyConfigSet(0x0);
			gpHalUsbPhyPowerControlSet(0, 0x1);
			gpHalUsbPhyPowerControlSet(1, 0x1);
			printk("Disable PHY CLK\n");
			gpHalScuUsbPhyClkEnable(0);
			printk("Disable USB0/USB1 CLK\n");
			gpHalScuClkEnable( SCU_A_PERI_USB1, SCU_A, 0);
			gp_enable_clock((int*)"SYS_A", 0);
		}
		else{
			printk("Host is working!\n");
			gpHalUsbPhyPowerControlSet(1, 0x1);
			printk("Disable USB DEVICE CLK\n");
			gpHalScuClkEnable(SCU_A_PERI_USB1, SCU_A, 0);
		}
	}
	return;
}
EXPORT_SYMBOL(gp_usb_en);

/**
 * @brief   USB force phy enable 
 * @param   en [IN]: enable or disable
 * @return  none
 */
static void gp_usb_phy_froce_en(unsigned int en)
{
	if(en)
	{
		gpHalScuUsbPhyClkEnable(1);
		gpHalUsbPhyPowerControlSet(0, 0x3);
		gpHalUsbPhyPowerControlSet(1, 0x3);	
	}
	else
	{
		gpHalUsbPhyPowerControlSet(0, 0x0);
		gpHalUsbPhyPowerControlSet(1, 0x0);	
	}
}

static void gp_usb_device_release(struct device * dev)
{
	//printk("[%s][%d]\n", __FUNCTION__, __LINE__);
}

static struct platform_device gp_usb_device = {
	.name	= "gp-usb",
	.id	= -1,
	.dev		= {
		.release = &gp_usb_device_release
	},
};


#ifdef CONFIG_PM
static unsigned int phyCurrentConfig = 0;
static unsigned int phy1SwConnect = 0;
static int gp_usb_suspend(struct platform_device *pdev, pm_message_t state){

//#ifdef CONFIG_PM_GPFB
#ifdef CONFIG_PM_SPEEDY
	/* GPFB_TBD */
	if (pm_device_down)
		return 0;
#endif
	//printk("[%s][%d]\n", __FUNCTION__, __LINE__);
	phyCurrentConfig = gpHalUsbPhyConfigGet();

	if( !((phyCurrentConfig & 0x03) == USB_HOST_PHY1_DEV_NULL) ) {
		printk("Disable SW\n");
		phy1SwConnect = gpHalUsbSlaveSwConnectGet();
		gp_usb_slave_sw_connect(0);
	}
	gp_usb_en( phyCurrentConfig, 0);

	return 0;
}

static int gp_usb_resume(struct platform_device *pdev){

//#ifdef CONFIG_PM_GPFB
#ifdef CONFIG_PM_SPEEDY
	/* GPFB_TBD */
	if (pm_device_down)
		return 0;
#endif

	//printk("[%s][%d][%x]\n", __FUNCTION__, __LINE__, phyCurrentConfig);
	gp_usb_en( phyCurrentConfig, 1);
	if( !((phyCurrentConfig & 0x03) == USB_HOST_PHY1_DEV_NULL) ) {
		/*Delay is needed for resume process.*/
		msleep(1);
		printk("Set SW to [%x]\n", phy1SwConnect);
		gp_usb_slave_sw_connect(phy1SwConnect);
	}
	return 0;
}
#else
#define gp_usb_suspend NULL
#define gp_usb_resume NULL
#endif

/**
 * @brief   wdt driver define
 */
static struct platform_driver gp_usb_driver = {
	//.probe	= gp_usb_control_probe,
	//.remove	= gp_usb_control_remove,
	.suspend = gp_usb_suspend,
	.resume = gp_usb_resume,
	.driver	= {
		.owner	= THIS_MODULE,
		.name	= "gp-usb"
	},
};


/**
 * @brief   Get default config from configuration making.
 * @return  0: PHY0 Host PHY1 Device 
 * 1: PHY0 Disable PHY1 Device 
 * 3: PHY0 Disable PHY1 Host
 * @see
 */
static int gp_usb_default_phy_config_get ( void ){
	struct gp_board_usb_s *pConfig = gp_board_get_config("usb", gp_board_usb_t);
	int phy0_config = pConfig->phy0_func_en_get();
	/*0: phy0 host, 1: phy0 disable*/
	int phy1_config = pConfig->phy1_func_sel_get();
	/*0: phy1 host, 1: phy1 slave, 2: phy1 host/slave, 3 phy1 disable*/
	//int host_speed_sel = pConfig->get_host_speed();
	int phyConfig = 0;
	if( phy0_config == PHY0_HOST ) {
		printk("PHY0 -> HOST\n");
	}
	else if( phy0_config == PHY0_DISABLE ){
		printk("PHY0 -> DISABLE\n");
	}
	if( phy1_config == PHY1_HOST || phy1_config == PHY1_HOST_SLAVE ) {
		printk("PHY1 -> HOST or HOST/SLAVE\n");
		phyConfig = USB_HOST_PHY1_DEV_NULL;
	}
	else if( phy1_config == PHY1_SLAVE ) {
		printk("PHY1 -> SLAVE\n");
		if( phy0_config == PHY0_HOST ) {
			phyConfig = USB_HOST_PHY0_DEV_PHY1;
		}
		else{
			phyConfig = USB_HOST_NULL_DEV_PHY1;
		}
	}
	else if( phy1_config == PHY1_DISABLE ) {
		printk("PHY1 -> DISABLE\n");
		phyConfig = USB_HOST_PHY0_DEV_PHY1;
	}
	if( phy0_config && (phy1_config == 0 || phy1_config == 2) ) {
		printk("Attention! Host is in PHY1 now. Please change HOST to PHY0 by manual set.\n");
	}
	return phyConfig;
}

static int gp_usb_intI2c_DataWrite (UINT32 slaveAddr, u8 *buf, int len) {
		UINT32 ret = 0, i = 0;
		int hd = 0;

		hd = gp_i2c_internal_request(slaveAddr, 0x0f);
		if(IS_ERR_VALUE(ret)){
			printk(KERN_ALERT "err gp_i2c_bus_request\n");
		}
	
		for(i=0; i<1; i++){
			gp_i2c_internal_write(hd, (unsigned char *)&buf[i], len);
		}
		gp_i2c_internal_release(hd);
		return SUCCESS;
}

#define PHY0_I2C_ID 0x08
#define PHY1_I2C_ID 0x0a
#define VOLTAGE_LEVEL_CHANGE_ADDRESS 0x0d

static int usb_voltage_level_change( int physelect, int level ) {
	int err;
	int phy_id_select = PHY0_I2C_ID;
	UINT8 buf[8] = {0};

	buf[0] = VOLTAGE_LEVEL_CHANGE_ADDRESS;	
	buf[1] = 0xa0 + level;	
		
	if ( physelect == 0 ) {
		phy_id_select = PHY0_I2C_ID;
	}
	else {
		phy_id_select = PHY1_I2C_ID;	
	}
 	printk("Voltage Up-PHY[%d]level[%x]\n", physelect, level);
	err = gp_usb_intI2c_DataWrite(phy_id_select, buf, 2);
 	return 0;
}

static int32_t  __init gp_usb_init(void)
{
	int ret = -ENXIO;
	int phyDefaultConfig = 0;
	struct gp_board_usb_s *pConfig = gp_board_get_config("usb", gp_board_usb_t);
	int phy0voltageUpValue = (pConfig->get_phy0_voltage_up_config != NULL) ? (pConfig->get_phy0_voltage_up_config()) : 0; 
	int phy1voltageUpValue = (pConfig->get_phy1_voltage_up_config != NULL) ? (pConfig->get_phy1_voltage_up_config()) : 0; 

	static int gpioHandle = 0;
	int pull_level;
	unsigned int value = 0;

	pUsbBuf = kmalloc (512, GFP_KERNEL);
	usb = (usb_info_t *)kzalloc(sizeof(usb_info_t), GFP_KERNEL);
	if (usb == NULL) {
		ret = -ENOMEM;
		DIAG_ERROR("gpio kmalloc fail\n");
		goto fail_kmalloc;
	}
	
	gp_usb_phy_froce_en(1);
	
	if(  phy0voltageUpValue != 0 ) {
		usb_voltage_level_change( 0, phy0voltageUpValue);
	}
	if(  phy1voltageUpValue != 0 ) {
		usb_voltage_level_change( 1, phy1voltageUpValue);
	}
	
	gp_usb_phy_froce_en(0);
	
	phyDefaultConfig = gp_usb_default_phy_config_get();
	gp_usb_en( phyDefaultConfig, 1);
	/* register device */
	usb->dev.name  = "usb_device";
	usb->dev.minor = MISC_DYNAMIC_MINOR;
	usb->dev.fops  = &usb_device_fops;
	ret = misc_register(&usb->dev);
	if (ret != 0) {
		printk("usb misc device register fail\n");
		goto fail_device_register;
	}
	
	/* Init VBUS detection PIN */
	gpioHandle = gp_gpio_request(pConfig->get_slave_vbus_config(), "spmp_udc");	
	pull_level = pConfig->get_slave_gpio_power_level();
	ret = gp_gpio_set_input(gpioHandle, pull_level);
	ret = gp_gpio_get_value(gpioHandle, &value);
	gp_gpio_release(gpioHandle);
	gpioHandle = 0;

	//sema_init( &sem, 0);
	platform_device_register(&gp_usb_device);
	platform_driver_register(&gp_usb_driver);
	return 0;

fail_device_register:
	kfree(usb);
	usb = NULL;
fail_kmalloc:
	return ret;
}

static void __exit gp_usb_exit(void)
{
	int ret = -ENXIO;
	int phyCurrentConfig = 0;
	phyCurrentConfig = gpHalUsbPhyConfigGet();
	gp_usb_en(phyCurrentConfig, 0);
	//gp_usb_host_en(0);
	if( pUsbBuf != NULL ) {
		kfree(pUsbBuf);
	}
	ret = misc_deregister(&usb->dev);
	if (ret != 0) {
		printk("usb misc device deregister fail\n");
	}
	kfree(usb);
	usb = NULL;

	platform_device_unregister(&gp_usb_device);
	platform_driver_unregister(&gp_usb_driver);
}

/* Declaration of the init and exit functions */
module_init(gp_usb_init);
module_exit(gp_usb_exit);

MODULE_AUTHOR("Generalplus");
MODULE_DESCRIPTION("GP USB Driver");
MODULE_LICENSE_GP;
