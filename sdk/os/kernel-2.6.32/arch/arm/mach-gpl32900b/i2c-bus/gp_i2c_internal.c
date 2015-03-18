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
 * @file    gp_i2c_bus.c
 * @brief   Implement of i2c bus driver.
 * @author  junp.zhang
 * @since   2010/10/12
 * @date    2010/10/12
 */
#include <mach/kernel.h>
#include <mach/module.h>
#include <mach/cdev.h>
#include <mach/diag.h>
#include <mach/gp_i2c_internal.h>
#include <mach/hal/hal_i2c_internal.h>
#include <mach/clk/gp_clk_core.h>

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/
#define I2C_BUS_WRITE 0
#define I2C_BUS_READ  1

#define I2C_RESTART_WITHOUT_STOP 0x01
/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/

typedef struct i2c_bus_handle_s {
	int slaveAddr;				/*!< @brief slave device address*/
	int clkRate;				/*!< @brief i2c bus clock rate */
} i2c_bus_handle_t;

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/

int open_count = 0;
DECLARE_MUTEX(sem);
 
/**
 * @brief   I2c clock enable/disable
 * @param   enable [in] 0:disable, 1:enable
 * @return  None
 * @see
 */
static void i2c_clock_enable(int enable)
{
	gp_enable_clock( (int*)"ARM_I2C", enable );
}

/**
 * @brief   I2C bus request function.
 * @param   slave_addr[in]: slave device ID
 * @param   clk_rate[in]: i2c bus clock rate(KHZ)
 * @return  i2c bus handle/ERROR_ID
 * @see
 */
int gp_i2c_internal_request(int slave_addr, int clk_rate)
{
	int ret = 0;
	i2c_bus_handle_t *hd = NULL;

	hd = (i2c_bus_handle_t *)kmalloc(sizeof(i2c_bus_handle_t), GFP_KERNEL);
	if (NULL == hd) {
		ret = -ENOMEM;
		goto __err_kmalloc;
	}

	memset(hd, 0, sizeof(i2c_bus_handle_t));

	hd->slaveAddr = slave_addr;
	hd->clkRate = clk_rate;

	if( open_count == 0) {
		i2c_clock_enable(1);
	}
	open_count++;

	/*init i2c bus register*/
	gpHalI2cBusInit();

	ret = (int)hd;
	return ret;

__err_kmalloc :
	return ret;
}
EXPORT_SYMBOL(gp_i2c_internal_request);

/**
 * @brief   I2C bus release function.
 * @param   handle[in]: i2c bus handle
 * @return  SP_OK(0)/SP_FAIL(1)
 * @see
 */
int gp_i2c_internal_release(int handle)
{
	int ret = SP_FAIL;

	i2c_bus_handle_t *hd = (i2c_bus_handle_t *)handle;
	if (NULL != hd) {
		kfree(hd);
		hd = NULL;
		ret = SP_OK;
	}

	open_count--;
	if( open_count == 0 ) {
		i2c_clock_enable(0);
	}

	return ret;
}
EXPORT_SYMBOL(gp_i2c_internal_release);

/**
 * @brief   I2C bus data transfer function
 * @param   handle[in]: i2c bus handle
 * @param   data[in]: data buffer
 * @param   len[in]: data length
 * @param   cmd[in]: I2C_BUS_WRITE/I2C_BUS_READ
 * @return  data length/ERROR_ID
 */
