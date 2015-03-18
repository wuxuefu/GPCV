/**************************************************************************
 *                                                                        *
 *         Copyright (c) 2011 by Generalplus Technology Co., Ltd.         *
 *                                                                        *
 *  This software is copyrighted by and is the property of Generalplus    *
 *  Technology Co., Ltd. All rights are reserved by Generalplus Technology*
 *  Co., Ltd. This software may only be used in accordance with the       *
 *  corresponding license agreement. Any unauthorized use, duplication,   *
 *  distribution, or disclosure of this software is expressly forbidden.  *
 *                                                                        *
 *  This Copyright notice MUST not be removed or modified without prior   *
 *  written consent of Generalplus Technology Co., Ltd.                   *
 *                                                                        *
 *  Generalplus Technology Co., Ltd. reserves the right to modify this    *
 *  software without notice.                                              *
 *                                                                        *
 *  Generalplus Technology Co., Ltd.                                      *
 *  19, Innovation First Road, Science-Based Industrial Park,             *
 *  Hsin-Chu, Taiwan, R.O.C.                                              *
 *                                                                        *
 **************************************************************************/

 /**
 * @file gp_rtc.c
 * @brief RTC driver interface 
 * @author
 */
#include <mach/kernel.h>
#include <mach/module.h>
#include <mach/diag.h>
#include <mach/hal/regmap/reg_rtc.h>
#include <mach/hal/hal_rtc.h>
#include <mach/hal/hal_clock.h>
#include <linux/notifier.h>
#include <linux/reboot.h>
#include <mach/clk/gp_clk_core.h>

/**************************************************************************
 *                          C O N S T A N T S                          *
 **************************************************************************/
#define RTC_CLK_RATE  32768     //32.768Khz

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
struct gp_rtc {
	int			irq_rtc;
	struct rtc_device	*rtc;
	spinlock_t		lock;		/* Protects this structure */
	struct rtc_time	rtc_alarm;
};

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/

/**************************************************************************
 * @brief   Rtc clock enable/disable
 * @param   enable [in] 0:disable, 1:enable
 * @return  None
 * @see
 */
#if 0 
static void rtc_clock_enable(int enable)
{
	//gpHalScuClkEnable( SCU_B_PERI_RTC, SCU_B, enable);
	
}
#endif
/**
 * @brief   RTC irq handler
 */
static irqreturn_t gp_rtcdrv_irq(int irq, void *dev_id)
{
	struct gp_rtc *rtc = (struct gp_rtc *)dev_id;
	unsigned int rtsr;
	unsigned long events = 0;

	spin_lock(&rtc->lock);

	/* get and clear interrupt sources */
	rtsr = gpHalRtcIntSrcGet();
	
	if (!rtsr) {
		spin_unlock(&rtc->lock);
		return IRQ_NONE;
	}
		
	/* update irq data & counter */
	if (rtsr & SEC_INTR_STATUS) {
		events |= RTC_UF | RTC_IRQF;
	}
	
	if (rtsr & ALARM_INTR_STATUS) {
		events |= RTC_AF | RTC_IRQF;
	}

	if (events){
		rtc_update_irq(rtc->rtc, 1, events);
	}
	
	spin_unlock(&rtc->lock);

	return IRQ_HANDLED;
}

/**
 * @brief   RTC driver open
 */
static int gp_rtcdrv_open(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct gp_rtc *rtc = (struct gp_rtc *)platform_get_drvdata(pdev);
	int ret = 0;
#if 0	
	printk("[Jerry] request rtc irq \n");	
	ret = request_irq(rtc->irq_rtc, gp_rtcdrv_irq, IRQF_DISABLED, "rtc_irq", dev);
	if (ret < 0) {
		DIAG_ERROR("RTC can't get irq %i, err %d\n", rtc->irq_rtc, ret);
		goto open_err;
	}	
#endif
	return 0;
open_err:
	
	return ret;
}
	
/**
 * @brief   RTC interrupt control
 */
static int gp_rtcdrv_int_ctl(struct device *dev, unsigned int cmd,
		unsigned long arg)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct gp_rtc *rtc = (struct gp_rtc *)platform_get_drvdata(pdev);
	unsigned char sec_int_en = 2, alarm_int_en = 2;	
	int ret = 0;

	DIAG_VERB("[%s:%s] [0x%x]\n", __FILE__, __FUNCTION__, cmd);
	spin_lock_irq(&rtc->lock);

	/* alarm/sec interrupt enable/disable */
	switch (cmd) {
	case RTC_AIE_OFF:
		alarm_int_en = 0;
		break;
	case RTC_AIE_ON:
		alarm_int_en = 1;
		break;
	case RTC_UIE_OFF:
		sec_int_en = 0;
		break;
	case RTC_UIE_ON:
		sec_int_en = 1;
		break;
	default:		
		ret = -ENOIOCTLCMD;
	}

	if (ret == 0){
		gpHalRtcIntEnable(sec_int_en, alarm_int_en);	
	}
	
	spin_unlock_irq(&rtc->lock);
	return ret;
}

