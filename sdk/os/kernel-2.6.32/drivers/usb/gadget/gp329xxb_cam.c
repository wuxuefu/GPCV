/***************************************************************************
* linux/drivers/usb/gadget/gp_cam.c
*
* GENERALPLUS GPL329xx series on-chip high speed USB CAM driver
*
* Copyright (C) 2014 GENERALPLUS
*
*	Author: Eugene Hsu
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
****************************************************************************/
#include <linux/blkdev.h>
#include <linux/completion.h>
#include <linux/dcache.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/fcntl.h>
#include <linux/file.h>
#include <linux/kref.h>
#include <linux/kthread.h>
#include <linux/limits.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include <linux/freezer.h>
#include <linux/utsname.h>
#include <linux/random.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <linux/videodev2.h>
#include <mach/gp_cache.h>
#include <mach/hardware.h>
#include <mach/general.h>
#include <mach/gp_usb.h>
#include <mach/gp_board.h>
#include "gadget_chips.h"
#include "usbstring.c"
#include "config.c"
#define NOT_EXPORT_USB_EP_AUTOCONFIG
#include "epautoconf.c"
#include "gp329xxb_udc.h"
#include "gp329xxb_cam.h"
#include <mach/gp_cdsp.h>

MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_LICENSE("Dual BSD/GPL");

module_param_named(width, mod_data.width, uint, S_IRUGO);
MODULE_PARM_DESC(width, "width");

module_param_named(height, mod_data.height, uint, S_IRUGO);
MODULE_PARM_DESC(height, "height");

struct gp_cam_adj_preference {
	struct work_struct work;	/** working queue*/
	int cmd;
	int value;
	int done;
};

struct gp_cam_adj_preference adj_prefer;
/* Global Variable and structure definition */
static struct cam_dev				*the_cam;
static struct usb_gadget_driver		cam_driver;

gp_cdsp_user_preference_t user_preference;
gpCdspSatHue_t sat_hue;
gpCdspEdge_t edge;

static int cam_get_uvc_state(void)
{
	if(the_cam != NULL)
	{
		//printk("cam_get_uvc_state = 0x%x\n", the_cam->state);	
		return the_cam->state;
	}	
	else
	{
		printk("cam gadget does not exist in %s\n", __func__);
		return -1;	
	}	
}	

static int cam_send_video_frame(void* buf, unsigned long len)
{
	int	rc;
	
	if(the_cam == NULL)
	{	
		printk("cam gadget does not exit in %s\n", __func__);
		return -1;
	}	
	
	the_cam->state = USB_UVC_SENDING_V_STATE;
	the_cam->iso_video_req->length = len;
	the_cam->iso_video_req->buf = buf;
	rc = usb_ep_queue(the_cam->iso_video_in_ep, the_cam->iso_video_req, GFP_ATOMIC);
	if (rc != 0 && rc != -ESHUTDOWN)
	{
		/* We can't do much more than wait for a reset */
		printk("error in submission: video ep --> %d\n", rc);
	}
	
	return rc;	
}

static int cam_send_aduio_data(void* buf, unsigned long len)
{
	int	rc;
	
	if(the_cam == NULL)
	{	
		printk("cam gadget does not exit in %s\n", __func__);
		return -1;
	}	
	
	the_cam->state = USB_UVC_SENDING_A_STATE;
	the_cam->iso_audio_req->length = len;
	the_cam->iso_audio_req->buf = buf;

	rc = usb_ep_queue(the_cam->iso_audio_in_ep, the_cam->iso_audio_req, GFP_ATOMIC);
	if (rc != 0 && rc != -ESHUTDOWN)
	{
		/* We can't do much more than wait for a reset */
		printk("error in submission: audio ep --> %d\n", rc);
	}
	
	return rc;	
}

static gp_uvc_control_t cam_uvc_handle = 
{
	.get_uvc_state = cam_get_uvc_state,
	.send_video_frame = cam_send_video_frame,
	.send_audio_data = cam_send_aduio_data
};

static int cam_set_halt(struct cam_dev *cam, struct usb_ep *ep)
{
	const char	*name;

	if (ep == cam->iso_video_in_ep)
		name = "isov-in";
	else if (ep == cam->iso_audio_in_ep)
		name = "isoa-in";	
	else
		name = ep->name;
	return usb_ep_set_halt(ep);
}

/* hs_iso_video_in_desc is used for search the ISO IN EP in epautoconf.c */
static struct usb_endpoint_descriptor hs_iso_video_in_desc =
{
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
    
	.bEndpointAddress =	USB_DIR_IN | 0x05,
	.bmAttributes =		USB_ENDPOINT_XFER_ISOC,
	/* wMaxPacketSize set by autoconfiguration */
};

/* hs_iso_audio_in_desc is used for search the ISO IN EP in epautoconf.c */
static struct usb_endpoint_descriptor hs_iso_audio_in_desc =
{
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,
    
	.bEndpointAddress =	USB_DIR_IN | 0x07,
	.bmAttributes =		USB_ENDPOINT_XFER_ISOC,
	/* wMaxPacketSize set by autoconfiguration */
};

#define USBSCSI_INQ_PRODUCT_TXT_LEN  16

static char 	longname[USBSCSI_INQ_PRODUCT_TXT_LEN]={0};
static char 	shortname[] = DRIVER_NAME;


/* Include GP's UVC descriptors */
#include "gp_cam_des.c"

/* get_config_buf(): To get the UVC configuration descriptor data */	
static int get_config_buf(struct usb_gadget *gadget, u8 *buf, u8 type, unsigned index)
{
	int	len;

	if (index > 0)
		return -EINVAL;

	len = USB_UVC_ConfigDescriptor[2] + (USB_UVC_ConfigDescriptor[3] << 8);
	if(type == USB_DT_OTHER_SPEED_CONFIG)
	{
		/* Other Speed configuration descriptor */
		USB_UVC_ConfigDescriptor[1] = USB_DT_OTHER_SPEED_CONFIG;
	}	
	else if(type == USB_DT_CONFIG)
	{
		/* Current Speed configuration descriptor */
		USB_UVC_ConfigDescriptor[1] = USB_DT_CONFIG;
	}	
	
	memcpy(buf, &USB_UVC_ConfigDescriptor, len);
	
	/* Then return the configuration descriptor */
	return len;
}

static int get_string_buf(struct usb_gadget *gadget, u8 *buf, u8 type, unsigned index)
{
	int	len;

	switch(index)
	{
		case 0:
			len = sizeof UVC_String0_Descriptor;
			memcpy(buf, &UVC_String0_Descriptor, len);
			break;
		
		case 1:
			len = sizeof UVC_String1_Descriptor;
			memcpy(buf, &UVC_String1_Descriptor, len);
			break;
			
		case 2:
			len = sizeof UVC_String2_Descriptor;
			memcpy(buf, &UVC_String2_Descriptor, len);
			break;
		
		case 3:
			len = sizeof UVC_String3_Descriptor;
			memcpy(buf, &UVC_String3_Descriptor, len);
			break;			
	
		case 4:
			len = sizeof UVC_String4_Descriptor;
			memcpy(buf, &UVC_String4_Descriptor, len);
			break;
			
		case 5:
			len = sizeof UVC_String5_Descriptor;
			memcpy(buf, &UVC_String5_Descriptor, len);
			break;		
		default:
			len = 0;
			printk("%s : error index %d in get string descriptor command\n", __func__, index);
			break;
	}	
	
	return len;
}

/******************************************************
    UVC API functions
******************************************************/
static void UVC_set_default_parameters(void)
{

}

