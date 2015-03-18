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
 * @file    gp_ti2c_bus.c
 * @brief   Implement of ti2c bus driver.
 * @author  Simon.Hsu
 * @since   2012/5/18
 * @date    2012/5/18
 */
#include <mach/kernel.h>
#include <mach/module.h>
#include <mach/cdev.h>
#include <mach/diag.h>
#include <mach/gp_ti2c_bus.h>
#include <mach/gp_board.h>
//#include <mach/gp_pin_grp.h>
#include <mach/hal/hal_ti2c_bus.h>
#include <mach/gp_apbdma0.h>
#include <mach/gp_gpio.h>
#include <mach/clk/gp_clk_core.h>

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/
#define TI2C_APBDMA_BUFSIZE		128*2

#define ti2c_no_int

spinlock_t	ti2c_lock;

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/

 
/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
typedef struct ti2c_bus_info_s {
	struct miscdevice dev;          /*!< @brief i2c bus device */
	struct semaphore sem;           /*!< @brief mutex semaphore for i2c bus ops */
	unsigned int open_count;        /*!< @brief i2c bus device open count */
} ti2c_bus_info_t;


/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/


/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static ti2c_bus_info_t *ti2c_bus = NULL;

#ifdef ti2c_no_int
static int gp_ti2c_int_mode_en = 0;	
#else
static int gp_ti2c_int_mode_en = 1;		//1 -- interrupt mode enable
#endif

struct semaphore semTransmitDataReady, semDMA;

static int ChkNack;

static int apbdma_handle = 0;
gpApbdma0Param_t apbdma_ti2c;
static UINT8 *dmabuf;
static int waitDma;

//Only for test
#define DebugCode	1

#if	DebugCode
int txint, dmaint, txdn, dmadn;
#endif
//static int ti2cpin;
//static int iolevel;

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

static void ti2c_clk_calc (ti2c_set_value_t* ti2c_set_value)
{
	unsigned long sys_apb_clk = 0;
	
	if( gp_clk_get_rate( (int*)"clk_sys_apb", (int*) &sys_apb_clk ) )
	{
		dev_dbg(NULL, "no peripheral clock\n");
		sys_apb_clk = 27000000;	
	}

	if( ti2c_set_value->clockPclk != sys_apb_clk ) {
		//Pclk wasn't set or changed.
		ti2c_set_value->clockTransform = (sys_apb_clk/ti2c_set_value->clockRate/1000/2)-1;
		ti2c_set_value->clockPclk = sys_apb_clk;
	//	printk("[Ti2C]clockRate[%d]clockTrans[%d]Pclk[%d]\n", ti2c_set_value->clockRate, ti2c_set_value->clockTransform, (int)ti2c_set_value->clockPclk );	 
	}
}

void gp_ti2c_irq_en(int en){
	//Enable global Ti2C INT
	gpHalTi2cIRQEn(en);
	//Enable local Ti2c INT
#ifdef ti2c_no_int
    gpHalTi2cIntEnSet( HAL_I2C_INT_ARBITRATION_LOST |
        				HAL_I2C_INT_NACK |
        	//			HAL_I2C_INT_TRANSMIT_READY |
        				HAL_I2C_INT_RECEIVE_READY, en);
#else
    gpHalTi2cIntEnSet( HAL_I2C_INT_ARBITRATION_LOST |
        				HAL_I2C_INT_NACK |
        				HAL_I2C_INT_TRANSMIT_READY |
        				HAL_I2C_INT_RECEIVE_READY, en);
#endif
}

/**
 * @brief   I2C opeartion mode
 * @param   en[in]: 1:interrupt mode, 0:polling mode
 * @return  
 */
void gp_ti2c_bus_int_mode_en ( int en )
{
	gp_ti2c_int_mode_en = en;
}
EXPORT_SYMBOL(gp_ti2c_bus_int_mode_en);


static void gpTi2cDmaInit( UINT8 *pDataA, UINT8 *pDataB, UINT32 dataCnta, UINT32 dataCntb, UINT8 dir ){
	gp_apbdma0_stop(apbdma_handle);
	apbdma_ti2c.dir = dir;
	apbdma_ti2c.buf0 = (char *)pDataA;
	apbdma_ti2c.ln0 = dataCnta;
	apbdma_ti2c.buf1 = (char *)pDataB;
	apbdma_ti2c.ln1 = dataCntb;
//	apbdma_ti2c.dataWidth = TRANS_8BIT;
	gp_apbdma0_en(apbdma_handle, apbdma_ti2c);
}

static void gpTi2cDmaRestart(void)
{
	gp_apbdma0_stop(apbdma_handle);
}

