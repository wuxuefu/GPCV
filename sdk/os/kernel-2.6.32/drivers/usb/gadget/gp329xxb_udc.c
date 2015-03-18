/*
 * linux/drivers/usb/gadget/gp329xxb_udc.c
 *
 * GENERALPLUS 329xx series on-chip high speed USB device controllers
 *
 * Copyright (C) 2013 GENERALPLUS
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/smp_lock.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/list.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/dma-mapping.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/usb.h>
#include <linux/usb/gadget.h>
#include <asm/byteorder.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/system.h>
#include <asm/unaligned.h>
#include <mach/irqs.h>
#include <mach/hardware.h>
#include <mach/clock_mgr/gp_clock.h>
#include <mach/hal/hal_clock.h>
#include <mach/common.h>
#include <mach/gp_gpio.h>
#include <mach/gp_board.h>
#include <mach/gp_usb.h>
#include <mach/hal/hal_usb.h>
#include <mach/hal/regmap/reg_scu.h>
#include <mach/regs-usbdev.h>
#include <mach/clk/gp_clk_core.h>
#include "gp329xxb_udc.h"

#define DRIVER_DESC		"USB Mass Storage Device"
#define DRIVER_VERSION	"8 Nov 2013"
#define DRIVER_AUTHOR	"eugenehsu<eugenehsu@generalplus.com>"
#define IS_USBHOST_PRESENT() (udc_read(UDC_LLCS_OFST) & LCS_VBUS_HIGH)

#define GP329XXB_UDC_NAME	"gp329xxb_udc"
#define SPMP_UDC_NAME		"spmp-udc"
#define GP329XXB_GADGET_NAME		"spmp_udc"

static const char gadget_name[] = GP329XXB_GADGET_NAME;
static const char ep0name [] = "ep0";

static void gp329xxb_udc_vbus_gpio_irq(void *_dev);
static irqreturn_t gp329xxb_udc_irq(int dummy, void* _dev);

/* usb_gadget_ops handler functions */
static int gp329xxb_udc_get_frame(struct usb_gadget *_gadget);
static int gp329xxb_udc_wakeup(struct usb_gadget *_gadget);
static int gp329xxb_udc_set_selfpowered(struct usb_gadget *gadget, int value);
static int gp329xxb_udc_vbus_session(struct usb_gadget *gadget, int is_active);
static int gp329xxb_udc_pullup(struct usb_gadget *gadget, int is_on);
static int gp329xxb_vbus_draw(struct usb_gadget *_gadget, unsigned ma);

/* usb_ep_ops handler functions */
static int gp329xxb_udc_ep_enable(struct usb_ep *_ep, const struct usb_endpoint_descriptor *desc);
static int gp329xxb_udc_ep_disable(struct usb_ep *_ep);
static struct usb_request* gp329xxb_udc_alloc_request(struct usb_ep *_ep, gfp_t mem_flags);
static void gp329xxb_udc_free_request(struct usb_ep *_ep, struct usb_request *_req);
static int gp329xxb_udc_queue(struct usb_ep *_ep, struct usb_request *_req, gfp_t gfp_flags);
static int gp329xxb_udc_dequeue(struct usb_ep *_ep, struct usb_request *_req);
static int gp329xxb_udc_set_halt(struct usb_ep *_ep, int value);

/* gp329xxb UDC local utility functions */
static void _ep0_get_setup_cmd(struct usb_ctrlrequest* req);

/*********** Variables declaration  ********************/
static struct gp329xxb_udc	*the_controller;
static void __iomem		*base_addr;
static int vbus_config = USB_SLAVE_VBUS_POWERON1;
static int gpioHandle = 0;
static u8 set_config_buf[8] = {0,9};		/* for getting a set config command usage */
static u64	rsrc_start;		/* May use definition */
static u64	rsrc_len;		/* May use definition */
static u32 udc_type = UDC_MSDC_TYPE;

#define SCUA_USBPHY_CFG SCUA_USBPHY_CFG1
#define BULK_OUT_DMA_WAIT_COUNT	100000		/* For EP2 bulk out wait counter */

static const struct usb_gadget_ops gp329xxb_ops =
{
	.get_frame			= gp329xxb_udc_get_frame,
	.wakeup				= gp329xxb_udc_wakeup,
	.set_selfpowered	= gp329xxb_udc_set_selfpowered,
	.pullup				= gp329xxb_udc_pullup,
	.vbus_session		= gp329xxb_udc_vbus_session,
	.vbus_draw			= gp329xxb_vbus_draw,
};

static const struct usb_ep_ops gp329xxb_ep_ops =
{
	.enable				= gp329xxb_udc_ep_enable,
	.disable			= gp329xxb_udc_ep_disable,
	.alloc_request		= gp329xxb_udc_alloc_request,
	.free_request		= gp329xxb_udc_free_request,
	.queue				= gp329xxb_udc_queue,
	.dequeue			= gp329xxb_udc_dequeue,
	.set_halt			= gp329xxb_udc_set_halt,
};

/* gp329xxb UDC structure */
static struct gp329xxb_udc memory =
{
	.gadget =
	{
		.ops		= &gp329xxb_ops,
		.ep0		= &memory.ep[0].ep,
		.name		= gadget_name,
		.dev =
		{
			.init_name	= "gadget",
		},
	},

	/* Endpoint 0 */
	.ep[0] =
	{
		.num				= ep_num_0,
		.ep =
		{
			.name			= ep0name,
			.ops			= &gp329xxb_ep_ops,
			.maxpacket		= EP0_FIFO_SIZE,
		},
		.udc				= &memory,
		.bEndpointAddress 	= 0,
		.bmAttributes		= USB_ENDPOINT_XFER_CONTROL,
	},

	/* Endpoint 1 */
	.ep[1] =
	{
		.num			= ep_num_1,
		.ep =
		{
			.name		= "ep1in-bulk",
			.ops		= &gp329xxb_ep_ops,
			.maxpacket	= EP12_FIFO_SIZE512,
		},
		.udc			= &memory,
		.fifo_size		= EP12_FIFO_SIZE,
		.bEndpointAddress = USB_DIR_IN | 1,
		.bmAttributes	= USB_ENDPOINT_XFER_BULK,
	},
	
	/* Endpoint 2 */
	.ep[2] =
	{
		.num			= ep_num_2,
		.ep =
		{
			.name		= "ep2out-bulk",
			.ops		= &gp329xxb_ep_ops,
			.maxpacket	= EP12_FIFO_SIZE512,
		},
		.udc			= &memory,
		.fifo_size		= EP12_FIFO_SIZE,
		.bEndpointAddress = 2,
		.bmAttributes	= USB_ENDPOINT_XFER_BULK,
	},
	
	/* Endpoint 5 for video iso in */
	.ep[3] =
	{
		.num			= ep_num_5,
		.ep =
		{
			.name		= "ep5in-iso",
			.ops		= &gp329xxb_ep_ops,
			.maxpacket	= EP5_FIFO_SIZE,
		},
		.udc			= &memory,
		.fifo_size		= EP5_FIFO_SIZE,
		.bEndpointAddress = 5,
		.bmAttributes	= USB_ENDPOINT_XFER_ISOC,
	},
	
	/* Endpoint 7 for video iso in */
	.ep[4] =
	{
		.num			= ep_num_7,
		.ep =
		{
			.name		= "ep7in-iso",
			.ops		= &gp329xxb_ep_ops,
			.maxpacket	= EP7_FIFO_SIZE,
		},
		.udc			= &memory,
		.fifo_size		= EP7_FIFO_SIZE,
		.bEndpointAddress = 7,
		.bmAttributes	= USB_ENDPOINT_XFER_ISOC,
	},
};

static inline u32 udc_read(u32 reg)
{
	u32 val = readl(base_addr + reg);
	rmb();
	return val;
}

static inline void udc_write(u32 value, u32 reg)
{
	writel(value, base_addr + reg);
	wmb();
}

static inline u8 udc_read_byte(u32 reg)
{
	u8 val = readb(base_addr + reg);
	rmb();
	return val;
}

static inline void udc_write_byte(u8 value, u32 reg)
{
	writeb(value, base_addr + reg);
	wmb();
}

/* To gp329xxb ep, udc and request */

static inline struct gp329xxb_ep *to_gp329xxb_ep(struct usb_ep *ep)
{
	return container_of(ep, struct gp329xxb_ep, ep);
}

static inline struct gp329xxb_udc *to_gp329xxb_udc(struct usb_gadget *gadget)
{
	return container_of(gadget, struct gp329xxb_udc, gadget);
}

static inline struct gp329xxb_request *to_gp329xxb_req(struct usb_request *req)
{
	return container_of(req, struct gp329xxb_request, req);
}

/*
 *	gp329xxb_udc_ep_done
 *	When a usb request was done, call this and enter the completed call back function to gadget driver.
 */
static void gp329xxb_udc_ep_done(struct gp329xxb_ep *ep, struct gp329xxb_request *req, int status)
{
	list_del_init(&req->queue);

	if (likely (req->req.status == -EINPROGRESS))
		req->req.status = status;
	else
		status = req->req.status;

	req->req.complete(&ep->ep, &req->req);
}

/**
 * @brief   USB Slave VBUS detect
 * @param   void
 * @return  1 connect, 0 deisconnect
 */
unsigned int gp329xxb_vbus_detect(void)
{
	unsigned int value = 0;

	if (vbus_config == USB_SLAVE_VBUS_NONE)
	{
		/*always connect*/
		return 1;
	} 
	else
	{
		if(gpioHandle)
		{
			gp_gpio_get_value(gpioHandle, &value);
		}
		else
		{
			value = udc_read(UDC_LLCS_OFST) & LCS_VBUS_HIGH;
		}
	}
	return value;
}
EXPORT_SYMBOL(gp329xxb_vbus_detect);

static void usb_gpio_cbk(unsigned long arg)
{
	struct gp329xxb_udc *udc = (struct gp329xxb_udc*)arg;
	unsigned int value;
	static unsigned int old_value = 0;
	
	gp_gpio_get_value(gpioHandle, &value);
	
	if(value != old_value)
	{
		if(udc->usb_gpio_cnt++ == GPIO_DET_DEBOUNCE)
		{
			if(value == 1)
			{
				/* Connect to charger */
				gp_usb_set_device_connectto(USB_DEVICE_CONNECT_TO_CHARGER);
				gpHalUsbSlaveSwConnect(1);
				/*Change to detect disconnect*/
				printk("USB Device Connect -GPIO\n");
			}
			else
			{
				gp_usb_set_device_connectto(USB_DEVICE_CONNECT_TO_NONE);
				gpHalUsbSlaveSwConnect(0);
				udc->driver->disconnect(&udc->gadget);
				printk("USB Device DisConnect -GPIO\n");
			}
			udc->usb_gpio_cnt = 0;
			old_value = value;
		}
	}
	else
	{
		udc->usb_gpio_cnt = 0;
	}	
	/* Update expire time */
	udc->usb_gpio_timer.expires = jiffies + HZ/USB_TIMER_DIV;	/* 10ms check once*/
	udc->usb_gpio_timer.data = (unsigned long)udc;
	
	add_timer(&udc->usb_gpio_timer);
}

/**
 * @brief   USB Slave VBUS GPIO interrupt config
 * @param   en [IN]: enable
 * @param   configIndex [IN]: gpio index
 * @return  void
 */
void gp329xxb_vbus_gpio_config(struct gp329xxb_udc *udc, unsigned int en, unsigned int configIndex)
{
	int ret;
	int pull_level;
	struct gp_board_usb_s *pConfig = gp_board_get_config("usb", gp_board_usb_t);
	unsigned int value = 0;
	
	if(en)
	{
		/* Use GPIO to detect VBUS */
		gpioHandle = gp_gpio_request(configIndex, (char *)gadget_name);
		pull_level = pConfig->get_slave_gpio_power_level();
		ret = gp_gpio_set_input(gpioHandle, pull_level);
		ret = gp_gpio_get_value(gpioHandle, &value);
		
		udc->usb_gpio_cnt = 0;
		init_timer(&udc->usb_gpio_timer);
		udc->usb_gpio_timer.expires = jiffies + HZ/USB_TIMER_DIV;	/* 10ms check once*/
		udc->usb_gpio_timer.function  = usb_gpio_cbk;
		udc->usb_gpio_timer.data = (unsigned long)udc;
		add_timer(&udc->usb_gpio_timer);
	}
	else
	{
		if(gpioHandle != 0)
		{
			del_timer(&udc->usb_gpio_timer);
			ret = gp_gpio_release(gpioHandle);
			gpioHandle = 0;
		}
	}
}

void gp329xxb_udc_irq_config_en(int en)
{
	struct gp329xxb_udc *udc = &memory;
	struct gp_board_usb_s *pConfig = gp_board_get_config("usb", gp_board_usb_t);
	int retval  = 0;
	vbus_config = pConfig->get_slave_vbus_config();

	printk("[%s][%d], [%d][%d]\n", __FUNCTION__, __LINE__, vbus_config, en);
	
	if(en)
	{
		/* irq setup after old hardware state is cleaned up *///
		retval = request_irq(IRQ_USB_DEV, gp329xxb_udc_irq, IRQF_DISABLED, gadget_name, udc);
		if(retval != 0)
		{
			printk("Error! Request USB irq Fail[%d]\n", retval);
		}
		if(vbus_config == USB_SLAVE_VBUS_POWERON1)
		{
			udc_write((CIE_FDISCONN_IE |CIE_FCONN_IE | CIE_VBUS_IE) , UDC_IE_OFST);
		}
		else if(vbus_config == USB_SLAVE_VBUS_NONE)
		{
		}
		else
		{
			/*GPIO Interrupt. Note:Config is a ramdom value. It's used for GPIO INDEX.*/
			gp329xxb_vbus_gpio_config(udc, 1, vbus_config);
		}
	}
	else
	{
		if(vbus_config != USB_SLAVE_VBUS_POWERON1 && vbus_config != USB_SLAVE_VBUS_NONE)
		{
			gp329xxb_vbus_gpio_config(udc, 0, vbus_config);
		}
		else if(vbus_config == USB_SLAVE_VBUS_POWERON1)
		{
			
		}
		free_irq(IRQ_USB_DEV, udc);
	}
}
EXPORT_SYMBOL(gp329xxb_udc_irq_config_en);