static int UVC_GetCur(const struct usb_ctrlrequest *ctrl, u8 *buf, int* len)
{
	u8 indexL = le16_to_cpu(ctrl->wIndex) & 0x00FF;
	u8 indexH = (le16_to_cpu(ctrl->wIndex) & 0xFF00) >> 8;
	u8 valueH = (le16_to_cpu(ctrl->wValue) & 0xFF00) >> 8;
	u16 data;
	struct v4l2_control ctl;

	switch(indexL)
	{
		case USB_UVC_VCIF_NUM:
			switch(indexH)
			{
				case 0:
					switch(valueH)
					{
						case VC_VIDEO_POWER_MODE_CONTROL:
							*len = 1;
							memcpy(buf, &UVC_PowerMode, *len);
							break;
						case VC_REQUEST_ERROR_CODE_CONTROL:
							*len = 1;
							memcpy(buf, &UVC_ErrorCode, *len);		
							UVC_ErrorCode = NO_ERROR_ERR;
							break;	
						default:
							UVC_ErrorCode = INVALID_CONTROL_ERR;
							break;
					}	
					break;
				
				case 1:
					switch (valueH)
					{
            			default:
 			            	UVC_ErrorCode = INVALID_CONTROL_ERR;
            			break;
         			}
					break;
				
				case 2:
					switch (valueH)
					{
            			default:
 			            	UVC_ErrorCode = INVALID_CONTROL_ERR;
            			break;
         			}
					break;
				case 3:
					switch (valueH)
					{
            			default:
 			            	UVC_ErrorCode = INVALID_CONTROL_ERR;
            			break;
         			}
					break;		
				
				case 4:
					switch(valueH)
					{
						case SU_INPUT_SELECT_CONTROL:
							*len = 1;
							memcpy(buf, &UVC_suSelectedInputPin, *len);		
							UVC_ErrorCode = NO_ERROR_ERR;
							break;
						default:
							UVC_ErrorCode = INVALID_CONTROL_ERR;
							break;
					}	
					break;
				
				case 5:    // ProcessingUnit
					switch (valueH)
					{
						case PU_BRIGHTNESS_CONTROL:
							ctl.id = MSG_CDSP_TARGET_AE;
							ctl.value = (s32)&(user_preference.ae_target);
							gp_cdsp_g_ctrl(&ctl);
					//		data = cpu_to_le16(UVC_puBrightnessCur);
							*len = 2;
							memcpy(buf, &(user_preference.ae_target), *len);	
							UVC_ErrorCode = NO_ERROR_ERR;
							break;
						case PU_HUE_CONTROL:
							ctl.id = MSG_CDSP_SAT_HUE_DAY;
							ctl.value = (s32)&sat_hue;
							gp_cdsp_g_ctrl(&ctl);
							user_preference.u_offset_day = sat_hue.u_offset;
							user_preference.v_offset_day = sat_hue.v_offset;
					//		data = cpu_to_le16(UVC_puHueCur);
							*len = 2;
							memcpy(buf, &user_preference.u_offset_day, *len);
							UVC_ErrorCode = NO_ERROR_ERR;
							break;
						case PU_CONTRAST_CONTROL:
							ctl.id = MSG_CDSP_SAT_HUE_DAY;
							ctl.value = (s32)&sat_hue;
							gp_cdsp_g_ctrl(&ctl);
							user_preference.y_scale_day = sat_hue.y_scale;
							user_preference.y_offset_day = sat_hue.y_offset;
					//		data = cpu_to_le16(UVC_puContrastCur);
							*len = 2;
							memcpy(buf, &user_preference.y_scale_day, *len);
							UVC_ErrorCode = NO_ERROR_ERR;	
							break;
						case PU_SATURATION_CONTROL:
							ctl.id = MSG_CDSP_SAT_HUE_DAY;
							ctl.value = (s32)&sat_hue;
							gp_cdsp_g_ctrl(&ctl);
							user_preference.u_scale_day = sat_hue.u_scale;
							user_preference.y_scale_day = sat_hue.y_scale;
					//		data = cpu_to_le16(UVC_puSaturationCur);
							*len = 2;
							memcpy(buf, &user_preference.u_scale_day, *len);
							UVC_ErrorCode = NO_ERROR_ERR;
							break;
						case PU_SHARPNESS_CONTROL:
							ctl.id = MSG_CDSP_EDGE;
							ctl.value = (s32)&edge;
							gp_cdsp_g_ctrl(&ctl);
							user_preference.edge_day = edge.ampga + edge.lhmode * 4;
					//		data = cpu_to_le16(UVC_puSharpnessCur);
							*len = 2;
							memcpy(buf, &user_preference.edge_day, *len);
							UVC_ErrorCode = NO_ERROR_ERR;	
							break;
						case PU_BACKLIGHT_COMPENSATION_CONTROL:
							ctl.id = V4L2_CID_BRIGHTNESS;
							gp_cdsp_g_ctrl(&ctl);
							user_preference.max_lum = ctl.value;
					//		data = cpu_to_le16(UVC_puBacklightCur);
							*len = 2;
							memcpy(buf, &user_preference.max_lum, *len);
							UVC_ErrorCode = NO_ERROR_ERR;	
							break;
						case PU_GAMMA_CONTROL:
							ctl.id = MSG_CDSP_WB_OFFSET_DAY;
							ctl.value = (s32)&user_preference.wb_offset_day;
							gp_cdsp_g_ctrl(&ctl);
							data = user_preference.wb_offset_day + 10;
					//		data = cpu_to_le16(UVC_puGammaCur);
							*len = 2;
							memcpy(buf, &data, *len);
							UVC_ErrorCode = NO_ERROR_ERR;	
							break;		
						default:
							UVC_ErrorCode = INVALID_CONTROL_ERR;
						break;
					}
				break;
					
				default:
          			UVC_ErrorCode = INVALID_UNIT_ERR;
          			break;
			}	
			break;	//USB_UVC_VCIF_NUM
		
		case USB_UVC_VSIF_NUM:
			switch (valueH)
			{
            	case VS_PROBE_CONTROL :
            		*len = sizeof(UVC_VIDEO_PROBE_AND_COMMIT_CONTROL);
            		memcpy(buf, &UVC_vsVideoProbe, *len);	
              		UVC_ErrorCode = NO_ERROR_ERR;
              		break;
              	case VS_COMMIT_CONTROL:	
              		*len = sizeof(UVC_VIDEO_PROBE_AND_COMMIT_CONTROL);
            		memcpy(buf, &UVC_vsVideoCommit, *len);	
              		UVC_ErrorCode = NO_ERROR_ERR;
              		break;
            	default:
              		UVC_ErrorCode = INVALID_CONTROL_ERR;
              		break;
          	}
			break;	
			
		default:
      		UVC_ErrorCode = UNKNOWN_ERR;
      		break;
	}	

	if (UVC_ErrorCode == NO_ERROR_ERR)
    	return 1;                                         // we handled the request
  	else
    	return 0; 											
}

static int UVC_GetDef(const struct usb_ctrlrequest *ctrl, u8 *buf, int* len)
{
	u8 indexL = le16_to_cpu(ctrl->wIndex) & 0x00FF;
	u8 indexH = (le16_to_cpu(ctrl->wIndex) & 0xFF00) >> 8;
	u8 valueH = (le16_to_cpu(ctrl->wValue) & 0xFF00) >> 8;
	u16 data;
	
	switch(indexL)
	{
		case USB_UVC_VCIF_NUM:
			switch(indexH)
			{
				case 1:
					switch(valueH)
					{
						default:
              				UVC_ErrorCode = INVALID_CONTROL_ERR;
              				break;
					}	
					break;
				
				case 2:
					switch(valueH)
					{
            			default:
              				UVC_ErrorCode = INVALID_CONTROL_ERR;
 				            break;
          			}
					break;
				
				case 3:
					switch(valueH)
					{
            			default:
             				UVC_ErrorCode = INVALID_CONTROL_ERR;
             			break;
          			}
					break;
				
				case 4:
					switch(valueH)
					{
            			case SU_INPUT_SELECT_CONTROL:
            			*len = 1;
            			memcpy(buf, &UVC_suSelectedInputPin, *len);
      			        UVC_ErrorCode = NO_ERROR_ERR;
             		    break;
          			} 
					break;
				case 5:
					 switch(valueH)
					 {
            			case PU_BRIGHTNESS_CONTROL:
            				*len = 2;
            				data = cpu_to_le16(UVC_puBrightnessDef);
            				memcpy(buf, &data, *len);
              				UVC_ErrorCode = NO_ERROR_ERR;
              				break;
              			case PU_HUE_CONTROL:
      			        	*len = 2;
            				data = cpu_to_le16(UVC_puHueDef);
            				memcpy(buf, &data, *len);
              				UVC_ErrorCode = NO_ERROR_ERR;
              				break;
              			case PU_CONTRAST_CONTROL:
      			        	*len = 2;
            				data = cpu_to_le16(UVC_puContrastDef);
            				memcpy(buf, &data, *len);
              				UVC_ErrorCode = NO_ERROR_ERR;	
              				break;	
              			case PU_SATURATION_CONTROL:
      			        	*len = 2;
            				data = cpu_to_le16(UVC_puSaturationDef);
            				memcpy(buf, &data, *len);
              				UVC_ErrorCode = NO_ERROR_ERR;	
              				break;
              			case PU_SHARPNESS_CONTROL:
      			        	*len = 2;
            				data = cpu_to_le16(UVC_puSharpnessDef);
            				memcpy(buf, &data, *len);
              				UVC_ErrorCode = NO_ERROR_ERR;		
              				break;
              			case PU_BACKLIGHT_COMPENSATION_CONTROL:
      			        	*len = 2;
            				data = cpu_to_le16(UVC_puBacklightDef);
            				memcpy(buf, &data, *len);
              				UVC_ErrorCode = NO_ERROR_ERR;		
              				break;
              			case PU_GAMMA_CONTROL:
      			        	*len = 2;
            				data = cpu_to_le16(UVC_puGammaDef);
            				memcpy(buf, &data, *len);
              				UVC_ErrorCode = NO_ERROR_ERR;		
              				break;	
          			}
					break;
				default:
          			UVC_ErrorCode = INVALID_UNIT_ERR;
          			break;			
			}	
			break;
		
		case USB_UVC_VSIF_NUM:
        	switch (valueH)
        	{ 
            	default:
            		UVC_ErrorCode = INVALID_CONTROL_ERR;
              		break;
          	}
		break;
		
		default:
			UVC_ErrorCode = UNKNOWN_ERR;
			break;
	}	

	if (UVC_ErrorCode == NO_ERROR_ERR)
    	return 1;                                         // we handled the request
  	else
    	return 0;                                        // if not handled we stall it
} 

