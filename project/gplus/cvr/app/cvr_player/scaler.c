#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "mach/gp_scale.h"
#include "stdio.h"
#include <string.h>
#include <unistd.h>
#include "mach/gp_scale2.h"
#include "mach/gp_display.h"
#include "scaler.h"
extern int g_DispMode;
extern gp_disp_pixelsize_t pixelSize;

int  scale1_act(
	void * input_addr,
	unsigned short input_w,
	unsigned short input_h,
	void * output_addr,
	unsigned short output_w,
	unsigned short output_h,
	unsigned int input_format,
	unsigned int out_format
)
{

	int nRet, scale_handle;
	gpScale2Format_t image;

	/* scale2 */
	scale_handle = open("/dev/scale2", O_RDWR);
	if(scale_handle < 0) {
		printf("scale2_handle fail\n");
		return -1;
	}

	memset(&image, 0, sizeof(gpScale2Format_t));
	image.input_format = input_format;
	image.input_width = input_w;
	image.input_height = input_h;
	image.input_visible_width = 0;
	image.input_visible_height = 0;
	image.input_x_offset = 0;
	image.input_y_offset = 0;
	image.input_y_addr =(int) input_addr;
	image.input_u_addr = 0;
	image.input_v_addr = 0;
	
	//image.output_format = C_SCALE2_CTRL_OUT_VYUY;
	image.output_format = out_format;//C_SCALE2_CTRL_OUT_VYUY;
	image.output_width = output_w;
	image.output_height = output_h;
	image.output_buf_width = output_w;
	image.output_buf_height = output_h;
	image.output_x_offset = 0;
	image.output_y_addr =(int) output_addr;
	image.output_u_addr = 0;
	image.output_v_addr = 0;	
	
	image.fifo_mode = C_SCALE2_CTRL_FIFO_DISABLE;
	image.scale_mode = C_SCALE2_BY_RATIO;
	image.digizoom_m = 10;
	image.digizoom_n = 10;

	/* scale2 start */
#if 1	
	nRet = ioctl(scale_handle, SCALE2_IOCTL_S_START, &image);
#else
	nRet = ioctl(scale_handle, SCALE2_IOCTL_S_START_WITHOUT_WAIT, &image);
#endif
	if(nRet == C_SCALE2_STATUS_DONE || nRet == C_SCALE2_STATUS_STOP) {
		nRet = 0;
	} else if(nRet == C_SCALE2_STATUS_BUSY) {	
		nRet = 0;
	} else if(nRet == C_SCALE2_STATUS_TIMEOUT) {
		printf("Scale2 time out\n");
		nRet = -1;
		goto __exit;
	} else if(nRet == C_SCALE2_STATUS_INIT_ERR) {
		printf("Scale2 init fail\n");
		nRet = -1;
		goto __exit;
	} else {
		printf("SCALE2_IOCTL_S_START fail\n");
		nRet = -1;
		goto __exit;
	}
	
	/* scale2 stop */
	nRet = ioctl(scale_handle, SCALE2_IOCTL_S_STOP, 0);
	if(nRet < 0) {
		printf("SCALE2_IOCTL_S_STOP fail\n");
	}
	
__exit:	
	close(scale_handle);
	return nRet;
}

