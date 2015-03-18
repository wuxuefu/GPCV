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
 * @file	gp_clk_cor.c
 * @brief	GP clock core file.
 * @author	Dunker Chen
 */

#include <mach/kernel.h>
#include <mach/module.h>
#include <mach/diag.h>
#include <mach/cdev.h>
#include <mach/typedef.h>
#include <mach/clk/gp_clk_core.h>
#include <mach/clk/clk-private.h>

/**************************************************************************
 *                           C O N S T A N T S                            *
**************************************************************************/

/**************************************************************************
 *                              M A C R O S                               *
**************************************************************************/

#ifdef CONFIG_PM
	static int gp_clock_suspend(struct platform_device *pdev, pm_message_t state)
	{
		return 0;
	}
	
	static int gp_clock_resume(struct platform_device *pdev)
	{
		return 0;
	}
#else
	#define gp_clock_suspend NULL
	#define gp_clock_resume NULL
#endif

/**************************************************************************
 *                          D A T A    T Y P E S                          *
**************************************************************************/

typedef struct gp_clock_s {
	struct miscdevice dev;     				
	//spinlock_t lock;
	//struct semaphore sem;
	int open_count;
} gp_clock_t;

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
**************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
**************************************************************************/

static int gp_clock_fops_open(struct inode *inode, struct file *file);
static int gp_clock_fops_release(struct inode *inode, struct file *file);
static long gp_clock_fops_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

static void gp_clock_device_release(struct device *dev);

/**************************************************************************
 *                         G L O B A L    D A T A                         *
**************************************************************************/

static gp_clock_t* gp_clock_info = NULL;

static struct file_operations gp_clock_fops = {
	.owner		= THIS_MODULE,
	.open		= gp_clock_fops_open,
	.release	= gp_clock_fops_release,
	.unlocked_ioctl = gp_clock_fops_ioctl,
};

static struct platform_device gp_clock_device = {
	.name	= "gp-clock",
	.id	= -1,
    .dev	= {
		.release = gp_clock_device_release,
    }
};

static struct platform_driver gp_clock_driver = {
	.driver		= {
		.name	= "gp-clock",
		.owner	= THIS_MODULE,
	},
	.suspend	= gp_clock_suspend,
	.resume		= gp_clock_resume,
};

/**************************************************************************
 *             F U N C T I O N    I M P L E M E N T A T I O N S           *
**************************************************************************/

/**
* @brief 		Register all clock. (Only use for system boot)
* @param 		xtal[in]: xtal clock.
* @return		None.
*/
void __init gp_register_baseclocks(
	unsigned long xtal)
{
	/* ----- PLL clock register ----- */
	gp_pll_init( xtal );
	/* ----- Reference clock register ----- */
	gp_ref_clk_init();
	/* ----- Bus clock register ----- */
	gp_bus_clk_init();
	/* ----- SCU clock register -----*/
	gp_scua_clk_init();
	gp_scub_clk_init();
	gp_scuc_clk_init();
}
/**
* @brief 		Print clock information.
* @param 		level[in]: Print level.
* @param 		clk[in]: Clock structure.
* @return		None.
*/
void gp_clk_print(
	int level, 
	struct clk *clk)
{
	char kmg[] = { '\0', 'K', 'M', 'G' };
	char *dbg_levle = (level) ?  KERN_NOTICE : KERN_DEBUG;
	unsigned long rate = clk->rate;
	unsigned long rate_level = 0;
	
	
	while( ( rate % 1000 == 0 ) &&  ( rate !=0 ) )
	{
		rate = rate / 1000;
		rate_level ++;	
	}
	
	printk("%s[%s], parent = %s, clk rate = %lu %cHZ\n", dbg_levle, clk->name, (clk->parent)? clk->parent->name: "ROOT", rate, kmg[rate_level] );
}