/**
 * @brief   RTC time read
 */
static int gp_rtcdrv_read_time(struct device *dev, struct rtc_time *tm)
{
//	struct gp_rtc *rtc = (struct gp_rtc *)dev_get_drvdata(dev);
	struct platform_device *pdev = to_platform_device(dev);
	struct gp_rtc *rtc = (struct gp_rtc *)platform_get_drvdata(pdev);
	unsigned int value = 0;
	unsigned long rtc_sec_cnt = 0;
	int ret = 0;

	spin_lock_irq(&rtc->lock);					
	ret = gpHalRtcGetTime(&value);	
	rtc_sec_cnt = value;
	spin_unlock_irq(&rtc->lock); 

	printk("read time: %d\n",rtc_sec_cnt);
	rtc_time_to_tm(rtc_sec_cnt, tm);
	printk("====>%d %d %d %d %d %d\n",tm->tm_year
		,tm->tm_mon
		,tm->tm_mday
		,tm->tm_hour
		,tm->tm_min
		,tm->tm_sec);	
	return 0;
}

/**
 * @brief   RTC time set
 */
static int gp_rtcdrv_set_time(struct device *dev, struct rtc_time *tm)
{
//	struct gp_rtc *rtc = (struct gp_rtc *)dev_get_drvdata(dev);
	struct platform_device *pdev = to_platform_device(dev);
	struct gp_rtc *rtc = (struct gp_rtc *)platform_get_drvdata(pdev);
	unsigned long time;
	unsigned long real_time = 0;
	int ret = 0;

//	printk("[%s:%s] :\n", __FILE__, __FUNCTION__);
//	printk("====>%d %d %d %d %d %d\n",tm->tm_year
//		,tm->tm_mon
//		,tm->tm_mday
//		,tm->tm_hour
//		,tm->tm_min
//		,tm->tm_sec);
	
	ret = rtc_tm_to_time(tm, &time);

//	printk("set time: %d\n",time);
	spin_lock_irq(&rtc->lock);

	/*Set time, and feedback the new time*/	
	if (ret == 0){
		ret = gpHalRtcSetTime(time);        
	}
	else{
		spin_unlock_irq(&rtc->lock); 	
		return ret;
	}

	spin_unlock_irq(&rtc->lock); 

	if (ret != 0) {
		return -EIO;
	}
	else {
		rtc_time_to_tm(real_time, tm);
	}
	
	return 0;
}

/**
 * @brief   RTC alarm read alarm
 */
static int gp_rtcdrv_read_alarm(struct device *dev, struct rtc_wkalrm *alrm)
{
//	struct gp_rtc *rtc = (struct gp_rtc *)dev_get_drvdata(dev);
	struct platform_device *pdev = to_platform_device(dev);
	struct gp_rtc *rtc = (struct gp_rtc *)platform_get_drvdata(pdev);
	unsigned int rtc_sec_cnt = 0;
	int ret = 0;	

	DIAG_VERB("[%s:%s]\n", __FILE__, __FUNCTION__);
	
	spin_lock_irq(&rtc->lock);		
	ret = gpHalRtcGetAlarmStatus(&alrm->enabled, &alrm->pending, &rtc_sec_cnt);
	spin_unlock_irq(&rtc->lock);  

	if (ret != 0) {
		return -EIO;
	}
	else {
		rtc_time_to_tm((unsigned long)rtc_sec_cnt, &alrm->time);
	}

	return 0;
}

/**
 * @brief   RTC alarm set alarm
 */
static int gp_rtcdrv_set_alarm(struct device *dev, struct rtc_wkalrm *alrm)
{
//	struct gp_rtc *rtc = (struct gp_rtc *)dev_get_drvdata(dev);
	struct platform_device *pdev = to_platform_device(dev);
	struct gp_rtc *rtc = (struct gp_rtc *)platform_get_drvdata(pdev);
	unsigned long time;
	int ret = 0;

	DIAG_VERB("[%s:%s]\n", __FILE__, __FUNCTION__);
	ret = rtc_tm_to_time(&alrm->time, &time);

	if (ret == 0)
	{
		spin_lock_irq(&rtc->lock);		
		ret = gpHalRtcSetAlarmStatus(alrm->enabled, alrm->pending, time);
		spin_unlock_irq(&rtc->lock);  
		if (ret != 0) {
			ret = -EIO;
		}
	}
	
	return ret;
}

