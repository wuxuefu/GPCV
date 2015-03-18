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
 * @file    ehset.c
 * @brief   Implement of usb Embedded Host High-Speed Electrical Test (EHSET) test fixture.
 * @author  Dunker Chen
 */

#include <mach/kernel.h>
#include <mach/module.h>
#include <linux/usb.h>
#include <linux/usb/ch11.h>
#include <mach/gp_i2c_bus.h>
#include <mach/hal/hal_usb.h>

/**************************************************************************
 *                           C O N S T A N T S                            *
**************************************************************************/

#define		IF_TEST_VID						0x1A0A

#define		TEST_SE0_NAK					0x0101
#define		TEST_J							0x0102
#define		TEST_K							0x0103
#define		TEST_PACKET						0x0104
#define		HS_HOST_PORT_SUSPEND_RESUME		0x0106
#define		SINGLE_STEP_GET_DEV_DESC		0x0107
#define		SINGLE_STEP_SET_FEATURE			0x0108

#define PHY0_I2C_ID							0x08
#define PHY1_I2C_ID							0x0a
#define TEST_PATTERN_GENERAL_ADDRESS		0x04
#define TEST_J_SET							0xa9
#define TEST_K_SET							0xaa

/**************************************************************************
 *                              M A C R O S                               *
**************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
**************************************************************************/

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
**************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
**************************************************************************/

static int gp_if_probe(struct usb_interface *intf, const struct usb_device_id *id);
static void gp_if_disconnect(struct usb_interface *intf);

/**************************************************************************
 *                         G L O B A L    D A T A                         *
**************************************************************************/

static struct usb_device_id usb_if_ids[] = {
	{ USB_DEVICE( IF_TEST_VID, TEST_SE0_NAK ) },
	{ USB_DEVICE( IF_TEST_VID, TEST_J ) },
	{ USB_DEVICE( IF_TEST_VID, TEST_K ) },
	{ USB_DEVICE( IF_TEST_VID, TEST_PACKET ) },
	{ USB_DEVICE( IF_TEST_VID, HS_HOST_PORT_SUSPEND_RESUME ) },
	{ USB_DEVICE( IF_TEST_VID, SINGLE_STEP_GET_DEV_DESC ) },
	{ USB_DEVICE( IF_TEST_VID, SINGLE_STEP_SET_FEATURE ) },
	{}
};

static struct usb_driver usb_if_driver = {
	.name =		"GP EHSET",
	.probe =	gp_if_probe,
	.disconnect =	gp_if_disconnect,
	.id_table =	usb_if_ids,
};

/**************************************************************************
 *             F U N C T I O N    I M P L E M E N T A T I O N S           *
**************************************************************************/

/**
* @brief 	Wait for scope.
* @param	counter[in]: Timer counter in second.
* @return 	None.
*/ 
static void gp_usb_wait_scope ( int counter )
{
	counter ++;
	while( counter --)
	{                      
		printk( KERN_INFO "WaitScope, counter = %d\n", counter);
		msleep(1000);
	}	
}

/**
* @brief 	Setting USB PHY by I2C.
* @param	phy[in]: USB PHY number.
* @param	buf[in]: Buffer pointer.
* @param	len[in]: Buffer Length.
* @return 	None.
*/ 
static void gp_usb_i2c_write ( int phy, unsigned char *buf, int len ) 
{
	int hd = 0;
	unsigned int slaveAddr = ( phy == 0 ) ? PHY0_I2C_ID : PHY1_I2C_ID;
	
	hd = gp_i2c_bus_request( slaveAddr, 0x0f );
	
	if(IS_ERR_VALUE(hd))
	{
		printk(KERN_ALERT "err gp_i2c_bus_request\n");
		return ;
	}
	
	gp_i2c_bus_write( hd, buf, len );
	
	gp_i2c_bus_release( hd );
}

/**
* @brief 	Check USB PHY number.
* @param	udev[in]: USB device.
* @return 	USB PHY number.
*/ 
static int gp_check_phy( struct usb_device *udev )
{
	if( gpHalUsbHostConfigGet() == 0 )
	{
		printk(KERN_INFO "HOST PHY 0\n");
		return 0;
	}
	else if( gpHalUsbHostConfigGet() == 1 )
	{
		printk(KERN_INFO "HOST PHY 1\n");
		return 0;
	}
	else
		return -1;
}