static void gp_ti2c_io_check(void)
{
/*	int handled, handlec;
	int value=0;
	int count=0;
	gpio_content_t ctxd;
	gpio_content_t ctxc;
	
	gpHalPinGrpSwitch(6,0);
	
	ctxd.pin_index = MK_GPIO_INDEX( 0, 0, 6, 13 );
	handled = gp_gpio_request(ctxd.pin_index, "TI2C_SDA");
	ctxc.pin_index = MK_GPIO_INDEX( 0, 0, 6, 14 );
	handlec = gp_gpio_request(ctxc.pin_index, "TI2C_SCL");	
	
	gp_gpio_set_function( handled, 0 );
	gp_gpio_set_direction( handled, GPIO_DIR_INPUT );
	gp_gpio_get_value( handled, &value);
	
	while(value==0) {
		DIAG_ERROR("[TI2C ERROR]: SDA is low.... set to output high\n");
		gp_gpio_set_direction( handlec, GPIO_DIR_OUTPUT );
		gp_gpio_set_output( handlec, 0, 0 );
		udelay(100);
		gp_gpio_set_output( handlec, 1, 0 );
		gp_gpio_get_value( handled, &value);
		count++;
		if(count>=50) {
			DIAG_ERROR("[TI2C ERROR]: SDA is low.... can't release\n");
			break;
		}
	}
	gp_gpio_release( handled );
	gp_gpio_release( handlec );
	udelay(100);
	gpHalPinGrpSwitch(6,1);*/
}

void ti2c_apbdmairq_callback(void)
{
	if(waitDma) {
		up(&semDMA);
		dmaint++;
	}
} EXPORT_SYMBOL(ti2c_apbdmairq_callback);

/**
 * @brief the timer/counter irq callback function handle
 * @param irq [in] irq index
 * @param dev_id [in] timer handle
 * @return IRQ_HANDLED
 */
static irqreturn_t gp_ti2c_irq_handler(int irq, void *ti2c_dev)
{	
	if( (gpHalTi2cIntFlagGet() & HAL_I2C_INT_RECEIVE_READY) == HAL_I2C_INT_RECEIVE_READY )	{
		gpHalTi2cIntFlagClr( HAL_I2C_INT_RECEIVE_READY );
	} else if( (gpHalTi2cIntFlagGet() & HAL_I2C_INT_NACK) == HAL_I2C_INT_NACK ) {
		gpHalTi2cIntFlagClr( HAL_I2C_INT_NACK );
		up(&semTransmitDataReady);
		txint++;
		ChkNack = 1;
	} else if( (gpHalTi2cIntFlagGet() & HAL_I2C_INT_TRANSMIT_READY)  == HAL_I2C_INT_TRANSMIT_READY ){
		gpHalTi2cIntFlagClr( HAL_I2C_INT_TRANSMIT_READY );
		if(!ChkNack) {
			up(&semTransmitDataReady);
			txint++;
		}
	} else if ( (gpHalTi2cIntFlagGet() & HAL_I2C_INT_ARBITRATION_LOST) == HAL_I2C_INT_ARBITRATION_LOST ){
		gpHalTi2cIntFlagClr( HAL_I2C_INT_ARBITRATION_LOST );
	} else {
		return IRQ_NONE;
		printk("[%s][%d]OO, no interrupt\n", __FUNCTION__, __LINE__);
	}
	
    return IRQ_HANDLED;
}

static int gp_ti2c_wait_TxReady(int ms)
{
	int ret=0;
#ifdef ti2c_no_int
	ret = gpHalTi2cWaitTxRdy(ms);
	if(ret<0) {
		DIAG_ERROR("[TI2C ERROR]: Transmit data fail(TimeOut or Nack), ack=%d\n", ChkNack);
		return -1;
	}
#else
	ret = down_timeout(&semTransmitDataReady, ms);
	if(ret<0 || ChkNack==1) {
		DIAG_ERROR("[TI2C ERROR]: Transmit data fail(TimeOut or Nack), ack=%d\n", ChkNack);
		if(ChkNack)
			txdn++;
		return -1;
	}
	txdn++;
#endif
	return 0;
}

static int gp_ti2c_Receive_Data(UINT8 *data)
{
	int ret;
	ret = gpHalTi2cWaitDR();
	if(ret<0) {
		DIAG_ERROR("[TI2C ERROR]:[%s][%d]--Receive data fail\n", __FUNCTION__, __LINE__);
		gpHalTi2cSendCmd(CMD_RCVR|CMD_GEN_STP);
		return ret;
	}
	*data = gpHalTi2cRxData();
	return ret;
}

static int gp_ti2c_Send_Data(UINT8 data)
{
	int ret;
	gpHalTi2cTxData(data);
	ret = gp_ti2c_wait_TxReady(30);
	if(ret<0) {
		gpHalTi2cSendCmd(CMD_XMIT|CMD_GEN_STP);
		ret = -1;
	}
	return ret;
}

static int gp_ti2c_Send_SlaveAddr(UINT8 slaveAddrMode, UINT16 slaveAddr, UINT8 mode)
{
	int ret=0;
	
	ret = gpHalTi2cSendCmd(CMD_XMIT|CMD_GEN_STR);
	if(ret<0) return ret;
	
	ret = gp_ti2c_Send_Data((slaveAddr & 0xff) | mode);
	if(ret<0) return ret;

	if(slaveAddrMode == TI2C_NORMAL_SLAVEADDR_16BITS) {
		ret = gp_ti2c_Send_Data((slaveAddr & 0xff00) >> 8);
		if(ret<0) return ret;
	}
	
	if(mode==1)
		ret = gpHalTi2cSendCmd(CMD_RCVR);

	return ret;
}