static const struct rtc_class_ops gp_rtc_ops = {
	.open = gp_rtcdrv_open,
	.ioctl = gp_rtcdrv_int_ctl,
	.read_time = gp_rtcdrv_read_time,
	.set_time = gp_rtcdrv_set_time,
	.read_alarm = gp_rtcdrv_read_alarm,
	.set_alarm = gp_rtcdrv_set_alarm,
};

/**
 * @brief   RTC driver enable
 */
static void gp_rtcdrv_enable(struct platform_device *pdev, int en)
{
	struct gp_rtc *rtc = (struct gp_rtc *)platform_get_drvdata(pdev);

	spin_lock_irq(&rtc->lock);	
	
	gpHalRtcEnable(en);

	spin_unlock_irq(&rtc->lock); 
}

/**
 * @brief   RTC driver probe
 */
static int __devinit gp_rtcdrv_probe(struct platform_device *pdev)
{
	int ret;
	struct device *dev = &pdev->dev;
	struct gp_rtc *rtc;
	struct resource *res;
	unsigned int wdt_ctl;
	
//	DIAG_INFO("entering [%s]\n", __FUNCTION__);

	rtc = kzalloc(sizeof(struct gp_rtc), GFP_KERNEL);
	if (!rtc)
		return -ENOMEM;

	spin_lock_init(&rtc->lock);

	platform_set_drvdata(pdev, rtc);		
	/* Enable RTC Macro*/	
	gp_rtcdrv_enable(pdev, 1);
	
	device_init_wakeup(dev, 1);

	rtc->irq_rtc = IRQ_REALTIME_CLOCK;
	printk("rtc irq = %d\n", IRQ_REALTIME_CLOCK);
	rtc->rtc = rtc_device_register("gp-rtc", &pdev->dev, &gp_rtc_ops, THIS_MODULE);
	if (IS_ERR(rtc->rtc)) {
		ret = PTR_ERR(rtc->rtc);
		DIAG_ERROR("Failed to register RTC device -> %d\n", ret);
		goto err_rtc;
	}	

	ret = request_irq(rtc->irq_rtc, gp_rtcdrv_irq, IRQF_DISABLED, "rtc_irq", rtc);//|IRQF_SHARED, "rtc_irq", rtc);
	if (ret) {
		DIAG_ERROR("RTC can't get irq %i, err %d\n", rtc->irq_rtc, ret);
		goto err_rtc;
	}		
	return 0;

err_rtc:
	gp_rtcdrv_enable(pdev, 0);
	kfree(rtc);
	platform_set_drvdata(pdev, NULL);
	return ret;
}

/**
 * @brief   RTC driver remove
 */
static int __devexit gp_rtcdrv_remove(struct platform_device *pdev)
{
	struct gp_rtc *rtc = (struct gp_rtc *)platform_get_drvdata(pdev);
	
	if (!rtc || IS_ERR(rtc->rtc)) {
		return 0;
	}
	rtc_device_unregister(rtc->rtc);
	
	free_irq(rtc->irq_rtc, rtc);	
	gp_rtcdrv_enable(pdev, 0);	
	kfree(rtc);
	platform_set_drvdata(pdev, NULL);

	return 0;
}

#ifdef CONFIG_PM
/**
 * @brief   RTC driver suspend
 */
static int gp_rtcdrv_suspend(struct platform_device *pdev, pm_message_t state)
{
	
	struct gp_rtc *rtc = (struct gp_rtc *)platform_get_drvdata(pdev);
	unsigned int wdt_ctl;
	
	//printk("[gp_rtcdrv_suspend]\n");
	if (state.event == PM_EVENT_FREEZE) {	/*Only free irq in hibernation flow*/
		printk("[gp_rtcdrv_suspend][free rtc irq in hibernation flow!] \n");
		free_irq(rtc->irq_rtc, rtc);
		rtc->irq_rtc = 0;
	}	

	gp_rtcdrv_enable(pdev, 0);

#if 0	
	if (device_may_wakeup(&pdev->dev))
		enable_irq_wake(rtc->irq_rtc);
#endif	
	return 0;
	
}

/**
 * @brief   RTC driver resume
 */
