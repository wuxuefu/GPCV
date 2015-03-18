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
#include <mach/module.h>
#include <mach/kernel.h>
#include <mach/cdev.h>
#include <linux/cdev.h>

#include <media/v4l2-common.h>
#include <media/v4l2-device.h>
#include <media/v4l2-ioctl.h>
#include <mach/sensor_mgr.h>
#include <mach/hal/hal_mipi.h>
#include <mach/gp_mipi.h>

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/
#define VIC_MIPI_RX0_PPU	IRQ_MIPI_RX0
#define VIC_MIPI_RX1_CDSP	IRQ_MIPI_RX1
#define C_MIPI_RX0			C_MIPI_RX_PPU
#define C_MIPI_RX1			C_MIPI_RX_CDSP

#define MIPI_IRQ_EN			0

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
#define RETURN(x)	{nRet = x; goto __return;}
#define DERROR	printk 
#if 1
#define DEBUG	printk
#else
#define DEBUG(...)
#endif

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
typedef struct gpMipiDev_s 
{
	struct miscdevice dev;
	struct semaphore  sem;
	unsigned char	idx;
	unsigned char 	open_cnt;
	unsigned char	start_flag;
	unsigned char	CurFrameStatus;
	gpMipiCfg_t		cfg;
	gpMipiEcc_t		ecc;
	gpMipiCCIR601_t	ccir601;
} gpMipiDev_t;
	
/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static gpMipiDev_t *p_mipi_dev[2];

void 
gp_mipi_set_irq(
	unsigned char idx, 
	unsigned int enable
)
{
	unsigned int bits;

	bits = MIPI_CCIR_SOF | 
			MIPI_HD_1BIT_ERR | 
			MIPI_HD_NBIT_ERR | 
			MIPI_DATA_CRC_ERR |
			MIPI_LANE0_SOT_SYNC_ERR;
	
	if(p_mipi_dev[idx]->cfg.lane_num == MIPI_2_LANE) {
		bits |= MIPI_LANE1_SOT_SYNC_ERR;
	}	 
		
	if(enable) {
		gpHalMipiSetIrq(idx, ENABLE, bits);
	} else {
		gpHalMipiSetIrq(idx, DISABLE, bits);
	}
}
EXPORT_SYMBOL(gp_mipi_set_irq);

unsigned int
gp_mipi_get_curframe_status(
	unsigned char idx
)
{
	unsigned int flag;

	gp_mipi_set_irq(idx, DISABLE);
	flag = p_mipi_dev[idx]->CurFrameStatus;
	p_mipi_dev[idx]->CurFrameStatus = 1;
#if MIPI_IRQ_EN == 1
	gp_mipi_set_irq(idx, ENABLE);
#endif	
	return flag;
}
EXPORT_SYMBOL(gp_mipi_get_curframe_status);

void
gp_mipi_set_io(
	unsigned int io
)
{
	if(io == C_IO_IS_MIPI_PIN) {
		gpHalMipiPhyInSelect(C_IO_IS_MIPI_PIN);
	} else if(io == C_IO_IS_CMOS_PIN) {
		gpHalMipiPhyInSelect(C_IO_IS_CMOS_PIN);
	}
}
EXPORT_SYMBOL(gp_mipi_set_io);

static void 
gp_mipi_init_para(
	gpMipiDev_t *argp
)
{
	argp->cfg.mipi_enable = ENABLE;
	argp->cfg.low_power_en = DISABLE;
	argp->cfg.sample_edge = D_PHY_SAMPLE_POS;
	argp->cfg.lane_num = MIPI_1_LANE;

	argp->ecc.ecc_check_en = ENABLE;
	argp->ecc.ecc_order = MIPI_ECC_ORDER3;
	argp->ecc.data_mask_time = 100;
	argp->ecc.check_hs_seq = MIPI_CHECK_HS_SEQ;

	argp->ccir601.data_from_mmr = DISABLE;
	argp->ccir601.data_type = MIPI_RAW8;
	argp->ccir601.h_size = 2048;
	argp->ccir601.v_size = 1536;
	argp->ccir601.h_back_porch = 0x0;
	argp->ccir601.h_front_porch = 0x4;
	argp->ccir601.blanking_line_en = ENABLE;
}