static int UVC_GetInfo(const struct usb_ctrlrequest *ctrl, u8 *buf, int* len)
{
	u8 indexL = le16_to_cpu(ctrl->wIndex) & 0x00FF;
	u8 indexH = (le16_to_cpu(ctrl->wIndex) & 0xFF00) >> 8;
	u8 valueH = (le16_to_cpu(ctrl->wValue) & 0xFF00) >> 8;
	u8 data1;
	
	switch(indexL)
	{
		case USB_UVC_VCIF_NUM:
			switch (indexH)
			{
				case 0:
					switch(valueH)
					{
						case VC_VIDEO_POWER_MODE_CONTROL:
							*len = 1;
							data1 = (SUPPORTS_GET | SUPPORTS_SET);
							memcpy(buf, &data1, *len);
							UVC_ErrorCode = NO_ERROR_ERR;
							break;
						
						case VC_REQUEST_ERROR_CODE_CONTROL:
							*len = 1;
							data1 = (SUPPORTS_GET);
							memcpy(buf, &data1, *len);
              				UVC_ErrorCode = NO_ERROR_ERR;
							break;
					}	
					break;
				
				case 1:
					switch(valueH)
					{          
            			default:
              				UVC_ErrorCode = INVALID_CONTROL_ERR;
             				 break;
          			}
					break;
				
				case 2:
					switch(valueH)
					{
            			default:
              				UVC_ErrorCode = INVALID_CONTROL_ERR;
              				break;
          			}
					break;
				
				case 3:
					switch(valueH)
					{
            			default:
              				UVC_ErrorCode = INVALID_CONTROL_ERR;
						    break;
          			}
					break;
					
				case 4:
					switch(valueH)
					{
            			case SU_INPUT_SELECT_CONTROL:
            			*len = 1;
						data1 = (SUPPORTS_GET | SUPPORTS_SET);
              			memcpy(buf, &data1, *len);
              			UVC_ErrorCode = NO_ERROR_ERR;
              			break;
          			}
					break;
				
				case 5:
					switch (valueH)
					{
            			case PU_BRIGHTNESS_CONTROL:
            			case PU_HUE_CONTROL:
              			case PU_CONTRAST_CONTROL:
              			case PU_SATURATION_CONTROL:
              			case PU_SHARPNESS_CONTROL:
              			case PU_BACKLIGHT_COMPENSATION_CONTROL:
              			case PU_GAMMA_CONTROL:
            				*len = 1;
            				data1 = (SUPPORTS_GET | SUPPORTS_SET);
              				memcpy(buf, &data1, *len);		
              				UVC_ErrorCode = NO_ERROR_ERR;
              				break;
          			}
					break;
					
				default:
          			UVC_ErrorCode = INVALID_UNIT_ERR;
			        break;		
			}	
			break;	//USB_UVC_VCIF_NUM
			
		case USB_UVC_VSIF_NUM:
			switch(valueH)
			{
            	default:
            		UVC_ErrorCode = INVALID_CONTROL_ERR;
            		break;
          	}
			break;
		
		default:
			UVC_ErrorCode = UNKNOWN_ERR;
			break;
	}	
	
	if (UVC_ErrorCode == NO_ERROR_ERR)
    	return 1;                                         // we handled the request
  	else
    	return 0;                                        // if not handled we stall it
}

static int UVC_GetLen(const struct usb_ctrlrequest *ctrl, u8 *buf, int* len)
{
	u8 indexL = le16_to_cpu(ctrl->wIndex) & 0x00FF;
	u8 indexH = (le16_to_cpu(ctrl->wIndex) & 0xFF00) >> 8;
	u8 valueH = (le16_to_cpu(ctrl->wValue) & 0xFF00) >> 8;
	
	switch(indexL)
	{
		case USB_UVC_VCIF_NUM:
			switch(indexH)
			{
				case 5:
					switch(valueH)
					{
						case PU_BRIGHTNESS_CONTROL:
							UVC_ErrorCode = INVALID_REQUEST_ERR;
							break;
						default:
							UVC_ErrorCode = INVALID_CONTROL_ERR;
							break;	
					}	
					break;
				
				default:
					UVC_ErrorCode = INVALID_UNIT_ERR;
					break;	
			}	
			break;
			
		case USB_UVC_VSIF_NUM:
			switch(valueH)
			{
            	default:
            		UVC_ErrorCode = INVALID_CONTROL_ERR;
              		break;
          	}
			break;
				
		default:
			UVC_ErrorCode = UNKNOWN_ERR;
			break;	
	}	
	
	if (UVC_ErrorCode == NO_ERROR_ERR)
    	return 1;                                         // we handled the request
  	else
    	return 0;                                        // if not handled we stall it
}

static int UVC_GetMax(const struct usb_ctrlrequest *ctrl, u8 *buf, int* len)
{
	u8 indexL = le16_to_cpu(ctrl->wIndex) & 0x00FF;
	u8 indexH = (le16_to_cpu(ctrl->wIndex) & 0xFF00) >> 8;
	u8 valueH = (le16_to_cpu(ctrl->wValue) & 0xFF00) >> 8;
	u16 data;
	
	switch(indexL)
	{
		case USB_UVC_VCIF_NUM:
			switch(indexH)
			{
				case 1:
					switch(valueH)
					{
            			default:                            
              				UVC_ErrorCode = INVALID_CONTROL_ERR;
              				break;
          			}
					break;
				
				case 2:
					switch(valueH)
					{
            			default:
              				UVC_ErrorCode = INVALID_CONTROL_ERR;
              				break;
          			}
					break;	
				
				case 3:
					switch(valueH)
					{
            			default:
              				UVC_ErrorCode = INVALID_CONTROL_ERR;
              				break;
          			}
					break;
				
				case 4:
					switch(valueH)
					{
            			case SU_INPUT_SELECT_CONTROL:
            				*len = 1;
            				memcpy(buf, &UVC_suSelectedInputPin, *len);
              				UVC_ErrorCode = NO_ERROR_ERR;
              				break;
            			default:
              				UVC_ErrorCode = INVALID_CONTROL_ERR;
              				break;
          			}
					break;
				
				case 5:
					switch(valueH)
					{
            			case PU_BRIGHTNESS_CONTROL:
            				*len = 2;
            				data = cpu_to_le16(UVC_puBrightnessMax);
            				memcpy(buf, &data, *len);
              				UVC_ErrorCode = NO_ERROR_ERR;
              				break;
              			case PU_HUE_CONTROL:
              				*len = 2;
              				data = cpu_to_le16(UVC_puHueMax);
            				memcpy(buf, &data, *len);
              				UVC_ErrorCode = NO_ERROR_ERR;
              				break;
              			case PU_CONTRAST_CONTROL:
              				*len = 2;
              				data = cpu_to_le16(UVC_puContrastMax);
            				memcpy(buf, &data, *len);
              				UVC_ErrorCode = NO_ERROR_ERR;
              				break;	
              			case PU_SATURATION_CONTROL:
              				*len = 2;
              				data = cpu_to_le16(UVC_puSaturationMax);
            				memcpy(buf, &data, *len);
              				UVC_ErrorCode = NO_ERROR_ERR;
              				break;		
              			case PU_SHARPNESS_CONTROL:
              				*len = 2;
              				data = cpu_to_le16(UVC_puSharpnessMax);
            				memcpy(buf, &data, *len);
              				UVC_ErrorCode = NO_ERROR_ERR;	
              				break;
              			case PU_BACKLIGHT_COMPENSATION_CONTROL:
              				*len = 2;
              				data = cpu_to_le16(UVC_puBacklightMax);
            				memcpy(buf, &data, *len);
              				UVC_ErrorCode = NO_ERROR_ERR;	
              				break;
              			case PU_GAMMA_CONTROL:
              				*len = 2;
              				data = cpu_to_le16(UVC_puGammaMax);
            				memcpy(buf, &data, *len);
              				UVC_ErrorCode = NO_ERROR_ERR;	
              				break;	
            			default:
              				UVC_ErrorCode = INVALID_CONTROL_ERR;
              				break;
          			}
					break;
				
				default:
					UVC_ErrorCode = INVALID_UNIT_ERR;
					break;		
			}	
			break;
		
		case USB_UVC_VSIF_NUM:
			switch(valueH)
			{
            	default:
            		UVC_ErrorCode = INVALID_CONTROL_ERR;
            		break;
          	}
			break;
				
		default:
			UVC_ErrorCode = UNKNOWN_ERR;
			break;
	}	

	if (UVC_ErrorCode == NO_ERROR_ERR)
    	return 1;                                         // we handled the request
  	else
    	return 0;                                        // if not handled we stall it
}