static int gp_ti2c_read(UINT8 mode, UINT8 slaveAddrMode, UINT16 slaveAddr, UINT8 *pbuf, UINT32 cnt)
{
	int ret=0, i;
	
	ChkNack = 0;
	ret = gp_ti2c_Send_SlaveAddr(slaveAddrMode, slaveAddr, 1);
	if(ret<0) goto __err_handle;
	
	for(i=0; i<cnt; i++) {
		ret = gp_ti2c_Receive_Data(pbuf+i);
		if(ret<0) goto __err_handle;
		if(i==cnt-1) {
			if(mode==TI2C_BURST_READ_STOP_ACKEND_MODE)
				ret = gpHalTi2cSendCmd(CMD_RCVR|CMD_GEN_STP|CMD_GEN_ACK);
			else
				ret = gpHalTi2cSendCmd(CMD_RCVR|CMD_GEN_STP|CMD_GEN_NACK);
		} else {
			ret = gpHalTi2cSendCmd(CMD_RCVR|CMD_GEN_ACK);
		}
		if(ret<0) goto __err_handle;
	}
	ret = gpHalTi2cWaitBusIdle();
	if(ret<0) DIAG_ERROR("[TI2C ERROR]:[%s][%d]--Bus idle fail\n", __FUNCTION__, __LINE__);
	
	gpHalTi2cReset();
	
__err_handle:
	return ret;
}

static int gp_ti2c_dmaread(UINT8 mode, UINT8 slaveAddrMode, UINT16 slaveAddr, UINT8 *pbuf, UINT32 cnt)
{
	int ret;
	UINT8 *phy_addr = NULL;

	waitDma = 1;
	ChkNack = 0;
	phy_addr = (UINT8 *)dma_map_single(NULL, pbuf, cnt, DMA_FROM_DEVICE);
	gpTi2cDmaInit(phy_addr, NULL, cnt, 0, 0);
	if(slaveAddrMode==TI2C_NORMAL_SLAVEADDR_16BITS) {
		ret = gp_ti2c_Send_SlaveAddr(slaveAddrMode, slaveAddr, 1);
		if(ret<0) goto __err_handle;
	}
	gpHalTi2cMasterFlowSetting(slaveAddrMode, slaveAddr, cnt, 2);
/*	if(cnt==1 && slaveAddrMode!=TI2C_NORMAL_SLAVEADDR_16BITS) {
		ret = gp_ti2c_wait_TxReady(30);
		if(ret<0) {
			gpHalTi2cSendCmd(CMD_RCVR|CMD_GEN_STP);
			goto __err_handle;
		}
	}*/
	ret = down_timeout(&semDMA, 30);
	if(ret<0){
		gpHalTi2cSendCmd(CMD_RCVR|CMD_GEN_STP);
		DIAG_ERROR("[TI2C ERROR]: DMAread Receive Data fail\n");
		goto __err_handle;
	}
	dmadn++;
	ret = gpHalTi2cSendCmd(CMD_RCVR|CMD_GEN_STP);
	if(ret<0) DIAG_ERROR("[TI2C ERROR]: DMAread Send stop fail\n");
	
	ret = gpHalTi2cWaitBusIdle();
	if(ret<0) DIAG_ERROR("[TI2C ERROR]: DMAread Bus idle fail\n");
	
__err_handle:
	waitDma = 0;
	gpHalTi2cClear();
	gpHalTi2cReset();
	gpTi2cDmaRestart();
	dma_unmap_single(NULL, (int)phy_addr, cnt, DMA_FROM_DEVICE);
	
	return ret;
}

static int gp_ti2c_write(UINT8 mode, UINT8 slaveAddrMode, UINT16 slaveAddr, UINT8 subAddrMode, UINT16 subAddr, UINT8 *pbuf, UINT32 cnt)
{
	int ret=0, i;
	
	ChkNack = 0;
	ret = gp_ti2c_Send_SlaveAddr(slaveAddrMode, slaveAddr, 0);
	if(ret<0) return ret;
	
	if(subAddrMode!=TI2C_NORMAL_SUBADDR_NO) {
		ret = gp_ti2c_Send_Data(subAddr & 0xff);
		if(ret<0) return ret;
		if(subAddrMode == TI2C_NORMAL_SUBADDR_16BITS) {
			ret = gp_ti2c_Send_Data((subAddr & 0xff00) >> 8);
			if(ret<0) return ret;
		}
	}
	for(i=0; i<cnt; i++) {
		ret = gp_ti2c_Send_Data(pbuf[i]);
		if(ret<0) return ret;
	}
	if(mode!=TI2C_BURST_READ_NOSTOPBIT_MODE) {
		ret = gpHalTi2cSendCmd(CMD_XMIT|CMD_GEN_STP);
		if(ret<0) DIAG_ERROR("[TI2C ERROR]: normal write mode Send stop fail\n");
	
		ret = gpHalTi2cWaitBusIdle();
		if(ret<0) DIAG_ERROR("[TI2C ERROR]: normal write mode Bus idle fail\n");
	}
	gpHalTi2cReset();
	return ret;
}

