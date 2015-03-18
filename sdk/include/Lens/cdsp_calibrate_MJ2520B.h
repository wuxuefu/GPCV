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

#ifndef _CDSP_CALIBRATE_MJ2520B_H_
#define _CDSP_CALIBRATE_MJ2520B_H_

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


static const unsigned short g_ar0330_lens_MJ2520B_r_b_gain[71][2] = 
{ // { r gain, b gain }  
#if 1
{41, 219},
{44, 215},
{48, 211},
{51, 207},
{54, 203},
{57, 200},
{60, 196},
{63, 192},
{65, 189},
{68, 185},
{70, 182},
{72, 178},
{74, 175},
{76, 171},
{78, 168},
{79, 165},
{81, 161},
{82, 158},
{84, 155},
{85, 152},
{86, 149},
{87, 146},
{88, 143},
{89, 140},
{90, 138},
{91, 135},
{91, 132},
{92, 130},
{92, 127},
{93, 124},
{93, 122},
{94, 120},
{94, 117},
{94, 115},
{95, 113},
{95, 110},
{95, 108},
{95, 106},
{96, 104},
{96, 102},
{96, 100},
{96, 98},
{96, 96},
{96, 94},
{96, 93},
{96, 91},
{96, 89},
{97, 88},
{97, 86},
{97, 85},
{97, 83},
{97, 82},
{98, 80},
{98, 79},
{98, 78},
{99, 77},
{99, 75},
{99, 74},
{100, 73},
{100, 72},
{101, 71},
{102, 70},
{103, 70},
{103, 69},
{104, 68},
{105, 67},
{106, 67},
{108, 66},
{109, 66},
{110, 65},
{112, 65}
#else
{56, 240},
{58, 234},
{60, 227},
{62, 221},
{64, 215},
{66, 209},
{67, 203},
{69, 197},
{71, 192},
{72, 186},
{74, 181},
{75, 176},
{76, 171},
{78, 166},
{79, 162},
{80, 157},
{82, 153},
{83, 149},
{84, 145},
{85, 141},
{86, 137},
{87, 134},
{88, 131},
{89, 127},
{90, 124},
{91, 121},
{91, 119},
{92, 116},
{93, 114},
{94, 111},
{94, 109},
{95, 107},
{96, 106},
{96, 104},
{97, 102},
{97, 101},
{98, 100},
{98, 99},
{99, 98},
{99, 97},
{100, 97},
{100, 96},
{101, 96},
{101, 96},
{102, 96},
{102, 96},
{102, 97},
{103, 97},
{103, 98},
{104, 99},
{104, 100},
{104, 101},
{105, 102},
{105, 103},
{105, 105},
{106, 107},
{106, 109},
{106, 111},
{107, 113},
{107, 115},
{107, 118},
{108, 121},
{108, 123},
{108, 126},
{109, 130},
{109, 133},
{109, 136},
{110, 140},
{110, 144},
{111, 148},
{111, 152}
#endif
};


