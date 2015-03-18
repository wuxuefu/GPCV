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
 * @file    icver.h
 * @brief   Get GP IC version header file.
 * @author  Dunker Chen
 */
 
#ifndef _GP_ICVER_H_
#define _GP_ICVER_H_

#ifdef __cplusplus
extern "C" {
#endif
/**************************************************************************
 *                         H E A D E R   F I L E S                        *
**************************************************************************/

/**************************************************************************
*                           C O N S T A N T S                             *
**************************************************************************/

#define MACH_GPL32900	1
#define MACH_GPL32900B	2

/**************************************************************************
*                          D A T A    T Y P E S
**************************************************************************/

typedef struct gp_icver_s
{
 	unsigned int		major;				/*!< @brief major version number. 1 for A version, 2 for B version ... */
 	unsigned int 		minor;        		/*!< @brief minor version number. Not use now, for future use. */
}
gp_icver_t;

/**************************************************************************
*               F U N C T I O N    D E C L A R A T I O N S
**************************************************************************/

/**
 * @brief Get GP IC version .
 * @return : all element 0 means error or not GP IC.
 */
extern gp_icver_t gpICVersion(void);

#ifdef __cplusplus
}
#endif

#endif /* _GP_ICVER_H_ */