/*
 * gp329xxb_udc_enable, init layer 1 registers
 */
static void gp329xxb_udc_enable(struct gp329xxb_udc *udc)
{
	struct gp_board_usb_s *pConfig = gp_board_get_config("usb", gp_board_usb_t);
	vbus_config = pConfig->get_slave_vbus_config();

	/* First init USB PHY clock */
	gpHalScuUsbPhyClkEnable(1);
	gpHalUsbPhyPowerControlSet(1, 0x0);
	/* init USB device controller clock */
	gp_enable_clock( (int*)"USB_DEVICE", 1);
	//printk("Enable USB1 CLK\n");

	if(vbus_config == USB_SLAVE_VBUS_POWERON1)
	{
		udc_write(CIE_DMA_IE, UDC_IE_OFST);
	}
	else if(vbus_config == USB_SLAVE_VBUS_NONE)
	{
		/*VBUS Interrupt isn't enable.*/
		udc_write((CIE_DMA_IE), UDC_IE_OFST);
	}
	else
	{
		/*GPIO Interrupt.*/
		udc_write((CIE_DMA_IE) , UDC_IE_OFST);
	}
	
	/* Clear 0x88 flag */
	udc_write(0xFFFFFFFF, UDC_IF_OFST);
	
	/* Clear 0x400 flag */
	udc_write(0xFFFFFFFF, UDC_LLCIF_OFST);
	
	/* Set 0x404 enable for EP0 setup, reset, suspend, resume, set config, host clear stall */
	udc_write(MASK_USBD_UDLC_IE_EP0S | MASK_USBD_UDLC_IE_RESET | MASK_USBD_UDLC_IE_SUSPEND
				 | MASK_USBD_UDLC_IE_RESUME | MASK_USBD_UDLC_IE_SCONF | MASK_USBD_UDLC_IE_HCS, UDC_LLCIE_OFST);
	

	/* disable EP12 auto-switch function */
	udc_write(udc_read(UDC_EP12PPC_OFST) & ~MASK_USBD_EP12_FIFO_C_ASE, UDC_EP12PPC_OFST);
	
	
	/* Check UDC type: MSDC or UVC */	
	if(udc_type == UDC_UVC_TYPE)
	{
		/* Let EP4 = Interface 0 , EP5 = Interface 1 */
		udc_write(0x1e, UDC_LCS3_OFST);
	
		/* Enable EP5 */
		udc_write(MASK_USBD_EP5_EN, UDC_EP5EN_OFST);
	
		/* enable EP5 header packet function */
		//udc_write(udc_read(UDC_EP5CTL_OFST) | MASK_USBD_EP5_CTL_EN, UDC_EP5CTL_OFST);
		//udc_write(udc_read(UDC_EP5HDCTRL_OFST) | (MASK_USBD_EP5_HDCTL_FRAMEPT | MASK_USBD_EP5_HDCTL_FRAMESTILL), UDC_EP5HDCTRL_OFST)
		udc_write(12, UDC_EP5HDLEN_OFST);
		udc_write(0x0b, UDC_EP5HDCTRL_OFST);
		udc_write(4, UDC_DVIDEO_REFCLOCK_OFST);
		
		printk("UVC init udc now\n");
	}
	else
	{
		/* EP12 = interface 0, default value */
		udc_write(0x1F, UDC_LCS3_OFST);
		/* Disable EP5 */
		udc_write(~MASK_USBD_EP5_EN, UDC_EP5EN_OFST);
	}	

	
	udc->gadget.speed = USB_SPEED_UNKNOWN;

	/*Enable VBUS Interrupt*/
	gp329xxb_udc_irq_config_en(1);
	
	/*Enable Software Connection*/
	if(gp329xxb_vbus_detect())
	{
		printk("VBUS dected, UDC SW connect\n");
		gpHalUsbSlaveSwConnect(1);
	}
	
	printk("%s done\n", __func__);
}

/*
 * gp329xxb_udc_disable
 */
static void gp329xxb_udc_disable(struct gp329xxb_udc *udc)
{
    unsigned int tmp2;
	/* Disable all interrupts */
	
	tmp2 = SCUA_USBPHY_CFG | USBPHY_XTAL_ENABLE;
	tmp2 &= (~(USBPHY1_POWER_CTRL));
	SCUA_USBPHY_CFG = tmp2;

	while((SCUA_USBPHY_CFG) & (USBPHY1_POWER_CTRL))
	{
		//printk(".");
		if(!gp329xxb_vbus_detect())
			break;
	}

	/*Disable Software Connection*/
	printk("UDC SW Disconnect\n");
	gpHalUsbSlaveSwConnect(0);
	/*Disable Vbus Interrupt*/
	gp329xxb_udc_irq_config_en(0);

	//enable PWREN_CPU1 in SCUB
	SCUB_PWRC_CFG = SCUB_PWRC_CFG | 0x01;
	
	udc_write(udc_read(UDC_LLCSET0_OFST) | LCSET0_SOFT_DISC | LCSET0_PWR_SUSP_N, UDC_LLCSET0_OFST);  //SET_SOFT_DISCON(), SET_PWR_SUSPEND();
	udc_write(0x0, UDC_LLCIE_OFST);  //disable IRQ 
	udc_write(0x0, UDC_IE_OFST); 	 //disable DMA and Vbus irq
	udc_write(0x0, UDC_LLCSET2_OFST);
	udc_write(LCSET0_DISC_SUSP_EN | LCSET0_CPU_WKUP_EN | LCSET0_PWR_SUSP_N | LCSET0_SOFT_DISC, UDC_LLCSET0_OFST);
	
	udc_write(LCSET0_SELF_POWER, UDC_LLCSET1_OFST);
	
	udc_write(EP0CS_CLR_EP0_OVLD, UDC_EP0CS_OFST);	// Free EP0 OUT buffer
	
	udc_write(EP12C_RESET_PIPO, UDC_EP12C_OFST);	//Reset EP12 PING/PONG fifo
	
	udc_write(udc_read(UDC_LLCSTL_OFST) | (LCSTL_CLREP2STL | LCSTL_CLREP1STL | LCSTL_CLREP0STL) , UDC_LLCSTL_OFST);
	
	udc_write(0, UDC_EP0DC_OFST);
	udc_write(udc_read(UDC_EP12C_OFST) | EP12C_RESET_PIPO, UDC_EP12C_OFST);

	//====clear flags before enable interrupt
	udc_write(0xFFFFFFFF, UDC_LLCIF_OFST);
	udc_write(0xFFFFFFFF, UDC_IF_OFST);//UDC_CIS_OFST);
	
	/* Set speed to unknown */
	udc->gadget.speed = USB_SPEED_UNKNOWN;
		
	/* Turn off USB device controller clock */
	gp_enable_clock( (int*)"USB_DEVICE", 0);
	printk("gp329xxb_udc_disable\n");
}

/*
 * gp329xxb_udc_init_data
 */
static void gp329xxb_udc_init_data(struct gp329xxb_udc *udc)
{
	u32 i;

	/* device/ep0 records init */
	INIT_LIST_HEAD(&udc->gadget.ep_list);
	INIT_LIST_HEAD(&udc->gadget.ep0->ep_list);

	for (i = 0; i < GP329xxB_MAXENDPOINTS; i++)
	{
		struct gp329xxb_ep *ep = &udc->ep[i];

		if (i != 0)
			list_add_tail(&ep->ep.ep_list, &udc->gadget.ep_list);

		ep->udc = udc;
		ep->desc = NULL;
		INIT_LIST_HEAD(&ep->queue);
		
		/* reset the gp329xxb_request pointer array */
		udc->ep_req[i] = NULL;
	}
}

/* from ecos code */
static void clearHwState_UDC(int a_iMode)
{
	int tmp;
	/*INFO: we don't disable udc interrupt when we are clear udc hw state,
	* 1.since when we are clearing, we are in ISR , will not the same interrupt reentry problem.
	* 2.after we finish clearing , we will go into udc ISR again, if there are interrupts occur while we are clearing ,we want to catch them
	*  immediately
	*/
	//===== check udc DMA state, and flush it =======
	if(udc_read(UDC_DMA_CS_OFST) & DMACS_DMA_EN)
	{
		udc_write(udc_read(UDC_DMA_CS_OFST) | DMACS_DMA_FLUSH,UDC_DMA_CS_OFST);
		while(!(udc_read(UDC_DMA_CS_OFST) & DMACS_DMA_FLUSHEND))
		{
			tmp++;
			if(tmp> 300000)
			{
				printk("##");
				tmp=0;
			}
		}
	}
	
	/*Disable Interrupt */
	//udc_write(0x0, UDC_LLCIE_OFST); 
	/*Clear Interrupt Flag*/
	udc_write(0xffffff, UDC_LLCIF_OFST); 
	/*Clear Interrupt Statue*/
	udc_write(0xffffff, UDC_LLCIS_OFST); 

	//EP0
	udc_write(EP0CS_CLR_EP0_OVLD, UDC_EP0CS_OFST); //clear ep0 out vld=1, clear set epo in vld=0, set ctl dir to OUT direction=0
	udc_write(0x0,UDC_EP0DC_OFST);
	//EP1
    udc_write(EP1SCS_CLR_IVLD | EP1SCS_RESET_FIFO ,UDC_EP1SCS_OFST);
	//EP2
	udc_write(EP12C_CLR_EP2_OVLD | EP12C_RESET_PIPO ,UDC_EP12C_OFST);
	//EP1/2 Filo CNT
	udc_write(0 ,UDC_EP12FCL_OFST);
	udc_write(udc_read(UDC_EP12FCH_OFST) | EP12FCH_RESET_CNTR, UDC_EP12FCH_OFST);
	udc_write(0 ,UDC_EP12FCH_OFST);
	
	/*Clear Stall Status*/
	if(a_iMode==0)
		udc_write((LCSTL_CLREP3STL | LCSTL_CLREP2STL | LCSTL_CLREP1STL | LCSTL_CLREP0STL), UDC_LLCSTL_OFST);

	// 2008/6/26, to prevent when PIPO IS IN , plug off intr occur; or comment this since each new CBW will set it again
	udc_write(0, UDC_EP12C_OFST);	
}

static int vbusIntr2_UDC(struct gp329xxb_udc *dev)
{
	unsigned int 	tmp;
	unsigned int 	llcset0;
	tmp = udc_read(UDC_LLCS_OFST);
	/*It always connects with host by none VBUS.*/
	if( (vbus_config == USB_SLAVE_VBUS_NONE) || (tmp & LCS_VBUS_HIGH))
	{
		llcset0 = udc_read(UDC_LLCSET0_OFST);
       if(llcset0 & LCSET0_SOFT_DISC)
		{
		   gpHalUsbSlaveSwConnect(1);
		   printk("USB Connect 3B0[%x] \n",udc_read(UDC_LLCSET0_OFST));
		}
	}
	else /* host absent */
	{
		gpHalUsbSlaveSwConnect(0);
		dev->driver->disconnect(&dev->gadget);
		printk("USB Disconnect\n");
		clearHwState_UDC(0);
	}

	/*Clear VBUS int source*/
	udc_write(udc_read(UDC_CIS_OFST) | CIS_VBUS_IF, UDC_CIS_OFST);
    return 0;
}

static void gp329xxb_udc_vbus_gpio_irq(void *_dev)
{
	struct gp329xxb_udc *udc = _dev;
	unsigned int value = 0;
	unsigned int debounceTime = 0x1fffff;

	gp_gpio_get_value(gpioHandle, &value);
	if(value == 1)
	{
		gpHalUsbSlaveSwConnect(1);
		/*Change to detect disconnect*/
		gp_gpio_irq_property(gpioHandle, (GPIO_IRQ_LEVEL_LOW << 8) | GPIO_IRQ_LEVEL_TRIGGER, &debounceTime);
		printk("USB cable connected -GPIO\n");
	}
	else
	{
		gpHalUsbSlaveSwConnect(0);
		udc->driver->disconnect(&udc->gadget);
		clearHwState_UDC(0);
		gp_gpio_irq_property(gpioHandle, (GPIO_IRQ_LEVEL_HIGH << 8) | GPIO_IRQ_LEVEL_TRIGGER, &debounceTime);
		printk("USB cable disconnected -GPIO\n");
	}
}

/**********************************************************
*	gp329xxb UDC local utility and access register functions
**********************************************************/
static inline u32 _udc_get_line_speed(void)
{
	u32 reg = udc_read(UDC_LLCS_OFST);
	
	if(reg & MASK_USBD_UDLC_CS_CURR_SPEED)
		return USB_SPEED_FULL;
	else
		return USB_SPEED_HIGH;	
}	

static void _udc_clear_ep_usb_request(struct gp329xxb_udc* udc, int status)
{
	u32 i, cnt = 0;
	
	for(i=0 ; i< GP329xxB_MAXENDPOINTS; i++)
	{
		struct gp329xxb_request* req = udc->ep_req[i];
		struct gp329xxb_ep* ep = &udc->ep[i];
		
		if(udc->ep_req[i] != NULL)
		{
			req->req.status = status;
			udc->ep_req[i] = NULL;
			gp329xxb_udc_ep_done(ep, req, status);
			cnt++;
		}	
	}		
	//printk("udc clear %d usb %s\n", cnt, (cnt > 1?"requests":"request"));
}	