static const unsigned int g_ar0330_lens_MJ2520B_gamma_table_test[] = 
{
0x04440d, 0x044411, 0x044415, 0x044419, 0x04441d, 0x110421, 0x011124, 0x044428, 
0x11112b, 0x04442f, 0x011132, 0x110436, 0x104439, 0x04413c, 0x04113f, 0x041142, 
0x041145, 0x041148, 0x10414b, 0x10444e, 0x010451, 0x041054, 0x004156, 0x041059, 
0x00415b, 0x04105e, 0x010460, 0x104063, 0x041065, 0x040467, 0x010169, 0x00416b, 
0x00406e, 0x004070, 0x004072, 0x004074, 0x010076, 0x010177, 0x040479, 0x10047b, 
0x00407d, 0x01007f, 0x100480, 0x001082, 0x010084, 0x001085, 0x010087, 0x000488, 
0x01008a, 0x00108b, 0x04008d, 0x00408e, 0x10018f, 0x010091, 0x001092, 0x000493, 
0x040095, 0x004096, 0x001097, 0x000198, 0x10009a, 0x04009b, 0x00409c, 0x00109d, 
0x00049e, 0x00019f, 0x1000a1, 0x1000a2, 0x0400a3, 0x0100a4, 0x0040a5, 0x0040a6, 
0x0010a7, 0x0004a8, 0x0004a9, 0x0001aa, 0x0000ac, 0x1000ad, 0x0400ae, 0x0100af, 
0x0100b0, 0x0040b1, 0x0010b2, 0x0004b3, 0x0001b4, 0x1000b6, 0x0400b7, 0x0100b8, 
0x0010b9, 0x0004ba, 0x1000bc, 0x0100bd, 0x0040be, 0x0004bf, 0x0400c1, 0x0040c2, 
0x0004c3, 0x0400c5, 0x0010c6, 0x1001c7, 0x0040c9, 0x1004ca, 0x0040cc, 0x0401cd, 
0x0040cf, 0x0101d0, 0x1004d2, 0x0040d4, 0x0101d5, 0x0404d7, 0x1010d9, 0x0010db, 
0x0040dd, 0x0040df, 0x0100e1, 0x0041e2, 0x0040e5, 0x0040e7, 0x1010e9, 0x0410eb, 
0x0104ed, 0x0041ef, 0x0410f2, 0x0104f4, 0x1040f7, 0x0104f9, 0x0410fc, 0x0004fe
};
static const short g_ar0330_lens_MJ2520B_color_matrix4gamma045[9] = 
{	
#if 0
	(short) ((1.42604256597971270000 *64) + 0.5),
	(short) ((-0.41710081159426571000 *64) + 0.5),
	(short) ((-0.00894175438544701950 *64) + 0.5),
	(short) ((-0.22435367269390588000 *64) + 0.5),
	(short) ((1.20898435999074730000 *64) + 0.5),
	(short) ((0.01536931270315864200 *64) + 0.5),
	(short) ((0.05689057258583490900 *64) + 0.5),
	(short) ((-0.82268141798101080000 *64) + 0.5),
	(short) ((1.76579084539517580000 *64) + 0.5)
#else //using comi
	(short) ((1.41584287504700510000 *64) + 0.5),
	(short) ((-0.3243545598584618400 *64) + 0.5),
	(short) ((-0.0914883151885433760 *64) + 0.5),
	(short) ((-0.3569149216976537100 *64) + 0.5),
	(short) ((1.37507786348114620000 *64) + 0.5),
	(short) ((-0.0181629417834924570 *64) + 0.5),
	(short) ((0.00964252667160129910 *64) + 0.5),
	(short) ((-0.7108056366196609000 *64) + 0.5),
	(short) ((1.70116310994805950000 *64) + 0.5)
#endif
};

static const short g_ar0330_lens_MJ2520B_awb_thr[19] = 
{
	200, // awbwinthr
	
	0*64, // sindata
	1*64, // cosdata 
	
	 30, // Ythr0
	90, // Ythr1
	144,	// Ythr2 
	200, // Ythr3 
	
	-5, // UL1N1 
	 4, // UL1P1 
	-3, // VL1N1 
	 3, // VL1P1 
	
	 -5, //UL1N2
	  5, //UL1P2
	 -4, //VL1N2
	  5, // VL1P2
	
	 -7, // UL1N3
	  6,	 //UL1P3
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

//awb r gain: 102, b gain: 96
sensor_calibration_t ar0330_cdsp_calibration = 
{
	.ob = g_ar0330_ob_table,
	.ob_size = sizeof(unsigned short)*11,
	.lenscmp = g_ar0330_lenscmp_table,
	.lenscmp_size =sizeof(unsigned short)*256 ,
	.wb_gain = g_ar0330_lens_MJ2520B_r_b_gain,
	.wbgain_size = sizeof(unsigned short)*71*2,
	.gamma1 = g_ar0330_lens_MJ2520B_gamma_table_test,
	.gamma1_size = sizeof(unsigned int)*128,
	.color_matrix1 = g_ar0330_lens_MJ2520B_color_matrix4gamma045,
	.matrix1_size = sizeof(short)*9,
	.gamma2 = g_ar0330_lens_MJ2520B_gamma_table_test,
	.gamma2_size = sizeof(unsigned int)*128,
	.color_matrix2 = g_ar0330_lens_MJ2520B_color_matrix4gamma045,
	.matrix2_size = sizeof(short)*9,
	.awb_thr = g_ar0330_lens_MJ2520B_awb_thr,
	.awb_size = sizeof(short)*19
};

#endif //endif _STREAM_H_
