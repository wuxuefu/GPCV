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

/*!
 * @file panel_common.c
 * @brief Panel common driver
 */
#include <linux/init.h>
#include <linux/configfs.h>
#include <linux/module.h>
#include <linux/kernel.h> /* printk() */
#include <linux/slab.h> /* kmalloc() */
#include <linux/fs.h> /* everything... */
#include <linux/errno.h> /* error codes */
#include <linux/types.h> /* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h> /* O_ACCMODE */
#include <asm/system.h> /* cli(), *_flags */
#include <asm/uaccess.h> /* copy_from/to_user */

#include <mach/panel_cfg.h>
#include <mach/hardware.h>
//#include <mach/regs-scu.h>
#include <mach/hal/hal_gpio.h>
#include <mach/hal/hal_clock.h>
#include <mach/hal/hal_disp.h>
#include <mach/gp_display.h>
#include <mach/clock_mgr/gp_clock.h>
#include <mach/gp_board.h>
#include <mach/gp_panel.h>
#include <mach/module.h>
//#include <mach/gp_pin_grp.h>
//#include <mach/hal/regmap/reg_scu.h>

MODULE_LICENSE_GP;

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/


/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
#if 0
	#define DEBUG	printk
#else
	#define DEBUG(...)
#endif

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/



/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/


/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/
static int32_t
panel_pin_group_setting(
	uint32_t mode,
	uint32_t enable
)
{
	uint32_t func;

	switch (mode) {
	case GID_PRGB888:
		if (enable)	func = 1;
		else		func = 0;

		gpHalGpioSetPadGrp((func << 16) | (7 << 8));
		gpHalGpioSetPadGrp((func << 16) | (8 << 8));
		gpHalGpioSetPadGrp((func << 16) | (9 << 8));
		gpHalGpioSetPadGrp((func << 16) | (10<< 8));
		gpHalGpioSetPadGrp((func << 16) | (11<< 8));
		gpHalGpioSetPadGrp((func << 16) | (12<< 8));
		gpHalGpioSetPadGrp((func << 16) | (13<< 8));
		gpHalGpioSetPadGrp((func << 16) | (14<< 8));
		gpHalGpioSetPadGrp((func << 16) | (15<< 8));
		gpHalGpioSetPadGrp((func << 16) | (16<< 8));
		gpHalGpioSetPadGrp((func << 16) | (47<< 8));
		gpHalGpioSetPadGrp((func << 16) | (48<< 8));
		gpHalGpioSetPadGrp((func << 16) | (49<< 8));
		gpHalGpioSetPadGrp((func << 16) | (50<< 8));
		gpHalGpioSetPadGrp((func << 16) | (51<< 8));

		break;
	case GID_PRGB666:
		break;
	case GID_PRGB565:
		break;
	case GID_SRGB888:
	case GID_SRGBM888:
		if (enable)	func = 1;
		else		func = 0;

		gpHalGpioSetPadGrp((func << 16) | (7 << 8));
		gpHalGpioSetPadGrp((func << 16) | (8 << 8));
		gpHalGpioSetPadGrp((func << 16) | (9 << 8));
		gpHalGpioSetPadGrp((func << 16) | (10<< 8));
		gpHalGpioSetPadGrp((func << 16) | (11<< 8));
		gpHalGpioSetPadGrp((func << 16) | (47<< 8));
		gpHalGpioSetPadGrp((func << 16) | (48<< 8));
		gpHalGpioSetPadGrp((func << 16) | (49<< 8));

		break;
	default:
		printk("[%s:%d], unknown mode %d\n", __FUNCTION__, __LINE__, mode);
		return -1;
	}

	return 0;
}

int32_t
panel_pin_drivingCurrent_setting(
        uint32_t drivingCurrent
)
{
	//gpHalGpioSetPadGrp((0 << 16) | (47<< 8));
    gpHalGpioSetDrivingCurrent(0<<24|15, drivingCurrent); /*IOA15*/

    gpHalGpioSetDrivingCurrent(1<<24|0, drivingCurrent); /*IOB0~IOB7*/
    gpHalGpioSetDrivingCurrent(1<<24|1, drivingCurrent);
    gpHalGpioSetDrivingCurrent(1<<24|2, drivingCurrent);
    gpHalGpioSetDrivingCurrent(1<<24|3, drivingCurrent);
    gpHalGpioSetDrivingCurrent(1<<24|4, drivingCurrent);
    gpHalGpioSetDrivingCurrent(1<<24|5, drivingCurrent);
    gpHalGpioSetDrivingCurrent(1<<24|6, drivingCurrent);
    gpHalGpioSetDrivingCurrent(1<<24|7, drivingCurrent);

    gpHalGpioSetDrivingCurrent(2<<24|8, drivingCurrent); /*IOC8~IOC15*/
    gpHalGpioSetDrivingCurrent(2<<24|9, drivingCurrent);
    gpHalGpioSetDrivingCurrent(2<<24|10, drivingCurrent);
    gpHalGpioSetDrivingCurrent(2<<24|11, drivingCurrent);
    gpHalGpioSetDrivingCurrent(2<<24|12, drivingCurrent);
    gpHalGpioSetDrivingCurrent(2<<24|13, drivingCurrent);
    gpHalGpioSetDrivingCurrent(2<<24|14, drivingCurrent);
    gpHalGpioSetDrivingCurrent(2<<24|15, drivingCurrent);

    gpHalGpioSetDrivingCurrent(3<<24|0, drivingCurrent); /*IOD0~IOD10*/
    gpHalGpioSetDrivingCurrent(3<<24|1, drivingCurrent);
    gpHalGpioSetDrivingCurrent(3<<24|2, drivingCurrent);
    gpHalGpioSetDrivingCurrent(3<<24|3, drivingCurrent);
    gpHalGpioSetDrivingCurrent(3<<24|4, drivingCurrent);
    gpHalGpioSetDrivingCurrent(3<<24|5, drivingCurrent);
    gpHalGpioSetDrivingCurrent(3<<24|6, drivingCurrent);
    gpHalGpioSetDrivingCurrent(3<<24|7, drivingCurrent);
    gpHalGpioSetDrivingCurrent(3<<24|8, drivingCurrent);
    gpHalGpioSetDrivingCurrent(3<<24|9, drivingCurrent);
    gpHalGpioSetDrivingCurrent(3<<24|10, drivingCurrent);

    gpHalGpioSetDrivingCurrent(3<<24|25, drivingCurrent);
    gpHalGpioSetDrivingCurrent(3<<24|26, drivingCurrent);
    gpHalGpioSetDrivingCurrent(3<<24|27, drivingCurrent);

    return 0;
}

int32_t
panel_common_init(
	uint32_t mode
)
{
	panel_pin_group_setting(mode, 1);
	return 0;
}

int32_t
panel_common_suspend(
	uint32_t mode
)
{
	panel_pin_group_setting(mode, 0);
	return 0;
}

int32_t
panel_common_resume(
	uint32_t mode
)
{
	panel_pin_group_setting(mode, 1);
	return 0;
}
