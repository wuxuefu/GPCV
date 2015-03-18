/**************************************************************************
 *                                                                        *
 *         Copyright (c) 2013 by Generalplus Inc.                         *
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
 *  No.19, Industry E. Rd. IV, Hsinchu Science Park,                      *
 *  Hsinchu City 30077, Taiwan, R.O.C.                                    *
 *                                                                        *
 **************************************************************************/
/**
 * @file hdmi.h
 * @brief HDMI driver header file
 * @author
 */

#ifndef _GP_HDMI_DEVICE_H_
#define _GP_HDMI_DEVICE_H_


/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/
//HDMI Timing Number----------------------------------------------------------------
enum {
	HDMI_640X480P60 = 0,
	HDMI_720X480P60,
	HDMI_1280X720P60,
	HDMI_1920X1080I60,
	HDMI_720X480I60,
	HDMI_1920X1080P60,
	HDMI_720X576P50,
	HDMI_1280X720P50,
	HDMI_1920X1080I50,
	HDMI_720X576I50,
	HDMI_1920X1080P50,
	HDMI_VIDEO_TIMING_NUM
};
//End------------------------------------------------------------------------------

//HDMI Packet Types------------------------------------------------------------------
//#define NULL									0x00
#define HDMI_ACR_PACKET					0x01
#define HDMI_AUDIOSAMPLE_PACKET			0x02
#define HDMI_GENERALCTRL_PACKET			0x03
#define HDMI_ACP_PACKET					0x04
#define HDMI_ISRC1_PACKET				0x05
#define HDMI_ISRC2_PACKET				0x06
#define HDMI_1BITAUDIOSAMPLE_PACKET		0x07
#define HDMI_DSTAUDIO_PACKET			0x08
#define HDMI_HBRAUDIOSTREAM_PACKET		0x09
#define HDMI_GAMUTMETADATA_PACKET		0x0a
#define HDMI_VENDORINFOFRAME_PACKET		0x81
#define HDMI_AVIINFOFRAME_PACKET		0x82
#define HDMI_SPDINFOFRAME_PACKET		0x83
#define HDMI_AUDIOINFOFRAME_PACKET		0x84
#define HDMI_MPEGINFOFRAME_PACKET		0x85
//End------------------------------------------------------------------------------

//HDMI X Packet---------------------------------------------------------------------
#define HDMI_0PACKET					0x0
#define HDMI_1PACKET					0x1
#define HDMI_2PACKET					0x2
#define HDMI_3PACKET					0x3

#define HDMI_AVIINFOFRAME_ROFFSET		16
#define HDMI_AVIINFOFRAME_FOFFSET		15
#define HDMI_AVIINFOFRAME_YOFFSET		13
#define HDMI_AVIINFOFRAME_AOFFSET		12
#define HDMI_AVIINFOFRAME_BOFFSET		10
#define HDMI_AVIINFOFRAME_SOFFSET		8
#define HDMI_AVIINFOFRAME_PROFFSET		8

//GCP
#define HDMI_GENERALCTRL_CLEAROFFSET	4
//End------------------------------------------------------------------------------

// Scan Information
#define HDMI_OVERSCANNED				0x1
#define HDMI_UNDERSCANNED				0x2

// Bar Data Present
#define HDMI_BARNOTPRESENT				0x0
#define HDMI_VERBARTPRESENT				0x1
#define HDMI_HORBARTPRESENT				0x2
#define HDMI_BOTHBARTPRESENT			0x3

// Active Format Information Present
#define HDMI_ACTIVEFORMAT_PRESENT		0x1

// Active Portion Aspect Ratio
#define HDMI_ASPECTRATIO_SAME			0x8
#define HDMI_ASPECTRATIO_4TO3			0x9
#define HDMI_ASPECTRATIO_16TO9			0xa
#define HDMI_ASPECTRATIO_14TO9			0xb

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

#endif //endif _GP_HDMI_DEVICE_H_
