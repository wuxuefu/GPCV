
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
#ifndef __GSENSOR_KXTF9_H__
#define __GSENSOR_KXTF9_H__

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/
/* gsensor id */
#define SLAVE_ADDR	0x2A
#define I2C_FREQ	30

/*  gsensor register */
#define CMD_RD_X	0x00
#define CMD_RD_Y	0x01
#define CMD_RD_STATE	0x02
#define CMD_WR_DET	0x04
#define CMD_RD_ID	0x08

#define GSENSOR_IOCTL_ID	'G'
#define GSENSOR_IOCTL_G_X	_IOR(GSENSOR_IOCTL_ID, 0x01, int)
#define GSENSOR_IOCTL_G_Y	_IOR(GSENSOR_IOCTL_ID, 0x02, int)
#define GSENSOR_IOCTL_G_STATE	_IOR(GSENSOR_IOCTL_ID, 0x03, int)
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

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/

#endif