static int UVC_GetMin(const struct usb_ctrlrequest *ctrl, u8 *buf, int* len)
{
	u8 indexL = le16_to_cpu(ctrl->wIndex) & 0x00FF;
	u8 indexH = (le16_to_cpu(ctrl->wIndex) & 0xFF00) >> 8;
	u8 valueH = (le16_to_cpu(ctrl->wValue) & 0xFF00) >> 8;
	u16 data;
	
	switch(indexL)
	{
		case USB_UVC_VCIF_NUM:
			switch(indexH)
			{
				case 1:
					switch(valueH)
					{
            			default:
              				UVC_ErrorCode = INVALID_CONTROL_ERR;
              				break;
          			}
					break;
				
				case 2:
					switch(valueH)
					{
            			default:
              				UVC_ErrorCode = INVALID_CONTROL_ERR;
              				break;
          			}
					break;
				
				case 3:
					switch(valueH)
					{
            			default:
              				UVC_ErrorCode = INVALID_CONTROL_ERR;
              				break;
          			}
					break;
				
				case 4:
					switch (valueH)
					{
            			case SU_INPUT_SELECT_CONTROL:
            				*len = 1;
            				memcpy(buf, &UVC_suSelectedInputPin, *len);
              				UVC_ErrorCode = NO_ERROR_ERR;
              				break;
            			default:
              				UVC_ErrorCode = INVALID_CONTROL_ERR;
              				break;
          			}
					break;	
				
				case 5:
					switch(valueH)
					{
		            	case PU_BRIGHTNESS_CONTROL:
		            		*len = 2;
		            		data = cpu_to_le16(UVC_puBrightnessMin);
		            		memcpy(buf, &data, *len);
		              		UVC_ErrorCode = NO_ERROR_ERR;
		              		break;
		              	case PU_HUE_CONTROL:
		              		*len = 2;
		            		data = cpu_to_le16(UVC_puHueMin);
		            		memcpy(buf, &data, *len);
		              		UVC_ErrorCode = NO_ERROR_ERR;
		              		break;	
		              	case PU_CONTRAST_CONTROL:
		              		*len = 2;
		            		data = cpu_to_le16(UVC_puContrastMin);
		            		memcpy(buf, &data, *len);
		              		UVC_ErrorCode = NO_ERROR_ERR;
		              		break;
		              	case PU_SATURATION_CONTROL:
		              		*len = 2;
		            		data = cpu_to_le16(UVC_puSaturationMin);
		            		memcpy(buf, &data, *len);
		              		UVC_ErrorCode = NO_ERROR_ERR;
		              		break;			
		              	case PU_SHARPNESS_CONTROL:
		              		*len = 2;
		            		data = cpu_to_le16(UVC_puSharpnessMin);
		            		memcpy(buf, &data, *len);
		              		UVC_ErrorCode = NO_ERROR_ERR;
		              		break;
		              	case PU_BACKLIGHT_COMPENSATION_CONTROL:
		              		*len = 2;
		            		data = cpu_to_le16(UVC_puBacklightMin);
		            		memcpy(buf, &data, *len);
		              		UVC_ErrorCode = NO_ERROR_ERR;
		              		break;
		              	case PU_GAMMA_CONTROL:
		              		*len = 2;
		            		data = cpu_to_le16(UVC_puGammaMin);
		            		memcpy(buf, &data, *len);
		              		UVC_ErrorCode = NO_ERROR_ERR;
		              		break;						
		            	default:
		              		UVC_ErrorCode = INVALID_CONTROL_ERR;
		              		break;
		          	}
					break;	
			}
			break;
		
		case USB_UVC_VSIF_NUM:
			switch(valueH)
			{
            	default:
            		UVC_ErrorCode = INVALID_CONTROL_ERR;
              		break;
        	}
			break;
				
		default:
			UVC_ErrorCode = UNKNOWN_ERR;
			break;
	}	

	if(UVC_ErrorCode == NO_ERROR_ERR)
    	return 1;                                         // we handled the request
  	else
    	return 0;                                        // if not handled we stall it
}

static int UVC_GetRes(const struct usb_ctrlrequest *ctrl, u8 *buf, int* len)
{
	u8 indexL = le16_to_cpu(ctrl->wIndex) & 0x00FF;
	u8 indexH = (le16_to_cpu(ctrl->wIndex) & 0xFF00) >> 8;
	u8 valueH = (le16_to_cpu(ctrl->wValue) & 0xFF00) >> 8;
	u16 data;

	switch(indexL)
	{
		case USB_UVC_VCIF_NUM:
			switch(indexH)
			{
				case 1:
					switch(valueH)
					{
            			default:
              				UVC_ErrorCode = INVALID_CONTROL_ERR;
              				break;
          			}
					break;
				
				case 2:
					switch(valueH)
					{
						default:
              				UVC_ErrorCode = INVALID_CONTROL_ERR;
              				break;
					}	
					break;
				
				case 3:
					switch(valueH)
					{
            			default:
              				UVC_ErrorCode = INVALID_CONTROL_ERR;
              				break;
          			}
					break;
	
				case 4:
					switch(valueH)
					{
            			case SU_INPUT_SELECT_CONTROL:
            				*len = 1;
            				memcpy(buf, &UVC_suSelectedInputPin, *len);
              				UVC_ErrorCode = NO_ERROR_ERR;
              				break;
            			default:
              				UVC_ErrorCode = INVALID_CONTROL_ERR;
              				break;
          			}
					break;
				
				case 5:
					switch(valueH)
					{
            			case PU_BRIGHTNESS_CONTROL:
            				*len = 2;
            				data = cpu_to_le16(UVC_puBrightnessRes);
		            		memcpy(buf, &data, *len);
		              		UVC_ErrorCode = NO_ERROR_ERR;
              				break;
              			case PU_HUE_CONTROL:
              				*len = 2;
            				data = cpu_to_le16(UVC_puHueRes);
		            		memcpy(buf, &data, *len);
		              		UVC_ErrorCode = NO_ERROR_ERR;
              				break;	
              			case PU_CONTRAST_CONTROL:
              				*len = 2;
            				data = cpu_to_le16(UVC_puContrastRes);
		            		memcpy(buf, &data, *len);
		              		UVC_ErrorCode = NO_ERROR_ERR;
              				break;		
              			case PU_SATURATION_CONTROL:
              				*len = 2;
            				data = cpu_to_le16(UVC_puSaturationRes);
		            		memcpy(buf, &data, *len);
		              		UVC_ErrorCode = NO_ERROR_ERR;
              				break;
              			case PU_SHARPNESS_CONTROL:
              				*len = 2;
            				data = cpu_to_le16(UVC_puSharpnessRes);
		            		memcpy(buf, &data, *len);
		              		UVC_ErrorCode = NO_ERROR_ERR;
              				break;
              			case PU_BACKLIGHT_COMPENSATION_CONTROL:
              				*len = 2;
            				data = cpu_to_le16(UVC_puBacklightRes);
		            		memcpy(buf, &data, *len);
		              		UVC_ErrorCode = NO_ERROR_ERR;
              				break;
              			case PU_GAMMA_CONTROL:
              				*len = 2;
            				data = cpu_to_le16(UVC_puGammaRes);
		            		memcpy(buf, &data, *len);
		              		UVC_ErrorCode = NO_ERROR_ERR;
              				break;							
 			        	default:
            				UVC_ErrorCode = INVALID_CONTROL_ERR;
              				break;
          			}
					break;
						
				default:
					UVC_ErrorCode = INVALID_UNIT_ERR;
					break;
			}	
			break;
		
		case USB_UVC_VSIF_NUM:
			switch(valueH)
			{
	        	default:
	        	    UVC_ErrorCode = INVALID_CONTROL_ERR;
	            	break;
        	}
			break;
		
		default:
			UVC_ErrorCode = UNKNOWN_ERR;
			break;		
	}	

	if (UVC_ErrorCode == NO_ERROR_ERR)
    	return 1;                                         // we handled the request
  	else
    	return 0;
}

static int get_uvc_req_buf(struct usb_gadget *gadget, u8 *buf, const struct usb_ctrlrequest *ctrl)
{
	int len = -1;
	u32 ret = 0;	
	
	
	if(ctrl->bRequestType & USB_DIR_IN)
	{
		switch(ctrl->bRequest)
		{
			/* For UVC device, support GET_CUR, GET_MIN, GET_MAX, GET_RES, GET_LEN, GET_INFO, GET_DEF class request*/
	        case GET_CUR:
	        	ret = UVC_GetCur(ctrl, buf, &len);
	        	break; 
	     
	        case GET_DEF:
	        	ret = UVC_GetDef(ctrl, buf, &len);
	        	break;
	           
	        case GET_INFO:
	        	ret = UVC_GetInfo(ctrl, buf, &len);
	        	break;	
	        
	        case GET_LEN:
	        	ret = UVC_GetLen(ctrl, buf, &len);
	        	break;
	      
	        case GET_MAX:
	        	ret = UVC_GetMax(ctrl, buf, &len);
	        	break;
	        	       
	        case GET_MIN:
	        	ret = UVC_GetMin(ctrl, buf, &len);
	        	break;
	        
	        case GET_RES:
	        	ret = UVC_GetRes(ctrl, buf, &len);
	        	break;					
	
	        default:
	        	ret = 0;
	            break;
		}
	}
	else
	{
		/* EP0 OUT */
		switch(ctrl->bRequest)
		{
			case SET_CUR:
				/* Get data cnt and enable EP0 OUT to get data */
				len = ctrl->wLength;
				/* enable EP0 OUT */
				//printk("ep0 out, len = %d\n", len);
				ret = 1;
				break;
			default:
	        	ret = 0;
	            break;
		}	
	}	
	
	/* if ret = 0 stall back to host */
	if(ret == 0)
	{
		len = -1;
	}	
	
	return len;
}	