/**
* @brief 	Driver probe function.
* @param	intf[in]: USB interface structure.
* @param	id[in]: USB device id.
* @return 	SUCCESS/ERROR_ID(negative value).
*/ 
static int gp_if_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
	struct usb_device *udev = interface_to_usbdev(intf);
	int phy = gp_check_phy(udev);
	struct usb_device *hub_udev = udev->parent;
	int port = udev->portnum;
	int status = 0;
	unsigned char buf[2];
	struct usb_port_status port_status;
	
	if( phy < 0 )
		return -1;
	
	switch (id->idProduct)	
	{
		case TEST_SE0_NAK:
			printk( KERN_INFO "TEST_SE0_NAK\n");
			gpHalUsbPGSet( phy, 7 );
			break;
		case TEST_J:
			printk( KERN_INFO "TEST_J\n");
			//gpHalUsbPGSet( phy, 9 );			// This pattern can't use pattern generate.
			buf[0] = TEST_PATTERN_GENERAL_ADDRESS;	
			buf[1] = TEST_J_SET;
			gp_usb_i2c_write( phy, buf, 2);
			break;
		case TEST_K:
			printk( KERN_INFO "TEST_K\n");
			//gpHalUsbPGSet( phy, 10 );			// This pattern can't use pattern generate.
			buf[0] = TEST_PATTERN_GENERAL_ADDRESS;	
			buf[1] = TEST_K_SET;
			gp_usb_i2c_write( phy, buf, 2);
			break;
		case TEST_PACKET:
			printk( KERN_INFO "TEST_PACKET high speed\n");
			gpHalUsbPGSet( phy, 1 );
			break;
		case HS_HOST_PORT_SUSPEND_RESUME:
			printk( KERN_INFO "HS_HOST_PORT_SUSPEND_RESUME\n");
			gp_usb_wait_scope( 15 );
			status = usb_control_msg(hub_udev, usb_sndctrlpipe(hub_udev, 0),
						USB_REQ_SET_FEATURE, USB_RT_PORT,
						USB_PORT_FEAT_SUSPEND, port, NULL, 0, 1000);
			if (status < 0)
				break; 
			gp_usb_wait_scope( 15 );
			status = usb_control_msg(hub_udev, usb_rcvctrlpipe(hub_udev, 0),
						USB_REQ_GET_STATUS, USB_DIR_IN | USB_RT_PORT, 0, port,
						&port_status, sizeof(port_status), 1000);
			if (status < 0)
				break; 	
			
			if( port_status.wPortStatus & USB_PORT_STAT_SUSPEND )
			{
				status = usb_control_msg(hub_udev, usb_sndctrlpipe(hub_udev, 0),
						USB_REQ_CLEAR_FEATURE, USB_RT_PORT,
						USB_PORT_FEAT_SUSPEND, port, NULL, 0, 1000);
						
				if (status < 0)
				{
					printk( KERN_INFO "can't resume port\n");
					break;
				} 		
				msleep(100);
				status = usb_control_msg(hub_udev, usb_rcvctrlpipe(hub_udev, 0),
						USB_REQ_GET_STATUS, USB_DIR_IN | USB_RT_PORT, 0, port,
						&port_status, sizeof(port_status), 1000);
			
				if( status>=0 && (port_status.wPortChange & USB_PORT_STAT_C_SUSPEND) )
				{
					status = usb_control_msg(hub_udev, usb_sndctrlpipe(hub_udev, 0),
							USB_REQ_CLEAR_FEATURE, USB_RT_PORT,
							USB_PORT_FEAT_C_SUSPEND, port, NULL, 0, 1000);	
				}
				else
				{
					printk( KERN_INFO "Not in resume state, change status = 0x%x\n", port_status.wPortChange );
					break;		
				}
			}
			else
			{
				printk( KERN_INFO "Not in spsend state, status = 0x%x\n", port_status.wPortStatus );
				break;		
			}
			break;
		case SINGLE_STEP_GET_DEV_DESC:
			printk( KERN_INFO "SINGLE_STEP_GET_DEV_DESC\n");
			gp_usb_wait_scope( 15 );
			{
				struct usb_device_descriptor *buf;
				buf = kmalloc(USB_DT_DEVICE_SIZE, GFP_KERNEL);
				if (!buf)
					return -ENOMEM;
				status = usb_control_msg(udev, usb_rcvctrlpipe(udev, 0),
							USB_REQ_GET_DESCRIPTOR, USB_DIR_IN,
							USB_DT_DEVICE << 8, 0,
							buf, USB_DT_DEVICE_SIZE,
							USB_CTRL_GET_TIMEOUT);
				kfree(buf);
			}  
			break;	
		case SINGLE_STEP_SET_FEATURE:
			/*
			 * GetDescriptor SETUP request -> 15secs delay -> IN & STATUS
			 *
			 * Note, this test is only supported on root hubs since the
			 * SetPortFeature handling can only be done inside the HCD's
			 * hub_control callback function.
			 */
			printk( KERN_INFO "SINGLE_STEP_SET_FEATURE\n");
			if (hub_udev != udev->bus->root_hub) 
			{
				printk( KERN_INFO  "SINGLE_STEP_SET_FEATURE test only supported on root hub\n");
				break;
			}
	
			status = usb_control_msg(hub_udev, usb_sndctrlpipe(hub_udev, 0),
						USB_REQ_SET_FEATURE, USB_RT_PORT,
						USB_PORT_FEAT_TEST,
						(6 << 8) | port,
						NULL, 0, 60 * 1000);
			break;
		default:
 			printk( KERN_INFO "Unsupported PID: 0x%x\n", id->idProduct);
			break;
	}
	return status;
}

/**
* @brief 	Driver disconnect function.
* @param	intf[in]: USB interface structure.
* @return 	None.
*/
static void gp_if_disconnect(struct usb_interface *intf)
{
	struct usb_device *udev = interface_to_usbdev(intf);
	printk( KERN_INFO "PID 0x%04x disconnect\n", udev->descriptor.idProduct);
}

/**
* @brief 	EHSET driver initial function.
* @return 	SUCCESS/ERROR_ID(negative value).
*/
static int __init gp_if_init(void)
{
	int retval;

	retval = usb_register(&usb_if_driver);
	if (retval == 0) {
		printk(KERN_INFO "GP USB IF TEST driver registered.\n");
	}
	return retval;
}

/**
* @brief 	EHSET driver exit function.
* @return 	None.
*/
static void __exit gp_if_exit(void)
{
	usb_deregister(&usb_if_driver) ;
}

module_init(gp_if_init);
module_exit(gp_if_exit);

/**************************************************************************
 *                  M O D U L E    D E C L A R A T I O N                  *
**************************************************************************/

MODULE_AUTHOR("Generalplus");
MODULE_DESCRIPTION("GP USB EHSET Driver");
MODULE_LICENSE_GP;