static inline void _ep0_handle_setup_command(struct gp329xxb_udc* udc, int src)
{
	int ret;

	/* reset the ep0datastatus to EP0_DATA_NONE */				
	udc->ep0datastatus = EP0_DATA_NONE;
	
	if(src == SRC_SET_CONFIG)
	{
		/* From set config command */
		set_config_buf[2] = gpHalUsbHostConfiged();
		memcpy(&udc->ctlreq, set_config_buf, 8);
	}	
	else if(src == SRC_STD_SETUP)
	{
		/* standard setup interrupt */
		memset(&udc->ctlreq, 0, sizeof(udc->ctlreq));
		_ep0_get_setup_cmd(&udc->ctlreq);
		/* Set EP0 data phase status */
		if(udc->ctlreq.bRequestType & USB_DIR_IN)
		{
			udc->ep0datastatus = EP0_DATA_IN;
		}
		else
		{
			udc->ep0datastatus = EP0_DATA_OUT;
		}			
		udc->ep0sendnull = 0;	/* reset ep0sendnull flag */
	}
	
	/* update current USB line state, full/high speed */
	udc->gadget.speed = _udc_get_line_speed();
	
	/* call gadget setup callback function */
	ret = udc->driver->setup(&udc->gadget, &udc->ctlreq);
		
	if(ret < 0)
	{
		/* error handler from gadget driver */
		printk("ret = 0x%x, error occurred after udc->driver->setup\n", ret);
	}	
}	

static inline void _ep0_get_setup_cmd(struct usb_ctrlrequest* req)
{
	u32* ptr;
	
	ptr = (u32*)req;
	*ptr = (u32)udc_read(UDC_EP0SDP_OFST);
	ptr++;
	*ptr = (u32)udc_read(UDC_EP0SDP_OFST);
	
#if 0	
	printk("Setup cmd got, reqtype 0x%x, req 0x%x, wValue 0x%x, wIndex 0x%x, wlen 0x%x\n", 
						req->bRequestType, req->bRequest, req->wValue, req->wIndex, req->wLength);
#endif						
}	

static inline u32 _ep0_send_in(u32* buf, int len)
{
	int fourbytecnt, remaintbyte, i;
    u32*  dataPtr = buf;
    u32	mask;
    u32 datalen;
	
	if(len == 0)
	{
		/* send null in packet */
		/* set ep0 direction IN */
		udc_write(udc_read(UDC_EP0CS_OFST) | MASK_USBD_EP0_CS_EP0_DIR, UDC_EP0CS_OFST);		
		/* reset ep0 data count */
		udc_write(0, UDC_EP0DC_OFST);
		/* clear EP0 IN interrupt flag */
		udc_write(MASK_USBD_UDLC_IF_EP0I, UDC_LLCIF_OFST);
		/* enable EP0 IN interrupt */
		udc_write(udc_read(UDC_LLCIE_OFST) | MASK_USBD_UDLC_IE_EP0I, UDC_LLCIE_OFST);
		/* set EP0 IN valid */
		udc_write(udc_read(UDC_EP0CS_OFST) | MASK_USBD_EP0_CS_SET_EP0_IN_VLD, UDC_EP0CS_OFST);
		return 0;
	}	
	else
	{
		/* send none zero length IN data */
		if(len > EP0_FIFO_SIZE)
    	{
        	datalen = EP0_FIFO_SIZE;
    	}
    	else
    	{
       		datalen = len;
    	}
    	
    	udc_write(udc_read(UDC_EP0CS_OFST) | MASK_USBD_EP0_CS_EP0_DIR, UDC_EP0CS_OFST);		/* set ep0 direction IN */
		udc_write(0, UDC_EP0DC_OFST);	/* reset ep0 data count */
		
		/* According to current datalen to send respective data bytes */
		fourbytecnt = datalen >> 2;
		remaintbyte = datalen & 0x03;
		
		//printk("_ep0_send_in datalen = %d, 4bytecnt = %d, remaintbyte= %d\r\n", datalen, fourbytecnt, remaintbyte);
		for(i = 0; i < fourbytecnt; i++)
		{
            udc_write_byte(VALID_BIT_4BYTE, UDC_EP0DPVALID_OFST);
            udc_write(*dataPtr, UDC_EP0DP_OFST);
            dataPtr++;
		}
		
		/* check remainder of 4 */
		if(remaintbyte)
		{
			switch(remaintbyte)
			{
				case 3:
					udc_write_byte(VALID_BIT_3BYTE, UDC_EP0DPVALID_OFST);
					mask = 0x00FFFFFF;
					break;
				case 2:
					udc_write_byte(VALID_BIT_2BYTE, UDC_EP0DPVALID_OFST);
					mask = 0x0000FFFF;
					break;
				case 1:
					udc_write_byte(VALID_BIT_1BYTE, UDC_EP0DPVALID_OFST);
					mask = 0x000000FF;
					break;			
			}
			udc_write(*dataPtr & mask, UDC_EP0DP_OFST);
		}
		
		/* set ep0 data count */
		udc_write(datalen, UDC_EP0DC_OFST);
		/* clear EP0 IN interrupt flag */
		udc_write(MASK_USBD_UDLC_IF_EP0I, UDC_LLCIF_OFST);
		/* enable EP0 IN */
		udc_write(udc_read(UDC_LLCIE_OFST) | MASK_USBD_UDLC_IE_EP0I, UDC_LLCIE_OFST);
		/* set EP0 IN valid */
		udc_write(udc_read(UDC_EP0CS_OFST) | MASK_USBD_EP0_CS_SET_EP0_IN_VLD, UDC_EP0CS_OFST);
		//printk("%s, data length 0x%x\n", __func__, datalen);
		return datalen;	
	}	
}	

static inline u32 _ep0_check_send_null_in(struct gp329xxb_udc* udc, struct usb_ctrlrequest* ctlreq, u32* datalen)
{
	if(*datalen > ctlreq->wLength)
	{
		*datalen = ctlreq->wLength;
	}		
	
	if(*datalen && ((*datalen & EP0_FIFO_SIZE_MASK) == 0)
		 && (ctlreq->wLength > *datalen))
	{
		return 1;
	}		
	else
	{
		return 0;
	}	
}	

static inline void _ep0_process_status_stage(struct gp329xxb_udc* udc)
{
	if(udc->ep0datastatus == EP0_DATA_IN)
	{
		/* enable ep0 out to get a null out */
		udc_write(udc_read(UDC_EP0CS_OFST) & ~MASK_USBD_EP0_CS_EP0_DIR, UDC_EP0CS_OFST);
	}	
	else if(udc->ep0datastatus ==EP0_DATA_OUT)
	{
		/* send null in */
		_ep0_send_in(NULL, 0);
	}	
}

static inline void _ep0_enable_out(void)
{
	/* Enable EP0 OUT to get out data */
	udc_write(udc_read(UDC_EP0CS_OFST) | MASK_USBD_EP0_CS_CLE_EP0_OUT_VLD, UDC_EP0CS_OFST);		/* Clear EP0 data out valid */
	udc_write(udc_read(UDC_EP0CS_OFST) & ~MASK_USBD_EP0_CS_EP0_DIR, UDC_EP0CS_OFST);
	udc_write(MASK_USBD_UDLC_IF_EP0O, UDC_LLCIF_OFST);								/* reset EP0 OUT interrupt flag */
	udc_write(udc_read(UDC_LLCIE_OFST) | MASK_USBD_UDLC_IE_EP0O, UDC_LLCIE_OFST);	/* Enable EP0 OUT interrupt */
}

static inline u32 _ep0_get_out_data(u32* buf, int len)
{
	int fourbytecnt, remaintbyte, i;
	u32 cnt, mask, remaindata;
	u32* dataptr = buf;
	
	/* Check data cnt in OUT FIFO */
	cnt = udc_read(UDC_EP0DC_OFST);
	
	if(cnt > len)
		cnt	= len;
		
	/* According to current datalen to send respective data bytes */
	fourbytecnt = cnt >> 2;
	remaintbyte = cnt & 0x03;	
	
	for(i = 0; i < fourbytecnt; i++)
	{
    	udc_write_byte(VALID_BIT_4BYTE, UDC_EP0DPVALID_OFST);
        *dataptr = udc_read(UDC_EP0DP_OFST);
        dataptr++;
	}
	
	if(remaintbyte)
	{
		switch(remaintbyte)
		{
			case 3:
				udc_write_byte(VALID_BIT_3BYTE, UDC_EP0DPVALID_OFST);
				mask = 0x00FFFFFF;
				break;
			case 2:
				udc_write_byte(VALID_BIT_2BYTE, UDC_EP0DPVALID_OFST);
				mask = 0x0000FFFF;
				break;
			case 1:
				udc_write_byte(VALID_BIT_1BYTE, UDC_EP0DPVALID_OFST);
				mask = 0x000000FF;
				break;			
		}
		remaindata = udc_read(UDC_EP0DP_OFST);
		*dataptr = remaindata & mask;
	}		

	return cnt;
}	

static inline u32* _epx_get_data_buffer(struct gp329xxb_ep* ep, struct gp329xxb_request* req)
{
	/* Depends on the data length, determine the buffer address for DMA/none DMA */
	//struct gp329xxb_udc* udc = ep->udc;
	u32* buf;

	/* Get physical buffer address for DMA */
	if(req->req.length >> 9)
	{
		if(req->req.dma == DMA_ADDR_INVALID)
		{
			req->req.dma = dma_map_single(ep->udc->gadget.dev.parent, req->req.buf, req->req.length,
							(ep->bEndpointAddress & USB_DIR_IN) ? DMA_TO_DEVICE : DMA_FROM_DEVICE);
		}	
		buf = (u32*)(req->req.dma+ req->req.actual);
	}
	else
	{
		buf = req->req.buf;
	}		
	return buf;
}	

static inline u32 _ep1_send_in(u32* buf, int len)
{
	int transferlen;
	u32*  dataPtr = buf;
	u32 fourbytecnt, remaintbyte, i;
	
	/* disable bulk in/out function*/
	udc_write(udc_read(UDC_EP12C_OFST) & ~MASK_USBD_EP12_C_EP12_ENA, UDC_EP12C_OFST);
	
	if(len >> 9)
	{	
		/* Calculate transfer data length in DMA mode, if length is larger then 512 bytes */
		transferlen = (len >> 9) << 9;
		/* Disable dma int, even disable DMA interrupt, but the interrupt will be got in ISR */
		udc_write(udc_read(UDC_IE_OFST) & ~MASK_USBD_UDC_IE_DMA, UDC_IE_OFST);
		/* Disable EP5 DMA function */
		udc_write(udc_read((UDC_EP5DMAEN_OFST) & ~MASK_USBD_EP5_DMA_EN), UDC_EP5DMAEN_OFST);
		/* reset FIFO, set bulk direction*/
		udc_write((udc_read(UDC_EP12C_OFST) | MASK_USBD_EP12_C_RESET_PIPO_FIFO | MASK_USBD_EP12_C_EP12_DIR), UDC_EP12C_OFST);
		/* enable autoswitch function */
		udc_write((udc_read(UDC_EP12PPC_OFST) | MASK_USBD_EP12_FIFO_C_ASE), UDC_EP12PPC_OFST);
		/* assign DMA data pointer */
		udc_write((u32)buf, UDC_DMA_DA_OFST);
		/* assign DMA data length */
		udc_write((udc_read(UDC_DMA_CS_OFST) | transferlen), UDC_DMA_CS_OFST);
		/* set bulk DMA direction = 0, IN */
		udc_write((udc_read(UDC_DMA_CS_OFST) & ~MASK_USBD_EP12_DMA_WRITE), UDC_DMA_CS_OFST);
		/* clear EP1 DMA IN finish and data received by host interrupt flag */
		udc_write(MASK_USBD_UDLC_IF_EP1INN, UDC_LLCIF_OFST);
		/* enable EP1 DMA IN finish and data received by host interrupt */
		udc_write((udc_read(UDC_LLCIE_OFST) | MASK_USBD_UDLC_IE_EP1INN), UDC_LLCIE_OFST);
		/* enable bulk DMA */
		udc_write((udc_read(UDC_DMA_CS_OFST) | MASK_USBD_EP12_DMA_EN), UDC_DMA_CS_OFST);
		/* enable bulk in/out function*/
		udc_write((udc_read(UDC_EP12C_OFST) | MASK_USBD_EP12_C_EP12_ENA), UDC_EP12C_OFST);
		
		return transferlen;
	}
	else
	{
		/* PIO mode */
		if(len > EP12_FIFO_SIZE)
		{
			transferlen = EP12_FIFO_SIZE;
		}
		else
		{
			transferlen = len;	
		}	
		
		fourbytecnt = transferlen >> 2;
		remaintbyte = transferlen & 0x03;
		
		/* disable auto switch FIFO */
		udc_write(udc_read(UDC_EP12PPC_OFST) & ~MASK_USBD_EP12_FIFO_C_ASE, UDC_EP12PPC_OFST);
		/* reset current and next EP12 FIFO, and set EP12 direction = IN, bit0 = 1 */
		udc_write(udc_read(UDC_EP12C_OFST) | MASK_USBD_EP12_C_RESET_PIPO_FIFO | MASK_USBD_EP12_C_EP12_DIR, UDC_EP12C_OFST);
		
		for(i=0; i < fourbytecnt; i++)
        {
			/* 4 btye vaild */
			udc_write_byte(VALID_BIT_4BYTE, UDC_EP12VALID_OFST);
			/* assign 4 bytes into data port of EP12 */
			udc_write(*dataPtr, UDC_EP12FDP_OFST);
			dataPtr++;
        }
        
        /* check remainder of 4 */
		if(remaintbyte != 0)
		{
			switch(remaintbyte)
			{
				case 3:
					udc_write_byte(VALID_BIT_3BYTE, UDC_EP12VALID_OFST);
					break;
				case 2:
					udc_write_byte(VALID_BIT_2BYTE, UDC_EP12VALID_OFST);
					break;
				case 1:
					udc_write_byte(VALID_BIT_1BYTE, UDC_EP12VALID_OFST);
					break;			
			}
			udc_write(*dataPtr, UDC_EP12FDP_OFST);
		}
				
		/* clear EP1 IN transaction interrupt flag */
		udc_write(MASK_USBD_UDLC_IF_EP1I, UDC_LLCIF_OFST);
		/* enable EP1 IN transaction interrupt */
		udc_write(udc_read(UDC_LLCIE_OFST) | MASK_USBD_UDLC_IE_EP1I, UDC_LLCIE_OFST);
		/* set EP12 IN data valid */
		udc_write(udc_read(UDC_EP12C_OFST) | MASK_USBD_EP12_C_SET_EP1_IVLD, UDC_EP12C_OFST);
		/* enable BULK IN */
		udc_write(udc_read(UDC_EP12C_OFST) | MASK_USBD_EP12_C_EP12_ENA, UDC_EP12C_OFST);
		/* manual switch EP12 PIPO */
		udc_write(udc_read(UDC_EP12PPC_OFST) | MASK_USBD_EP12_FIFO_C_SWITCH_BUF, UDC_EP12PPC_OFST);
		
		return transferlen;
	}		
}	

