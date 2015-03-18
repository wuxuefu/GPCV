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
#ifndef _GP_MIPI_H_
#define _GP_MIPI_H_

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/
#ifndef ENABLE
#define ENABLE 		1
#endif
#ifndef DISABLE
#define DISABLE		0
#endif

/* IO Setting for GPL32900B */
#define C_IO_IS_CMOS_PIN 		0
#define C_IO_IS_MIPI_PIN 		1

/* MIPI PHY to PPU / CDSP */
#define C_MIPI_RX_PPU			0
#define C_MIPI_RX_CDSP			1

/* mipi clock sample edge*/
#define D_PHY_SAMPLE_POS		0x0
#define D_PHY_SAMPLE_NEG		0x1

/* mipi lane */
#define MIPI_1_LANE				0x0
#define MIPI_2_LANE				0x1

/* mipi seperate clock source */
#define MIPI_D_PHY_CLK			0x0
#define MIPI_SPEREATE_CLK		0x1

/* mipi ecc order*/
#define MIPI_ECC_ORDER0			0x0
#define MIPI_ECC_ORDER1			0x1
#define MIPI_ECC_ORDER2			0x2
#define MIPI_ECC_ORDER3			0x3

/* mipi check method*/
#define MIPI_CHECK_LP_00		0x0
#define MIPI_CHECK_HS_SEQ		0x1

/* mipi format */
#define MIPI_YUV422				0x0
#define MIPI_RGB888				0x1
#define MIPI_YUV565				0x2
#define MIPI_RAW8				0x3
#define MIPI_RAW10				0x4
#define MIPI_RAW12				0x5
#define MIPI_GENERIC_8_BIT		0x6
#define MIPI_USER_DEFINE_BYTE	0x7

/* irq flag */
#define MIPI_LANE0_SOT_SYNC_ERR	0x01
#define MIPI_HD_1BIT_ERR 		0x02
#define MIPI_HD_NBIT_ERR		0x04
#define MIPI_DATA_CRC_ERR		0x08
#define MIPI_LANE1_SOT_SYNC_ERR	0x10
#define MIPI_CCIR_SOF 			0x20

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
typedef struct gpMipiCfg_s
{
	/* mipi clock out set */
	unsigned int mipi_sep_clk_en;	/* 0:use mipi input clk, 1:use separate clk */ 
	unsigned int mipi_sep_clk;		/* separate clock */
	unsigned int mipi_sep_clk_src;	/* 0: CEVA pll, 1:USB pll*/
	unsigned int byte_clk_edge;		/* 0:posedge, 1:negedge */

	/* global configure */
	unsigned char mipi_enable;
	unsigned char low_power_en;		/* 0:disable, 1:enable */
	unsigned char sample_edge;		/* 0:posedge, 1:negedge */
	unsigned char lane_num;			/* 0:1 lane, 1:2 lane */
}gpMipiCfg_t;

typedef struct gpMipiEcc_s
{
	/* ecc */
	unsigned int ecc_check_en;		/* 0:disable, 1:enable */
	unsigned int ecc_order;			/* 0~3 */
	unsigned int data_mask_time;	/* unit is ns, HS_PREPARE time*/
	unsigned int check_hs_seq;		/* 0:disable, 1:enable */
}gpMipiEcc_t;

typedef struct gpMipiCCIR601_s
{	
	/* data format */
	unsigned char data_from_mmr;	/* 0:auto dtect, 1:mmr decide data type */
	unsigned char data_type;		/* 0~7, data type */
	unsigned char data_type_to_cdsp;/* 0: data to ppu csi if, 1:data to cdsp if*/
	unsigned char reserved0;
	
	/* ccir601 timing */
	unsigned short h_size;			/* 0~0xFFFF */
	unsigned short v_size;			/* 0~0xFFFF */
	unsigned char h_back_porch;		/* 0~0xF */
	unsigned char h_front_porch;	/* 0~0xF */
	unsigned char blanking_line_en;	/* 0:disable, 1:enable */
	unsigned char reserved1;
}gpMipiCCIR601_t;	
	
/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/
void gp_mipi_set_irq(unsigned char idx, unsigned int enable);
unsigned int gp_mipi_get_curframe_status(unsigned char idx);
void gp_mipi_set_io(unsigned int io);

/* Ioctl for device node definition */
#define MIPI_IOCTL_ID           'M'
#define MIPI_IOCTL_S_CFG  		_IOW(MIPI_IOCTL_ID, 2, gpMipiCfg_t)
#define MIPI_IOCTL_G_CFG  		_IOR(MIPI_IOCTL_ID, 3, gpMipiCfg_t)
#define MIPI_IOCTL_S_ECC  		_IOW(MIPI_IOCTL_ID, 4, gpMipiEcc_t)
#define MIPI_IOCTL_G_ECC  		_IOR(MIPI_IOCTL_ID, 5, gpMipiEcc_t)
#define MIPI_IOCTL_S_CCIR601	_IOW(MIPI_IOCTL_ID, 6, gpMipiCCIR601_t)
#define MIPI_IOCTL_G_CCIR601	_IOR(MIPI_IOCTL_ID, 7, gpMipiCCIR601_t)
#define MIPI_IOCTL_S_START		_IOW(MIPI_IOCTL_ID, 8, int)
#define MIPI_IOCTL_S_IO_PIN		_IOW(MIPI_IOCTL_ID, 9, int)

#endif /* _GP_MIPI_H_ */