/**
* @brief 	Clock get function 
* @param 	clock_name[in]: base/device clock name to get current base clock
* @param   freq[out]: clock real setting value
* @return  SP_OK(0)/ERROR_ID
*/
int gp_clk_get_rate( 
	int *clock_name,
	int*freq )
{
	struct clk *clkp;

	clkp = clk_get(NULL, (char *)clock_name);
	if (IS_ERR(clkp) || clkp == NULL){
		printk("ERROR clock name[%s]", (clkp == NULL) ? "NULL" : (char *)clkp);
		return -ENOENT;
	}

	*freq = clk_get_rate(clkp);
	
	clk_put(clkp);
	
	return 0;

}
EXPORT_SYMBOL(gp_clk_get_rate);

/**
* @brief 	Clock set function
* @param 	clock_name[in]: base/device clock name to get current base clock
* @param   freq[in]: clock real setting value
* @return  SP_OK(0)/ERROR_ID
*/
int gp_clk_set_rate( 
	int *clock_name,
	unsigned long freq )
{
	struct clk *clkp;
	int ret;

	clkp = clk_get(NULL, (char *)clock_name);
	if (IS_ERR(clkp) || clkp == NULL){
		printk("ERROR clock name[%s]", (clkp == NULL) ? "NULL" : (char *)clkp);
		return -ENOENT;
	}

	ret = clk_set_rate(clkp, freq );
	
	clk_put(clkp);
	
	return ret;

}
EXPORT_SYMBOL(gp_clk_set_rate);

/**
* @brief 	Enable/Disable clock interface function
* @param 	clock_name[in]: base/device clock name
* @param 	enable[in]:  1: enable, 0 : diable
* @return 	SUCCESS/FAIL.
*/
int gp_enable_clock(
	int *clock_name,
	int enable)
{
	struct clk *clkp;
	int ret = SP_SUCCESS;

	printk( KERN_DEBUG "[%s][%d] run, clock_name=[%s]\n", __FUNCTION__, __LINE__, (char *)clock_name);

	clkp = clk_get(NULL, (char *)clock_name);
	if (IS_ERR(clkp) || clkp == NULL){
		printk(KERN_ERR "Failed to Get clock %s\n",(char *)clock_name);
    	return -ENOENT;
    }

	if( enable )
		ret = clk_prepare_enable(clkp);
	else
		clk_disable_unprepare(clkp);
	
	clk_put(clkp);

	return ret;
}
EXPORT_SYMBOL(gp_enable_clock);

/**
* @brief 	Set clock parent
* @param 	clock_name[in]: base/device clock name
* @param 	parent_name[in]:  parent name
* @return 	SUCCESS/FAIL.
*/
int gp_clk_set_parent(
	int *clock_name,
	int *parent_name)
{
	struct clk *clk, *parent;
	int ret = SP_SUCCESS;

	printk( KERN_DEBUG "[%s][%d] set parent, clock_name=[%s]\n", __FUNCTION__, __LINE__, (char *)clock_name);

	clk = clk_get(NULL, (char *)clock_name);
	if (IS_ERR(clk) || clk == NULL){
		printk(KERN_ERR "Failed to Get clock %s\n",(char *)clock_name);
    	return -ENOENT;
    }
    
    parent = clk_get(NULL, (char *)parent_name);
	if (IS_ERR(parent) || parent == NULL){
		printk(KERN_ERR "Failed to Get clock %s\n",(char *)parent_name);
		clk_put(clk);
    	return -ENOENT;
    }
    
    ret = clk_set_parent( clk, parent );
	
	clk_put(clk);
	clk_put(parent);

	return ret;
}
EXPORT_SYMBOL(gp_clk_set_parent);

/**
* @brief 	Clock open function.
* @param 	inode [in]: Inode pointer.
* @param 	file [in]: File pointer.
* @return: 	SUCCESS(0)
*/
static int gp_clock_fops_open(struct inode *inode, struct file *file)
{
	if (!gp_clock_info) {
		DIAG_ERROR("Driver not initial\n");
		return -ENXIO;
	}

	gp_clock_info->open_count++;

	return 0;
}