static inline u32 _ep2_get_data_cnt(void)
{
	u32 ep12status = udc_read(UDC_EP12PPC_OFST) & EP12PPC_CURR_BUF_NUM;
	
	if(ep12status)
	{
		/* PING buffer cnt */
		return (u32)((udc_read(UDC_EP12FCH_OFST) << 8) | udc_read(UDC_EP12FCL_OFST));
	}
	else
	{
		/* PONG buffer cnt */
		return (u32)((udc_read(UDC_EP12POFDHB_OFST) << 8) | udc_read(UDC_EP12POFDLB_OFST));	
	}		
}	

static inline void _ep2_switch_fifo_for_valid_buffer(void)
{
	if(udc_read(UDC_EP12PPC_OFST) & MASK_USBD_EP12_FIFO_C_A_EP2_OVLD)
	{
		udc_write(udc_read(UDC_EP12PPC_OFST) | MASK_USBD_EP12_FIFO_C_SWITCH_BUF, UDC_EP12PPC_OFST);
	}	
}	

static inline u32 _ep2_get_pio_data(struct usb_request* req, int len)
{
	u32 fourbytecnt, remaintbyte, i;
	u32* dataptr = (u32*)req->buf;
	u32 mask, remaindata;
	
	u32 cnt = _ep2_get_data_cnt();
	
	if(len != cnt)
	{
		printk("Get PIO bulk out data length error, want %d but %d\n", len, cnt);
	}	
	
	fourbytecnt = cnt >> 2;
	remaintbyte = cnt & 0x3;
	
	for(i=0 ;i<fourbytecnt; i++)
	{
		udc_write_byte(VALID_BIT_4BYTE, UDC_EP12VALID_OFST);
		*dataptr = udc_read(UDC_EP12FDP_OFST);
		dataptr++;	/* add 4 bytes cnt */
	}	
	
	/* check remainder of 4 */
	if(remaintbyte != 0)
	{
		switch(remaintbyte)
		{
			case 3:
				udc_write(VALID_BIT_3BYTE, UDC_EP12VALID_OFST);
				mask = 0x00FFFFFF;
				break;
			case 2:
				udc_write(VALID_BIT_2BYTE, UDC_EP12VALID_OFST);
				mask = 0x0000FFFF;
				break;
			case 1:
				udc_write(VALID_BIT_1BYTE, UDC_EP12VALID_OFST);
				mask = 0x000000FF;
				break;			
		}
		remaindata = udc_read(UDC_EP12FDP_OFST);
		*dataptr = remaindata & mask;
	}
	
	/* reset EP2 OVLD flag */
	udc_write(udc_read(UDC_EP12C_OFST) | MASK_USBD_EP12_C_CLR_EP2_OVLD, UDC_EP12C_OFST);
	//printk("%s, want %d get %d in PIO mode\n", __func__, len , cnt);
	
	return cnt;	
}	

static inline u32 _ep2_porcess_out(u32* buf, int len)
{	
	u32 cnt = len >> 9;		/* check 512 bytes data size */
	u32 length = 1 << 9;	/* transfer multiple of 512 bytes per round for DMA */
	u32 waitbulkoutcnt, waitdmadonecnt;
	u32 status, outcnt;
	
	/* disable bulk in/out */
	udc_write(udc_read(UDC_EP12C_OFST) & ~MASK_USBD_EP12_C_EP12_ENA, UDC_EP12C_OFST);
	
	if(cnt)
	{
		/* bulk out DMA transfer, seperate into 512 bytes packet size to transfer. This is a none interrupt mode */
		length = cnt << 9;
		/* flush bulk out DMA */
		udc_write(udc_read(UDC_DMA_CS_OFST) | MASK_USBD_EP12_DMA_FIFO_FLUSH, UDC_DMA_CS_OFST);
		while(!(udc_read(UDC_DMA_CS_OFST) & MASK_USBD_EP12_DMA_FLUSH)); 
		//printk("_ep2_porcess_out, DMA buf = 0x%x, length = %d[%d][cnt:%d], 0x0[0x%x] 0x330[0x%x] 0x334[0x%x]\n", buf, length, len, cnt
		//	, udc_read(UDC_DMA_CS_OFST), udc_read(UDC_EP12C_OFST) ,udc_read(UDC_EP12PPC_OFST));	
		/* reset EP12 FIFO */
		udc_write(udc_read(UDC_EP12C_OFST) | MASK_USBD_EP12_C_RESET_PIPO_FIFO, UDC_EP12C_OFST);
		/* disable buik FIFO auto switch */
		udc_write(udc_read(UDC_EP12PPC_OFST) & ~MASK_USBD_EP12_FIFO_C_ASE, UDC_EP12PPC_OFST);
		/* set direction = 0, OUT */
		udc_write(udc_read(UDC_EP12C_OFST) & ~MASK_USBD_EP12_C_EP12_DIR, UDC_EP12C_OFST);
		/* assign DMA data pointer */
		udc_write((u32)buf, UDC_DMA_DA_OFST);
		/* assign DMA data length */
		udc_write(udc_read(UDC_DMA_CS_OFST) | length, UDC_DMA_CS_OFST);
		/* set bulk DMA direction = 1, OUT */
		udc_write(udc_read(UDC_DMA_CS_OFST) | MASK_USBD_EP12_DMA_WRITE, UDC_DMA_CS_OFST);
		/* enable bulk out DMA */
		udc_write(udc_read(UDC_DMA_CS_OFST) | MASK_USBD_EP12_DMA_EN, UDC_DMA_CS_OFST);
		/* enable bulk in/out */
		udc_write(udc_read(UDC_EP12C_OFST) | MASK_USBD_EP12_C_EP12_ENA, UDC_EP12C_OFST);
		
		/* start to poll the the other and current EP2 out buffer valid flag */
		waitbulkoutcnt = BULK_OUT_DMA_WAIT_COUNT;
		outcnt = 0;
		while(waitbulkoutcnt-- && (outcnt < cnt))
		{
			/* get 0x334 register status */
			u32 reg = udc_read(UDC_EP12PPC_OFST);
			
			if(!(reg & MASK_USBD_EP12_FIFO_C_EP2_OVLD) &&
				reg & MASK_USBD_EP12_FIFO_C_A_EP2_OVLD)
			{
				/* disable bulk in/out */
				udc_write(udc_read(UDC_EP12C_OFST) & ~MASK_USBD_EP12_C_EP12_ENA, UDC_EP12C_OFST);
				/* A 512 bytes data was received from host, 0x334[1] write 1 to switch buffer for transfer data by DMA */	
				udc_write(udc_read(UDC_EP12PPC_OFST) | MASK_USBD_EP12_FIFO_C_SWITCH_BUF, UDC_EP12PPC_OFST);
				
				waitdmadonecnt = BULK_OUT_DMA_WAIT_COUNT;
				
				while(waitdmadonecnt--)
				{
					/* update 0x334 status */
					reg = udc_read(UDC_EP12PPC_OFST);
					
					if(!(reg & MASK_USBD_EP12_FIFO_C_EP2_OVLD) &&
						!(reg & MASK_USBD_EP12_FIFO_C_A_EP2_OVLD))
					{
						/* DMA done then enable bulk in/out for next tranfer */
						outcnt++;
						if(outcnt < cnt)
						{
							/* enable bulk in/out */
							udc_write(udc_read(UDC_EP12C_OFST) | MASK_USBD_EP12_C_EP12_ENA, UDC_EP12C_OFST);
							//printk("do DMA out, outcnt %d, cnt %d\n", outcnt, cnt);
						}
						else if(outcnt == cnt)
						{
							/* disable bulk in/out*/
						}	
						/* reset the timeout counter for next 512 transfer */
						waitbulkoutcnt = BULK_OUT_DMA_WAIT_COUNT;
						/* break waitdmadonecnt loop to get next 512 bytes out data */
						break;
					}
				}
				/* check if timeout for waitdmadonecnt */
				if(waitdmadonecnt == 0)
				{
					/* error in doing out DMA */
					printk("Do bulk out DMA but timeout!!\n");
				}		
			}		
		}
		/* disable bulk in/out */
		udc_write(udc_read(UDC_EP12C_OFST) & ~MASK_USBD_EP12_C_EP12_ENA, UDC_EP12C_OFST);
		
		if(waitbulkoutcnt == 0 && (outcnt != cnt))
		{
			/* error in getting bulk out  */
			printk("Do get bulk out but got out data failed in outcnt %d, status = %d!!\n", outcnt, status);
			return (outcnt << 9);
		}			
		else
		{
			//printk("_ep2_porcess_out DMA done, out outcnt[%d], cnt[%d]\n", outcnt, cnt);
			return length;
		}	
	}		
	else
	{
		/* none DMA bulk out transfer, enable bulk out first then wait OUT transaction done in interrupt */
		/* reset EP12 FIFO */
		udc_write(udc_read(UDC_EP12C_OFST) | MASK_USBD_EP12_C_RESET_PIPO_FIFO, UDC_EP12C_OFST);
		/* disable buik FIFO auto switch */
		udc_write(udc_read(UDC_EP12PPC_OFST) & ~MASK_USBD_EP12_FIFO_C_ASE, UDC_EP12PPC_OFST);
		/* set direction = 0, OUT */
		udc_write(udc_read(UDC_EP12C_OFST) & ~MASK_USBD_EP12_C_EP12_DIR, UDC_EP12C_OFST);
		/* clear bulk out interrupt flag */
		udc_write(MASK_USBD_UDLC_IF_EP2O, UDC_LLCIF_OFST);
		/* enable bulk out interrupt */
		udc_write(udc_read(UDC_LLCIE_OFST) | MASK_USBD_UDLC_IE_EP2O, UDC_LLCIE_OFST);
		/* enable bulk in/out */
		udc_write(udc_read(UDC_EP12C_OFST) | MASK_USBD_EP12_C_EP12_ENA, UDC_EP12C_OFST);
		
		return 0;
	}	
}	

static inline u32 _ep5_porcess_in(u32* buf, int len)
{
	/* flush to clear 0x148[1], to toggle EOF bit field */	 
	udc_write(udc_read(UDC_EP5CTL_OFST) | MASK_USBD_EP5_FLUSH, UDC_EP5CTL_OFST);
	udc_write(udc_read(UDC_EP5CTL_OFST) & ~MASK_USBD_EP5_FLUSH, UDC_EP5CTL_OFST);
	/* 0x148 = 1  */
	udc_write(0x01, UDC_EP5FRAMCTL_OFST);
	
	/* Change EP12 DMA control for EP5 using */
	udc_write(udc_read(UDC_EP5DMAEN_OFST) | MASK_USBD_EP5_DMA_EN, UDC_EP5DMAEN_OFST);
	udc_write((u32)buf, UDC_DMA_DA_OFST);
	udc_write(udc_read(UDC_DMA_CS_OFST) | len, UDC_DMA_CS_OFST);
	udc_write(udc_read(UDC_DMA_CS_OFST) & ~MASK_USBD_EP12_DMA_WRITE, UDC_DMA_CS_OFST);
	
	udc_write(MASK_USBD_UDLC_IF_EP5IEND, UDC_LLCIF_OFST);
	udc_write(udc_read(UDC_LLCIE_OFST) | MASK_USBD_UDLC_IE_EP5IEND, UDC_LLCIE_OFST);
	
	/* Enable EP5 DMA */
	udc_write(udc_read(UDC_DMA_CS_OFST) | MASK_USBD_EP12_DMA_EN, UDC_DMA_CS_OFST);
	
	return len;
}	