static int gp_ti2c_dmawrite(UINT8 mode, UINT8 slaveAddrMode, UINT16 slaveAddr, UINT8 subAddrMode, UINT16 subAddr, UINT8 *pbuf, UINT32 cnt)
{
	int ret=0;
	int bufbCnt=0;
	int writeCnt=0;
	UINT8 *pa;
	
	ChkNack = 0;
	if(slaveAddrMode==TI2C_NORMAL_SLAVEADDR_16BITS)
		dmabuf[bufbCnt++] = (slaveAddr & 0xff00) >> 8;
	if( subAddrMode == TI2C_NORMAL_SUBADDR_8BITS || subAddrMode == TI2C_NORMAL_SUBADDR_16BITS ) {
		dmabuf[bufbCnt++] = subAddr & 0xff;
		if( subAddrMode == TI2C_NORMAL_SUBADDR_16BITS )
			dmabuf[bufbCnt++] = (subAddr & 0xff00) >> 8;
	}
	if( (bufbCnt+cnt)>TI2C_APBDMA_BUFSIZE ) {
		writeCnt = TI2C_APBDMA_BUFSIZE - bufbCnt;
	} else {
		writeCnt = cnt;
	}
	memcpy((char *)(dmabuf+bufbCnt), pbuf, writeCnt);
	pa = (UINT8 *)dma_map_single(NULL, dmabuf, TI2C_APBDMA_BUFSIZE, DMA_TO_DEVICE);
	
	gpHalTi2cMasterFlowSetting(slaveAddrMode, slaveAddr, bufbCnt+writeCnt, 1);
/*	ret = gp_ti2c_wait_TxReady(30);
	if(ret<0){
		gpHalTi2cSendCmd(CMD_XMIT|CMD_GEN_STP);
		goto __err_handle;
	}*/
	while((bufbCnt+cnt)!=0) {
//		gpHalTi2cSetDataCnt(bufbCnt+writeCnt);
		gpTi2cDmaInit(pa, NULL, bufbCnt+writeCnt, 0, 1);
		ret = gp_ti2c_wait_TxReady(30);
		if(ret<0) {
			ChkNack = 0;
			/* clear FIFO work around solution */
	//		gpHalTi2cMasterFlowSetting(HAL_I2C_NORMAL_SLAVEADDR_16BITS, 0x00, 1, 1);
	//		gp_ti2c_wait_TxReady(20);
			gpHalTi2cSendCmd(CMD_XMIT|CMD_GEN_STP);
			goto __err_handle;
		}
		bufbCnt = 0;
		cnt = cnt - writeCnt;
		if(cnt!=0) {
			if(cnt>TI2C_APBDMA_BUFSIZE) {
				writeCnt = TI2C_APBDMA_BUFSIZE;
			} else {
				writeCnt = cnt;
			}
			pbuf = pbuf + TI2C_APBDMA_BUFSIZE;
			memcpy((char *)(dmabuf), pbuf, writeCnt);
		}
	}
	
	if(mode!=TI2C_BURST_READ_NOSTOPBIT_MODE)
		ret = gpHalTi2cSendCmd(CMD_XMIT|CMD_GEN_STP);
		
__err_handle:
	gpHalTi2cClear();
	gpHalTi2cReset();
	gpTi2cDmaRestart();
	dma_unmap_single(NULL, (int)pa, TI2C_APBDMA_BUFSIZE, DMA_TO_DEVICE);
	return ret;
}


/**
 * @brief   I2C bus request function.
 * @param   ti2c_set_value[in]: i2c handle
 * @return  i2c bus handle/ERROR_ID
 * @see
 */
int gp_ti2c_bus_request(ti2c_set_value_t *ti2c_set_value)
{
	ti2c_bus->open_count++;
	if(ti2c_set_value->clockRate!=0)
		ti2c_clk_calc(ti2c_set_value);
	return 0;
}
EXPORT_SYMBOL(gp_ti2c_bus_request);

/**
 * @brief   I2C bus release function.
 * @param   handle[in]: i2c bus handle
 * @return  SP_OK(0)/SP_FAIL(1)
 * @see
 */
int gp_ti2c_bus_release(ti2c_set_value_t* ti2c_set_value)
{
	printk("release\n");
	ti2c_bus->open_count--;
 	return 0;
}
EXPORT_SYMBOL(gp_ti2c_bus_release);

