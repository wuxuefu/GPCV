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
 * @file    gp_usb_vendor_cmd.h
 * @brief   Declaration of usb vendor command.
 * @author  
 */
 
#ifndef _GP_USB_VENDOR_CMD_H_
#define _GP_USB_VENDOR_CMD_H_

/**************************************************************************
 *                           C O N S T A N T S                            *
**************************************************************************/

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

/**
* @brief 	Check vendor command function.
* @param	fsg[in]: fsg device struct pointer.
* @return 	0 for success, others number means fail.
*/ 
extern int gp_usb_vendor_check_cmd(struct fsg_dev *fsg);

/**
* @brief 	Vendor process function.
* @param	fsg[in]: fsg device struct pointer.
* @return 	0 for success, others number means fail.
*/ 
extern int do_gp_vendor(struct fsg_dev *fsg);

/**
* @brief 	Vendor command 0xf0f0.
* @param	fsg[in]: fsg device struct pointer.
* @param	bh[in]: fsg buffer struct pointer.
* @return 	0 for success, others number means fail.
*/ 
extern int do_gp_setvid(struct fsg_dev *fsg, struct fsg_buffhd *bh);
#endif