static inline u32 _ep7_porcess_in(u32* buf, int len)
{
	udc_write(udc_read(UDC_EP7CTL_OFST) | (MASK_USBD_EP7_CTL_VLD | MASK_USBD_EP7_CTL_EN), UDC_EP7CTL_OFST);
	udc_write((u32)buf, UDC_DMA_ADS_OFST);
	udc_write(udc_read(UDC_DMA_AC_OFST) | len, UDC_DMA_AC_OFST);
	
	udc_write(MASK_USBD_UDLC_IF_EP7IEND, UDC_LLCIF_OFST);
	udc_write(udc_read(UDC_LLCIE_OFST) | MASK_USBD_UDLC_IE_EP7IEND, UDC_LLCIE_OFST);
	
	/* Enable EP7 DMA */
	udc_write(udc_read(UDC_DMA_AC_OFST) | MASK_USBD_EP7_DMA_EN, UDC_DMA_AC_OFST);
	return len;
}


/* Handler for endpoints of gp329xxb, so far supports EP0/EP1/EP2 */
static int _udc_ep0_handler(struct gp329xxb_ep* ep, struct gp329xxb_request* req)
{
	u32 cnt;
	struct gp329xxb_udc* udc = ep->udc;
	/* Check the direction of current setup command */

	if(udc->ep0datastatus == EP0_DATA_IN)
	{
		/* Check if send null in paket for normal IN data transfer */
		udc->ep0sendnull = _ep0_check_send_null_in(udc, &udc->ctlreq, &(req->req.length));
		
		/* Send EP0 IN */
		cnt = _ep0_send_in((u32*)req->req.buf, req->req.length);
		if(cnt)
		{
			req->req.actual += cnt;
			udc->ep_req[ep_num_0] = req;							/* record the current gp329xxb request pointer for interrupt usage */
		}
		else
		{
			udc->ep_req[ep_num_0] = NULL;
		}	
	}
	else if(udc->ep0datastatus == EP0_DATA_OUT)
	{
		/* Enable EP0 OUT */
		_ep0_enable_out();
		if(req->req.length)
			udc->ep_req[ep_num_0] = req;		/* record the current gp329xxb request pointer for interrupt usage */
		else
			udc->ep_req[ep_num_0] = NULL;	
	}		

	return 0;
}	

static int _udc_ep1_handler(struct gp329xxb_ep* ep, struct gp329xxb_request* req)
{
	struct gp329xxb_udc* udc = ep->udc;
	u32 cnt;
	u32* buf;
	
	/* thread environment */
	if(req->req.length)
	{
		/* Send EP1 IN by request from gadget driver and update USB request actual count */	
		udc->ep_req[ep_num_1] = req;
		buf = _epx_get_data_buffer(ep, req);		
		/* update USB request actual count */
		req->req.actual += req->req.length;
		cnt = _ep1_send_in(buf, req->req.length);	
	}
	else
	{
		/* usb request with 0 length */
		udc->ep_req[ep_num_1] = NULL;
		gp329xxb_udc_ep_done(ep, req, 0);
		//printk("_udc_ep1_handler, req len = 0\n");
	}	
	return 0;
}

static int _udc_ep2_handler(struct gp329xxb_ep* ep, struct gp329xxb_request* req)
{
	struct gp329xxb_udc* udc = ep->udc;
	u32* buf;
	u32 cnt;
	
	/* thread environment */
	if(req->req.length)
	{
		udc->ep_req[ep_num_2] = req;
	
		buf = _epx_get_data_buffer(ep, req);
		/* Enable EP2 IN by request from gadget driver */	
		cnt = _ep2_porcess_out(buf, req->req.length);
		if(cnt >> 9)
		{
			/* set gp329xxb request handler to NULL, and done back to gadget driver if DMA transfer */
			udc->ep_req[ep_num_2] = NULL;
			req->req.actual += cnt;
			gp329xxb_udc_ep_done(ep, req, 0);
		}	
	}
	else
	{
		/* usb request with 0 length */
		udc->ep_req[ep_num_2] = NULL;
	}	
	return 0;
}

static inline void _ep5_flush(void)
{
	udc_write(udc_read(UDC_DMA_CS_OFST) & ~MASK_USBD_EP12_DMA_EN, UDC_DMA_CS_OFST);
	udc_write(udc_read(UDC_EP5CTL_OFST) | MASK_USBD_EP5_FLUSH, UDC_EP5CTL_OFST);
	udc_write(udc_read(UDC_EP5CTL_OFST) & ~MASK_USBD_EP5_FLUSH, UDC_EP5CTL_OFST);
	udc_write(udc_read(UDC_DMA_CS_OFST) | MASK_USBD_EP12_DMA_FIFO_FLUSH, UDC_DMA_CS_OFST);
	while(!(udc_read(UDC_DMA_CS_OFST) & MASK_USBD_EP12_DMA_FLUSH));	/* wait flush done */
}	

static inline void _ep5_stop(void)
{
	udc_write(udc_read(UDC_EP5CTL_OFST) | MASK_USBD_EP5_FLUSH, UDC_EP5CTL_OFST);
	udc_write(udc_read(UDC_EP5CTL_OFST) & ~MASK_USBD_EP5_FLUSH, UDC_EP5CTL_OFST);
	udc_write(udc_read(UDC_DMA_CS_OFST) & ~MASK_USBD_EP12_DMA_EN, UDC_DMA_CS_OFST);
	//udc_write(udc_read(UDC_DMA_CS_OFST) | MASK_USBD_EP12_DMA_FIFO_FLUSH, UDC_DMA_CS_OFST);
	//while(!(udc_read(UDC_DMA_CS_OFST) & MASK_USBD_EP12_DMA_FLUSH));	/* wait flush done */
}	

static int _udc_ep5_handler(struct gp329xxb_ep* ep, struct gp329xxb_request* req)
{
	struct gp329xxb_udc* udc = ep->udc;
	u32 cnt;
	u32 timecnt = EP5_CHECK_TIMEOUT;
	
	/* thread environment */
	if(req->req.length)
	{
		udc->ep_req[ep_num_5] = req;
		/* The DMA buffer address was translated to physical address in gp_usb.c */
		//printk("ep5: buf 0x%x, len 0x%x(%d KB)\n", req->req.buf, req->req.length, req->req.length >> 10);
		/* Enable EP2 IN by request from gadget driver */	
		cnt = _ep5_porcess_in(req->req.buf, req->req.length);
		
		while(timecnt--)
		{
			if(udc_read(UDC_EP5RPTR_OFST))
			{
				timecnt = EP5_CHECK_TIMEOUT;
				break;
			}	
			
			udelay(100);
		}	
		
		if(timecnt != EP5_CHECK_TIMEOUT)
		{
			_ep5_flush();
			req->req.actual = req->req.length;
			udc->ep_req[ep_num_5] = NULL;
			gp329xxb_udc_ep_done(ep, req, 0);
			//printk("EP5 flush\n");
		}	
			
	}
	else
	{
		/* usb request with 0 length */
		req->req.actual = req->req.length;
		udc->ep_req[ep_num_5] = NULL;
		/* flush EP5 */
		_ep5_flush();
		gp329xxb_udc_ep_done(ep, req, 0);
		printk("Flush EP5\n");
	}		

	return 0;
}

static inline void _ep7_flush(void)
{	
	udc_write(udc_read(UDC_EP7CTL_OFST) | MASK_USBD_EP7_BUF_FLUSH, UDC_EP7CTL_OFST);
	udc_write(udc_read(UDC_EP7CTL_OFST) & ~MASK_USBD_EP7_BUF_FLUSH, UDC_EP7CTL_OFST);
	udc_write(udc_read(UDC_EP7DMA_OFST) & ~MASK_USBD_EP7_DMA_EN, UDC_EP7DMA_OFST);
	udc_write(0, UDC_EP7DMA_OFST);
	udc_write(udc_read(UDC_EP7DMA_OFST) | MASK_USBD_EP7_DMA_FLUSH, UDC_EP7DMA_OFST);
	while((udc_read(UDC_EP7DMA_OFST) & MASK_USBD_EP7_DMA_FLUSH));	/* wait flush done */
}

static inline void _ep7_stop(void)
{	
	udc_write(udc_read(UDC_EP7DMA_OFST) & ~MASK_USBD_EP7_DMA_EN, UDC_EP7DMA_OFST);
	udc_write(udc_read(UDC_EP7CTL_OFST) | MASK_USBD_EP7_BUF_FLUSH, UDC_EP7CTL_OFST);
	udc_write(udc_read(UDC_EP7CTL_OFST) & ~MASK_USBD_EP7_BUF_FLUSH, UDC_EP7CTL_OFST);
	udc_write(0, UDC_EP7DMA_OFST);
}

static int _udc_ep7_handler(struct gp329xxb_ep* ep, struct gp329xxb_request* req)
{
	struct gp329xxb_udc* udc = ep->udc;
	u32 cnt;
	u32 timecnt = EP7_CHECK_TIMEOUT;
	
	/* thread environment */
	if(req->req.length)
	{
		udc->ep_req[ep_num_7] = req;
		/* The DMA buffer address was translated to physical address in gp_usb.c */
		//printk("ep7: buf 0x%x, len 0x%x(%d KB)\n", req->req.buf, req->req.length, req->req.length >> 10);
		cnt = _ep7_porcess_in(req->req.buf, req->req.length);	
		while(timecnt--)
		{
			if(udc_read(UDC_EP7RPTR_OFST))
			{
				timecnt = EP7_CHECK_TIMEOUT;
				break;
			}	
			
			udelay(100);
		}	
		
		if(timecnt != EP7_CHECK_TIMEOUT)
		{
			_ep7_flush();
			req->req.actual = req->req.length;
			udc->ep_req[ep_num_7] = NULL;
			gp329xxb_udc_ep_done(ep, req, 0);
			//printk("EP7 flush\n");
		}	
	}
	else
	{
		/* usb request with 0 length */
		req->req.actual = req->req.length;
		udc->ep_req[ep_num_7] = NULL;
		/* flush EP7 */
		_ep7_flush();
		//_ep7_stop();
		gp329xxb_udc_ep_done(ep, req, 0);
		printk("Flush EP7\n");
	}		

	return 0;
}

static inline void _ep0_reset(void)
{
	/* clear EP0 OUT valid */
	udc_write(MASK_USBD_EP0_CS_CLE_EP0_OUT_VLD, UDC_EP0CS_OFST);
	/* clear 0x350 data count */
	udc_write(0x0, UDC_EP0DC_OFST);
}	

static inline void _ep12_reset(void)
{
	/* Check each EP DMA status, if exist then flush it */
	/* EP12 DMA */
	if(udc_read(UDC_DMA_CS_OFST) & MASK_USBD_EP12_DMA_EN)
	{
		udc_write(udc_read(UDC_DMA_CS_OFST) | MASK_USBD_EP12_DMA_FIFO_FLUSH, UDC_DMA_CS_OFST);
		while(!(udc_read(UDC_DMA_CS_OFST) & MASK_USBD_EP12_DMA_FLUSH)); 
	}
	
	/* clear EP1 special control register */
    udc_write(MASK_USBD_EP1_CLR_IN_VALID | MASK_USBD_EP1_RESET_EP1S_FIFO, UDC_EP1SCS_OFST);
	/* clear EP2 OUT valid and reset FIFO */
	udc_write(MASK_USBD_EP12_C_CLR_EP2_OVLD | MASK_USBD_EP12_C_RESET_PIPO_FIFO, UDC_EP12C_OFST);
	//printk("_ep12_reset, 0x334 = 0x%x\n", udc_read(UDC_EP12PPC_OFST));
	/* swtich PING/PONG buffer */
	udc_write(udc_read(UDC_EP12PPC_OFST) | MASK_USBD_EP12_FIFO_C_SWITCH_BUF, UDC_EP12PPC_OFST);
	/* clear EP2 OUT valid and reset FIFO */
	udc_write(MASK_USBD_EP12_C_CLR_EP2_OVLD | MASK_USBD_EP12_C_RESET_PIPO_FIFO, UDC_EP12C_OFST);
	/* reset FIFO count including PING(0x368 0x36C)/PONG(0x39C 0x3A0) */
	udc_write(0 ,UDC_EP12FCL_OFST);
	udc_write(udc_read(UDC_EP12FCH_OFST) | MASK_USBD_EP12_RESET_PING_CNTR, UDC_EP12FCH_OFST);
	udc_write(0 ,UDC_EP12FCH_OFST);
	udc_write(0 ,UDC_EP12POFDLB_OFST);
	udc_write(udc_read(UDC_EP12POFDHB_OFST) | MASK_USBD_EP12_RESET_PING_CNTR, UDC_EP12POFDHB_OFST);
	udc_write(0 ,UDC_EP12POFDHB_OFST);
	
	/* diable EP12 bulk in/out */
	udc_write(udc_read(UDC_EP12PPC_OFST) & ~MASK_USBD_EP12_C_EP12_ENA, UDC_EP12C_OFST);
	/* disable EP12 auto-switch function */
	udc_write(udc_read(UDC_EP12PPC_OFST) & ~MASK_USBD_EP12_FIFO_C_ASE, UDC_EP12PPC_OFST); 
}	

static inline void _ep5_reset(void)
{
	/* flush EP5 */
	udc_write(udc_read(UDC_EP5CTL_OFST) | MASK_USBD_EP5_FLUSH, UDC_EP5CTL_OFST);
	udc_write(udc_read(UDC_EP5CTL_OFST) & ~MASK_USBD_EP5_FLUSH, UDC_EP5CTL_OFST);
	
	udc_write(udc_read(UDC_DMA_CS_OFST) & ~MASK_USBD_EP12_DMA_EN, UDC_DMA_CS_OFST); /* disable 12 DMA */
	udc_write(udc_read(UDC_DMA_CS_OFST) | MASK_USBD_EP12_DMA_FIFO_FLUSH, UDC_DMA_CS_OFST); /* flush 12 DMA */
	while(!(udc_read(UDC_DMA_CS_OFST) & MASK_USBD_EP12_DMA_FLUSH));	/* wait flush done */
}	