int gp_ti2c_bus_init()
{
	static int initial=0;
	int ret;
	
	if(initial==0) {
		/* request apbdma channel for TI2C */
/*		apbdma_ti2c.module = TI2C;
		apbdma_handle = gp_apbdma_request(0, apbdma_ti2c);
		if(apbdma_handle<0) {
			DIAG_ERROR("[TI2C INFO]: TI2C request apbdma fail\n");
			goto __err_dma_request;
		}*/
		apbdma_ti2c.module = TI2C;
		apbdma_handle = gp_apbdma0_request(5);
		if(apbdma_handle<0) {
			DIAG_ERROR("[TI2C INFO]: TI2C request apbdma fail\n");
			goto __err_dma_request;
		}		
		gp_apbdma0_stop(apbdma_handle);
		dmabuf = kmalloc(TI2C_APBDMA_BUFSIZE, GFP_DMA);
		if(dmabuf==NULL) {
			DIAG_ERROR("[TI2C ERROR]: apbdma buf kmalloc fail\n");
			goto __err_kmalloc_fail;
		}
		printk("dmabuf malloc success\n");
		gpHalTi2cBusInit();
	
		/* register irq & enable interrupt */
		sema_init( &semTransmitDataReady, 0);
		sema_init( &semDMA, 0);
		
		gp_apbdma0_irq_attach(apbdma_handle, (gp_apbdma0_irq_handle_t)&ti2c_apbdmairq_callback);
		
		ret = request_irq(IRQ_TI2C, gp_ti2c_irq_handler, IRQF_DISABLED, "ti2c", ti2c_bus);
		if(ret) {
			DIAG_ERROR("[TI2C ERROR]: request irq fail\n");
			goto __err_request_irq;
		}
		
	//	gp_ti2c_apbclk_en();
		gp_ti2c_irq_en(1);
	
//		gpHalPinGrpSwitch(6,1);
	}
	initial=1;
	return 0;

__err_request_irq:
	kfree(dmabuf);
__err_kmalloc_fail:
	gp_apbdma0_release(apbdma_handle);
__err_dma_request:
	return -1;
}
EXPORT_SYMBOL(gp_ti2c_bus_init);

int using_flag = 0;
/**
 * @brief   I2C bus write function
 * @param   handle[in]: i2c bus handle
 * @return  data length/-ERROR_ID
 */
int gp_ti2c_bus_xfer(ti2c_set_value_t* hd)
{
	int ret=0;
	int subAddr=0;
	int i;
	static int flag=0;
		
	if( hd->slaveAddr == 0 || hd->clockRate == 0 ) {
		DIAG_ERROR("[TI2C ERROR]: Invalid variable. slaveAddr[0x%x]clockRate[%d]\n", hd->slaveAddr, hd->clockRate);	 
		return -EIO;
	}
#ifdef ti2c_no_int
	if(using_flag == 1) {
		DIAG_ERROR("someone use this function right now\n");
		return -88;
	}
	using_flag = 1;
#else
	if (down_interruptible(&ti2c_bus->sem) != 0) {
		return -ERESTARTSYS;
	}
#endif
	
	if(gp_ti2c_int_mode_en)
		hd->apbdmaEn=1;
	else
		hd->apbdmaEn=0;
#ifdef ti2c_no_int
	if(flag == 0) {
		ti2c_clk_calc(hd);
		gpHalTi2cBusSetClkRate(hd->clockTransform);
		flag = 1;
	}
#else
	ti2c_clk_calc(hd);
	gpHalTi2cBusSetClkRate(hd->clockTransform);
#endif
		
	if(hd->transmitMode==TI2C_NORMAL_READ_MODE) {
		if(hd->apbdmaEn==1)
			ret = gp_ti2c_dmaread(hd->transmitMode, hd->slaveAddrMode, hd->slaveAddr, hd->pBuf, hd->dataCnt);
		else 
			ret = gp_ti2c_read(hd->transmitMode, hd->slaveAddrMode, hd->slaveAddr, hd->pBuf, hd->dataCnt);
		if(ret<0) goto __err_recovery;
	} else if(hd->transmitMode==TI2C_NORMAL_WRITE_MODE) {
		subAddr = (hd->pSubAddr == 0)? 0:*hd->pSubAddr;
		if(hd->apbdmaEn==1)
			ret = gp_ti2c_dmawrite(hd->transmitMode, hd->slaveAddrMode, hd->slaveAddr, hd->subAddrMode, subAddr, hd->pBuf, hd->dataCnt);
		else
			ret = gp_ti2c_write(hd->transmitMode, hd->slaveAddrMode, hd->slaveAddr, hd->subAddrMode, subAddr, hd->pBuf, hd->dataCnt);
		if(ret<0) goto __err_recovery;
	} else if(hd->transmitMode==TI2C_BURST_WRITE_MODE) {
		subAddr = (hd->pSubAddr == 0)? 0:*hd->pSubAddr;
		for(i=0; i<hd->dataCnt; i++) {
			if(hd->apbdmaEn==1)
				ret = gp_ti2c_dmawrite(hd->transmitMode, hd->slaveAddrMode, hd->slaveAddr, hd->subAddrMode, subAddr, (hd->pBuf+i), 1);
			else
				ret = gp_ti2c_write(hd->transmitMode, hd->slaveAddrMode, hd->slaveAddr, hd->subAddrMode, subAddr, (hd->pBuf+i), 1);
			if(ret<0) goto __err_recovery;
		}
	} else if(hd->transmitMode==TI2C_BURST_READ_STOP_MODE ||
				hd->transmitMode==TI2C_BURST_READ_NOSTOPBIT_MODE ||
				hd->transmitMode==TI2C_BURST_READ_STOP_ACKEND_MODE ) {
		subAddr = (hd->pSubAddr == 0)? 0:*hd->pSubAddr;
		
		if(hd->apbdmaEn==1) {
			ret = gp_ti2c_dmawrite(hd->transmitMode, hd->slaveAddrMode, hd->slaveAddr, hd->subAddrMode, subAddr, NULL, 0);
			if(ret<0) goto __err_recovery;
			ret = gp_ti2c_dmaread(hd->transmitMode, hd->slaveAddrMode, hd->slaveAddr, hd->pBuf, hd->dataCnt);
			if(ret<0) goto __err_recovery;
		} else {
			ret = gp_ti2c_write(hd->transmitMode, hd->slaveAddrMode, hd->slaveAddr, hd->subAddrMode, subAddr, NULL, 0);
			if(ret<0) goto __err_recovery;
			ret = gp_ti2c_read(hd->transmitMode, hd->slaveAddrMode, hd->slaveAddr, hd->pBuf, hd->dataCnt);
			if(ret<0) goto __err_recovery;
		}
	} else {
		DIAG_ERROR("[TI2C ERROR]: Mode not support\n");
		return -1;
	}
	gpHalTi2cClrFIFO();

#if	DebugCode
	if( txint!=txdn ) {
		gpHalTi2cSendStop();
		txint = txdn = 0;
		sema_init( &semTransmitDataReady, 0 );
		DIAG_ERROR("[TI2C ERROR]: tx ready not match\n");
	} 
	if(dmaint!=dmadn) {
		gpHalTi2cSendStop();
		dmaint = dmadn = 0;
		sema_init( &semDMA, 0 );
		DIAG_ERROR("[TI2C ERROR]: dma ready not match\n");
	}
#endif
#ifdef ti2c_no_int
	using_flag = 0;
#else
	up(&ti2c_bus->sem);
#endif
	return ret;

__err_recovery:
	DIAG_ERROR("[TI2C ERROR]: Ti2c error occur device is [%s]\n", hd->pDeviceString);
	sema_init( &semTransmitDataReady, 0 );
	sema_init( &semDMA, 0 );
	gpHalTi2cClrFIFO();
//	gp_ti2c_io_check();				//Check SDA & SCL IO status and if SDA can't pull high, toggle SCL to let device release SDA pin.
	ret = gpHalTi2cSendStop();
	if(ret<0) {
		DIAG_ERROR("[TI2C ERROR]: recovery send stop fail\n");
	}
	gpTi2cDmaRestart();
#ifdef ti2c_no_int
	using_flag = 0;
#else
	up(&ti2c_bus->sem);
#endif
	
	return -1;
}
EXPORT_SYMBOL(gp_ti2c_bus_xfer);