static int i2c_bus_xfer(int handle, unsigned char* data, unsigned int len, int cmd)
{
	int ret = -ENXIO;
	int i = 0;
	i2c_bus_handle_t *hd = NULL;

	hd = (i2c_bus_handle_t *)handle;

	if (hd->clkRate <= 0) {
		DIAG_ERROR("ERROR: i2c clock rate must to be more than zero\n");
		return -EINVAL;
	}

	if (down_interruptible(&sem) != 0) {
		return -ERESTARTSYS;
	}

	if (cmd == I2C_BUS_WRITE) {
		ret = gpHalI2cBusStartTran(hd->slaveAddr, hd->clkRate, I2C_BUS_WRITE, 0);
		if (ret == SP_FAIL) {
			DIAG_ERROR("ERROR: write slave device address fail\n");
			goto out;
		}

		for (i = 0; i < len; i++) {
			ret = gpHalI2cBusMidTran(&data[i], I2C_BUS_WRITE, 0);
			if (ret == SP_FAIL) {
				DIAG_ERROR("ERROR: write data fail\n");
				goto out;
			}
		}

		gpHalI2cBusStopTran(I2C_BUS_WRITE);

	} else {
		ret = gpHalI2cBusStartTran(hd->slaveAddr, hd->clkRate, I2C_BUS_READ, 1);
		if (ret == SP_FAIL) {
			DIAG_ERROR("ERROR: write slave device address fail\n");
			goto out;
		}

		for (i = 0; i < len; i++) {
			if(i == (len - 1)) {
				ret = gpHalI2cBusMidTran(&data[i], I2C_BUS_READ, 0);
			} else {
				ret = gpHalI2cBusMidTran(&data[i], I2C_BUS_READ, 1);
			}

			if (ret == SP_FAIL) {
				DIAG_ERROR("ERROR: read data fail\n");
				goto out;
			}
		}

		gpHalI2cBusStopTran(I2C_BUS_READ);
	}

	ret = i;
	up(&sem);

	return ret;

out:
	up(&sem);
	ret = -ENXIO;
	return ret;
}

/**
 * @brief   I2C bus write function
 * @param   handle[in]: i2c bus handle
 * @param   data[in]: data to write
 * @param   len[in]: data length
 * @return  data length/ERROR_ID
 */
int gp_i2c_internal_write(int handle, unsigned char* data, unsigned int len)
{
	return	i2c_bus_xfer(handle, data, len, I2C_BUS_WRITE);
}
EXPORT_SYMBOL(gp_i2c_internal_write);

/**
 * @brief   I2C bus read function
 * @param   handle[in]: i2c bus handle
 * @param   data[out]: data buffer
 * @param   len[in]: data length
 * @return  data length/ERROR_ID
 */
int gp_i2c_internal_read(int handle, unsigned char* data, unsigned int len)
{
	return	i2c_bus_xfer(handle, data, len, I2C_BUS_READ);
}
EXPORT_SYMBOL(gp_i2c_internal_read);

/**
 * @Leo add in 2012-09-02
 * @brief   I2C bus data transfer function
 * @param   handle[in]: i2c bus handle
 * @param   subaddr[in]: sub-address buffer
 * @param   length[in]: sub-address length
 * @param   data[in]: data buffer
 * @param	data_len[in]: data length
 * @param   cmd[in]: I2C_BUS_WRITE/I2C_BUS_READ
 * @param	mode[in]:i2c restart without stop or not: 1、I2C_RESTART_WITHOUT_STOP 0、I2C_RESTART_WITH_STOP
 * @return  data length/ERROR_ID
 */

