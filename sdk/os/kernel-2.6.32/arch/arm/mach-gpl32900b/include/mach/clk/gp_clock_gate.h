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
 * @file	gp_clk_gate.h
 * @brief	GP gated clock header file.
 * @author	Dunker Chen
 */
 
#ifndef _GP_CLK_GATE_H_
#define _GP_CLK_GATE_H_
 
/**************************************************************************
 *                         H E A D E R   F I L E S                        *
**************************************************************************/

/**************************************************************************
*                           C O N S T A N T S                             *
**************************************************************************/

#define CLK_INIT_EN		BIT(0)		/* Clock enable when init */
#define CLK_INIT_DIS	BIT(1)		/* Clock disable when init */
#define CLK_RST_EN		BIT(2)		/* Clock reset before clock enable */
#define CLK_RST_DIS		BIT(3)		/* Clock reset after clock disable */

/**************************************************************************
*                          D A T A    T Y P E S
**************************************************************************/

struct clk_gp_gate_para{
	void __iomem	*gate_reg;
	void __iomem	*reset_reg;
	u8				gate_bit;
	u8				reset_bit;
	u8				flag;
}; 

/**************************************************************************
*               F U N C T I O N    D E C L A R A T I O N S
***************************************************************************/

/**
* @brief 		GP clock gate register.
* @param 		dev[in]: Device that is registering this clock.
* @param 		name[in]: Name of this clock.
* @param 		parent_name[in]: Name of this clock's parent.
* @param 		flags[in]: Framework-specific flags for this clock.
* @param 		para[in]: GP clock gate parameter.
* @return		Clock struct pointer.
*/
extern struct clk *clk_register_gp_gate(
	struct device *dev,
	const char *name,
	const char *parent_name,
	unsigned long flags,
	struct clk_gp_gate_para *para);

#endif /* _GP_SD_H_ */