void _udc_reset(void)
{
	/* prevent LNK to suspend PHY*/
	udc_write(udc_read(UDC_LLCSET0_OFST) | MASK_USBD_UDLC_CS0_PWR_SUSP_N , UDC_LLCSET0_OFST);
	
	_ep0_reset();
	
	if(udc_type == UDC_MSDC_TYPE)
		_ep12_reset();
	else if(udc_type == UDC_UVC_TYPE)
		_ep5_reset();	
		
	/* clear EP1/2/3 stall status */
	udc_write((MASK_USBD_UDLC_STL_CLREP0STL | MASK_USBD_UDLC_STL_CLREP1STL | MASK_USBD_UDLC_STL_CLREP2STL), UDC_LLCSTL_OFST);
	
	/* clear 0x400 flag */
	udc_write(0xFFFFFFFF, UDC_LLCIF_OFST);
	
	/*Clear 0x408 flag */
	udc_write(0xFFFFFFFF, UDC_LLCIS_OFST); 
	
	/* clear 0x88 flag */
	udc_write(0xFFFFFFFF, UDC_IF_OFST);
	
	/* clear 0x410 flag */
	udc_write(0xFFFFFFFF, UDC_SREQIF_OFST);
	
	//printk("_udc_reset, 0x000 = 0x%x, 0x400 = 0x%x, 0x404 = 0x%x, 0x330 = 0x%x, 0x334= 0x%x\n", 
	//			udc_read(UDC_DMA_CS_OFST), udc_read(UDC_LLCIF_OFST), udc_read(UDC_LLCIE_OFST), udc_read(UDC_EP12C_OFST), udc_read(UDC_EP12PPC_OFST));
}	

/*
* Handlers of gp329xxb UDC interrupt service routines
*
*/
/* Handle 0x400 UDLC flag register */
static void _udc_udlc_flag_isr_handle(struct gp329xxb_udc* udc, int flag)
{
	if(flag & MASK_USBD_UDLC_IF_RESET)
	{
		printk("Got RST signal\n");
		/* Bit0, received a reset signal */
		udc_write(MASK_USBD_UDLC_IF_RESET, UDC_LLCIF_OFST);
		/* reset UDC */
		_udc_reset();
		/* Check USB request in UDC for each endpoint, if any remove it to notify gadget driver */
		_udc_clear_ep_usb_request(udc, -ECONNRESET);
	}    
    else if(flag & MASK_USBD_UDLC_IF_SUSPEND)
    {
		printk("Got suspend signal\n");
    	/* Bit16, received a suspend signal */
    	udc_write(MASK_USBD_UDLC_IF_SUSPEND, UDC_LLCIF_OFST);
    	udc->driver->suspend(&udc->gadget);
	}
	else if(flag & MASK_USBD_UDLC_IF_RESUME)
	{
		//printk("MASK_USBD_UDLC_IF_RESUME\n");
		/* Bit17, reveived a resume signal */
	    udc_write(MASK_USBD_UDLC_IF_RESUME, UDC_LLCIF_OFST);
	    udc->driver->resume(&udc->gadget);
	}
	else if(flag & MASK_USBD_UDLC_IF_SCONF)
	{
		//printk("MASK_USBD_UDLC_IF_SCONF\n");
		/* Bit18, reveived a set configuration command from host*/
	    udc_write(MASK_USBD_UDLC_IF_SCONF, UDC_LLCIF_OFST); 
		_ep0_handle_setup_command(udc, SRC_SET_CONFIG);
	}
	else if(flag & MASK_USBD_UDLC_IF_HCS)
	{
		//printk("MASK_USBD_UDLC_IF_HCS\n");
		/* Bit12, UDLC HOST_CLEAR_STALL interrupt flag */
		udc_write(MASK_USBD_UDLC_IF_HCS, UDC_LLCIF_OFST);
		//printk("Clear stall, 0x3bc = 0x%x\n", udc_read(UDC_LLCSTL_OFST));
	}
	/* The corresponding bit of rDLCIE_UDLC interrupt enable register will be set to 0 to disable interrupt */
	else if(flag & MASK_USBD_UDLC_IF_EP0S)
	{
		//printk("MASK_USBD_UDLC_IF_EP0S\n");
		udc_write(MASK_USBD_UDLC_IF_EP0S, UDC_LLCIF_OFST);
		/* Bit2, got a setup command, check FIFO is valid and updated */
		if(udc_read(UDC_EP0CS_OFST) & (MASK_USBD_EP0_CS_EP0_SFIFO_VALID | MASK_USBD_EP0_CS_EP0_SFIFO_UPDATE))
		{
			/* USB Device Receive non-standard or Get/Set Descriptor Setup Packet */
        	_ep0_handle_setup_command(udc, SRC_STD_SETUP);
		}
		else
		{
			printk("Got Setup cmd interrupt, but fifo is invalid\n");
		}	
	}
	else if(flag & MASK_USBD_UDLC_IF_EP0I)
	{
		/* Bit4, EP0 IN transaction */
		udc_write((udc_read(UDC_LLCIE_OFST) & ~MASK_USBD_UDLC_IE_EP0I), UDC_LLCIE_OFST);
		udc_write(MASK_USBD_UDLC_IF_EP0I, UDC_LLCIF_OFST);
		if(!(udc_read(UDC_EP0CS_OFST) & MASK_USBD_EP0_CS_EP0_IVLD))
		{
			/* EP0 IN data has been received by host */
			if(udc->ep_req[ep_num_0] != NULL)
			{
				struct gp329xxb_request* req = udc->ep_req[ep_num_0];	/* Get EP0 request handler */
				struct gp329xxb_ep* ep = &udc->ep[ep_num_0];
				u32 cnt;
				
				if(req->req.length >= req->req.actual)
				{
					
					if(req->req.length > req->req.actual)
					{
						cnt = _ep0_send_in((u32*)((char*)req->req.buf + req->req.actual), (req->req.length - req->req.actual));
						if(cnt)
						{
							req->req.actual += cnt;
						}	
						//printk("EP0 IN ACK, length[%d] != autual[%d]\n", req->req.length, req->req.actual);
					}
					else
					{
						/* acutal reach length */
						if(udc->ep0sendnull)
						{
							_ep0_send_in(NULL, 0);		/* Send NULL IN packet to end this data IN transaction */
							udc->ep0sendnull = 0;
							//printk("EP0 IN ACK, ep0 send null\n");
						}
						else
						{	
							/* req length = 0, set gp329xxb request handler to NULL */
							//printk("EP0 IN ACK, ep0 done\n");
							udc->ep_req[ep_num_0] = NULL;
							_ep0_process_status_stage(udc);
							gp329xxb_udc_ep_done(ep, req, 0);
						}	
					}	
				}
				else
				{
					printk("EP0 IN ACK, but actual larger than length\n");
					udc->ep_req[ep_num_0] = NULL;
					gp329xxb_udc_ep_done(ep, req, -ECOMM);
				}		
			}	
		}
		else
		{
			udc->ep_req[ep_num_0] = NULL;
		}		
	}		
	else if(flag & MASK_USBD_UDLC_IF_EP0O)
	{
		//printk("MASK_USBD_UDLC_IF_EP0O\n");
		/* Bit3 EP0 OUT transaction */
		udc_write((udc_read(UDC_LLCIE_OFST) & ~MASK_USBD_UDLC_IE_EP0O), UDC_LLCIE_OFST);
		udc_write(MASK_USBD_UDLC_IF_EP0O, UDC_LLCIF_OFST);
		
		if(udc_read(UDC_EP0CS_OFST) & MASK_USBD_EP0_CS_EP0_OVLD)
		{
			if(udc->ep_req[ep_num_0] != NULL)
			{
				struct gp329xxb_request* req = udc->ep_req[ep_num_0];	/* Get EP0 request handler */
				struct gp329xxb_ep* ep = &udc->ep[ep_num_0];
				u32 cnt;
				
				/* A OUT data received, start to get it */
				cnt = _ep0_get_out_data((u32*)req->req.buf, req->req.length);
				if(cnt)
				{
					req->req.actual += cnt;
				}
				
				if(req->req.length >= req->req.actual)
				{
					if(req->req.length > req->req.actual)
					{
						/* There is more data from host to get */
						_ep0_enable_out();
					}
					else
					{
						/* last OUT data got, end this request and reset gp329xxb request hadler */
						udc->ep_req[ep_num_0] = NULL;
						_ep0_process_status_stage(udc);
						gp329xxb_udc_ep_done(ep, req, 0);
					}		
				}
				else
				{
					printk("EP0 OUT ACK, but actual larger than length\n");
					udc->ep_req[ep_num_0] = NULL;
					gp329xxb_udc_ep_done(ep, req, -ECOMM);
				}		
			}	
		}
	}
	else if(flag & MASK_USBD_UDLC_IF_EP2O)
	{
		//printk("MASK_USBD_UDLC_IF_EP2O\n");
		/* Bit7, Check EP2 BULK OUT */
		udc_write((udc_read(UDC_LLCIE_OFST) & ~MASK_USBD_UDLC_IE_EP2O), UDC_LLCIE_OFST);
		udc_write(MASK_USBD_UDLC_IF_EP2O, UDC_LLCIF_OFST);
		/* disable bulk in/out first */
		udc_write(udc_read(UDC_EP12C_OFST) & ~MASK_USBD_EP12_C_EP12_ENA, UDC_EP12C_OFST);
		/* disable auto switch FIFO */
		udc_write(udc_read(UDC_EP12PPC_OFST) & ~MASK_USBD_EP12_FIFO_C_ASE, UDC_EP12PPC_OFST);	
		
		if(udc->ep_req[ep_num_2] != NULL)
		{
			struct gp329xxb_request* req = udc->ep_req[ep_num_2];	/* Get EP2 request handler */
			struct gp329xxb_ep* ep = &udc->ep[ep_num_2];
			u32 cnt;
			
			//printk("EP2 OUT interrupt in PIO mode\n");
			/* PIO mode, switch fifo for valid buffer */
			_ep2_switch_fifo_for_valid_buffer();
			cnt = _ep2_get_pio_data(&req->req, req->req.length);
			
			if(cnt)
				req->req.actual += cnt;
			
			if(req->req.actual == req->req.length)
			{
				gp329xxb_udc_ep_done(ep, req, 0);
				udc->ep_req[ep_num_2] = NULL;
			}				
		}
	}
	else if(flag & MASK_USBD_UDLC_IF_EP1INN)
	{
		//printk("MASK_USBD_UDLC_IF_EP1INN\n");
		/* Bit15, Check EP1 BULK DMA IN transaction */
		udc_write((udc_read(UDC_LLCIE_OFST) & ~MASK_USBD_UDLC_IE_EP1INN), UDC_LLCIE_OFST);
		udc_write(MASK_USBD_UDLC_IF_EP1INN, UDC_LLCIF_OFST);
		/* disable bulk in/out first */
		udc_write(udc_read(UDC_EP12C_OFST) & ~MASK_USBD_EP12_C_EP12_ENA, UDC_EP12C_OFST);
		/* disable auto switch FIFO */
		udc_write(udc_read(UDC_EP12PPC_OFST) & ~MASK_USBD_EP12_FIFO_C_ASE, UDC_EP12PPC_OFST);	
		
		/* DMA bulk IN done */
		if(udc->ep_req[ep_num_1] != NULL)
		{
			struct gp329xxb_request* req = udc->ep_req[ep_num_1];	/* Get EP1 request handler */
			struct gp329xxb_ep* ep = &udc->ep[ep_num_1];
			
			//printk("EP1 DMA IN done, req len = %d, req actual = %d\n", req->req.length, req->req.actual);
			udc->ep_req[ep_num_1] = NULL;
			gp329xxb_udc_ep_done(ep, req, 0);
		}	
	}
	else if(flag & MASK_USBD_UDLC_IF_EP1I)
	{
		//printk("MASK_USBD_UDLC_IF_EP1I\n");
		/* Bit6, Check EP1 BULK IN transaction */
		udc_write((udc_read(UDC_LLCIE_OFST) & ~MASK_USBD_UDLC_IE_EP1I), UDC_LLCIE_OFST);
		udc_write(MASK_USBD_UDLC_IF_EP1I, UDC_LLCIF_OFST);
		/* disable bulk in/out first */
		udc_write(udc_read(UDC_EP12C_OFST) & ~MASK_USBD_EP12_C_EP12_ENA, UDC_EP12C_OFST);
		/* disable auto switch FIFO */
		udc_write(udc_read(UDC_EP12PPC_OFST) & ~MASK_USBD_EP12_FIFO_C_ASE, UDC_EP12PPC_OFST);	
		
		/* PIO bulk IN done */
		if(udc->ep_req[ep_num_1] != NULL)
		{
			struct gp329xxb_request* req = udc->ep_req[ep_num_1];	/* Get EP1 request handler */
			struct gp329xxb_ep* ep = &udc->ep[ep_num_1];
			
			udc->ep_req[ep_num_1] = NULL;
			gp329xxb_udc_ep_done(ep, req, 0);
		}
	}		
	else if(flag & MASK_USBD_UDLC_IF_EP3I)
	{
		//printk("MASK_USBD_UDLC_IF_EP3I\n");
		/* Bit14, Check EP3 INT IN transaction */
		udc_write((udc_read(UDC_LLCIE_OFST) & ~MASK_USBD_UDLC_IE_EP3I), UDC_LLCIE_OFST);
		udc_write(MASK_USBD_UDLC_IF_EP3I, UDC_LLCIF_OFST);
	}
	else if(flag & MASK_USBD_UDLC_IF_EP4I)
	{
		//printk("MASK_USBD_UDLC_IF_EP4I\n");
		/* Bit20, Check EP4 INT IN transaction */
		udc_write((udc_read(UDC_LLCIE_OFST) & ~MASK_USBD_UDLC_IE_EP4I), UDC_LLCIE_OFST);
		udc_write(MASK_USBD_UDLC_IF_EP4I, UDC_LLCIF_OFST);
	}
	else if(flag & MASK_USBD_UDLC_IF_EP6I)
	{
		//printk("MASK_USBD_UDLC_IF_EP6I\n");
		/* Bit22, Check EP6 INT IN transaction */
		udc_write((udc_read(UDC_LLCIE_OFST) & ~MASK_USBD_UDLC_IE_EP6I), UDC_LLCIE_OFST);
		udc_write(MASK_USBD_UDLC_IF_EP6I, UDC_LLCIF_OFST);
	}
	else if(flag & MASK_USBD_UDLC_IF_EP5I)
	{
		//printk("MASK_USBD_UDLC_IF_EP5I\n");
		/* Bit21, Check EP5 ISO IN transaction, this bit occurred when host send an IN token to device*/
		udc_write((udc_read(UDC_LLCIE_OFST) & ~MASK_USBD_UDLC_IE_EP5I), UDC_LLCIE_OFST);
		udc_write(MASK_USBD_UDLC_IF_EP5I, UDC_LLCIF_OFST);
	}
	else if(flag & MASK_USBD_UDLC_IF_EP7I)
	{
		//printk("MASK_USBD_UDLC_IF_EP7I\n");
		/* Bit23, Check EP7 ISO IN transaction, this it is 1 when host send an IN token to device */
		udc_write((udc_read(UDC_LLCIE_OFST) & ~MASK_USBD_UDLC_IE_EP7I), UDC_LLCIE_OFST);
		udc_write(MASK_USBD_UDLC_IF_EP7I, UDC_LLCIF_OFST);
	}
	else if(flag & MASK_USBD_UDLC_IF_EP5IEND)
	{
		//printk("MASK_USBD_UDLC_IF_EP5IEND\n");
		/* Bit24, Check EP5 ISO IN transaction end */
		udc_write((udc_read(UDC_LLCIE_OFST) & ~MASK_USBD_UDLC_IE_EP5IEND), UDC_LLCIE_OFST);
		udc_write(MASK_USBD_UDLC_IF_EP5IEND, UDC_LLCIF_OFST);
		udc_write(MASK_USBD_UDLC_IF_EP5I, UDC_LLCIF_OFST);
		
		/* Video ISO IN done */
		if(udc->ep_req[ep_num_5] != NULL)
		{
			struct gp329xxb_request* req = udc->ep_req[ep_num_5];	/* Get EP5 request handler */
			struct gp329xxb_ep* ep = &udc->ep[ep_num_5];
			
			req->req.actual = req->req.length;
			udc->ep_req[ep_num_5] = NULL;
			gp329xxb_udc_ep_done(ep, req, 0);
		}
		
	}
	else if(flag & MASK_USBD_UDLC_IF_EP7IEND)
	{
		//printk("MASK_USBD_UDLC_IF_EP7IEND\n");
		/* Bit25, Check EP7 ISO IN transaction end */
		udc_write((udc_read(UDC_LLCIE_OFST) & ~MASK_USBD_UDLC_IE_EP7IEND), UDC_LLCIE_OFST);
		udc_write(MASK_USBD_UDLC_IF_EP7IEND, UDC_LLCIF_OFST);
		
		/* Audio ISO IN done */
		if(udc->ep_req[ep_num_7] != NULL)
		{
			struct gp329xxb_request* req = udc->ep_req[ep_num_7];	/* Get EP5 request handler */
			struct gp329xxb_ep* ep = &udc->ep[ep_num_5];
			
			req->req.actual = req->req.length;
			udc->ep_req[ep_num_7] = NULL;
			gp329xxb_udc_ep_done(ep, req, 0);
		}
	}
	else if(flag & MASK_USBD_UDLC_IF_EP0N)
	{
		//printk("MASK_USBD_UDLC_IF_EP0N\n");
		/* Check EP0 Nak */
		udc_write(MASK_USBD_UDLC_IF_EP0N, UDC_LLCIF_OFST);
	}
	else if(flag & MASK_USBD_UDLC_IF_EP1N)
	{
		//printk("MASK_USBD_UDLC_IF_EP1N\n");
		/* Check EP1 Nak */
		udc_write(MASK_USBD_UDLC_IF_EP1N, UDC_LLCIF_OFST);
	}
	else if(flag & MASK_USBD_UDLC_IF_EP2N)
	{
		//printk("MASK_USBD_UDLC_IF_EP2N\n");
		/* Check EP2 Nak */
		udc_write(MASK_USBD_UDLC_IF_EP2N, UDC_LLCIF_OFST);
	}
	else
	{
		printk(" %s, flag 0x%x could not be handled\r\n", __func__, flag);
	}	
}	

