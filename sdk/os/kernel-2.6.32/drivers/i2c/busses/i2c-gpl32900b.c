
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>

#include <mach/gp_ti2c_bus.h>
#include <mach/gp_apbdma0.h>
#include <mach/hal/hal_common.h>
#include <mach/hal/hal_ti2c_bus.h>

struct gpl32900b_if {
	u8 __iomem		*reg; /* memory mapped registers */
	int			irq;
	spinlock_t		lock;
	struct i2c_msg		*msgs; /* messages currently handled */
	int			msgs_num; /* nb of msgs to do */
	int			msgs_push; /* nb of msgs read/written */
	int			msgs_done; /* nb of msgs finally handled */
	unsigned		push; /* nb of bytes read/written in msg */
	unsigned		done; /* nb of bytes finally handled */
	int			timeout_count; /* timeout retries left */
	struct timer_list	timeout_timer;
	struct i2c_adapter	adap;
	struct completion	complete;
	struct clk		*clk;
	struct resource		*res;
};

static struct gpl32900b_if gpl32900b_if;
ti2c_set_value_t hd;

static int gpl32900b_master_xfer(struct i2c_adapter *adap,
				struct i2c_msg *msgs, int num)
{
	int ret, i;
	
	if(num==0)
		return 0;
		
	for(i=0; i<num; i++) {
		if(msgs[i].flags & I2C_M_RD)
			hd.transmitMode = TI2C_NORMAL_READ_MODE;
		else
			hd.transmitMode = TI2C_NORMAL_WRITE_MODE;
				
		if(msgs[i].flags & I2C_M_TEN)
			hd.slaveAddrMode = TI2C_NORMAL_SLAVEADDR_16BITS;
		else
			hd.slaveAddrMode = TI2C_NORMAL_SLAVEADDR_8BITS;
		
		hd.slaveAddr = msgs[i].addr << 1;
		hd.clockRate = 100;
		hd.pBuf = msgs[i].buf;
		hd.subAddrMode = TI2C_NORMAL_SUBADDR_NO;
		hd.pSubAddr = 0;
		hd.dataCnt = msgs[i].len;
		ret = gp_ti2c_bus_xfer(&hd);
		if(ret<0)
			return i;
	}
	
	return num;
}

static u32 gpl32900b_functionality(struct i2c_adapter *adap)
{
	return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL;
}

static struct i2c_algorithm gpl32900b_algorithm = {
	.master_xfer   = gpl32900b_master_xfer,
	.functionality = gpl32900b_functionality,
};


static int gp_i2c_probe(struct platform_device *pdev)
{
	struct i2c_adapter *p_adap;
	struct gpl32900b_if *iface = &gpl32900b_if;

	int rc, ret;
	
	ret = gp_ti2c_bus_init();
	if (ret < 0) {
		goto __err_i2c_bus_fail;
	}

	p_adap = &iface->adap;
	strlcpy(p_adap->name, pdev->name, sizeof(p_adap->name));
	p_adap->algo = &gpl32900b_algorithm;
	p_adap->algo_data = iface;
	p_adap->nr = 0;
	p_adap->class = 0;
	p_adap->retries = 5;
	p_adap->dev.parent = &pdev->dev;
	rc = i2c_add_numbered_adapter(p_adap);
	if (rc<0)
		printk("error add adapter\n");

	printk("add adapter success\n");	
	
	return 0;

__err_i2c_bus_fail:
	return -1;
}

static int gp_i2c_remove(struct platform_device *pdev)
{
	struct gpl32900b_if *iface = platform_get_drvdata(pdev);
	
	i2c_del_adapter(&iface->adap);
	return 0;
}

static struct platform_driver gpl32900b_i2c_driver = {
	.probe		= gp_i2c_probe,
	.remove		= gp_i2c_remove,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "gp32900b-i2c",
	//	.pm	= S3C24XX_DEV_PM_OPS,
	},
};

static int __init i2c_adap_gpl32900b_init(void)
{
	return platform_driver_register(&gpl32900b_i2c_driver);
}
subsys_initcall(i2c_adap_gpl32900b_init);

static void __exit i2c_adap_gpl32900b_exit(void)
{
	platform_driver_unregister(&gpl32900b_i2c_driver);
}
module_exit(i2c_adap_gpl32900b_exit);

MODULE_DESCRIPTION("GPL32900B I2C Bus driver");
MODULE_AUTHOR("Simon Hsu, <simonhsu@generalplus.com.tw>");
MODULE_LICENSE("GPL");
