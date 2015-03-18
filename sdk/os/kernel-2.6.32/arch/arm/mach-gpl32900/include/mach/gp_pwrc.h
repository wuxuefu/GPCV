/**************************************************************************
 *                                                                        *
 *         Copyright (c) 2011 by Generalplus Inc.                         *
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
 * @file    gp_pwrc.h
 * @brief   Declaration of PWRC driver.
 * @author  Daniel Huang
 */

#ifndef _GP_PWRC_H_
#define _GP_PWRC_H_

#include <mach/typedef.h>

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

/*
 * Ioctl for device node definition
 */
#define SPMP_PWRC_MAGIC		'P'

#define PWRC_IOCTL_ENABLE_BATTERY_DETECT              _IO(SPMP_PWRC_MAGIC,0x01)		/*!< @brief pwrc enable/disable battery detect */	
#define PWRC_IOCTL_BATTERY_SELECT                              _IO(SPMP_PWRC_MAGIC,0x02)		/*!< @brief pwrc battery selection */	
#define PWRC_IOCTL_OPERATION_MODE		            _IO(SPMP_PWRC_MAGIC,0x03)		/*!< @brief pwrc operation mode */	
#define PWRC_IOCTL_ENABLE_DCDC                                   _IO(SPMP_PWRC_MAGIC,0x04)		/*!< @brief pwrc enable/disable DCDC */
/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

/**
 * @brief pwrc request function
 * @return success: pwrc handle,  erro: NULL
 */
int gp_pwrc_request(void);

/**
 * @brief pwrc release function
 * @param handle [in] pwrc handle
 * @return success: 0,  erro: erro_id
 */
int gp_pwrc_release(int handle);

/**
 * @brief pwrc battery detect enable function
 * @param handle [in] pwrc handle
 * @return success: 0,  erro: erro_id
 */
int gp_pwrc_enable_battery_detect(int handle);

/**
 * @brief pwrc battery detect disable function
 * @param handle [in] pwrc handle
 * @return success: 0,  erro: erro_id
 */
int gp_pwrc_disable_battery_detect(int handle);

/**
 * @brief pwrc battery detect disable function
 * @param handle [in] pwrc handle, [in] battery type
 * @return success: 0,  erro: erro_id
 */
int gp_pwrc_battery_select(int handle, int type);

/**
 * @brief pwrc battery detect disable function
 * @param handle [in] pwrc handle, [in] operation mode
 * @return success: 0,  erro: erro_id
 */
int gp_pwrc_operation_mode_select(int handle, int mode);

/**
 * @brief pwrc enble function
 * @param handle [in] pwrc handle
 * @return success: 0,  erro: erro_id
 */
int gp_pwrc_enable_dcdc(int handle);

/**
 * @brief pwrc disable function
 * @param handle [in] pwrc handle
 * @return success: 0,  erro: erro_id
 */
int gp_pwrc_disable_dcdc(int handle);

/**
 * @brief   pwrc device register
 */
int gp_pwrc_device_register(void);

/**
 * @brief   pwrc device unregister
 */
void gp_pwrc_device_unregister(void);

#endif	/*_GP_PWRC_H_*/