static void
gp_mipi_set_cfg(
	unsigned char idx,
	gpMipiCfg_t	*argp
)
{
	gpHalMipiSetGloblaCfg(idx, argp->low_power_en, argp->sample_edge, argp->lane_num);
}

static void
gp_mipi_set_ecc(
	unsigned char idx,
	gpMipiEcc_t	*argp
)
{
	unsigned char data_mask_cnt;
	unsigned int clock_out, delta_t;
	struct clk *clock;

	clock = clk_get(NULL, "clk_ref_sys");
	clock_out = clk_get_rate(clock);
	
	/* change to ns */
	delta_t = 1000*1000000 / clock_out;

	/* must > 40ns */
	if(argp->data_mask_time < 40) {
		argp->data_mask_time = 40;
	}

	/* get data mask count */
	data_mask_cnt = argp->data_mask_time / delta_t;
	if(argp->data_mask_time / delta_t) {
		data_mask_cnt ++;
	}
	
	gpHalMipiSetEcc(idx, argp->ecc_order, argp->ecc_check_en);
	gpHalMipiSetMaskCnt(idx, data_mask_cnt, argp->check_hs_seq);
	DEBUG("data_mask_time[%d], %dns\n", data_mask_cnt, data_mask_cnt*delta_t);
}

static void
gp_mipi_set_ccir601(
	unsigned char idx,
	gpMipiCCIR601_t	*argp
)
{
	if(argp->data_from_mmr) {
		gpHalMipiSetDataFmt(idx, 
							argp->data_from_mmr, 
							argp->data_type);
	} else {
		gpHalMipiSetDataFmt(idx, DISABLE, 0);
	}
	
	if(argp->data_type == MIPI_YUV422) {
		gpHalMipiSetImageSize(idx, 1, 
							argp->h_size << 1, 
							argp->v_size);
	} else {
		gpHalMipiSetImageSize(idx, 0, 
							argp->h_size, 
							argp->v_size);
	}
	
	gpHalMipiSetCCIR601IF(idx, 
						argp->h_back_porch, 
						argp->h_front_porch, 
						argp->blanking_line_en);
}

