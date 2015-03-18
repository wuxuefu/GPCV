#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <linux/types.h>
#include <linux/ioctl.h>
#include <linux/fb.h>

#include <mach/common.h>
#include <mach/typedef.h>

#include <mach/gp_display.h>
#include <mach/gp_ceva.h>
#include <mach/gp_scale.h>
#include "chunkmem.h"
#include <dlfcn.h>
#include "mach/gp_scale2.h"
#include "gp_ovg.h"
#include "ceva.h"
#include "image_decode.h"
#include <mach/gp_chunkmem.h>
#include "icver.h"
//#include "photo_decode.h" 
//#include "image_decode.h"
#include "gp_on2.h"

extern int g_DispMode;
extern gp_disp_pixelsize_t pixelSize;
int 
jpg_scaler_process(
	scale_content_t sct
)
{
	int ret = 0;
	int devHandle;
	
	scale_content_t sct2;
	
	devHandle = open("/dev/scalar", O_RDONLY);
	
	if (devHandle < 0)
		return -1;
		
	if (sct.src_img.width > 2048 || sct.dst_img.width > 2048)
	{
		sct2 = sct;
		
		sct2.clip_rgn.x = 0;
		sct2.clip_rgn.y = 0;
		sct2.clip_rgn.width = sct.src_img.width/2;
		sct2.clip_rgn.height = sct.src_img.height;
			
		sct2.scale_rgn.x = 0;
		sct2.scale_rgn.y = 0;
		sct2.scale_rgn.width = sct.dst_img.width/2;
		sct2.scale_rgn.height = sct.dst_img.height;
		
		if (ioctl(devHandle, SCALE_IOCTL_TRIGGER, &sct2) < 0) {
			printf("scale_start fail\n");
			ret = -1;
		}
		
		sct2.clip_rgn.x = sct.src_img.width/2;
		sct2.clip_rgn.y = 0;
			
		sct2.scale_rgn.x = sct.dst_img.width/2;
		sct2.scale_rgn.y = 0;
		
		if (ioctl(devHandle, SCALE_IOCTL_TRIGGER, &sct2) < 0) {
			printf("scale_start fail\n");
			ret = -1;
		}
		
		close(devHandle);
		
		return ret;
	}
	
	if (ioctl(devHandle, SCALE_IOCTL_TRIGGER, &sct) < 0) {
		printf("scale_start fail\n");
		ret = -1;
	}
	close(devHandle);
	
	return ret;
}