/**
* @brief 	Clock release function.
* @param 	inode [in]: Inode pointer.
* @param 	file [in]: File pointer.
* @return: 	SUCCESS(0)
*/
static int gp_clock_fops_release(struct inode *inode, struct file *file)
{
	if (gp_clock_info->open_count <= 0) {
		DIAG_ERROR("Clock device already close\n");
		gp_clock_info->open_count = 0;
		return -ENXIO;
	}
	else {
		gp_clock_info->open_count -- ;
		return 0;
	}
}

/**
* @brief 	Clock io control function.
* @param 	file [in]: File pointer.
* @param 	cmd [in]: Command.
* @param 	arg [in]: Argument.
* @return: 	SUCCESS(0)/ERROR_ID
*/
static long gp_clock_fops_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	long ret = -ENOTTY;

	switch (cmd) {
	case IOCTL_GP_CLOCK_SET:	
		// for test 
		//ret = gp_all_clock_change(arg, 0);
		break;
	case IOCTL_GP_CLOCK_ARM:
	{
		//gp_clock_group_t group;
		//copy_from_user(&group, (void __user*)arg, sizeof(gp_clock_group_t));
		//ret = gp_clk_set_arm(&group);
		break;
	}
	case IOCTL_GP_CLOCK_CEVA:
	{
		//gp_clock_group_t group;
		//copy_from_user(&group, (void __user*)arg, sizeof(gp_clock_group_t));
		//ret = gp_clk_set_ceva(&group);
		break;
	}
	case IOCTL_GP_CLOCK_SYS:
		// for test 
		//ret = gp_all_clock_change(arg, 3);
		break;
	case IOCTL_GP_CLOCK_ENABLE:
		ret = gp_enable_clock((int *)arg, 1);
		break;
	case IOCTL_GP_CLOCK_DISABLE:
		ret = gp_enable_clock((int *)arg, 0);
		break;
	case IOCTL_GP_CLOCK_USAGE_DUMP:
		//gp_dump_clock_usage((int *)arg);
		break;
	case IOCTL_GP_CLOCK_DUMP_ALL:
		//gp_dump_clock();
		break;
	case IOCTL_GP_CLOCK_SPLL_SEL:
	{
		int val=0;
		get_user(val, (int *)arg);
		//ret = gp_arm_use_spll(val);
		break;
	}

	default:
		break;
	}
	return ret;
}

/**
 * @brief   device release function 
 * @return  None
 * @see
 */
static void gp_clock_device_release(struct device *dev)
{
	return ;
}

/**
* @brief 	Clock driver initial function.
* @return 	SUCCESS/ERROR_ID.
*/
static int __init gp_clock_module_init(void)
{
	int ret = 0;

	gp_clock_info = kzalloc(sizeof(gp_clock_t),GFP_KERNEL);
	if ( NULL == gp_clock_info ) {
		return -ENOMEM;
	}	

	platform_device_register(&gp_clock_device);
	ret = platform_driver_register(&gp_clock_driver);
	if (ret) {
		DIAG_ERROR("%s: failed to add adc driver\n", __func__);
		return ret;
	}
	/* register misc device */
	gp_clock_info->dev.name = "clock-mgr";
	gp_clock_info->dev.minor = MISC_DYNAMIC_MINOR;
	gp_clock_info->dev.fops  = &gp_clock_fops;
	ret = misc_register(&gp_clock_info->dev);
	if ( ret != 0 ) {
		DIAG_ERROR(KERN_ALERT "misc register fail\n");
		kfree(gp_clock_info);
		return ret;
	}

	return ret;
}

/**
* @brief 	Clock driver exit function.
* @return 	None.
*/
static void __exit gp_clock_module_exit(void)
{
	platform_device_unregister(&gp_clock_device);
	platform_driver_unregister(&gp_clock_driver);

	misc_deregister(&gp_clock_info->dev);

	if (gp_clock_info) 
		kfree(gp_clock_info);
	gp_clock_info = NULL;
}

/**************************************************************************
 *                  M O D U L E    D E C L A R A T I O N                  *
**************************************************************************/

module_init(gp_clock_module_init);
module_exit(gp_clock_module_exit);
MODULE_LICENSE_GP;