/**
 * @brief   I2C bus device open function
 */
static int ti2c_bus_open(struct inode *inode, struct file *flip)
{
	int ret = 0;
	ti2c_set_value_t *hd = NULL;

	if (down_interruptible(&ti2c_bus->sem) != 0) {
		return -ERESTARTSYS;
	}

	hd = (ti2c_set_value_t *)kmalloc(sizeof(ti2c_set_value_t), GFP_KERNEL);
	if (NULL == hd) {
		ret = -ENOMEM;
		goto __BUS_OPEN_ERR;
	}
	memset(hd, 0, sizeof(ti2c_set_value_t));

	flip->private_data = (ti2c_set_value_t *)hd;
	
	gp_ti2c_bus_request(hd);
	
	up(&ti2c_bus->sem);	
	return 0;
	
__BUS_OPEN_ERR:
	up(&ti2c_bus->sem);
	return ret;
}

/**
 * @brief   I2C bus device ioctl function
 */
static long ti2c_bus_ioctl(struct file *flip, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	int dataCount = 0;
	int tempaddr;
	
	ti2c_set_value_t *hd = NULL;
	ti2c_set_value_t ti2c_write_data;
	ti2c_set_value_t ti2c_read_data;
	char *pMemData;
	hd = (ti2c_set_value_t *)flip->private_data;
	
	switch(cmd) {
		case TI2C_BUS_ATTR_SET:
		{
			ti2c_set_value_t handle;
			memset(&handle, 0, sizeof(handle));
			
			if (copy_from_user(&handle, (void __user*)arg, sizeof(handle))) {
				return -EFAULT;
			}
			hd->clockRate = handle.clockRate;
			hd->transmitMode = handle.transmitMode;
			hd->slaveAddrMode = handle.slaveAddrMode;
			hd->subAddrMode = handle.subAddrMode;
			hd->slaveAddr = handle.slaveAddr;
			hd->pBuf = handle.pBuf;
			hd->dataCnt = handle.dataCnt;
			ti2c_clk_calc(hd);
		}
		break;
		
		case TI2C_BUS_ADDRESS_WRITE:
		{
			if (copy_from_user((void*) &ti2c_write_data, (const void __user *) arg, sizeof(ti2c_set_value_t))) {
				ret = -EIO;
				break;
			}
			dataCount = ti2c_write_data.dataCnt;
			pMemData = kmalloc(dataCount, GFP_KERNEL);
			if (!pMemData) {
				ret = -ENOMEM;
				break;
			}
			if (copy_from_user((void*) pMemData, (const void __user *) ti2c_write_data.pBuf, dataCount)) {
				ret = -EIO;
				kfree(pMemData);
				break;
			}
			ti2c_write_data.pBuf = pMemData;
			ret = gp_ti2c_bus_xfer(&ti2c_write_data);
			kfree(pMemData);
		}
		break;
		
		case TI2C_BUS_ADDRESS_READ:
		{
			if (copy_from_user((void*) &ti2c_read_data, (const void __user *) arg, sizeof(ti2c_set_value_t))) {
				ret = -EIO;
				break;
			}
			dataCount = ti2c_read_data.dataCnt;
			pMemData = kmalloc(dataCount, GFP_KERNEL);
			if (!pMemData) {
				ret = -ENOMEM;
				break;
			}
			tempaddr = (int)ti2c_read_data.pBuf;
			ti2c_read_data.pBuf = pMemData;
			ret = gp_ti2c_bus_xfer(&ti2c_read_data);
			if(ret>=0) {
				ti2c_read_data.pBuf = (unsigned char *)tempaddr;
				if (copy_to_user ((void __user *) ti2c_read_data.pBuf, (const void *) pMemData, dataCount)) {
					ret = -EIO;
				}
			}
			kfree(pMemData);
		}
		break;
		
		default:
			ret = -ENOTTY;
		break;
	}
	return ret;
}

