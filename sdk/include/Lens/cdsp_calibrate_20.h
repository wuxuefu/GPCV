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
 *  No.19, Industry E. Rd. IV, Hsinchu Science Park,                      *
 *  Hsinchu City 30078, Taiwan, R.O.C.                                    *
 *                                                                        *
 **************************************************************************/
/**
 * @file cdsp_calibrate.h
 * @brief cdsp_calibrate header file
 * @author 
 */

#ifndef _CDSP_CALIBRATE_20_H_
#define _CDSP_CALIBRATE_20_H_

#include "mach/sensor_mgr.h"

static const unsigned short g_ar0330_lenscmp_table[] = 
{
	0x100, 0x100, 0x101, 0x101, 0x101, 0x101, 0x102, 0x103, 
	0x103, 0x103, 0x104, 0x104, 0x104, 0x105, 0x105, 0x106, 
	0x106, 0x107, 0x106, 0x106, 0x107, 0x107, 0x108, 0x108, 
	0x109, 0x109, 0x10a, 0x10a, 0x10a, 0x10b, 0x10b, 0x10c, 
	0x10c, 0x10d, 0x10e, 0x10f, 0x110, 0x110, 0x110, 0x110, 
	0x111, 0x111, 0x112, 0x112, 0x113, 0x114, 0x114, 0x115, 
	0x115, 0x116, 0x116, 0x117, 0x118, 0x119, 0x11a, 0x11b, 
	0x11b, 0x11c, 0x11d, 0x11e, 0x11f, 0x121, 0x121, 0x122, 
	0x123, 0x124, 0x126, 0x127, 0x128, 0x129, 0x12a, 0x12c, 
	0x12d, 0x12e, 0x12f, 0x131, 0x132, 0x132, 0x134, 0x135, 
	0x137, 0x138, 0x139, 0x13b, 0x13c, 0x13d, 0x13f, 0x140, 
	0x141, 0x143, 0x144, 0x145, 0x147, 0x149, 0x149, 0x14b, 
	0x14d, 0x14d, 0x14f, 0x150, 0x152, 0x154, 0x155, 0x157, 
	0x157, 0x159, 0x15b, 0x15c, 0x15e, 0x15f, 0x161, 0x162, 
	0x163, 0x164, 0x166, 0x167, 0x168, 0x169, 0x16b, 0x16c, 
	0x16e, 0x16f, 0x171, 0x172, 0x173, 0x175, 0x175, 0x178, 
	0x179, 0x17a, 0x17c, 0x17d, 0x17f, 0x180, 0x182, 0x183, 
	0x185, 0x187, 0x188, 0x18a, 0x18d, 0x18e, 0x190, 0x192, 
	0x194, 0x197, 0x19a, 0x19e, 0x19f, 0x1a2, 0x1a1, 0x177, 
	0x151, 0x12e, 0x10e, 0x0f1, 0x0d7, 0x0bf, 0x0aa, 0x097, 
	0x085, 0x075, 0x067, 0x05a, 0x04f, 0x045, 0x03c, 0x034, 
	0x02d, 0x027, 0x021, 0x01c, 0x018, 0x014, 0x011, 0x00e, 
	0x00c, 0x00a, 0x008, 0x006, 0x005, 0x004, 0x003, 0x002, 
	0x001, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 

};


static const unsigned short g_ar0330_lens_20_r_b_gain[71][2] = 
{ // { r gain, b gain }  
{37, 228},
{40, 223},
{43, 217},
{46, 212},
{49, 207},
{52, 202},
{55, 197},
{57, 193},
{60, 188},
{62, 184},
{64, 179},
{66, 175},
{68, 171},
{70, 167},
{72, 163},
{74, 159},
{75, 155},
{77, 152},
{79, 148},
{80, 145},
{81, 142},
{83, 138},
{84, 135},
{85, 132},
{86, 129},
{87, 127},
{88, 124},
{89, 121},
{89, 119},
{90, 117},
{91, 114},
{91, 112},
{92, 110},
{93, 108},
{93, 107},
{94, 105},
{94, 103},
{94, 102},
{95, 100},
{95, 99},
{95, 98},
{96, 97},
{96, 96},
{96, 95},
{96, 94},
{96, 94},
{96, 93},
{97, 93},
{97, 92},
{97, 92},
{97, 92},
{97, 92},
{97, 92},
{97, 92},
{98, 93},
{98, 93},
{98, 94},
{98, 94},
{98, 95},
{98, 96},
{99, 97},
{99, 98},
{99, 99},
{100, 100},
{100, 102},
{100, 103},
{101, 105},
{101, 107},
{102, 108},
{102, 110},
{103, 112}
};