/* Handle 0x88 USB device controller register flag */
static void _udc_ctl_flag_isr_handle(struct gp329xxb_udc* udc, int flag)
{
	if((flag & MASK_USBD_UDC_IF_DMA))
    {	
    	printk("MASK_USBD_UDC_IF_DMA\n");
    	/* Bit25, EP12 and EP5 BULK+ISO finish DMA interrupt flag */
    	udc_write(udc_read(UDC_IE_OFST) & ~MASK_USBD_UDC_IE_DMA, UDC_IE_OFST);
    	udc_write(MASK_USBD_UDC_IF_DMA, UDC_IF_OFST);
	}    	
	else if((flag & MASK_USBD_UDC_IF_EP89_DMA))
    {	
    	//printk("MASK_USBD_UDC_IF_EP89_DMA\n");
    	/* Bit29, EP89 BULK DMA finish interrupt flag */
    	udc_write(udc_read(UDC_IE_OFST) & ~MASK_USBD_UDC_IE_EP89_DMA, UDC_IE_OFST);
    	udc_write(MASK_USBD_UDC_IF_EP89_DMA, UDC_IF_OFST);
	}
	else if((flag & MASK_USBD_UDC_IF_EPAB_DMA))
    {	
    	//printk("MASK_USBD_UDC_IF_EPAB_DMA\n");
    	/* Bit30, EPAB BULK DMA finish interrupt flag */
    	udc_write(udc_read(UDC_IE_OFST) & ~MASK_USBD_UDC_IE_EPAB_DMA, UDC_IE_OFST); 
    	udc_write(MASK_USBD_UDC_IF_EPAB_DMA, UDC_IF_OFST);
	}
	else if((flag & MASK_USBD_UDC_IF_AUDIO_DMA))
	{
		//printk("MASK_USBD_UDC_IF_AUDIO_DMA\n");
		/* Bit28, EPAB BULK DMA finish interrupt flag */
		udc_write(udc_read(UDC_IE_OFST) & ~MASK_USBD_UDC_IF_AUDIO_DMA, UDC_IE_OFST); 
		udc_write(MASK_USBD_UDC_IE_AUDIO_DMA, UDC_IF_OFST);
	}
	else if((flag & MASK_USBD_UDC_IF_VBUS))
	{
		printk("MASK_USBD_UDC_IF_VBUS\n");
		/* Bit24, VBUS interrupt flag */
		udc_write(MASK_USBD_UDC_IF_VBUS, UDC_IF_OFST);
	}
	else
	{
		printk(" %s, flag 0x%x could not be handled\r\n", __func__, flag);
	}	
}	

static void _udc_std_req_flag_isr_handle(struct gp329xxb_udc* udc, int flag)
{
	if(flag & MASK_USBD_STDREQ_IF_SET_INTF)
    {
    	/* Bit9, Set interface interrupt flag */
    	//printk("MASK_USBD_STDREQ_IF_SET_INTF\n");
    	udc_write(MASK_USBD_STDREQ_IF_SET_INTF, UDC_SREQIF_OFST);
	}		
	if(flag & MASK_USBD_STDREQ_IF_CLR_EP0_STALL)
    {
    	/* Bit0, Clear feature interrupt flag for EP0 */
    	//printk("MASK_USBD_STDREQ_IF_CLR_EP0_STALL\n");
    	udc_write(MASK_USBD_STDREQ_IF_CLR_EP0_STALL, UDC_SREQIF_OFST);
	}
	if(flag & MASK_USBD_STDREQ_IF_CLR_EPX_STALL)
    {
    	/* Bit1, Clear feature interrupt flag (other EP stall) */
    	//printk("MASK_USBD_STDREQ_IF_CLR_EPX_STALL\n");
    	udc_write(MASK_USBD_STDREQ_IF_CLR_EPX_STALL, UDC_SREQIF_OFST);
	}
	else
	{
		printk(" %s, flag 0x%x could not be handled\r\n", __func__, flag);
	}		
}	

static irqreturn_t gp329xxb_udc_irq(int dummy, void *_udc)
{
	struct gp329xxb_udc *udc = _udc;
	int udc_udlc_flag, udc_ctl_flag, udc_std_req_flag;
	int udc_udlc_flag_en, udc_ctl_flag_en, udc_std_req_flag_en;

	/* In this IRQ, handle registers 0x400/0404(flag/enable), 0x084/0x088(enable/flag), 0x410/0x414(flag/enable) */
	udc_udlc_flag = udc_read(UDC_LLCIF_OFST);						/* 0x400 */
	udc_udlc_flag_en = udc_read(UDC_LLCIE_OFST);					/* 0x404 */
	
	udc_ctl_flag = udc_read(UDC_IF_OFST) & 0xFF000000;				/* 0x088 */
	udc_ctl_flag_en = udc_read(UDC_IE_OFST) & 0xFF000000;			/* 0x084 */
	
	udc_std_req_flag = udc_read(UDC_SREQIF_OFST);					/* 0x410 */
	udc_std_req_flag_en = udc_read(UDC_SREQIE_OFST);				/* 0x414 */
	
	if(udc_udlc_flag & udc_udlc_flag_en)
	{
		_udc_udlc_flag_isr_handle(udc, udc_udlc_flag);
	}
	else if(udc_ctl_flag & udc_ctl_flag_en)
	{
		_udc_ctl_flag_isr_handle(udc, udc_ctl_flag);
	}
	else if(udc_std_req_flag & udc_std_req_flag_en)
	{
		_udc_std_req_flag_isr_handle(udc, udc_std_req_flag);
	}			
	return IRQ_HANDLED;
}

/* UDC endpoint operation function for gadget driver */
/*
 *	gp329xxb_udc_ep_enable
 */
static int gp329xxb_udc_ep_enable(struct usb_ep *_ep,
				 const struct usb_endpoint_descriptor *desc)
{
	struct gp329xxb_udc	*udc;
	struct gp329xxb_ep	*ep;
	u32			max;

	ep = to_gp329xxb_ep(_ep);

	if (!_ep || !desc || ep->desc
			|| _ep->name == ep0name
			|| desc->bDescriptorType != USB_DT_ENDPOINT)
		return -EINVAL;

	udc = ep->udc;
	if (!udc->driver || udc->gadget.speed == USB_SPEED_UNKNOWN)
		return -ESHUTDOWN;

	max = le16_to_cpu(desc->wMaxPacketSize) & 0x1fff;

	_ep->maxpacket = max & 0x7ff;
	ep->desc = desc;
	ep->bEndpointAddress = desc->bEndpointAddress;


	/* Clear EPs FIFO and reset status specified EP */    
	switch (ep->num)
	{
       case 0:
       		/* reset ep0 data count */
			udc_write(0, UDC_EP0DC_OFST);
			/* clear EP0 IN/OUT interrupt flag */
			udc_write(MASK_USBD_UDLC_IF_EP0I | MASK_USBD_UDLC_IE_EP0I, UDC_LLCIF_OFST);
			/* disable EP0 IN and OUT interrupt */
			udc_write(udc_read(UDC_LLCIE_OFST) & ~(MASK_USBD_UDLC_IE_EP0I | MASK_USBD_UDLC_IE_EP0O), UDC_LLCIE_OFST);
		 	break;
	   case 1:
       case 2:
       		/* disable auto switch FIFO */
			udc_write(udc_read(UDC_EP12PPC_OFST) & ~MASK_USBD_EP12_FIFO_C_ASE, UDC_EP12PPC_OFST);
			/* reset current and next EP12 FIFO, and set EP12 direction = IN, bit0 = 1 */
			udc_write(udc_read(UDC_EP12C_OFST) | MASK_USBD_EP12_C_RESET_PIPO_FIFO | MASK_USBD_EP12_C_EP12_DIR, UDC_EP12C_OFST);
		 	/* disable BULK IN/OUT */
			udc_write(udc_read(UDC_EP12C_OFST) & ~MASK_USBD_EP12_C_EP12_ENA, UDC_EP12C_OFST);
		 	break;
	
		default:
			return -EINVAL;
	}

	/* clear EP stall function */
	gp329xxb_udc_set_halt(_ep, 0);
	
	//printk("%s, %s enable, num %d\n", __func__, _ep->name, ep->num);
	
	return 0;
}

/*
 * gp329xxb_udc_ep_disable
 */
