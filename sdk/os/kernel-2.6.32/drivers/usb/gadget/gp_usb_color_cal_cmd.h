/**************************************************************************
 *                                                                        *
 *         Copyright (c) 2014 by Generalplus Inc.                         *
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
 *  No.19, Industry E. Rd. IV, Hsinchu Science Park                       *
 *  Hsinchu City 30078, Taiwan, R.O.C.                                    *
 *                                                                        *
 **************************************************************************/
 /**
 * @file    gp_usb_color_cal_cmd.h
 * @brief   Declaration of usb vendor command for color calibration.
 * @author  
 */
 
#ifndef _GP_USB_COLOR_CAL_CMD_H_
#define _GP_USB_COLOR_CAL_CMD_H_

/**************************************************************************
 *                           C O N S T A N T S                            *
**************************************************************************/
#define CMD_SHOW_COLOR		0xFE80
#define CMD_KEY_INIT		0xFE81
#define CMD_KEY_PROTECT		0xFE82
#define CMD_UI_START		0xFE83
#define CMD_REG_READ		0xFD28
#define CMD_REG_WRITE		0xFD2A

#define COLOR_CAL_SCSI_INQUIRY_LEN	44
/**************************************************************************
 *                              M A C R O S                               *
**************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
**************************************************************************/

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
**************************************************************************/
extern u8 SCSIInquiryDataForColorCal[];
/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
**************************************************************************/
extern int is_spyder_vendor_cmd(int cmd);
extern int do_gp_color_cal(struct fsg_dev *fsg, struct fsg_buffhd *bh);

#endif