static const unsigned int g_ar0330_lens_20_gamma_table_test[] = 
{
0x154514, 0x15151a, 0x151520, 0x151526, 0x11452c, 0x145132, 0x114537, 0x05143d, 
0x051142, 0x051147, 0x11114c, 0x114451, 0x045155, 0x11445a, 0x11115e, 0x044562, 
0x044467, 0x04446b, 0x11046f, 0x111172, 0x044476, 0x11107a, 0x04417d, 0x011180, 
0x110484, 0x110487, 0x10448a, 0x11048d, 0x010490, 0x011093, 0x041096, 0x104198, 
0x04049b, 0x10419d, 0x0404a0, 0x0041a2, 0x1010a5, 0x0404a7, 0x0101a9, 0x0040ac, 
0x1010ae, 0x1010b0, 0x1010b2, 0x0404b4, 0x0404b6, 0x1004b8, 0x1010ba, 0x1010bc, 
0x1010be, 0x0040c0, 0x0040c2, 0x0100c4, 0x0101c5, 0x0401c7, 0x0404c9, 0x1004cb, 
0x1010cd, 0x0010cf, 0x0040d1, 0x0040d3, 0x0040d5, 0x0100d7, 0x0101d8, 0x0041da, 
0x0040dd, 0x0040df, 0x1010e1, 0x0410e3, 0x0404e5, 0x0101e7, 0x1040ea, 0x0404ec, 
0x0041ee, 0x0410f1, 0x1041f3, 0x0104f6, 0x0410f9, 0x1041fb, 0x0004fe, 0x0000ff, 
0x0000ff, 0x0000ff, 0x0000ff, 0x0000ff, 0x0000ff, 0x0000ff, 0x0000ff, 0x0000ff, 
0x0000ff, 0x0000ff, 0x0000ff, 0x0000ff, 0x0000ff, 0x0000ff, 0x0000ff, 0x0000ff, 
0x0000ff, 0x0000ff, 0x0000ff, 0x0000ff, 0x0000ff, 0x0000ff, 0x0000ff, 0x0000ff, 
0x0000ff, 0x0000ff, 0x0000ff, 0x0000ff, 0x0000ff, 0x0000ff, 0x0000ff, 0x0000ff, 
0x0000ff, 0x0000ff, 0x0000ff, 0x0000ff, 0x0000ff, 0x0000ff, 0x0000ff, 0x0000ff, 
0x0000ff, 0x0000ff, 0x0000ff, 0x0000ff, 0x0000ff, 0x0000ff, 0x0000ff, 0x0000ff
};
static const short g_ar0330_lens_20_color_matrix4gamma045[9] = 
{	
	(short) ((1.44037666198350010000 *64) + 0.5),
	(short) ((-0.49773247595781145000 *64) + 0.5),
	(short) ((0.05735581397431136600 *64) + 0.5),
	(short) ((-0.24280286687812647000 *64) + 0.5),
	(short) ((1.14122126422549570000 *64) + 0.5),
	(short) ((0.10158160265263062000 *64) + 0.5),
	(short) ((0.06650598143995087300 *64) + 0.5),
	(short) ((-0.87114265413464620000 *64) + 0.5),
	(short) ((1.80463667269469520000 *64) + 0.5)
};

static const short g_ar0330_lens_20_awb_thr[19] = 
{
	200, // awbwinthr
	
	0*64, // sindata
	1*64, // cosdata 
	
	 30, // Ythr0
	90, // Ythr1
	144,	// Ythr2 
	200, // Ythr3 
	
	-4, // UL1N1 
	 4, // UL1P1 
	-3, // VL1N1 
	 4, // VL1P1 
	
	 -5, //UL1N2
	  5, //UL1P2
	 -5, //VL1N2
	  6, // VL1P2
	
	 -6, // UL1N3
	  5,	 //UL1P3
	 -5, // VL1N3
	  5, //VL1P3
};

static const short g_ar0330_ob_table[11] = 
{
	0, // obautoen
	0, // ob_type
	0, //obHOffset
	0, // obVOffset
	
	1, // obmanuen
	42, // maunob = 42

	1, // wboffseten
	0, // wbo_r
	0, // wbo_gr
	0, // wbo_gb
	0 // wbo_b
};

//awb r gain: 95, b gain: 96
sensor_calibration_t ar0330_cdsp_calibration = 
{
	.ob = g_ar0330_ob_table,
	.ob_size = sizeof(unsigned short)*11,
	.lenscmp = g_ar0330_lenscmp_table,
	.lenscmp_size =sizeof(unsigned short)*256 ,
	.wb_gain = g_ar0330_lens_20_r_b_gain,
	.wbgain_size = sizeof(unsigned short)*71*2,
	.gamma1 = g_ar0330_lens_20_gamma_table_test,
	.gamma1_size = sizeof(unsigned int)*128,
	.color_matrix1 = g_ar0330_lens_20_color_matrix4gamma045,
	.matrix1_size = sizeof(short)*9,
	.gamma2 = g_ar0330_lens_20_gamma_table_test,
	.gamma2_size = sizeof(unsigned int)*128,
	.color_matrix2 = g_ar0330_lens_20_color_matrix4gamma045,
	.matrix2_size = sizeof(short)*9,
	.awb_thr = g_ar0330_lens_20_awb_thr,
	.awb_size = sizeof(short)*19
};

#endif //endif _STREAM_H_
