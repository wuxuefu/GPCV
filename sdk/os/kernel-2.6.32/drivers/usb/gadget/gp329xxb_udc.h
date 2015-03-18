#ifndef _GP329xxB_UDC_H
#define _GP329xxB_UDC_H

/* Warning : ep0 has a fifo of 64 bytes */
#define EP0_FIFO_SIZE		 64    // control Endpoint
#define EP0_FIFO_SIZE_MASK	 0x3f	
#define EP12_FIFO_SIZE		 512   

#define EP12_FIFO_SIZE64		64    //full speed
#define EP12_FIFO_SIZE512		512   //high speed
#define EP3_FIFO_SIZE			64    //interrupt in Endpoint
#define EP5_FIFO_SIZE			1024  //iso video in Endpoint
#define EP7_FIFO_SIZE			256  //iso audio in Endpoint
#define GP329xxB_MAXENDPOINTS	5
#define DEFAULT_POWER_STATE		0x00

#define USBPHY_XTAL_ENABLE		(1 << 2)
#define USBPHY1_POWER_CTRL		(1 <<12)
#define	DMA_ADDR_INVALID		~((dma_addr_t)0)

/* For endpoint's PIO access */
#define VALID_BIT_4BYTE		0x0F
#define VALID_BIT_3BYTE		0x07
#define VALID_BIT_2BYTE		0x03
#define VALID_BIT_1BYTE		0x01
#define VALID_BIT_0BYTE		0x00

/* Added for detecting VBUS */
#define USB_TIMER_DIV	100			/* 1/USB_TIMER_DIV second */
#define GPIO_DET_DEBOUNCE	5	

/* For checking EP5/EP7 done timer */
#define CHECK_DONE_COUNT	100000
#define EP5_CHECK_TIMEOUT	10	/* 10 * 100us */
#define EP7_CHECK_TIMEOUT	15	/* 10 * 100us */

struct gp329xxb_request
{
	struct list_head		queue;		/* ep's requests */
	struct usb_request		req;
};

struct gp329xxb_ep
{
	struct list_head					queue;
	unsigned long						last_io;	/* jiffies timestamp */
	struct usb_gadget					*gadget;
	struct gp329xxb_udc					*udc;
	const struct usb_endpoint_descriptor *desc;
	struct usb_ep		ep;
	u32					num;
	u32					bEndpointAddress;
	u32					bmAttributes;
	u32					fifo_size;
};

struct gp329xxb_udc
{
	spinlock_t					lock;
	struct gp329xxb_ep			ep[GP329xxB_MAXENDPOINTS];
	struct gp329xxb_request*	ep_req[GP329xxB_MAXENDPOINTS];
	struct usb_gadget			gadget;
	struct usb_gadget_driver	*driver;
	struct usb_ctrlrequest 		ctlreq;
	struct timer_list			usb_gpio_timer;
	int 			usb_gpio_cnt;
	int				address;
	int				ep0datastatus;
	u16 			ep0sendnull;
};

enum ep0_data_status
{
	EP0_DATA_NONE,		/* nothing to do */
	EP0_DATA_IN,		/* Data IN */
	EP0_DATA_OUT,		/* Data OUT */
};

enum setup_cmd_src
{
	SRC_STD_SETUP,
	SRC_SET_CONFIG,
};

enum ep_num_index
{
	ep_num_0 = 0,	/* 0 */
	ep_num_1,		/* 1 */
	ep_num_2,		/* 2 */
	ep_num_5,		/* 3 */
	ep_num_7,		/* 4 */
};

enum udc_init_type
{
	UDC_MSDC_TYPE,
	UDC_UVC_TYPE,
	UDC_NONE_TYPE
};	

#endif
