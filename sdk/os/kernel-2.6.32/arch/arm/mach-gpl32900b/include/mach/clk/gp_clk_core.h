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
 * @file	gp_clk_core.h
 * @brief	GP gated clock header file.
 * @author	Dunker Chen
 */
 
#ifndef _GP_CLK_CORE_H_
#define _GP_CLK_CORE_H_
 
/**************************************************************************
 *                         H E A D E R   F I L E S                        *
**************************************************************************/

#include <mach/clk/clk-private.h>

/**************************************************************************
*                           C O N S T A N T S                             *
**************************************************************************/

#define CLK_DEBUG		1
#define CLK_INFO		0

/* Ioctl for device node definition */
#define GP_CLOCK_MAGIC	'O'
#define IOCTL_GP_CLOCK_SET			_IOW(GP_CLOCK_MAGIC,1,unsigned int)
#define IOCTL_GP_CLOCK_ARM			_IOW(GP_CLOCK_MAGIC,2,unsigned int)
#define IOCTL_GP_CLOCK_CEVA			_IOW(GP_CLOCK_MAGIC,3,unsigned int)
#define IOCTL_GP_CLOCK_SYS			_IOW(GP_CLOCK_MAGIC,4,unsigned int)
#define IOCTL_GP_CLOCK_ENABLE		_IOW(GP_CLOCK_MAGIC,5,unsigned int)
#define IOCTL_GP_CLOCK_DISABLE		_IOW(GP_CLOCK_MAGIC,6,unsigned int)
#define IOCTL_GP_CLOCK_USAGE_DUMP	_IOW(GP_CLOCK_MAGIC,7,unsigned int)
#define IOCTL_GP_CLOCK_DUMP_ALL		_IOW(GP_CLOCK_MAGIC,8,unsigned int)
#define IOCTL_GP_CLOCK_SPLL_SEL		_IOW(GP_CLOCK_MAGIC,9,unsigned int)

/**************************************************************************
*                          D A T A    T Y P E S
**************************************************************************/

/**************************************************************************
*               F U N C T I O N    D E C L A R A T I O N S
***************************************************************************/

/**
* @brief 		PLL initialization function.
* @param 		xtal[in]: xtal clock.
* @return		None.
*/
extern void __init gp_pll_init(
	unsigned long xtal);

/**
* @brief 		Reference clock initialization function.
* @return		None.
*/
extern void __init gp_ref_clk_init(void);

/**
* @brief 		Bus clock initialization function.
* @return		None.
*/
extern void __init gp_bus_clk_init(void);

/**
* @brief 		SCU_A clock initialization function.
* @return		None.
*/
extern void __init gp_scua_clk_init(void);

/**
* @brief 		SCU_B clock initialization function.
* @return		None.
*/
extern void __init gp_scub_clk_init(void);

/**
* @brief 		SCU_C clock initialization function.
* @return		None.
*/
extern void __init gp_scuc_clk_init(void);

/**
* @brief 		Register all clock.
* @param 		xtal[in]: xtal clock.
* @return		None.
*/
extern void __init gp_register_baseclocks(
	unsigned long xtal);

/**
* @brief 		Register all clock.
* @param 		xtal[in]: xtal clock.
* @return		None.
*/
void __init gp_register_baseclocks(
	unsigned long xtal);
	
/**
* @brief 		Print clock information.
* @param 		level[in]: Print level.
* @param 		clk[in]: Clock structure.
* @return		None.
*/
extern void gp_clk_print(
	int level, 
	struct clk *clk);
	
/**
* @brief 	Clock get function
* @param 	clock_name[in]: base/device clock name to get current base clock
* @param   freq[out]: clock real setting value
* @return  SP_OK(0)/ERROR_ID
*/
extern int gp_clk_get_rate(
	int *clock_name,
	int*freq);
	
/**
* @brief 	Clock set function
* @param 	clock_name[in]: base/device clock name to get current base clock
* @param   freq[in]: clock real setting value
* @return  SP_OK(0)/ERROR_ID
*/
extern int gp_clk_set_rate( 
	int *clock_name,
	unsigned long freq );

/**
* @brief 	Enable/Disable clock interface function
* @param 	clock_name[in]: base/device clock name
* @param 	enable[in]:  1: enable, 0 : diable
* @return 	SUCCESS/FAIL.
*/
extern int gp_enable_clock(
	int *clock_name,
	int enable);

/**
* @brief 	Set clock parent
* @param 	clock_name[in]: base/device clock name
* @param 	parent_name[in]:  parent name
* @return 	SUCCESS/FAIL.
*/
extern int gp_clk_set_parent(
	int *clock_name,
	int *parent_name);

#endif /* _GP_CLK_CORE_H_ */