static void gp_adj_prefer_work(struct work_struct *work)
{
	struct v4l2_control ctl;
	
	switch(adj_prefer.cmd)
	{
		case PU_BRIGHTNESS_CONTROL:
			user_preference.ae_target = adj_prefer.value;
			ctl.id = MSG_CDSP_TARGET_AE;
			ctl.value = (s32)&(user_preference.ae_target);
			gp_cdsp_s_ctrl(&ctl);	
		break;
		case PU_HUE_CONTROL:
			user_preference.v_offset_day -= (user_preference.u_offset_day - adj_prefer.value);
			if(user_preference.v_offset_day < UVC_puHueMin)
				user_preference.v_offset_day = UVC_puHueMin;
			if(user_preference.v_offset_day > UVC_puHueMax)
				user_preference.v_offset_day = UVC_puHueMax;
			user_preference.u_offset_day = adj_prefer.value;
			sat_hue.u_offset = user_preference.u_offset_day;
			sat_hue.v_offset = user_preference.v_offset_day;

			ctl.id = MSG_CDSP_SAT_HUE_DAY;
			ctl.value = (int)&sat_hue;
			gp_cdsp_s_ctrl(&ctl);
		break;
		case PU_CONTRAST_CONTROL:
			user_preference.y_offset_day += (user_preference.y_scale_day - adj_prefer.value) * -2;
			if(user_preference.y_offset_day > 127)
				user_preference.y_offset_day = 127;
			if(user_preference.y_offset_day < -128)
				user_preference.y_offset_day = -128;
			user_preference.y_scale_day = adj_prefer.value;
							
			sat_hue.y_scale = user_preference.y_scale_day;
			sat_hue.y_offset = user_preference.y_offset_day;
			ctl.id = MSG_CDSP_SAT_HUE_DAY;
			ctl.value = (int)&sat_hue;
			gp_cdsp_s_ctrl(&ctl);	
		break;
		case PU_SATURATION_CONTROL:
			sat_hue.u_scale = user_preference.u_scale_day = adj_prefer.value; // blud
			sat_hue.v_scale = user_preference.y_scale_day = adj_prefer.value; // red
			ctl.id = MSG_CDSP_SAT_HUE_DAY;
			ctl.value = (int)&sat_hue;
			gp_cdsp_s_ctrl(&ctl);	
		break;
		case PU_SHARPNESS_CONTROL:
			user_preference.edge_day = adj_prefer.value;
			edge.ampga = adj_prefer.value % 4;
			edge.lhmode = adj_prefer.value / 4;
			ctl.id = MSG_CDSP_EDGE;
			ctl.value = (s32)&edge;
			gp_cdsp_s_ctrl(&ctl);
		break;
		case PU_GAMMA_CONTROL:
			user_preference.wb_offset_day = adj_prefer.value - 10;
			ctl.id = MSG_CDSP_WB_OFFSET_DAY;
			ctl.value = (s32)&user_preference.wb_offset_day;
			gp_cdsp_s_ctrl(&ctl);
		break;
		case PU_BACKLIGHT_COMPENSATION_CONTROL:
			user_preference.max_lum = adj_prefer.value;
			ctl.id = V4L2_CID_BRIGHTNESS;
			ctl.value = user_preference.max_lum;
			gp_cdsp_s_ctrl(&ctl);
		break;
	}
	
	adj_prefer.done = 1;
}

static int UVC_SetCur(const struct usb_ctrlrequest *ctrl, u8 *buf, int len)
{
 	u8 indexL = le16_to_cpu(ctrl->wIndex) & 0x00FF;
	u8 indexH = (le16_to_cpu(ctrl->wIndex) & 0xFF00) >> 8;
	u8 valueH = (le16_to_cpu(ctrl->wValue) & 0xFF00) >> 8;
	s16 wTemp;
	//struct v4l2_control ctl;
	
	switch(indexL)
  	{                       // select the Interface
		case USB_UVC_VCIF_NUM:								// Video Control Interface
    	switch (indexH)
    	{												// select the Entity
	        case 0:										// interface control requests
	          switch(valueH)
	          {            								// select the  selector control
		      		case VC_VIDEO_POWER_MODE_CONTROL:
		            	UVC_PowerMode = buf[0];
		            	UVC_ErrorCode = NO_ERROR_ERR;
		            	break;
		            default:
		            	UVC_ErrorCode = INVALID_CONTROL_ERR;
		              	break;
	          }
	        	break;
			case 1:                                     // InputTerminal (camera)         see usbdesc.c
				switch(valueH)
	          	{               							// select the  selector control
	          		default:
	          			UVC_ErrorCode = INVALID_CONTROL_ERR;
	              		break;
	          	}
	       		break;
	        case 2:                                     // InputTerminal (composite)      see usbdesc.c
	        	switch(valueH)
	          	{											// select the  selector control
	          		default:
	            		UVC_ErrorCode = INVALID_CONTROL_ERR;
	            		break;
	          	}
	        	break;
	        case 3:                                     // OutputTerminal                 see usbdesc.c
				switch(valueH)
				{ 						              // select the  selector control
					default:
						UVC_ErrorCode = INVALID_CONTROL_ERR;
						break;
				}
				break;
	        case 4:                                   // SelectorUnit                   see usbdesc.c
				switch(valueH)
				{ 						              // select the  selector control
					case SU_INPUT_SELECT_CONTROL:
			  			UVC_suSelectedInputPin = buf[0];
			  			UVC_ErrorCode = NO_ERROR_ERR;
						  break;
					default:
						UVC_ErrorCode = INVALID_CONTROL_ERR;
					  	break;
				}
				break;
	        case 5:                                            // ProcessingUnit                 see usbdesc.c
				switch(valueH)
				{               // select the  selector control
	          		case PU_BRIGHTNESS_CONTROL:                    // only Brightness is supported   see usbdesc.c
		            	wTemp = buf[0] | (buf[1] << 8);
		              	if ((wTemp >= UVC_puBrightnessMin) &&        // check the value to set
		                	  (wTemp <= UVC_puBrightnessMax))
		          		{   
							if(adj_prefer.done) {
								adj_prefer.done = 0;
								adj_prefer.cmd = valueH;
								adj_prefer.value = wTemp;
								schedule_work(&adj_prefer.work);
								UVC_puBrightnessCur = wTemp; 
								UVC_ErrorCode = NO_ERROR_ERR;
							}
		              	}
		              	else
		              	{
		                	UVC_ErrorCode = OUT_OF_RANGE_ERR;
		              	}
		              	break;
		              	
		            case PU_HUE_CONTROL:                    // Hue is supported   see usbdesc.c
		            	wTemp = buf[0] | (buf[1] << 8);
		              	if ((wTemp >= UVC_puHueMin) &&        // check the value to set
		                	  (wTemp <= UVC_puHueMax))
		          		{  					
							if(adj_prefer.done) {
								adj_prefer.done = 0;
								adj_prefer.cmd = valueH;
								adj_prefer.value = wTemp;
								schedule_work(&adj_prefer.work);
								UVC_puHueCur = wTemp; 
								UVC_ErrorCode = NO_ERROR_ERR;
							}
		              	}
		              	else
		              	{
		                	UVC_ErrorCode = OUT_OF_RANGE_ERR;
		              	}
		              	break;
		              	
		            case PU_CONTRAST_CONTROL:                    // Contrast is supported   see usbdesc.c
		            	wTemp = buf[0] | (buf[1] << 8);
		              	if ((wTemp >= UVC_puContrastMin) &&        // check the value to set
		                	  (wTemp <= UVC_puContrastMax))
		          		{   
							if(adj_prefer.done) {
								adj_prefer.done = 0;
								adj_prefer.cmd = valueH;
								adj_prefer.value = wTemp;
								schedule_work(&adj_prefer.work);
								UVC_puContrastCur = wTemp; 
								UVC_ErrorCode = NO_ERROR_ERR;
							}
		              	}
		              	else
		              	{
		                	UVC_ErrorCode = OUT_OF_RANGE_ERR;
		              	}
		              	break; 	
		              
		          	case PU_SATURATION_CONTROL:                    // Saturation is supported   see usbdesc.c
		            	wTemp = buf[0] | (buf[1] << 8);
		              	if ((wTemp >= UVC_puSaturationMin) &&        // check the value to set
		                	  (wTemp <= UVC_puSaturationMax))
		          		{   
							if(adj_prefer.done) {
								adj_prefer.done = 0;
								adj_prefer.cmd = valueH;
								adj_prefer.value = wTemp;
								schedule_work(&adj_prefer.work);
								UVC_puSaturationCur = wTemp; 
								UVC_ErrorCode = NO_ERROR_ERR;
							}
		              	}
		              	else
		              	{
		                	UVC_ErrorCode = OUT_OF_RANGE_ERR;
		              	}
		              	break; 	 	
	            	
	            	case PU_SHARPNESS_CONTROL:                   	// Sharpness is supported   see usbdesc.c
		            	wTemp = buf[0] | (buf[1] << 8);
		              	if ((wTemp >= UVC_puSharpnessMin) &&		// check the value to set
		                	  (wTemp <= UVC_puSharpnessMax))
		          		{   
							if(adj_prefer.done) {
								adj_prefer.done = 0;
								adj_prefer.cmd = valueH;
								adj_prefer.value = wTemp;
								schedule_work(&adj_prefer.work);
								UVC_puSharpnessCur = wTemp; 
								UVC_ErrorCode = NO_ERROR_ERR;
							}
		              	}
		              	else
		              	{
		                	UVC_ErrorCode = OUT_OF_RANGE_ERR;
		              	}
		              	break;
		              	
		             case PU_BACKLIGHT_COMPENSATION_CONTROL:        // Backlight is supported   see usbdesc.c
		            	wTemp = buf[0] | (buf[1] << 8);
		              	if ((wTemp >= UVC_puBacklightMin) &&        // check the value to set
		                	  (wTemp <= UVC_puBacklightMax))
		          		{   
							if(adj_prefer.done) {
								adj_prefer.done = 0;
								adj_prefer.cmd = valueH;
								adj_prefer.value = wTemp;
								schedule_work(&adj_prefer.work);
								UVC_puBacklightCur = wTemp; 
								UVC_ErrorCode = NO_ERROR_ERR;
							}
		              	}
		              	else
		              	{
		                	UVC_ErrorCode = OUT_OF_RANGE_ERR;
		              	}
		              	break;
		              
		             case PU_GAMMA_CONTROL:        					// Gamma is supported   see usbdesc.c
		            	wTemp = buf[0] | (buf[1] << 8);
		              	if ((wTemp >= UVC_puGammaMin) &&        	// check the value to set
		                	  (wTemp <= UVC_puGammaMax))
		          		{   
							if(adj_prefer.done) {
								adj_prefer.done = 0;
								adj_prefer.cmd = valueH;
								adj_prefer.value = wTemp;
								schedule_work(&adj_prefer.work);
								UVC_puGammaCur = wTemp; 
								UVC_ErrorCode = NO_ERROR_ERR;
							}
		              	}
		              	else
		              	{
		                	UVC_ErrorCode = OUT_OF_RANGE_ERR;
		              	}
		              	break;	   	
	            	
	            	default:
	              		UVC_ErrorCode = INVALID_CONTROL_ERR;
	              		break;
	          	}
			break;
	        default:
	        	UVC_ErrorCode = INVALID_UNIT_ERR;
	          break;
		}
		break;

		case USB_UVC_VSIF_NUM:                                 // Video Streaming Interface
			switch (valueH)
          	{               // select the selector control
		        case VS_PROBE_CONTROL :
		          memcpy((void *)&UVC_vsVideoProbe, (const void *)buf, sizeof(UVC_VIDEO_PROBE_AND_COMMIT_CONTROL));
		          UVC_vsVideoProbe.dwMaxPayloadTransferSize = maxtransfersize;
		          UVC_ErrorCode = NO_ERROR_ERR;
		          break;
		        case VS_COMMIT_CONTROL:
		          memcpy((void *)&UVC_vsVideoCommit, (const void *)buf, sizeof(UVC_VIDEO_PROBE_AND_COMMIT_CONTROL));
		          UVC_vsVideoCommit.dwMaxPayloadTransferSize = maxtransfersize;
		          UVC_ErrorCode = NO_ERROR_ERR;
		          break;
		        default:
		          UVC_ErrorCode = INVALID_CONTROL_ERR;
		          break;
			}
      		break;

		default:
      		UVC_ErrorCode = UNKNOWN_ERR;
 		     break;
	}

	if (UVC_ErrorCode == NO_ERROR_ERR)
    	return 1;                                     // we handled the request
  	else
    	return 0;                                     // if not handled we stall it
}