int 
scaler_process_zoom(
	unsigned int input_addr,
	unsigned short input_w,
	unsigned short input_h,
	unsigned int output_addr,
	unsigned short output_w,
	unsigned short output_h,
	int srcformat,
	int dstformat,
	int zoom
)
{
	int ret = 0;
	int isOutput4x3;
	int crop_w = input_w;
	int crop_h = input_h;
	int clip_y = 0;
	int handle;

	char aspect = 1; //1:for aspect screen, 0: for full screen
	
	scale_content_t sct;
	
	handle = open("/dev/scalar", O_RDONLY);
	if (!handle)
	{
		printf("cannot open /dev/scalar \n");
		return -1;
	}

	int DispHeight, DispWidth;
	int DispX, DispY;
	int par_w, par_h;

	if(g_DispMode == SP_DISP_OUTPUT_TV) {
		par_w = 10;
		par_h = 11;
	}
	else {
		par_w = (pixelSize.width*output_h)/output_w;
		par_h = pixelSize.height;
	}

	printf("par_w %d par_h %d\n", par_w, par_h);

	if(aspect) { // full screen
		if ((input_w*10/input_h) <= (output_w*10/output_h))
		{
			DispWidth = (output_h* input_w*par_h)/(input_h*par_w);
			DispWidth = (DispWidth >> 3) << 3;
			if(DispWidth > output_w) {
				DispWidth = output_w;
			}
			DispHeight = output_h;
		}
		else
		{
			DispHeight = (output_w* input_h*par_w)/(input_w*par_h);
			DispHeight = (DispHeight >> 1) << 1;
			DispWidth = output_w;
		}
		DispX = (output_w-DispWidth)/2;
		DispY = (output_h-DispHeight)/2;
	}
	else { //scale display
		DispX = 0;
		DispY = 0;
		DispWidth = output_w;
		DispHeight = output_h;
	}
	printf("[%s][scale Size] = %dx%d %dx%d\n", __FUNCTION__, DispX, DispY, DispWidth, DispHeight);
	//isOutput4x3 = (output_w*3 == output_h*4) ? 1:0;
	
	//if (isOutput4x3)
	if(g_DispMode == SP_DISP_OUTPUT_TV) {
		//crop_w = (4*input_h)/(3);
		//crop_w = (output_h*input_w*par_h)/(input_h*par_w);
	}

	memset(&sct, 0, sizeof(sct));
	sct.src_img.pData = (void *)input_addr;
	sct.src_img.width = input_w;
	sct.src_img.height = input_h;
	sct.src_img.bpl = input_w*2;
	sct.src_img.type = srcformat;

	sct.dst_img.pData = (void *)output_addr;
	sct.dst_img.width = output_w;
	sct.dst_img.height = output_h;
	sct.dst_img.bpl = output_w*2;
	sct.dst_img.type = dstformat;
	
	if (zoom > 100)
	{
		crop_w = ((100*crop_w/zoom) >> 4) << 4;
		crop_h = crop_w * input_h / input_w;
		clip_y = (input_h - crop_h)/2;
		printf("zoom:%d crop_w:%d crop_h:%d clip_y: %d\n", zoom, crop_w, crop_h, clip_y);

	}
	if (input_w > 2048 || output_w > 2048)
	{
		sct.clip_rgn.x = (input_w - crop_w)/2;
		sct.clip_rgn.y = clip_y;
		sct.clip_rgn.width = crop_w/2;
		sct.clip_rgn.height = crop_h;
		
		/*sct.scale_rgn.x = 0;
		sct.scale_rgn.y = 0;
		sct.scale_rgn.width = output_w/2;
		sct.scale_rgn.height = output_h;*/
		sct.scale_rgn.x = DispX;
		sct.scale_rgn.y = DispY;
		sct.scale_rgn.width = DispWidth/2;
		sct.scale_rgn.height = DispHeight;
		
		if (ioctl(handle/*scaleInfo.devHandle*/, SCALE_IOCTL_TRIGGER, &sct) < 0) {
			printf("scale_start fail\n");
			ret = -1;
		}
		
		sct.clip_rgn.x += crop_w/2;
		sct.clip_rgn.y = clip_y;
		sct.clip_rgn.width = crop_w/2;
		sct.clip_rgn.height = crop_h;
		
		/*sct.scale_rgn.x += output_w/2;
		sct.scale_rgn.y = 0;
		sct.scale_rgn.width = output_w/2;
		sct.scale_rgn.height = output_h;*/
		sct.scale_rgn.x += (DispWidth/2);
		sct.scale_rgn.y = DispY;
		sct.scale_rgn.width = DispWidth/2;
		sct.scale_rgn.height = DispHeight;
		
		if (ioctl(handle/*scaleInfo.devHandle*/, SCALE_IOCTL_TRIGGER, &sct) < 0) {
			printf("scale_start fail\n");
			ret = -1;
		}
		close(handle);
		return ret;
	}
	
	sct.clip_rgn.x = (input_w - crop_w)/2;
	sct.clip_rgn.y = clip_y;
	sct.clip_rgn.width = crop_w;
	sct.clip_rgn.height = crop_h;
		
	/* sct.scale_rgn.x = 0;
	sct.scale_rgn.y = 0;
	sct.scale_rgn.width = output_w;
	sct.scale_rgn.height = output_h;*/
	sct.scale_rgn.x = DispX;
	sct.scale_rgn.y = DispY;
	sct.scale_rgn.width = DispWidth;
	sct.scale_rgn.height = DispHeight;
	
	if (ioctl(handle/*scaleInfo.devHandle*/, SCALE_IOCTL_TRIGGER, &sct) < 0) {
		printf("scale_start fail\n");
		ret = -1;
	}
	
	close(handle);
	return ret;
}
int 
scaler_process(
	unsigned int input_addr,
	unsigned short input_w,
	unsigned short input_h,
	unsigned int output_addr,
	unsigned short output_w,
	unsigned short output_h,
	int srcformat,
	int dstformat
)
{
	int ret = 0;
	int devHandle;
	
	scale_content_t sct;

	memset(&sct, 0, sizeof(sct));
	sct.src_img.pData = (void *)input_addr;
	sct.src_img.width = input_w;
	sct.src_img.height = input_h;
	sct.src_img.bpl = input_w*2;

	sct.src_img.type = srcformat;
	sct.dst_img.pData = (void *)output_addr;
	sct.dst_img.width = output_w;
	sct.dst_img.height = output_h;
	sct.dst_img.bpl = output_w*2;
	sct.dst_img.type = dstformat;
	
	devHandle = open("/dev/scalar", O_RDONLY);
	
	if (devHandle < 0)
		return -1;
		
	if (input_w > 2048 || output_w > 2048)
	{
		sct.clip_rgn.x = 0;
		sct.clip_rgn.y = 0;
		sct.clip_rgn.width = input_w/2;
		sct.clip_rgn.height = input_h;
			
		sct.scale_rgn.x = 0;
		sct.scale_rgn.y = 0;
		sct.scale_rgn.width = output_w/2;
		sct.scale_rgn.height = output_h;
		
		if (ioctl(devHandle, SCALE_IOCTL_TRIGGER, &sct) < 0) {
			printf("scale_start fail\n");
			ret = -1;
		}
		
		sct.clip_rgn.x = input_w/2;
		sct.clip_rgn.y = 0;
		sct.clip_rgn.width = input_w/2;
		sct.clip_rgn.height = input_h;
			
		sct.scale_rgn.x = output_w/2;
		sct.scale_rgn.y = 0;
		sct.scale_rgn.width = output_w/2;
		sct.scale_rgn.height = output_h;
		
		if (ioctl(devHandle, SCALE_IOCTL_TRIGGER, &sct) < 0) {
			printf("scale_start fail\n");
			ret = -1;
		}
		
		close(devHandle);
		
		return ret;
	}
		
	sct.clip_rgn.x = 0;
	sct.clip_rgn.y = 0;
	sct.clip_rgn.width = input_w;
	sct.clip_rgn.height = input_h;
		
	sct.scale_rgn.x = 0;
	sct.scale_rgn.y = 0;
	sct.scale_rgn.width = output_w;
	sct.scale_rgn.height = output_h;
	
	if (ioctl(devHandle, SCALE_IOCTL_TRIGGER, &sct) < 0) {
		printf("scale_start fail\n");
		ret = -1;
	}
	close(devHandle);
	
	return ret;
}