void* Gdjpegdecode(void *filepath, gp_bitmap_t *bitmap, int owidth, int oheight,gp_size_t* rect, int thumbnail)
{
	FILE* input;
	int size, out_size, format;
	char* pInput;
	char* pOutput = NULL;
	int ret = 0;
	cevaDecode_t vdt;
	int bufW, bufH;
	int dar_w, dar_h;

	ret = gpOn2Codec_Load(CEVA_CODEC_TYPE_JPG);
	if (ret < 0)
	{
		printf("Load Codec fail\n");
		return 0;
	}
	ret = gpOn2Codec_Init(NULL);
	if (ret < 0)
	{
		printf("Codec Initial fail\n");
		gpOn2Codec_Unload();
		return 0;
	}
	
	input = fopen(filepath, "rb");
	if (!input)
	{
		printf("cannot open file %s\n", filepath);
		goto exit;
	}
	
	fseek(input, 0, SEEK_END);
	size = ftell(input);
	fseek(input, 0, SEEK_SET);
	
	pInput = gpChunkMemAlloc(size);
	
	if (!pInput)
	{
		printf("Cannot allocate chunk memory buffer.\n");
		fclose(input);
		goto exit;
	}
	
	fread(pInput, 1, size, input);
	fclose(input);
		
	vdt.pInBuf    = pInput;
	vdt.nUsefulByte = size;
	vdt.flags = VFLAG_GET_INFO_ONLY | VFLAG_RGB565OUT;
	
	if (thumbnail)
		vdt.flags |= VFLAG_THUMBNAIL;
		
	ret = gpOn2Codec_Exec(&vdt);
	
	if (ret < 0)
	{
		if (thumbnail)
		{
			thumbnail = 0;
			vdt.flags &= ~VFLAG_THUMBNAIL;
			ret = gpOn2Codec_Exec(&vdt);
		}
		if (ret < 0)
		{
			printf("Get image info fail\n");
			goto exit;
		}
	}
	
	//buffer size must be 16 pixel alignment
	bufW = ((vdt.width +0xF) >> 4) << 4;
	bufH = ((vdt.height +0xF) >> 4) << 4;
	
	if (vdt.flags & VFLAG_RGB565OUT) 
	{
		out_size = bufW * bufH * 2;
		format = SP_BITMAP_RGB565;
	} 
	else if(vdt.flags & VFLAG_YUV422OUT) 
	{
		out_size = bufW * bufH * 2;
		format = SP_BITMAP_YCbYCr;
	} 
	else if (vdt.flags & VFLAG_YUV422SEMI) 
	{
		out_size = bufW * bufH *  2;
		format = SP_BITMAP_SEMI422;
	} 
	else if (vdt.flags & VFLAG_YUV400) 
	{
		out_size = bufW * bufH;
		format = SP_BITMAP_YCbCr400;
	}
	else if (vdt.flags & VFLAG_YUV444SEMI) 
	{
		out_size = bufW * bufH * 3;
		format = SP_BITMAP_SEMI444;
	} 
	else 
	{
		out_size = bufW * bufH * 3 /2 ;
		format  = SP_BITMAP_SEMI420;
	}
	
	printf("get image width = %d height = %d stride = %d strideChroma = %d format = %d\n", vdt.width, vdt.height, vdt.stride, vdt.strideChroma, format);
	
	pOutput = gpChunkMemAlloc(out_size);
	if (!pOutput)
	{
		printf("Cannot allocate chunk memory buffer.\n");
		goto exit;
	}
	
	vdt.pFrameBuf = pOutput;
	vdt.nFrameBufSize = out_size;
	vdt.flags = VFLAG_RGB565OUT;
	
	if (thumbnail)
		vdt.flags |= VFLAG_THUMBNAIL;
	
	ret = gpOn2Codec_Exec(&vdt);
	
	if (ret < 0 || (vdt.flags & VFLAG_FRAMEOUT) == 0)
	{
		printf("Decode Image fail\n");
		gpChunkMemFree(pOutput);
		pOutput = NULL;
		goto exit;
	}

	rect->width = vdt.width;
	rect->height = vdt.height;
	
	if ((format != SP_BITMAP_RGB565) ||
		(owidth && oheight && (owidth != vdt.width || oheight != vdt.height)) )
	{
		char* out;
		scale_content_t sct;
		
		if(!owidth) owidth = vdt.width;
		if(!oheight) oheight = vdt.height;
		
		if(!thumbnail) {
			out = gpChunkMemAlloc(owidth * oheight * 2);
		}
		
		memset(&sct, 0, sizeof(sct));
		
		sct.src_img.width    = vdt.width;
		sct.src_img.height   = vdt.height;
		sct.src_img.type	 = format;
		sct.src_img.bpl      = vdt.stride;
		sct.src_img.pData    = vdt.pOutBuf[0];
		sct.src_img.pDataU   = vdt.pOutBuf[1];
		sct.src_img.pDataV   = vdt.pOutBuf[2];
		sct.src_img.strideUV = vdt.strideChroma;
		
		if(!thumbnail) {
			sct.dst_img.pData = (void *)out;

			sct.scale_rgn.x      = 0;
			sct.scale_rgn.y      = 0;
			sct.scale_rgn.width  = owidth;
			sct.scale_rgn.height = oheight;
	
		}
		else {
			int DispHeight, DispWidth;
			int DispX, DispY;
			int par_w, par_h;
			char aspect = 1; //1:for aspect screen, 0: for full screen

			if(g_DispMode == SP_DISP_OUTPUT_TV) {
				par_w = 10;
				par_h = 11;
				//aspect = 0;
			}
			else {
				par_w = (pixelSize.width*bitmap->validRect.height)/bitmap->validRect.width;
				//par_w = pixelSize.height;
				par_h = pixelSize.height;
				//aspect = 1;
			}

			printf("par_w %d par_h %d\n", par_w, par_h);

			if(aspect) { // full screen
				printf("get jpeg size: %dx%d (disp:%dx%d)\n", vdt.width, vdt.height, bitmap->validRect.width, bitmap->validRect.height);
				if ((vdt.width *10/vdt.height) <= (bitmap->validRect.width*10/bitmap->validRect.height))
				{
					DispWidth = (bitmap->validRect.height* vdt.width*par_h)/(vdt.height*par_w);
					DispWidth = (DispWidth >> 3) << 3;
					if(DispWidth > bitmap->validRect.width) {
						DispWidth = bitmap->validRect.width;
					}
					DispHeight = bitmap->validRect.height;
				}
				else
				{
					DispHeight = (bitmap->validRect.width* vdt.height*par_w)/(vdt.width*par_h);
					DispHeight = (DispHeight >> 1) << 1;
					DispWidth = bitmap->validRect.width;
				}
				DispX = (bitmap->validRect.width-DispWidth)/2;
				DispY = (bitmap->validRect.height-DispHeight)/2;
			}
			else { //scale display
				DispX = 0;
				DispY = 0;
				DispWidth = bitmap->validRect.width;
				DispHeight = bitmap->validRect.height;
			}
			printf("[%s][thumbnail scale Size] = %dx%d %dx%d\n", __FUNCTION__, DispX, DispY, DispWidth, DispHeight);

			sct.dst_img.pData = (void *)bitmap->pData;
			sct.dst_img.validRect.x = 0;
			sct.dst_img.validRect.y = 0;
			sct.dst_img.validRect.width = owidth;
			sct.dst_img.validRect.height = oheight;

			/*sct.scale_rgn.x      = bitmap->validRect.x;
			sct.scale_rgn.y      = bitmap->validRect.y;
			sct.scale_rgn.width  = bitmap->validRect.width;
			sct.scale_rgn.height = bitmap->validRect.height;*/
			sct.scale_rgn.x      = DispX + bitmap->validRect.x;
			sct.scale_rgn.x		 = (sct.scale_rgn.x>>2)<<2;
			sct.scale_rgn.y      = DispY + bitmap->validRect.y;
			sct.scale_rgn.width  = DispWidth;
			sct.scale_rgn.height = DispHeight;
		}
		sct.dst_img.width = owidth;
		sct.dst_img.height = oheight;
		sct.dst_img.bpl = owidth*2;
		sct.dst_img.type = SP_BITMAP_RGB565;
		
		sct.clip_rgn.x       = 0;
		sct.clip_rgn.y       = 0;
		sct.clip_rgn.width   = vdt.width;
		sct.clip_rgn.height  = vdt.height;

		jpg_scaler_process(sct);
		
		gpChunkMemFree(pOutput);
		if(!thumbnail) {
			pOutput = out;
		}
		else {
			pOutput = bitmap->pData;
		}
		
		rect->width = owidth;
		rect->height = oheight;
	}	
exit:
	gpOn2Codec_Uninit(NULL);
	gpOn2Codec_Unload();
	
	if (pInput)
		gpChunkMemFree(pInput);
	
	return pOutput;
}