/******************************************************
    UAC API functions
******************************************************/
static int UAC_GetCur(const struct usb_ctrlrequest *ctrl, u8 *buf, int* len)
{
	//u8 indexL = le16_to_cpu(ctrl->wIndex) & 0x00FF;
	//u8 indexH = (le16_to_cpu(ctrl->wIndex) & 0xFF00) >> 8;
	u8 valueH = (le16_to_cpu(ctrl->wValue) & 0xFF00) >> 8;
	u16 data;
	
	switch(valueH)
	{
		case AUDIO_MUTE_CONTROL:
			*len = 1;
			memcpy(buf, &Mute, *len);
			break;
			
		case AUDIO_VOLUME_CONTROL:
			*len = 2;
			data = cpu_to_le16(VolCur);
			memcpy(buf, &data, *len);
			break;
		
		default:
			return 0;
	}	
	
	return 1;
}	

static int UAC_GetMin(const struct usb_ctrlrequest *ctrl, u8 *buf, int* len)
{
	//u8 indexL = le16_to_cpu(ctrl->wIndex) & 0x00FF;
	//u8 indexH = (le16_to_cpu(ctrl->wIndex) & 0xFF00) >> 8;
	u8 valueH = (le16_to_cpu(ctrl->wValue) & 0xFF00) >> 8;
	u16 data;

	switch(valueH)
	{
		case AUDIO_VOLUME_CONTROL:
			*len = 2;
			data = cpu_to_le16(VolMin);
			memcpy(buf, &data, *len);
			return 1;
		
		default:
			break;
	}	
	return 0;
}

static int UAC_GetMax(const struct usb_ctrlrequest *ctrl, u8 *buf, int* len)
{
	//u8 indexL = le16_to_cpu(ctrl->wIndex) & 0x00FF;
	//u8 indexH = (le16_to_cpu(ctrl->wIndex) & 0xFF00) >> 8;
	u8 valueH = (le16_to_cpu(ctrl->wValue) & 0xFF00) >> 8;
	u16 data;
	
	switch(valueH)
	{
		case AUDIO_VOLUME_CONTROL:
			*len = 2;
			data = cpu_to_le16(VolMax);
			memcpy(buf, &data, *len);
			return 1;
		
		default:
			break;
	}
			
	return 0;	
}

static int UAC_GetRes(const struct usb_ctrlrequest *ctrl, u8 *buf, int* len)
{
	//u8 indexL = le16_to_cpu(ctrl->wIndex) & 0x00FF;
	//u8 indexH = (le16_to_cpu(ctrl->wIndex) & 0xFF00) >> 8;
	u8 valueH = (le16_to_cpu(ctrl->wValue) & 0xFF00) >> 8;
	u16 data;

	switch(valueH)
	{
		case AUDIO_VOLUME_CONTROL:
			*len = 2;
			data = cpu_to_le16(VolRes);
			memcpy(buf, &data, *len);
			return 1;
		
		default:
			break;
	}
			
	return 0;	
}

static int get_uac_req_buf(struct usb_gadget *gadget, u8 *buf, const struct usb_ctrlrequest *ctrl)
{
	int len = -1;
	u32 ret = 0;
	
	switch(ctrl->bRequest)
	{

		case GET_CUR:
        	ret = UAC_GetCur(ctrl, buf, &len);
        	break; 
        	   
        case GET_MIN:
        	ret = UAC_GetMin(ctrl, buf, &len);
        	break;
           
        case GET_MAX:
        	ret = UAC_GetMax(ctrl, buf, &len);
        	break;	

        case GET_RES:
        	ret = UAC_GetRes(ctrl, buf, &len);
        	break;			
        	
        default:
        	ret = 0;
            break;
			
	}	
	
	/* if ret = 0 stall back to host */
	if(ret == 0)
	{
		len = -1;
	}	
	
	return len;
}

static int UAC_SetCur(const struct usb_ctrlrequest *ctrl, u8 *buf, int len)
{
 	//INT8U indexL = setupmsg->wIndex & 0x00FF;
	//INT8U indexH = (setupmsg->wIndex & 0xFF00) >> 8;
	//INT8U valueL = setupmsg->wValue & 0x00FF;
	u8 valueH = (le16_to_cpu(ctrl->wValue) & 0xFF00) >> 8;

	switch(valueH)
	{
		case AUDIO_MUTE_CONTROL:
			Mute = buf[0];	
			break;
		
		case AUDIO_VOLUME_CONTROL:
			VolCur = (buf[1] << 8) | buf[0];	
			break;	
	}	
	
	return 1;
}	
	

