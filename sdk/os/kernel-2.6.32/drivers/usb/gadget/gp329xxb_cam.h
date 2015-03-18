#ifndef _GP_CAM_H
#define _GP_CAM_H

#define DRIVER_DESC		"USB CAM Device"
#define DRIVER_NAME		"Generic UVC Device"
#define DRIVER_VERSION	"12 MAY 2014"
#define DRIVER_AUTHOR	"eugenehsu<eugenehsu@generalplus.com>" 

#define WIDTH_640	640
#define WIDTH_1024	1024
#define WIDTH_1280	1280
#define WIDTH_1920	1920

#define HEIGHT_480	480
#define HEIGHT_768	768
#define HEIGHT_720	720
#define HEIGHT_1080	1080

/* Module initialization parameters */
static struct
{
	unsigned int	width;
	unsigned int	height;
} 

mod_data =	// Default values
{
	.width			= WIDTH_640,
	.height			= HEIGHT_480,
};

/* Big enough to hold our biggest descriptor */
#define EP0_BUFSIZE	512
#define DELAYED_STATUS	(EP0_BUFSIZE + 999)	// An impossibly large value

struct cam_dev
{
	/* lock protects: state, all the req_busy's, and cbbuf_cmnd */
	spinlock_t			lock;
	struct usb_gadget	*gadget;

	/* reference counting: wait until all LUNs are released */
	struct kref			ref;

	struct usb_ep		*ep0;		// Handy copy of gadget->ep0
	struct usb_request	*ep0req;	// For control responses
	unsigned int		ep0_req_tag;
	const char			*ep0req_name;
	
	u32					state;

	struct usb_ctrlrequest *ctrl;

	/* ISO Video IN EP */
	struct usb_ep		*iso_video_in_ep;
	struct usb_request	*iso_video_req;	// For ISO Video IN request
	
	/* ISO audio IN EP */
	struct usb_ep		*iso_audio_in_ep;
	struct usb_request	*iso_audio_req;	// For ISO audio IN request
};

extern int gp_usb_gadget_unregister_driver(struct usb_gadget_driver *driver);
extern int gp_usb_gadget_register_driver(struct usb_gadget_driver *driver);
extern void gp_usb_set_udc_type(u32 type);
extern void gp_register_uvc_handler(gp_uvc_control_t* control);
#endif