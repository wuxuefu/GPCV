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
 * @file    gp_usb_vendor.h
 * @brief   Declaration of usb vendor command.
 * @author  Dunker Chen
 */
#ifndef _GP_USB_VENDOR_H_
#define _GP_USB_VENDOR_H_

/**************************************************************************
 *                           C O N S T A N T S                            *
**************************************************************************/

#define GP_VENDOR_CMD_NUM	512

/**************************************************************************
 *                              M A C R O S                               *
**************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
**************************************************************************/

struct usb_vendor_ops {
	char *name;
	void *priv;
	int	(*get_scsi)(void* priv, void *scsi, unsigned char dir, unsigned int data_size, unsigned int * act_data_size);
	int	(*set_csw)(void* priv, unsigned int *sense);
	int	(*read) (void* priv, void* buf, unsigned int ln);
	int	(*write) (void* priv, void* buf, unsigned int ln);
};

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
**************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
**************************************************************************/

/**
* @brief 	Register GP vendor command function.
* @param	cmd[in]: Vendor command number. 0~511
* @param 	ops[in]: Vendor command operation function pointer.
* @return 	0 for success, others number means fail.
*/ 
extern int gp_usb_register_vendor(unsigned int cmd, struct usb_vendor_ops *ops);

/**
* @brief 	Unregister GP vendor command function.
* @param	cmd[in]: Vendor command number. 0~511
* @return 	None.
*/ 
extern void gp_usb_unregister_vendor(unsigned int cmd);

/**
* @brief 	List all vendor command.
* @return 	None.
*/ 
extern void gp_usb_vendor_list(void);

#endif /*_GP_USB_VENDOR_H_ */