static long 
gp_mipi_ioctl(
	struct file *filp, 
	unsigned int cmd, 
	unsigned long arg
)
{
	int nRet = 0;
	int io;
	gpMipiDev_t *pMipiDev = filp->private_data;

	if(pMipiDev == 0) {
		return EBADF;
	}
	
	if(down_interruptible(&pMipiDev->sem) != 0) {
		return -ERESTARTSYS;
	}
	
	switch(cmd)
	{
	case MIPI_IOCTL_S_CFG:
		nRet = copy_from_user((void *)&pMipiDev->cfg, (void *)arg, sizeof(gpMipiCfg_t));
		if(nRet < 0) {
			RETURN(-EINVAL);
		}
		
		if(pMipiDev->start_flag) {
			gp_mipi_set_cfg(pMipiDev->idx, &pMipiDev->cfg);
		}
		break;
		
	case MIPI_IOCTL_G_CFG:
		nRet = copy_to_user((void *)arg, (void *)&pMipiDev->cfg, sizeof(gpMipiCfg_t));
		break;
		
	case MIPI_IOCTL_S_ECC:
		nRet = copy_from_user((void *)&pMipiDev->ecc, (void *)arg, sizeof(gpMipiEcc_t));
		if(nRet < 0) {
			RETURN(-EINVAL);
		}
		
		if(pMipiDev->start_flag) {
			gp_mipi_set_ecc(pMipiDev->idx, &pMipiDev->ecc);
		}
		break;
	case MIPI_IOCTL_G_ECC:
		nRet = copy_to_user((void *)arg, (void *)&pMipiDev->ecc, sizeof(gpMipiEcc_t));
		break;
		
	case MIPI_IOCTL_S_CCIR601:
		nRet = copy_from_user((void *)&pMipiDev->ccir601, (void *)arg, sizeof(gpMipiCCIR601_t));
		if(nRet < 0) {
			RETURN(-EINVAL);
		}

		if(pMipiDev->start_flag) {
			gp_mipi_set_ccir601(pMipiDev->idx, &pMipiDev->ccir601);
		}
		break;
		
	case MIPI_IOCTL_G_CCIR601:
		nRet = copy_to_user((void *)arg, (void *)&pMipiDev->ccir601, sizeof(gpMipiCCIR601_t));
		break;
		
	case MIPI_IOCTL_S_START:
		if(arg) {
			char sensor_name[32];
			int i;
			
			nRet = copy_from_user((void *)sensor_name, (void *)arg, sizeof(sensor_name));
			if(nRet < 0) {
				RETURN(-EINVAL);
			}
			
			for(i=0; i<4; i++) {
				struct v4l2_subdev *sd;
				callbackfunc_t *cb;
				char *port;
				sensor_config_t *sensor;
				
				nRet = gp_get_sensorinfo(i, (int*)&sd, (int*)&cb, (int*)&port, (int*)&sensor);
				if(nRet < 0) {
					RETURN(-EINVAL);
				}
				
				if(strcmp(sensor_name, sd->name) == 0) {
					printk("mipi setting = %s\n", sensor_name);
					pMipiDev->cfg.mipi_sep_clk_en = sensor->mipi_config->mipi_sep_clk_en;
					pMipiDev->cfg.mipi_sep_clk = sensor->mipi_config->mipi_sep_clk;
					pMipiDev->cfg.mipi_sep_clk_src = sensor->mipi_config->mipi_sep_clk_src;
					pMipiDev->cfg.byte_clk_edge = sensor->mipi_config->byte_clk_edge;
					pMipiDev->cfg.low_power_en = sensor->mipi_config->low_power_en;
					pMipiDev->cfg.sample_edge = sensor->mipi_config->byte_clk_edge;
					pMipiDev->cfg.lane_num = sensor->mipi_config->lane_num;

					pMipiDev->ecc.ecc_check_en = sensor->mipi_config->ecc_check_en;
					pMipiDev->ecc.ecc_order = sensor->mipi_config->ecc_order;
					pMipiDev->ecc.data_mask_time = sensor->mipi_config->data_mask_time;
					pMipiDev->ecc.check_hs_seq = sensor->mipi_config->check_hs_seq;
					break;
				}
			}
		} 
		gp_mipi_set_cfg(pMipiDev->idx, &pMipiDev->cfg);
		gp_mipi_set_ecc(pMipiDev->idx, &pMipiDev->ecc);
		gp_mipi_set_ccir601(pMipiDev->idx, &pMipiDev->ccir601);
		gpHalMipiPhyOutSelect(pMipiDev->idx);
		pMipiDev->start_flag = 1;
		pMipiDev->CurFrameStatus = 1;
	#if MIPI_IRQ_EN == 1	
		gp_mipi_set_irq(pMipiDev->idx, ENABLE);
	#endif
		break;

	case MIPI_IOCTL_S_IO_PIN:
		nRet = copy_from_user((void *)&io, (void *)arg, sizeof(int));
		if(nRet < 0) {
			RETURN(-EINVAL);
		}
		gp_mipi_set_io(io);
		break;
		
	default:
		RETURN(-ENOTTY);	/* Inappropriate ioctl for device */
	}

__return:
	up(&pMipiDev->sem);
	return nRet; 
}