/**
 * @brief   I2C bus device write function
 */
static ssize_t ti2c_bus_write(struct file *flip, const char __user *buf, size_t count, loff_t *oppos)
{
	int ret = 0;
	ti2c_set_value_t *hd = NULL;
	hd = (ti2c_set_value_t *)flip->private_data;
	
	if (copy_from_user(hd, buf, sizeof(ti2c_set_value_t))) {
		ret = -EFAULT;
		goto __err_copy;
	}
	ret = gp_ti2c_bus_xfer(hd);
	if (ret < 0) {
		DIAG_ERROR("[TI2C ERROR]: ti2c bus write fail\n");
		ret = -ENXIO;
		goto __err_copy;
	}
	
__err_copy:
	return ret;
}

/**
 * @brief   I2C bus device read function
 */
static ssize_t ti2c_bus_read(struct file *flip, char __user *buf, size_t count, loff_t *oppos)
{
	int ret = 0;
	int dataCount = 0;
	int tempaddr;
	char *pMemData;
	
	ti2c_set_value_t *hd = NULL;
	hd = (ti2c_set_value_t *)flip->private_data;
	
	if (copy_from_user(hd, buf, sizeof(ti2c_set_value_t))) {
		ret = -EFAULT;
		goto __err_copy;
	}

	dataCount = hd->dataCnt;
	pMemData = kmalloc(dataCount, GFP_KERNEL);
	if (!pMemData) {
		ret = -ENOMEM;
		goto __err_copy;
	}
	tempaddr = (int)hd->pBuf;
	hd->pBuf = pMemData;
	
	ret = gp_ti2c_bus_xfer(hd);
	if (ret < 0) {
		DIAG_ERROR("[TI2C ERROR]: ti2c read fail\n");
		ret = -ENXIO;	
		goto __err_copy;
	} else {
		hd->pBuf = (unsigned char *)tempaddr;
		if (copy_to_user ((void __user *) hd->pBuf, (const void *) pMemData, dataCount)) {
			ret = -EIO;
		}
	}
	kfree(pMemData);
	
__err_copy:
	return ret;
}

/**
 * @brief   I2C bus device release function
 */
static int ti2c_bus_release(struct inode *inode, struct file *flip)
{
	int ret = -ENXIO;
	
	if (down_interruptible(&ti2c_bus->sem) != 0) {
		return -ERESTARTSYS;
	}
	
	gp_ti2c_bus_release((ti2c_set_value_t*)flip->private_data);
	kfree((int *)flip->private_data);
	
	if (0 == ti2c_bus->open_count) {
		ret = 0;
	}
	up(&ti2c_bus->sem);
	return ret;
}

/**
 * @brief   i2c device release
 */
static void gp_ti2c_device_release(struct device *dev)
{
	DIAG_INFO("remove i2c device ok\n");
}

static struct platform_device gp_ti2c_device = {
	.name	= "gp-ti2c",
	.id	= 0,
	.dev	= {
		.release = gp_ti2c_device_release,
	}
};

static const struct file_operations ti2c_bus_fops = {
	.owner 		= THIS_MODULE,
	.open 		= ti2c_bus_open,
	.release		= ti2c_bus_release,
	.unlocked_ioctl	= ti2c_bus_ioctl,
	.read 		= ti2c_bus_read,
	.write 		= ti2c_bus_write,
};

