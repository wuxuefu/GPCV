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
 * @file    gp_i2c_bus.h
 * @brief   Declaration of i2c bus driver.
 * @author  junp.zhang
 * @since   2010/10/12
 * @date    2010/10/12
 */
#ifndef _GP_TI2C_BUS_H_
#define _GP_TI2C_BUS_H_

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

/* Ioctl for device node definition */
#define TI2C_BUS_IOCTL_ID         'I'
#define TI2C_BUS_ATTR_SET    _IOW(TI2C_BUS_IOCTL_ID, 0, int)

#define TI2C_BUS_ADDRESS_WRITE	_IOW(TI2C_BUS_IOCTL_ID, 1, ti2c_set_value_t)
#define TI2C_BUS_ADDRESS_READ	_IOR(TI2C_BUS_IOCTL_ID, 1, ti2c_set_value_t*)

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/

/*transmitionMode*/
enum {
	TI2C_NORMAL_WRITE_MODE = 0,
	TI2C_BURST_WRITE_MODE,
	TI2C_NORMAL_READ_MODE,
	TI2C_BURST_READ_STOP_MODE,
	TI2C_BURST_READ_NOSTOPBIT_MODE,
	TI2C_BURST_READ_STOP_ACKEND_MODE
};

/*slaveAddrMode*/
enum {
	TI2C_NORMAL_SLAVEADDR_8BITS = 0,
	TI2C_NORMAL_SLAVEADDR_16BITS
};

/*subAddrMode*/
enum {
	TI2C_NORMAL_SUBADDR_NO = 0,
	TI2C_NORMAL_SUBADDR_8BITS = 1,
	TI2C_NORMAL_SUBADDR_16BITS = 3
};

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
#if 0
typedef struct ti2c_bus_attr_s{
	unsigned int slaveAddr;			/*!< @brief slave device address*/
	unsigned int clkRate;				/*!< @brief i2c bus clock rate */
}ti2c_bus_attr_t;
typedef struct ti2c_bus_address_rw_t{
	//ti2c_bus_attr_t handle;
	unsigned int address;
	unsigned int count;
	char *data;
}ti2c_bus_address_rw_t;
#endif

typedef struct ti2c_set_value_s {
	unsigned char* pDeviceString;
	unsigned char deviceCount;
	unsigned int clockRate;
	unsigned int clockTransform;
	unsigned long clockPclk;
	unsigned char transmitMode;
	unsigned char slaveAddrMode;
	unsigned char subAddrMode;
	unsigned short slaveAddr;  
	unsigned short* pSubAddr;
	unsigned char* pBuf;
	unsigned int dataCnt;
	unsigned int irq;
	unsigned char apbdmaEn;
    void (*irq_handler)(void *);
} ti2c_set_value_t;
/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

/**
 * @brief   I2C bus request function.
 * @param   slaveAddr[in]: slave device ID
 * @param   clkRate[in]: i2c bus clock rate
 * @return  i2c bus handle/ERROR_ID
 * @see
 */ 
int gp_ti2c_bus_request(ti2c_set_value_t *ti2c_set_value);

/**
 * @brief   I2C bus release function.
 * @param   handle[in]: i2c bus handle 
 * @return  SP_OK(0)/ERROR_ID
 * @see
 */
int gp_ti2c_bus_release(ti2c_set_value_t * ti2c_set_value);

/**
 * @brief   I2C bus write function
 * @param   handle[in]: i2c bus handle
 * @param   data[in]: data to write
 * @param   len[in]: data length
 * @return  data length/I2C_FAIL(-1)
 */
int gp_ti2c_bus_write(int handle, unsigned char* data, unsigned int len);

/**
 * @brief   I2C bus write function
 * @param   handle[in]: i2c bus handle
 * @param		address[in]: address to write
 * @param   data[in]: data to write
 * @return  
 */
int gp_ti2c_bus_address_write(int handle, unsigned int address, int8_t data);

/**
 * @brief   I2C bus read function
 * @param   handle[in]: i2c bus handle
 * @param   data[out]: data read from slave device
 * @param   len[in]: data length
 * @return  data length/I2C_FAIL(-1)
 */
int gp_ti2c_bus_read (int handle, unsigned char* data, unsigned int len);

/**
 * @brief   I2C bus read function
 * @param   handle[in]: i2c bus handle
 * @param		address[in]: address to read
 * @param   readBuf[out]: data buffer
 * @return  
 */
int gp_ti2c_bus_address_read(int handle, unsigned int address, unsigned char* readBuf);

/**
 * @brief   I2C bus read function
 * @param   handle[in]: i2c bus handle
 * @param		address[in]: address to read
 * @param   readBuf[out]: data buffer
 * @return  
 */
int gp_ti2c_bus_normal_read(int handle, unsigned int address, unsigned char* readBuf, unsigned int size);

/**
 * @brief   I2C bus write function
 * @param   handle[in]: i2c bus handle
 * @param		address[in]: address to write
 * @param   data[in]: data to write
 * @return  
 */
int gp_ti2c_bus_normal_write(int handle, unsigned int address, int8_t data);

/**
 * @brief   I2C bus data transfer function
 * @param   handle[in]: i2c bus handle
 * @return  data length/ERROR_ID
 */
int gp_ti2c_bus_xfer(ti2c_set_value_t* handle);

/**
 * @brief   Bus Barricade for i2c driver (Only for CMOS Sensor)
 * @param   handle[in]: i2c bus handle
 * @param   en[in]: 1 enable or 0 disable
 * @return  SP_OK(0)/SP_FAIL(1)
 * @see
 */
int gp_ti2c_bus_barricade( ti2c_set_value_t* ti2c_set_value, int en);

/**
 * @brief   I2C opeartion mode
 * @param   en[in]: 1:interrupt mode, 0:polling mode
 * @return  
 */
void gp_ti2c_bus_int_mode_en ( int en );

void ti2c_apbdmairq_callback(void);
int gp_ti2c_bus_init(void);

#endif /*_GP_TI2C_BUS_H_ */
