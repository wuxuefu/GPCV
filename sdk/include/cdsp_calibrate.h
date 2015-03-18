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

#ifndef _CDSP_CALIBRATE_H_
#define _CDSP_CALIBRATE_H_

#include "mach/sensor_mgr.h"

#define LENS_GP 0
#define LENS_8172 1
#define LENS_PW650 2
#define LENS_20 3
#define LENS_MJ2520B 4
#define LENS_3006 5

#define LENS_CALI LENS_GP

#if (LENS_GP == LENS_CALI)
#include "Lens/cdsp_calibrate_gp.h"
#elif (LENS_8172 == LENS_CALI)
#include "Lens/cdsp_calibrate_8172.h"
#elif (LENS_PW650 == LENS_CALI)
#include "Lens/cdsp_calibrate_pw650.h"
#elif (LENS_20 == LENS_CALI)
#include "Lens/cdsp_calibrate_20.h"
#elif (LENS_MJ2520B == LENS_CALI)
#include "Lens/cdsp_calibrate_MJ2520B.h"
#elif (LENS_3006 == LENS_CALI)
#include "Lens/cdsp_calibrate_3006.h"
#endif


#endif //endif _STREAM_H_