static void cam_disconnect(struct usb_gadget *gadget)
{
	struct cam_dev		*cam = get_gadget_data(gadget);
	
	cam->state = USB_UVC_NOT_READY_STATE;
	
	//printk("disconnect or port reset\n");
}

static int ep0_queue(struct cam_dev *cam)
{
	int	rc;

	rc = usb_ep_queue(cam->ep0, cam->ep0req, GFP_ATOMIC);
	if (rc != 0 && rc != -ESHUTDOWN)
	{
		/* We can't do much more than wait for a reset */
		printk("error in submission: %s --> %d\n", cam->ep0->name, rc);
	}
	return rc;
}

static void ep0_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct cam_dev		*cam = ep->driver_data;

	if(req->actual > 0)
		//printk("%s --> %s, buf %p, actual %d\n", __func__, cam->ep0req_name, req->buf, req->actual);
	
	if (req->status || req->actual != req->length)
		printk("%s --> %d, %u/%u\n", __func__, req->status, req->actual, req->length);
	
	if (req->status == -ECONNRESET)		// Request was cancelled
		usb_ep_fifo_flush(ep);

	if(cam->ctrl && ((cam->ctrl->bRequestType & 0x80) == USB_DIR_OUT))
	{
		u16	indexL = le16_to_cpu(cam->ctrl->wIndex) & 0x00FF;
		/* Process EP0 OUT data */
		if(indexL == USB_UVC_VCIF_NUM || indexL == USB_UVC_VSIF_NUM)
		{
			UVC_SetCur(cam->ctrl, req->buf, req->actual);	/* UVC class set request */	
		} 
		else if(indexL == USB_UAC_VCIF_NUM || indexL == USB_UAC_VSIF_NUM)
		{
			UAC_SetCur(cam->ctrl, req->buf, req->actual);	/* UAC class set request */
		}
	}	
	/* let control message pointer point to NULL */
	cam->ctrl = NULL;
}

/* ISO vidoe endpoint completion handlers. These always run in_irq. */
static void iso_v_in_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct cam_dev		*cam = ep->driver_data;

	if (req->status || req->actual != req->length)
		printk("%s --> %d, %u/%u\n", __func__, req->status, req->actual, req->length);
	if (req->status == -ECONNRESET)		// Request was cancelled
		usb_ep_fifo_flush(ep);

	/* Hold the lock while we update the request and buffer states */
	smp_wmb();
	spin_lock(&cam->lock);
	cam->state = USB_UVC_SEND_V_DONE_STATE;
	spin_unlock(&cam->lock);
}

/* ISO audio endpoint completion handlers. These always run in_irq. */
static void iso_a_in_complete(struct usb_ep *ep, struct usb_request *req)
{
	struct cam_dev		*cam = ep->driver_data;

	if (req->status || req->actual != req->length)
		printk("%s --> %d, %u/%u\n", __func__, req->status, req->actual, req->length);
	if (req->status == -ECONNRESET)		// Request was cancelled
		usb_ep_fifo_flush(ep);

	/* Hold the lock while we update the request and buffer states */
	smp_wmb();
	spin_lock(&cam->lock);
	cam->state = USB_UVC_SEND_A_DONE_STATE;
	spin_unlock(&cam->lock);
	//printk("iso_a_in_complete\n");
}

static void cam_config_uvc_resolution_to_descriptor(void)
{
	/* config UVC width */
	switch(mod_data.width)
	{
		case WIDTH_640:
		case WIDTH_1024:
		case WIDTH_1280:
		case WIDTH_1920:
			uvc_width = mod_data.width;
			break;	
		
		default:
			/* defualt width = 640 */
			uvc_width = WIDTH_640;
			break;	
	}	
	
	/* config UVC height */
	switch(mod_data.height)
	{
		case HEIGHT_480:
		case HEIGHT_768:
		case HEIGHT_720:
		case HEIGHT_1080:
			uvc_height = mod_data.height;
			break;	
		
		default:
			/* defualt height = 480 */
			uvc_height = HEIGHT_480;
			break;	
	}	
	
	maxframesize = (uvc_width * uvc_height * 2) ;
	printk("cam config descriptor w[%d]/h[%d], maxframesize = 0x%x\n", uvc_width, uvc_height, maxframesize);
	
	/* Config the UVC descriptors */
	UVC_vsVideoCommit.dwMaxVideoFrameSize = maxframesize;
	UVC_vsVideoProbe.dwMaxVideoFrameSize = maxframesize;
	USB_UVC_ConfigDescriptor[VSFRAMDES_WIDTH_POS] = (u8)(uvc_width & 0x00FF);
	USB_UVC_ConfigDescriptor[VSFRAMDES_WIDTH_POS+1] = (u8)((uvc_width & 0xFF00) >> 8);
	USB_UVC_ConfigDescriptor[VSFRAMDES_HEIGHT_POS] = (u8)(uvc_height & 0x00FF);
	USB_UVC_ConfigDescriptor[VSFRAMDES_HEIGHT_POS+1] = (u8)((uvc_height & 0xFF00) >> 8);
	USB_UVC_ConfigDescriptor[VSFRAMDES_FRAMESIZE_POS] =(u8)(maxframesize & 0x000000FF);
	USB_UVC_ConfigDescriptor[VSFRAMDES_FRAMESIZE_POS+1] =(u8)((maxframesize & 0x0000FF00) >> 8);
	USB_UVC_ConfigDescriptor[VSFRAMDES_FRAMESIZE_POS+2] =(u8)((maxframesize & 0x00FF0000) >> 16);
	USB_UVC_ConfigDescriptor[VSFRAMDES_FRAMESIZE_POS+3] =(u8)((maxframesize & 0xFF000000) >> 24);
}	

static int class_setup_req(struct cam_dev *cam, const struct usb_ctrlrequest *ctrl)
{
	struct usb_request	*req = cam->ep0req;
	int			value = -EOPNOTSUPP;
	u16			indexL = le16_to_cpu(ctrl->wIndex) & 0x00FF;
	
	
	if((ctrl->bRequestType & 0x80) == USB_DIR_OUT)
	{
		/* enable EP0 OUT to get data */
		value = ctrl->wLength;
		cam->ctrl = (struct usb_ctrlrequest*)ctrl;
	}	
	else if(ctrl->bRequestType & USB_DIR_IN)
	{
		/* The direction of EP0 request is handled in UDC layer*/	
		if(indexL == USB_UVC_VCIF_NUM || indexL == USB_UVC_VSIF_NUM)
		{
			/* Process UVC class request */
			value = get_uvc_req_buf(cam->gadget, req->buf, ctrl);
		}	
		else if(indexL == USB_UAC_VCIF_NUM || indexL == USB_UAC_VSIF_NUM)
		{
			/* Process UAC class request */
			value = get_uac_req_buf(cam->gadget, req->buf, ctrl);
		}	
	}
	
	if (value == -EOPNOTSUPP)
		printk("unknown class-specific control req ""%02x.%02x v%04x\n",
			ctrl->bRequestType, ctrl->bRequest,
			le16_to_cpu(ctrl->wValue));
	
	/* stall ep0 when the result of class request is error*/
	if(value == -1)
		cam_set_halt(cam, cam->ep0);
			
	return value;
}

static int standard_setup_req(struct cam_dev *cam, const struct usb_ctrlrequest *ctrl)
{
	struct usb_request	*req = cam->ep0req;
	int			value = -EOPNOTSUPP;
	u16			w_index = le16_to_cpu(ctrl->wIndex);
	u16			w_value = le16_to_cpu(ctrl->wValue);

	/* Usually this just stores reply data in the pre-allocated ep0 buffer,
	 * but config change events will also reconfigure hardware. */
	//printk("%s: request %d\n", __func__,ctrl->bRequest);
	switch (ctrl->bRequest)
	{

		case USB_REQ_GET_DESCRIPTOR:
			if (ctrl->bRequestType != (USB_DIR_IN | USB_TYPE_STANDARD |
					USB_RECIP_DEVICE))
			{
				/*For bad descriptor test of CV test of WHQL*/
				usb_ep_set_halt(cam->ep0);
				break;
			}
			switch (w_value >> 8)
			{
				case USB_DT_DEVICE:
					value = sizeof(USB_UVC_DeviceDescriptor);
					//printk("USB_DT_DEVICE val = %d\n", value);
					memcpy(req->buf, &USB_UVC_DeviceDescriptor, value);
					break;
				case USB_DT_DEVICE_QUALIFIER:
					if (!gadget_is_dualspeed(cam->gadget))
						break;
					value = sizeof USB_UVC_Qualifier_Descriptor_TBL;
					memcpy(req->buf, &USB_UVC_Qualifier_Descriptor_TBL, value);
					break;
		
				case USB_DT_OTHER_SPEED_CONFIG:
					if (!gadget_is_dualspeed(cam->gadget))
						break;
					goto get_config;
				case USB_DT_CONFIG:
get_config:
					value = get_config_buf(cam->gadget, req->buf, w_value >> 8, w_value & 0xff);					
					//printk("USB_DT_CONFIG len = %d\n", value);		
					break;
		
				case USB_DT_STRING:
					value = get_string_buf(cam->gadget, req->buf, w_value >> 8, w_value & 0xff);
					//printk("USB_DT_STRING len = %d\n", value);		
					break;
					
				case 0x0F:
					value = -EINVAL;
					break;
		
				default:
					printk("[%s][%d][%x]\n", __FUNCTION__, __LINE__, w_value);
			}
			
			break;
	
		/* One config, two speeds */
		case USB_REQ_SET_CONFIGURATION:
			cam->state = USB_UVC_IDLE_STATE;
			printk("Got set config\n");
			value = DELAYED_STATUS;
			break;
	
		case USB_REQ_SET_INTERFACE:
			if (ctrl->bRequestType != (USB_DIR_OUT| USB_TYPE_STANDARD |
					USB_RECIP_INTERFACE))
				break;			
			break;
			
		default:
			printk("unknown control req %02x.%02x v%04x i%04x l%u\n",
				ctrl->bRequestType, ctrl->bRequest,
				w_value, w_index, le16_to_cpu(ctrl->wLength));
	}

	return value;
}