irqreturn_t
gp_mipi_isr(
	int irq,
	void *dev_id
)
{
	unsigned int status;
	gpMipiDev_t *pDev = (gpMipiDev_t *)dev_id;
	
	status = gpHalMipiGetIrqStatus(pDev->idx);
	if(status == 0) {
		return IRQ_NONE;
	}

	if(status & MIPI_LANE0_SOT_SYNC_ERR) {
		DERROR("Lane0_SOT_Sync_ERR\n");
		pDev->CurFrameStatus = 0;
	} 

	if(status & MIPI_HD_1BIT_ERR) {
		DERROR("HD_1BIT_ERR\n");
		pDev->CurFrameStatus = 0;
	}

	if(status & MIPI_HD_NBIT_ERR) {
		DERROR("HD_NBIT_ERR\n");
		pDev->CurFrameStatus = 0;
	}

	if(status & MIPI_DATA_CRC_ERR) {
		DERROR("DATA_CRC_ERR\n");
		pDev->CurFrameStatus = 0;
	}

	if(status & MIPI_LANE1_SOT_SYNC_ERR) {
		DERROR("Lane1_SOT_Sync_ERR\n");
		pDev->CurFrameStatus = 0;
	}

	if(status & MIPI_CCIR_SOF) {
		DERROR("CCIR_SOF\n");
	} 

	return IRQ_HANDLED;
}

static int 
gp_mipi0_open(
	struct inode *inode, 
	struct file *filp
)
{
	if(p_mipi_dev[C_MIPI_RX0]->open_cnt == 0) {
		DEBUG(KERN_WARNING "Mipi0_Open\n");
		filp->private_data = p_mipi_dev[C_MIPI_RX0];		
		p_mipi_dev[C_MIPI_RX0]->open_cnt = 1;
		p_mipi_dev[C_MIPI_RX0]->start_flag = 0;
		gp_mipi_init_para(p_mipi_dev[C_MIPI_RX0]);		
		gpHalMipiSetModuleClk(C_MIPI_RX0, ENABLE);
		gpHalMipiSetEnable(C_MIPI_RX0, ENABLE);
		/* set mipi pin input */
		gp_mipi_set_io(C_IO_IS_MIPI_PIN);
		return 0;
	}
	DERROR(KERN_WARNING "Mipi0_OpenFail\n");
	return -1;
}

static int 
gp_mipi1_open(
	struct inode *inode, 
	struct file *filp
)
{
	if(p_mipi_dev[C_MIPI_RX1]->open_cnt == 0) {
		DEBUG(KERN_WARNING "Mipi1_Open\n");
		filp->private_data = p_mipi_dev[C_MIPI_RX1];		
		p_mipi_dev[C_MIPI_RX1]->open_cnt = 1;
		p_mipi_dev[C_MIPI_RX1]->start_flag = 0;
		gp_mipi_init_para(p_mipi_dev[C_MIPI_RX1]);		
		gpHalMipiSetModuleClk(C_MIPI_RX1, ENABLE);
		gpHalMipiSetEnable(C_MIPI_RX1, ENABLE);
		/* set mipi pin input */
		gp_mipi_set_io(C_IO_IS_MIPI_PIN);
		return 0;
	}
	DERROR(KERN_WARNING "Mipi1_OpenFail\n");
	return -1;
}

static int 
gp_mipi_release(
	struct inode *inode, 
	struct file *filp
)
{
	gpMipiDev_t *pMipiDev = filp->private_data;
	
	if(pMipiDev->open_cnt == 1) {
		DEBUG(KERN_WARNING "Mipi_%d_Close\n", pMipiDev->idx);
		pMipiDev->open_cnt = 0;
		pMipiDev->start_flag = 0;
		gp_mipi_set_irq(pMipiDev->idx, DISABLE);
		gpHalMipiSetEnable(pMipiDev->idx, DISABLE);
		gpHalMipiSetModuleClk(pMipiDev->idx, DISABLE);
		return 0;
	}
	DERROR(KERN_WARNING "Mipi%d_CloseFail\n", pMipiDev->idx);
	return -1;
}

struct file_operations mipi0_fops = 
{
	.owner = THIS_MODULE, 
	.unlocked_ioctl = gp_mipi_ioctl,
	.open = gp_mipi0_open,
	.release = gp_mipi_release,
};

struct file_operations mipi1_fops = 
{
	.owner = THIS_MODULE, 
	.unlocked_ioctl = gp_mipi_ioctl,
	.open = gp_mipi1_open,
	.release = gp_mipi_release,
};

static void 
gp_mipi_device_release(
	struct device *dev
)                       
{                                                                           
	DEBUG("remove mipi device ok\n");                                      
}                                                                           
                                                                            