static int gp_ti2c_probe(struct platform_device *pdev)
{
	int ret=0;
	
//	iolevel = 1;
//	ti2cpin = gp_gpio_request(MK_GPIO_INDEX(3,0,43,27), "touch_reset");
//	gp_gpio_set_output(ti2cpin, iolevel, 0);
	
	if( gp_enable_clock( (int*)"TI2C", 1 ) )
	{
		dev_dbg(NULL, "TI2C clock enable fail\n");
		return -1;
	}
	
	ti2c_bus = (ti2c_bus_info_t *)kmalloc(sizeof(ti2c_bus_info_t),  GFP_KERNEL);
	if (ti2c_bus == NULL) {
		ret = -ENOMEM;
		DIAG_ERROR("[TI2C ERROR]: ti2c_bus kmalloc fail\n");
		goto __err_kmalloc;
	}
	memset(ti2c_bus, 0, sizeof(ti2c_bus_info_t));
	ChkNack = 0;
	waitDma = 0;
	txint=txdn=dmaint=dmadn=0;		//for test
	
	/* initialize ti2c device structure */
	sema_init(&ti2c_bus->sem, 1);
	ti2c_bus->dev.name  = "ti2c";
	ti2c_bus->dev.minor = MISC_DYNAMIC_MINOR;
	ti2c_bus->dev.fops  = &ti2c_bus_fops;
	
//	spin_lock_init(ti2c_lock);
	using_flag = 0;
	
	/* register device */
	ret = misc_register(&ti2c_bus->dev);
	if (ret != 0) {
		DIAG_ERROR("[TI2C ERROR]: ti2c device register fail\n");
		goto __err_device_register;
	}
	
	ret = gp_ti2c_bus_init();
	if (ret < 0) {
		DIAG_ERROR("[TI2C ERROR]: ti2c bus init fail\n");
		goto __err_i2c_bus_fail;
	}
	return ret;

__err_i2c_bus_fail:
	misc_deregister(&ti2c_bus->dev);
__err_device_register:
	kfree(ti2c_bus);
	ti2c_bus = NULL;
__err_kmalloc:
	return ret;
}

static int gp_ti2c_remove(struct platform_device *pdev){
	gp_ti2c_irq_en(0);
//	free_irq(IRQ_APBDMA, (void *)ti2c_bus);
	gp_apbdma0_release(apbdma_handle);
//	gpHalPinGrpSwitch(6,0);
	misc_deregister(&ti2c_bus->dev);
	kfree(dmabuf);
	kfree(ti2c_bus);
	ti2c_bus = NULL;
	return 0;
}

static int gp_ti2c_suspend(struct platform_device *pdev, pm_message_t state)
{
	int ret;
	int retry=0;
__TI2C_SUSPEND_RETRY:
	ret = down_timeout(&ti2c_bus->sem, 500);
	if (ret != 0) {
		DIAG_ERROR("[TI2C INFO]: Get Sem Retry\n", __FUNCTION__, __LINE__);
		retry++;
		if(retry<=5)
			goto __TI2C_SUSPEND_RETRY;
		else {
			DIAG_ERROR("[TI2C ERR]: TI2C suspend fail\n");
			ret = gpHalTi2cSendStop();
			sema_init( &semTransmitDataReady, 0 );
			sema_init( &semDMA, 0 );
			gpHalTi2cClrFIFO();
			gp_ti2c_io_check();
		}
	}
	gp_apbdma0_release(apbdma_handle);
	apbdma_handle = 0;
	gp_ti2c_irq_en(0);
//	gpHalPinGrpSwitch(6,0);
	return 0;
}

static int gp_ti2c_resume(struct platform_device *pdev)
{	
/*	apbdma_ti2c.module = TI2C;
	apbdma_handle = gp_apbdma_request(0, apbdma_ti2c);
	if(apbdma_handle!=TI2C) {
		DIAG_ERROR("[TI2C INFO]: TI2C request apbdma fail\n");
	}*/
	
	if( gp_enable_clock( (int*)"TI2C", 1 ) )
	{
		dev_dbg(NULL, "TI2C clock enable fail\n");
		return -1;
	}
	apbdma_ti2c.module = TI2C;
	apbdma_handle = gp_apbdma0_request(5);
	gp_apbdma0_stop(apbdma_handle);
	
	gp_apbdma0_irq_attach(apbdma_handle, (gp_apbdma0_irq_handle_t)&ti2c_apbdmairq_callback);
	waitDma = 0;
	
	gpHalTi2cBusInit();
	gp_ti2c_irq_en(1);
	
	/*init i2c bus register*/
//	gpHalPinGrpSwitch(6,1);
	up(&ti2c_bus->sem);
	return 0;
}

/**
 * @brief   ti2c driver define
 */
static struct platform_driver gp_ti2c_driver = {
	.probe  = gp_ti2c_probe,
	.remove = gp_ti2c_remove,
	.suspend = gp_ti2c_suspend,
	.resume = gp_ti2c_resume,
	.driver	= {
		.owner	= THIS_MODULE,
		.name	= "gp-ti2c"
	}
};

 /**
 * @brief   I2C bus driver init function
 */
static int  __init ti2c_bus_init(void)
{
	platform_device_register(&gp_ti2c_device);
	return platform_driver_register(&gp_ti2c_driver);
}

/**
 * @brief   I2C bus driver exit function
 */
static void __exit ti2c_bus_exit(void)
{
	platform_device_unregister(&gp_ti2c_device);
	platform_driver_unregister(&gp_ti2c_driver);
}

module_init(ti2c_bus_init);
module_exit(ti2c_bus_exit);

/**************************************************************************
 *                  M O D U L E    D E C L A R A T I O N                  *
 **************************************************************************/

MODULE_AUTHOR("Generalplus");
MODULE_DESCRIPTION("Generalplus Ti2c Bus Driver");
MODULE_LICENSE_GP;