#if 0
void* Gdjpegdecode(void *filepath,int owidth,int oheight,gp_size_t* rect, int t)
{

	int image_size;
	int ret = 0;
	int status = 0;	
	void *image_buff = NULL;
	gp_icver_t icver;
	gpDecodeImg_t *OutputImg;
	
	icver = gpICVersion();
	//HwVersion = icver.major;
	
	Image_Codec_Load(CEVA_CODEC_TYPE_JPG,0);
	OutputImg = (gpDecodeImg_t*)malloc(sizeof(gpDecodeImg_t));
	memset(OutputImg,0,sizeof(gpDecodeImg_t));
					
	//decode jpeg
	ret = jpeg_parse_header(filepath,&(OutputImg->ImgHeader));
	if (ret<0)
	{
		printf("====gpJpegGetHeaderInfo Fail=====\n");
		return NULL;		
	}
	rect->width = OutputImg->ImgHeader.ImgMainWidth;
	rect->height = OutputImg->ImgHeader.ImgMainHeight;		
	//output format	
	if((owidth != 0)&&(oheight != 0))
	{
		OutputImg->width = owidth;
		OutputImg->height = oheight;
	}else
	{
		OutputImg->width = OutputImg->ImgHeader.ImgMainWidth;
		OutputImg->height = OutputImg->ImgHeader.ImgMainHeight;		
	}
	//width Align
	rect->width=OutputImg->width=((OutputImg->width+1)>>1)<<1;
	rect->height=OutputImg->height=((OutputImg->height+1)>>1)<<1;
	

	if(icver.major==MACH_GPL32900)
	{
		OutputImg->type = C_SCALE2_CTRL_OUT_RGB565;//C_SCALE2_CTRL_OUT_VYUY;
		OutputImg->ScalerType = C_SCALER2_CTRL;		
	}
	else
	{
		
		//if(OutputImg->width>2048)
		{
			OutputImg->ScalerType = C_SCALER2_CTRL;
			//OutputImg->type = C_SCALE2_CTRL_OUT_VYUY;//
            OutputImg->type = C_SCALE2_CTRL_OUT_RGB565;
		}
		
		/*else
		{
			OutputImg->ScalerType = C_SCALER1_CTRL;	
			//OutputImg->type=SP_BITMAP_YUYV;//
            OutputImg->type = SP_BITMAP_RGB565;
		}*/
		
	}
	image_size = OutputImg->width * OutputImg->height * 2;
	image_buff = (void *)gpChunkMemAlloc(image_size);
	if(image_buff == NULL)
	{
		printf("decode memory malloc fail!!\n");	
	}
	
	status = jpeg_image_decode_outbuf(filepath,OutputImg,(UINT8 *)image_buff);
	if(status != 0)
	{
		printf("decode fail!!\n");
		gpChunkMemFree(image_buff);
		image_buff =NULL;
	}	
	Image_Codec_Unload();
	free((void*)OutputImg);
	if((status!=0)/*&&(rect->width*rect->height<1024*600)*/)
	{
		printf("use sw decode!\n");
		void * image_rgb;
		image_rgb=(void *)soft_decode_jpg(filepath,&rect->width,&rect->height);
		//scale change YUV
		//image_buff=gpChunkMemAlloc(rect->width*rect->height*2);
		//scale1_act(image_rgb,rect->width,rect->height,image_buff,rect->width,rect->height,C_SCALE2_CTRL_IN_RGB565,C_SCALE2_CTRL_OUT_RGB);
		image_buff = image_rgb;
		//gpChunkMemFree(image_rgb);
	}

	return image_buff;
}

#endif



	