static struct platform_device gp_mipi0_device = {                             
	.name = "gp-mipi0",                                                         
	.id	= C_MIPI_RX0,                                                                  
	.dev = 
	{                                                                 
		.release = gp_mipi_device_release,                                       
	},                                                                        
};

static struct platform_device gp_mipi1_device = {                             
	.name = "gp-mipi1",                                                         
	.id	= C_MIPI_RX1,                                                                  
	.dev = 
	{                                                                 
		.release = gp_mipi_device_release,                                       
	},                                                                        
};


#ifdef CONFIG_PM
static int 
gp_mipi0_suspend(
	struct platform_device *pdev, 
	pm_message_t state
)
{
	if(p_mipi_dev[C_MIPI_RX0]->open_cnt > 0) {
		if(down_interruptible(&p_mipi_dev[C_MIPI_RX0]->sem) != 0) {
			return -ERESTARTSYS;
		}
		
		gpHalMipiSetModuleClk(C_MIPI_RX0, DISABLE);
		up(&p_mipi_dev[C_MIPI_RX0]->sem);
	}
	return 0;
}

static int 
gp_mipi1_suspend(
	struct platform_device *pdev, 
	pm_message_t state
)
{
	if(p_mipi_dev[C_MIPI_RX1]->open_cnt > 0) {
		if(down_interruptible(&p_mipi_dev[C_MIPI_RX1]->sem) != 0) {
			return -ERESTARTSYS;
		}
		
		gpHalMipiSetModuleClk(C_MIPI_RX1, DISABLE);
		up(&p_mipi_dev[C_MIPI_RX1]->sem);
	}
	return 0;
}

static int 
gp_mipi0_resume(
	struct platform_device *pdev
)
{
	if(p_mipi_dev[C_MIPI_RX0]->open_cnt > 0) {
		gpHalMipiSetModuleClk(C_MIPI_RX0, ENABLE);
	}
	return 0;
}

static int 
gp_mipi1_resume(
	struct platform_device *pdev
)
{
	if(p_mipi_dev[C_MIPI_RX1]->open_cnt > 0) {
		gpHalMipiSetModuleClk(C_MIPI_RX1, ENABLE);
	}
	return 0;
}
#else
#define gp_mipi0_suspend NULL
#define gp_mipi1_suspend NULL
#define gp_mipi0_resume NULL
#define gp_mipi1_resume NULL
#endif

static struct platform_driver gp_mipi0_driver = 
{
	.suspend = gp_mipi0_suspend,
	.resume = gp_mipi0_resume,
	.driver	= 
	{
		.owner	= THIS_MODULE,
		.name	= "gp-mipi0"
	},
};

static struct platform_driver gp_mipi1_driver = 
{
	.suspend = gp_mipi1_suspend,
	.resume = gp_mipi1_resume,
	.driver	= 
	{
		.owner	= THIS_MODULE,
		.name	= "gp-mipi1"
	},
};