static int cam_setup(struct usb_gadget *gadget,
		const struct usb_ctrlrequest *ctrl)
{
	struct cam_dev		*cam = get_gadget_data(gadget);
	int			rc;
	int			w_length = le16_to_cpu(ctrl->wLength);

	++cam->ep0_req_tag;		// Record arrival of a new request
	cam->ep0req->context = NULL;
	cam->ep0req->length = 0;
	
	if ((ctrl->bRequestType & USB_TYPE_MASK) == USB_TYPE_CLASS)
		rc = class_setup_req(cam, ctrl);
	else
		rc = standard_setup_req(cam, ctrl);
			
	/* Respond with data/status or defer until later? */
	if (rc >= 0 && rc != DELAYED_STATUS)
	{
		rc = min(rc, w_length);
		cam->ep0req->length = rc;
		cam->ep0req->zero = rc < w_length && (rc % gadget->ep0->maxpacket) == 0;
		cam->ep0req_name = (ctrl->bRequestType & USB_DIR_IN ?
				"ep0-in" : "ep0-out");
		rc = ep0_queue(cam);
	}

	/* Device either stalls (rc < 0) or reports success */
	return rc;
}

static void cam_release(struct kref *ref)
{
	struct cam_dev	*cam = container_of(ref, struct cam_dev, ref);

	kfree(cam);
}

static void cam_unbind(struct usb_gadget *gadget)
{
	struct cam_dev		*cam = get_gadget_data(gadget);
	struct usb_request	*req = cam->ep0req;
	struct usb_request	*iso_video_req = cam->iso_video_req;
	struct usb_request	*iso_audio_req = cam->iso_audio_req;

	printk("cam_unbind\n");
	
	/* Free the request and buffer for endpoint 0 */
	if (req)
	{
		kfree(req->buf);
		usb_ep_free_request(cam->ep0, req);
	}

	/* Free the request and buffer for ISO endpoint */
	if (iso_video_req)
	{
		iso_video_req->buf = NULL;
		usb_ep_free_request(cam->iso_video_in_ep, req);
	}
	
	if (iso_audio_req)
	{
		iso_audio_req->buf = NULL;
		usb_ep_free_request(cam->iso_audio_in_ep, req);
	}

	/* Unregister gp_usb_uvc_handler */
	gp_register_uvc_handler((gp_uvc_control_t*)NULL);

	set_gadget_data(gadget, NULL);

}

static int __init cam_bind(struct usb_gadget *gadget)
{
	int	rc = -ENOMEM;
	struct cam_dev		*cam = the_cam;
	struct usb_ep		*ep;
	struct usb_request	*req;

	cam->gadget = gadget;
	set_gadget_data(gadget, cam);
	cam->ep0 = gadget->ep0;
	cam->ep0->driver_data = cam;

	/* Find all the endpoints we will use */
	usb_ep_autoconfig_reset(gadget);
	
	/* Find EP5 for Video ISO in */
	ep = usb_ep_autoconfig(gadget, &hs_iso_video_in_desc);
	
	if (!ep)
		goto autoconf_fail;
		
	ep->driver_data = cam;		// claim the endpoint
	cam->iso_video_in_ep = ep;
	
	/* Find EP7 for Video ISO in */
	ep = usb_ep_autoconfig(gadget, &hs_iso_audio_in_desc);
	
	if (!ep)
		goto autoconf_fail;
		
	ep->driver_data = cam;		// claim the endpoint
	cam->iso_audio_in_ep = ep;

	/* Allocate the request and buffer for endpoint 0 */
	cam->ep0req = req = usb_ep_alloc_request(cam->ep0, GFP_KERNEL);
	if (!req)
		goto out;

	/* alloc 512 bytes for EP0 descriptor */		
	req->buf = kmalloc(EP0_BUFSIZE, GFP_KERNEL);
	if (!req->buf)
		goto out;
		
	/* register the complete function */
	req->complete = ep0_complete;

	/* Allocate the request and buffer for ISO Video IN endpoint */
	cam->iso_video_req = req = usb_ep_alloc_request(cam->iso_video_in_ep, GFP_KERNEL);
	if (!req)
		goto out;

	/* Set ISO IN EP buffer pointer to NULL */		
	req->buf = NULL;

	/* register the complete function */
	req->complete = iso_v_in_complete;

	/* Allocate the request and buffer for ISO Audio IN endpoint */
	cam->iso_audio_req = req = usb_ep_alloc_request(cam->iso_audio_in_ep, GFP_KERNEL);
	if (!req)
		goto out;

	/* Set ISO IN EP buffer pointer to NULL */		
	req->buf = NULL;

	/* register the complete function */
	req->complete = iso_a_in_complete;

	/* This should reflect the actual gadget power source */
	usb_gadget_set_selfpowered(gadget);	

	/* Register gp_usb_uvc_handler */
	gp_register_uvc_handler(&cam_uvc_handle);
	
	/* According to parameter, config the height/width */
	cam_config_uvc_resolution_to_descriptor();

	UVC_set_default_parameters();
	return 0;

autoconf_fail:
	printk("unable to autoconfigure all endpoints\n");
	rc = -ENOTSUPP;

out:
	cam->state = USB_UVC_NOT_READY_STATE;	// The thread is dead
	cam_unbind(gadget);
	
	return rc;
}

static void cam_suspend(struct usb_gadget *gadget)
{
	//struct cam_dev		*cam = get_gadget_data(gadget);
	//cam->state = USB_UVC_NOT_READY_STATE;
	//printk("cam_suspend\n");
}

static void cam_resume(struct usb_gadget *gadget)
{
	//struct cam_dev		*cam = get_gadget_data(gadget);
	//printk("cam_resume\n");
}

static struct usb_gadget_driver		cam_driver =
{
	.speed		= USB_SPEED_HIGH,
	.function	= (char *) longname,
	.bind		= cam_bind,
	.unbind		= cam_unbind,
	.disconnect	= cam_disconnect,
	.setup		= cam_setup,
	.suspend	= cam_suspend,
	.resume		= cam_resume,

	.driver		=
	{
		.name		= (char *) shortname,
		.owner		= THIS_MODULE,
		// .release = ...
		// .suspend = ...
		// .resume = ...
	},
};

static int __init cam_alloc(void)
{
	struct cam_dev		*cam;

	cam = kzalloc(sizeof *cam, GFP_KERNEL);
	if (!cam)
		return -ENOMEM;
	spin_lock_init(&cam->lock);
	kref_init(&cam->ref);

	the_cam = cam;
	return 0;
}

static int __init cam_init(void)
{
	int		rc;
	struct cam_dev	*cam;

	if ((rc = cam_alloc()) != 0)
		return rc;
	cam = the_cam;
	
	/* Init cam gadget state */
	cam->state = USB_UVC_NOT_READY_STATE;
	
	/* Set UDC type = UVC */
	gp_usb_set_udc_type(UDC_UVC_TYPE);
	
	if ((rc = gp_usb_gadget_register_driver(&cam_driver)) != 0)
		kref_put(&cam->ref, cam_release);
		
	INIT_WORK(&adj_prefer.work, gp_adj_prefer_work);
	adj_prefer.cmd = 0;
	adj_prefer.done = 1;
		
	return rc;
}
module_init(cam_init);

static void __exit cam_cleanup(void)
{
	struct cam_dev	*cam = the_cam;

	/* Unregister the driver if the thread hasn't already done so */
	gp_usb_gadget_unregister_driver(&cam_driver);
	
	kref_put(&cam->ref, cam_release);
}
module_exit(cam_cleanup);