static int i2c_multi_xfer(int handle,  unsigned char* subaddr, int length, unsigned char* data, unsigned int data_len, int cmd, int mode)
{
	int ret = -ENXIO;
	int i = 0;
	i2c_bus_handle_t *hd = NULL;

	hd = (i2c_bus_handle_t *)handle;

	if (hd->clkRate <= 0) {
		DIAG_ERROR("ERROR: i2c clock rate must to be more than zero\n");
		return -EINVAL;
	}

	if (down_interruptible(&sem) != 0) {
		return -ERESTARTSYS;
	}

	if (cmd == I2C_BUS_WRITE) {
		ret = gpHalI2cBusStartTran(hd->slaveAddr, hd->clkRate, I2C_BUS_WRITE, 0);
		if (ret == SP_FAIL) {
			DIAG_ERROR("ERROR: write slave device address fail\n");
			goto out;
		}

		for(i = 0; i < length; i ++) {
			ret = gpHalI2cBusMidTran(&subaddr[i], I2C_BUS_WRITE, 0);
			if (ret == SP_FAIL) {
				DIAG_ERROR("ERROR: write data fail\n");
				goto out;
			}
		}

		for (i = 0; i < data_len; i++) {
			ret = gpHalI2cBusMidTran(&data[i], I2C_BUS_WRITE, 0);
			if (ret == SP_FAIL) {
				DIAG_ERROR("ERROR: write data fail\n");
				goto out;
			}
		}

		gpHalI2cBusStopTran(I2C_BUS_WRITE);

	} else {
		ret = gpHalI2cBusStartTran(hd->slaveAddr, hd->clkRate, I2C_BUS_WRITE, 1);
		if (ret == SP_FAIL) {
			printk("Leo Set Address Error 1\n");
			DIAG_ERROR("ERROR: write slave device address fail\n");
			printk("hd->slaveAddr = %d\n", hd->slaveAddr);
			goto out;
		}

		for(i = 0; i < length; i ++) {
			ret = gpHalI2cBusMidTran(&subaddr[i], I2C_BUS_WRITE, 0);
			if (ret == SP_FAIL) {
				DIAG_ERROR("ERROR: write data fail\n");
				goto out;
			}
		}

		if(mode == I2C_RESTART_WITHOUT_STOP) {
			ret = gpHalI2cBusRestartTran(hd->slaveAddr);
			if (ret == SP_FAIL) {
				DIAG_ERROR("ERROR: restart signal generate fail\n");
				printk("hd->slaveWrAddr = %d\n", hd->slaveAddr);
				goto out;
			}
		} else {
			gpHalI2cBusStopTran(I2C_BUS_WRITE);

			ret = gpHalI2cBusStartTran(hd->slaveAddr, hd->clkRate, I2C_BUS_READ, 1);
			if (ret == SP_FAIL) {
				DIAG_ERROR("ERROR: read slave device address fail\n");
				printk("hd->slaveWrAddr = %d\n", hd->slaveAddr);
				goto out;
			}
		}

		for (i = 0; i < data_len; i++) {
			if(i == (data_len - 1)) {
				ret = gpHalI2cBusMidTran(&data[i], I2C_BUS_READ, 0);
			} else {
				ret = gpHalI2cBusMidTran(&data[i], I2C_BUS_READ, 1);
			}

			if (ret == SP_FAIL) {
				DIAG_ERROR("ERROR: read data fail\n");
				goto out;
			}
		}
		gpHalI2cBusStopTran(I2C_BUS_READ);
	}

	ret = i;
	up(&sem);

	return ret;

out:
	up(&sem);
	ret = -ENXIO;
	return ret;
}


/**
 * @Leo add in 2012-09-02
 * @brief   I2C bus write function
 * @param   handle[in]: i2c bus handle
 * @param   subaddr[in]: sub-address buffer
 * @param   length[in]: sub-address length
 * @param   data[in]: data buffer
 * @param	data_len[in]: data length
 * @param	mode[in]:i2c restart without stop or not: 1、I2C_RESTART_WITHOUT_STOP 0、I2C_RESTART_WITH_STOP
 * @return  data length/ERROR_ID
 */
int gp_i2c_internal_multi_write(int handle, unsigned char* subaddr, unsigned length, unsigned char* data, unsigned int data_len, int mode)
{
	return	i2c_multi_xfer(handle, subaddr, length, data, data_len, I2C_BUS_WRITE, mode);
}
EXPORT_SYMBOL(gp_i2c_internal_multi_write);

/**
 * @Leo add in 2012-09-02
 * @brief   I2C bus read function
 * @param   handle[in]: i2c bus handle
 * @param   subaddr[in]: sub-address buffer
 * @param   length[in]: sub-address length
 * @param   data[in]: data buffer
 * @param	data_len[in]: data length
 * @param	mode[in]:i2c restart without stop or not: 1、I2C_RESTART_WITHOUT_STOP 0、I2C_RESTART_WITH_STOP
 * @return  data length/ERROR_ID
 */
int gp_i2c_internal_multi_read(int handle, unsigned char* subaddr, unsigned length, unsigned char* data, unsigned int data_len, int mode)
{
	return	i2c_multi_xfer(handle, subaddr, length, data, data_len, I2C_BUS_READ, mode);
}
EXPORT_SYMBOL(gp_i2c_internal_multi_read);


/**************************************************************************
 *                  M O D U L E    D E C L A R A T I O N                  *
 **************************************************************************/

MODULE_AUTHOR("Generalplus");
MODULE_DESCRIPTION("Generalplus I2c Bus Driver");
MODULE_LICENSE_GP;