static int __init 
mipi_init_module(
	void
)
{
	int nRet = -ENOMEM;
	
	DEBUG(KERN_WARNING "ModuleInit: mipi0/mipi1\n");
	p_mipi_dev[C_MIPI_RX0] = (gpMipiDev_t *)kzalloc(sizeof(gpMipiDev_t), GFP_KERNEL);
	if(p_mipi_dev[C_MIPI_RX0] == NULL) {
		DERROR("mipi0 kmalloc fail\n");
		RETURN(-ENOMEM);
	}

	p_mipi_dev[C_MIPI_RX1] = (gpMipiDev_t *)kzalloc(sizeof(gpMipiDev_t), GFP_KERNEL);
	if(p_mipi_dev[C_MIPI_RX1] == NULL) {
		DERROR("mipi1 kmalloc fail\n");
		RETURN(-ENOMEM);
	}

	/* request irq for mipi interrupt */
	nRet = request_irq(VIC_MIPI_RX0_PPU, 
						gp_mipi_isr, 
						IRQF_DISABLED,
						"MIPI_RX0_IRQ", 
						p_mipi_dev[C_MIPI_RX0]);
	if (nRet < 0) {
		DERROR("mipi0 irq fail\n");
		RETURN(-ENXIO);
	}

	/* request irq for mipi interrupt */
	nRet = request_irq(VIC_MIPI_RX1_CDSP, 
						gp_mipi_isr, 
						IRQF_DISABLED,
						"MIPI_RX1_IRQ", 
						p_mipi_dev[C_MIPI_RX1]);
	if (nRet < 0) {
		DERROR("mipi0 irq fail\n");
		RETURN(-ENXIO);
	}
	
	/* initialize */
	init_MUTEX(&p_mipi_dev[C_MIPI_RX0]->sem);
	init_MUTEX(&p_mipi_dev[C_MIPI_RX1]->sem);

	/* register char device */
	p_mipi_dev[C_MIPI_RX0]->dev.name  = "mipi";
	p_mipi_dev[C_MIPI_RX0]->dev.minor = MISC_DYNAMIC_MINOR;
	p_mipi_dev[C_MIPI_RX0]->dev.fops  = &mipi0_fops;
	p_mipi_dev[C_MIPI_RX0]->idx = C_MIPI_RX0;
	nRet = misc_register(&p_mipi_dev[C_MIPI_RX0]->dev);
	if(nRet != 0) {
		DERROR("mipi0 device register fail\n");
		RETURN(-ENXIO);
	}

	/* register char device */
	p_mipi_dev[C_MIPI_RX1]->dev.name  = "mipi1";
	p_mipi_dev[C_MIPI_RX1]->dev.minor = MISC_DYNAMIC_MINOR;
	p_mipi_dev[C_MIPI_RX1]->dev.fops  = &mipi1_fops;
	p_mipi_dev[C_MIPI_RX1]->idx = C_MIPI_RX1;
	nRet = misc_register(&p_mipi_dev[C_MIPI_RX1]->dev);
	if(nRet != 0) {
		DERROR("mipi1 device register fail\n");
		RETURN(-ENXIO);
	}
	
	/* register platform driver */
	platform_device_register(&gp_mipi0_device);
	platform_driver_register(&gp_mipi0_driver);

	platform_device_register(&gp_mipi1_device);
	platform_driver_register(&gp_mipi1_driver);

__return:
	if(nRet < 0) {
		DERROR(KERN_WARNING "MipiInitFail\n");
		kfree(p_mipi_dev[C_MIPI_RX0]);
		kfree(p_mipi_dev[C_MIPI_RX1]);
		p_mipi_dev[C_MIPI_RX0] = NULL;
		p_mipi_dev[C_MIPI_RX1] = NULL;
	}
	return nRet;
}

static void __exit 
mipi_exit_module(
	void
)
{
	/* free char device */
	misc_deregister(&p_mipi_dev[C_MIPI_RX0]->dev);
	misc_deregister(&p_mipi_dev[C_MIPI_RX1]->dev);

	free_irq(VIC_MIPI_RX0_PPU, p_mipi_dev[C_MIPI_RX0]);
	free_irq(VIC_MIPI_RX1_CDSP, p_mipi_dev[C_MIPI_RX1]);

	kfree(p_mipi_dev[C_MIPI_RX0]);
	kfree(p_mipi_dev[C_MIPI_RX1]);
	p_mipi_dev[C_MIPI_RX0] = NULL;
	p_mipi_dev[C_MIPI_RX1] = NULL;

	platform_device_unregister(&gp_mipi0_device);
	platform_driver_unregister(&gp_mipi0_driver);

	platform_device_unregister(&gp_mipi1_device);
	platform_driver_unregister(&gp_mipi1_driver);
}

module_init(mipi_init_module);
module_exit(mipi_exit_module);

/**************************************************************************
 *                  M O D U L E    D E C L A R A T I O N                  *
 **************************************************************************/
MODULE_AUTHOR("Generalplus");
MODULE_DESCRIPTION("Generalplus MIPI Driver");
MODULE_LICENSE_GP;
MODULE_VERSION("1.0");