static int gp_rtcdrv_resume(struct platform_device *pdev)
{
	
	struct gp_rtc *rtc = (struct gp_rtc *)platform_get_drvdata(pdev);
	unsigned int wdt_ctl;
	

	gp_rtcdrv_enable(pdev, 1);	

	if (rtc->irq_rtc == 0) {
		rtc->irq_rtc = IRQ_REALTIME_CLOCK;
		request_irq(rtc->irq_rtc, gp_rtcdrv_irq, IRQF_DISABLED, "rtc_irq", rtc);	//|IRQF_SHARED, "rtc_irq", rtc);	
	}	
#if 0	
	if (device_may_wakeup(&pdev->dev))
		disable_irq_wake(rtc->irq_rtc);
#endif	
	return 0;
}
#else
#define gp_rtcdrv_suspend	NULL
#define gp_rtcdrv_resume	NULL
#endif

static struct platform_driver gp_rtc_driver = {
	.probe		= gp_rtcdrv_probe,
	.remove		= gp_rtcdrv_remove,
	.suspend		= gp_rtcdrv_suspend,
	.resume		= gp_rtcdrv_resume,
	.driver		= {
		.name		= "gp-rtc",
		.owner		= THIS_MODULE,
	},
};

static struct resource gp_rtc_resources[] = {
	[0] = {
		.start  = 0x9000B000,
		.end	   = 0x9000BFFF,
		.flags  = IORESOURCE_MEM,
	},
	[1] = {
		.start  = IRQ_REALTIME_CLOCK,
		.end    = IRQ_REALTIME_CLOCK,
		.flags  = IORESOURCE_IRQ,
	},
};

static struct platform_device gp_device_rtc = {
	.name		= "gp-rtc",
	.id		= -1,
	.num_resources  = ARRAY_SIZE(gp_rtc_resources),
	.resource       = gp_rtc_resources,
};

#define GP_REBOOT_NORMAL 0x000A
#define GP_REBOOT_RECOVERY 0x000B

static int gp_rtc_notify_reboot(struct notifier_block *this,
					unsigned long code, void *unused)
{
	unsigned char *mode = (unsigned char *)unused;
	
	DIAG_INFO("[%s] [code = 0x%x] into rtc served field!\n", __FUNCTION__, code);
	if (code == SYS_RESTART && mode) {
		unsigned short val = gpHalRtcDummyRead();
		DIAG_INFO("save reboot cmd [%s] into rtc served field!\n", mode);
		
		if (strcmp(mode, "recovery") == 0){
			gpHalRtcDummyWrite(((val & 0xFFF0) | GP_REBOOT_RECOVERY));
		}
		else {
			gpHalRtcDummyWrite(((val & 0xFFF0) | GP_REBOOT_NORMAL));
		}
	}	
	else if ((code == SYS_DOWN) || (code == SYS_HALT) || (code == SYS_POWER_OFF)) {
		printk("gp_rtc_notify_reboot poweroff\n");
		gpHalRtcIntEnable(2, 0); // disable alarm irq
	}

	return NOTIFY_DONE;
}

static struct notifier_block gp_rtc_notifier = {
	.notifier_call =	gp_rtc_notify_reboot,
};

/**
 * @brief   RTC driver init
 */
static int __init gp_rtcdrv_init(void)
{
	int ret = 0;

	if( gp_enable_clock( (int*)"RTC", 1 ) )
	{
		DIAG_ERROR("RTC clock enable fail\n");
		return -1;
	}

	ret = platform_device_register(&gp_device_rtc);
	if (ret < 0) {
		DIAG_ERROR("unable to register rtc device: %d\n", ret);
	}
	
	ret = platform_driver_register(&gp_rtc_driver);
	if (ret < 0) {
		DIAG_ERROR("platform_driver_register rtc returned %d\n", ret);
	}
	
	register_reboot_notifier(&gp_rtc_notifier);

	return ret;	
}

/**
 * @brief   RTC driver exit
 */
static void __exit gp_rtcdrv_exit(void)
{
	platform_device_unregister(&gp_device_rtc);
	platform_driver_unregister(&gp_rtc_driver);
	unregister_reboot_notifier(&gp_rtc_notifier);
	gp_enable_clock( (int*)"RTC", 0 );
}

module_init(gp_rtcdrv_init);
module_exit(gp_rtcdrv_exit);

/**************************************************************************
 *                  M O D U L E    D E C L A R A T I O N                  *
 **************************************************************************/
MODULE_AUTHOR("Generalplus");
MODULE_DESCRIPTION("GP Realtime Clock Driver (RTC)");
MODULE_LICENSE_GP;