static int gp329xxb_udc_ep_disable(struct usb_ep *_ep)
{
	struct gp329xxb_ep *ep = to_gp329xxb_ep(_ep);

	if (!_ep || !ep->desc)
	{
		return -EINVAL;
	}

	ep->desc = NULL;
	
	while (!list_empty (&ep->queue))
	{
		struct gp329xxb_request *req;
		req = list_entry (ep->queue.next, struct gp329xxb_request, queue);
		gp329xxb_udc_ep_done(ep, req, -ESHUTDOWN);
	}
	
	/* Clear EPs FIFO and reset status specified EP */    
	switch (ep->num)
	{
       case 0:
       		/* reset ep0 data count */
			udc_write(0, UDC_EP0DC_OFST);
		 	break;
	   case 1:
       case 2:
       		/* disable auto switch FIFO */
			udc_write(udc_read(UDC_EP12PPC_OFST) & ~MASK_USBD_EP12_FIFO_C_ASE, UDC_EP12PPC_OFST);
			/* reset current and next EP12 FIFO, and set EP12 direction = IN, bit0 = 1 */
			udc_write(udc_read(UDC_EP12C_OFST) | MASK_USBD_EP12_C_RESET_PIPO_FIFO | MASK_USBD_EP12_C_EP12_DIR, UDC_EP12C_OFST);
		 	/* disable BULK IN/OUT */
			udc_write(udc_read(UDC_EP12C_OFST) & ~MASK_USBD_EP12_C_EP12_ENA, UDC_EP12C_OFST);
		 	break;
	
		default:
			return -EINVAL;
	}

	//printk("%s, %s disabled\n", __func__, _ep->name);

	return 0;
}

/*
 * gp329xxb_udc_alloc_request
 */
static struct usb_request* gp329xxb_udc_alloc_request(struct usb_ep *_ep, gfp_t mem_flags)
{
	struct gp329xxb_request *req;

	//printk("%s(%p,%d)\n", __func__, _ep, mem_flags);
	if (!_ep)
		return NULL;

	req = kzalloc (sizeof(struct gp329xxb_request), mem_flags);
	if (!req)
		return NULL;

	req->req.dma = DMA_ADDR_INVALID;
	INIT_LIST_HEAD (&req->queue);
	return &req->req;
}

/*
 * gp329xxb_udc_free_request
 */
static void gp329xxb_udc_free_request(struct usb_ep *_ep, struct usb_request *_req)
{
	struct gp329xxb_ep	*ep = to_gp329xxb_ep(_ep);
	struct gp329xxb_request	*req = to_gp329xxb_req(_req);


	if (!ep || !_req || (!ep->desc && _ep->name != ep0name))
		return;

	WARN_ON (!list_empty (&req->queue));
	kfree(req);
}

/*
 *	gp329xxb_udc_queue
 */
static int gp329xxb_udc_queue(struct usb_ep *_ep, struct usb_request *_req,
		gfp_t gfp_flags)
{
	struct gp329xxb_request	*req = to_gp329xxb_req(_req);
	struct gp329xxb_ep	*ep = to_gp329xxb_ep(_ep);
	struct gp329xxb_udc	*udc;
	u32			ret = FAIL;
	
	udc = ep->udc;
	if(unlikely (!udc->driver || udc->gadget.speed == USB_SPEED_UNKNOWN))
	{
		printk("speed unknow\n");
		return -ESHUTDOWN;
	}
	
	_req->status = -EINPROGRESS;	/* This status will be changed in gp329xxb_udc_ep_doone */
	_req->actual = 0;		/* reset the usb request actual data sending count */

	/* ignore the highest bit for direction IN/OUT then enter the corresponding endpoint's handler */
	switch(ep->bEndpointAddress & 0x7F)
	{
		case 0:
			ret = _udc_ep0_handler(ep, req);		/* Contol pipe */
			break;
		
		case 1:
			ret = _udc_ep1_handler(ep, req);		/* Bulk IN pipe */
			break;
		
		case 2:
			ret = _udc_ep2_handler(ep, req);		/* Bulk OUT pipe */
			break;	
		
		case 5:
			ret = _udc_ep5_handler(ep, req);		/* Video ISO IN pipe */;
			break;
		
		case 7:
			ret = _udc_ep7_handler(ep, req);		/* Audio ISO IN pipe */;
			break;
			
		default:
			printk("No handler for endpoint number 0x%x\n", ep->bEndpointAddress & 0x7F);
			return -EINVAL;
	}	

	return ret;		/* 0 = successful */
}

/*
 *	gp329xxb_udc_dequeue
 */
static int gp329xxb_udc_dequeue(struct usb_ep *_ep, struct usb_request *_req)
{
	struct gp329xxb_ep	*ep = to_gp329xxb_ep(_ep);
	struct gp329xxb_udc	*udc;
	int			retval = -EINVAL;
	struct gp329xxb_request	*req = NULL;

	if (!the_controller->driver)
		return -ESHUTDOWN;

	if (!_ep || !_req)
		return retval;

	udc = to_gp329xxb_udc(ep->gadget);

	list_for_each_entry (req, &ep->queue, queue)
	{
		if (&req->req == _req)
		{
			list_del_init (&req->queue);
			_req->status = -ECONNRESET;
			retval = 0;
			break;
		}
	}

	if (retval == 0)
	{
		gp329xxb_udc_ep_done(ep, req, -ECONNRESET);
	}

	return retval;
}

/*
 * gp329xxb_udc_set_halt
 */
static int gp329xxb_udc_set_halt(struct usb_ep *_ep, int value)
{
	struct gp329xxb_ep	*ep = to_gp329xxb_ep(_ep);
	u32			idx;

	if (unlikely (!_ep || (!ep->desc && ep->ep.name != ep0name)))
	{
		//printk("%s: inval 2\n", __func__);
		return -EINVAL;
	}

	idx = ep->bEndpointAddress & 0x7F;

	if(idx == 0)
	{
		if (value)
		{
			/* send ep0 stall */
			//printk("udc set halt idx=%d val=%d \n",idx, value);
			udc_write((udc_read(UDC_LLCSTL_OFST) | MASK_USBD_UDLC_STL_SETEP0STL) , UDC_LLCSTL_OFST);
		}	
		else
		{
			/* clear ep0 stall */
			//printk("udc clear halt idx=%d val=%d \n",idx, value);
			udc_write((udc_read(UDC_LLCSTL_OFST) | MASK_USBD_UDLC_STL_CLREP0STL) , UDC_LLCSTL_OFST);
		}		
	} 
	else
	{
		if(value)
       	{
			//printk("udc set halt(stall) idx=%d val=%d \n",idx,value);
			switch (ep->bEndpointAddress & 0x7F)
			{
				case 1:
					/* send ep1 stall */
					udc_write(udc_read(UDC_LLCSTL_OFST) |  MASK_USBD_UDLC_STL_SETEP1STL , UDC_LLCSTL_OFST);
					break;
			   case 2:
			   		/* send ep2 stall */
				   	udc_write(udc_read(UDC_LLCSTL_OFST) | MASK_USBD_UDLC_STL_SETEP2STL , UDC_LLCSTL_OFST);
					break;
			   default:
					return -EINVAL;
			}
		}
	   else
	   {
	   		//printk("udc clear halt(stall) idx=%d val=%d \n",idx,value);
			switch (ep->bEndpointAddress & 0x7F)
			{
				case 1:
					/* clear ep1 stall */
					udc_write(udc_read(UDC_LLCSTL_OFST) | MASK_USBD_UDLC_STL_CLREP1STL , UDC_LLCSTL_OFST);   
					break;
				case 2:
					/* clear ep2 stall */
					udc_write(udc_read(UDC_LLCSTL_OFST) | MASK_USBD_UDLC_STL_CLREP2STL , UDC_LLCSTL_OFST);
					break;
				default:
					return -EINVAL;
			}
		}
	}
	
	return 0;
}

/* usb_gadget_ops */
static int gp329xxb_udc_get_frame(struct usb_gadget *_gadget)
{
	return -EOPNOTSUPP;
}

static int gp329xxb_udc_wakeup(struct usb_gadget *_gadget)
{
	return -EOPNOTSUPP;
}

static int gp329xxb_udc_set_selfpowered(struct usb_gadget *gadget, int value)
{
	return -EOPNOTSUPP;
}

static int gp329xxb_udc_vbus_session(struct usb_gadget *gadget, int is_active)
{
	return -EOPNOTSUPP;
}

static int gp329xxb_udc_pullup(struct usb_gadget *gadget, int is_on)
{
	return -EOPNOTSUPP;
}

static int gp329xxb_vbus_draw(struct usb_gadget *_gadget, unsigned ma)
{
	return -EOPNOTSUPP;
}

void gp_usb_set_udc_type(u32 type)
{
	udc_type = type;
	printk("Set 329xxb UDC type %d\n", udc_type);
}	

EXPORT_SYMBOL(gp_usb_set_udc_type);

/*
 *	gp_usb_gadget_register_driver
 */
int gp_usb_gadget_register_driver(struct usb_gadget_driver *driver)
{
	struct gp329xxb_udc *udc = the_controller;
	int		retval;

	/* Sanity checks */
	if(!udc)
	{
		printk("gp_usb_gadget_register_driver, udc not found\n");
		return -ENODEV;
	}	

	if(udc->driver)
	{
		printk("gp_usb_gadget_register_driver, udc -EBUSY\n");
		return -EBUSY;
	}	

	if(!driver->bind || !driver->setup
			|| driver->speed < USB_SPEED_FULL)
	{
		printk(KERN_ERR "Invalid driver: bind %p setup %p speed %d\n",
			driver->bind, driver->setup, driver->speed);
		return -EINVAL;
	}

	if(!driver->unbind)
	{
		printk(KERN_ERR "Invalid driver: no unbind method\n");
		return -EINVAL;
	}

	/* Hook the driver */
	udc->driver = driver;
	udc->gadget.dev.driver = &driver->driver;

	/* Bind the driver */
	if((retval = device_add(&udc->gadget.dev)) != 0)
	{
		printk(KERN_ERR "Error in device_add() : %d\n",retval);
		goto register_error;
	}

	printk("binding gadget driver '%s'\n", driver->driver.name);

	if((retval = driver->bind(&udc->gadget)) != 0)
	{
		device_del(&udc->gadget.dev);
		goto register_error;
	}

	/* Enable udc */
	gp329xxb_udc_enable(udc);

	return 0;

register_error:
	udc->driver = NULL;
	udc->gadget.dev.driver = NULL;
	return retval;
}

/*
 *	gp_usb_gadget_unregister_driver
 */
int gp_usb_gadget_unregister_driver(struct usb_gadget_driver *driver)
{
	struct gp329xxb_udc *udc = the_controller;

	if (!udc)
		return -ENODEV;

	if (!driver || driver != udc->driver || !driver->unbind)
		return -EINVAL;

	/* Disable udc */
	gp329xxb_udc_disable(udc);
	
	if (driver->disconnect)
		driver->disconnect(&udc->gadget);

	device_del(&udc->gadget.dev);

	driver->unbind (&udc->gadget);

	udc->driver = NULL;
	
	return 0;
}

/* For platform device dirver usage */
/*
 *	probe - binds to the platform device
 */
static int gp329xxb_udc_probe(struct platform_device *pdev)
{
	struct gp329xxb_udc *udc = &memory;
	struct device *dev = &pdev->dev;
	int retval;

    //printk("got and enabled clocks\n");
	spin_lock_init(&udc->lock);

	rsrc_start = 0x93006000;
	rsrc_len   = 0x1000;

	base_addr = ioremap(rsrc_start, rsrc_len);
	if(!base_addr)
	{
		retval = -ENOMEM;
		goto err_mem;
	}

	device_initialize(&udc->gadget.dev);
	udc->gadget.dev.parent = &pdev->dev;
	udc->gadget.dev.dma_mask = pdev->dev.dma_mask;

	the_controller = udc;
	platform_set_drvdata(pdev, udc);

	gp329xxb_udc_init_data(udc);

	dev_dbg(dev, "probe ok\n");
	
	return 0;

err_mem:
	release_mem_region(rsrc_start, rsrc_len);

	return retval;
}

/*
 *	gp329xxb_udc_remove
 */
static int gp329xxb_udc_remove(struct platform_device *pdev)
{
	struct gp329xxb_udc *udc = platform_get_drvdata(pdev);
	//unsigned int irq;
	dev_dbg(&pdev->dev, "%s()\n", __func__);
	if (udc->driver)
		return -EBUSY;

	iounmap(base_addr);
	release_mem_region(rsrc_start, rsrc_len);

	platform_set_drvdata(pdev, NULL);

	dev_dbg(&pdev->dev, "%s: remove ok\n", __func__);
	return 0;
}

static int gp329xxb_udc_suspend(struct platform_device *pdev, pm_message_t message)
{
	return 0;
}

static int gp329xxb_udc_resume(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver udc_driver_gp329xxb =
{
	.driver		= 
	{
		.name	= SPMP_UDC_NAME,
		.owner	= THIS_MODULE,
	},
	.probe		= gp329xxb_udc_probe,
	.remove		= gp329xxb_udc_remove,
	.suspend	= gp329xxb_udc_suspend,
	.resume		= gp329xxb_udc_resume,
};

static int __init gp_udc_init(void)
{
	int retval;
	
	retval = platform_driver_register(&udc_driver_gp329xxb);
	if (retval)
		goto err;
	
	printk("gp_udc_init\n");
	return 0;

err:
	printk(KERN_ERR "udc_int error!\n");
	return retval;
}

static void __exit gp_udc_exit(void)
{
	platform_driver_unregister(&udc_driver_gp329xxb);
}

EXPORT_SYMBOL(gp_usb_gadget_unregister_driver);
EXPORT_SYMBOL(gp_usb_gadget_register_driver);

module_init(gp_udc_init);
module_exit(gp_udc_exit);

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:gp329xxb-usbgadget");
MODULE_ALIAS("platform:gp329xxb-usbgadget");
