/* Necessary includes for device drivers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "mach/typedef.h"
#include "mach/gp_display.h"
#include "mach/gp_chunkmem.h"
#include "disp.h"

#define DAR_W 95
#define DAR_H 54

typedef struct dispManager_s {
	UINT32 layer;
	int fdDisp;
	gp_disp_res_t resolution;

	int fdMem;
	chunk_block_t memBlock;
	gp_bitmap_t fb[2];		// framebuffer
	int fbIndex;
} dispManager_t;


UINT32
dispCreate(
	HANDLE *pHandle,
	UINT32 layer
)
{
	dispManager_t *pDisp;

	pDisp = (dispManager_t *)malloc(sizeof(dispManager_t));
	memset(pDisp, 0, sizeof(dispManager_t));
	pDisp->layer = layer;
	
	/* Opening the device dispDev */
	pDisp->fdDisp = open("/dev/disp0", O_RDWR);
	ioctl(pDisp->fdDisp, DISPIO_SET_INITIAL, 0);
	ioctl(pDisp->fdDisp, DISPIO_GET_PANEL_RESOLUTION, &pDisp->resolution);
	//pDisp->resolution.width = 320;
	//pDisp->resolution.height = 240;
	/* Allocate framebuffer */
	int bufferSize = pDisp->resolution.width * pDisp->resolution.height * 2;
	pDisp->fdMem = open("/dev/chunkmem", O_RDWR);
	pDisp->memBlock.size = bufferSize * 2;
	ioctl(pDisp->fdMem, CHUNK_MEM_ALLOC, (unsigned long)&pDisp->memBlock);

	
	/* Setup bitmap double buffer */
	for (int i = 0; i < 2; i++) {
		gp_bitmap_t *fb = &pDisp->fb[i];
		fb->width = pDisp->resolution.width;
		fb->height = pDisp->resolution.height;
		fb->bpl = fb->width * 2;
		if (pDisp->layer == DISP_LAYER_PRIMARY) {
			//fb->type = SP_BITMAP_4Y4Cb4Y4Cr;
			fb->type = SP_BITMAP_RGB565;	
		}
		else {
			fb->type = SP_BITMAP_RGB565;	
		}
		fb->pData = pDisp->memBlock.addr + i * bufferSize;
	}
	if(pDisp->layer != DISP_LAYER_PRIMARY) {
		memset(pDisp->memBlock.addr, 0, bufferSize);
	}
	else {
		memset(pDisp->memBlock.addr, 0, bufferSize);
	}

	
	if (pDisp->layer == DISP_LAYER_PRIMARY) {
		ioctl(pDisp->fdDisp, DISPIO_SET_PRI_BITMAP, &pDisp->fb[pDisp->fbIndex]);
		ioctl(pDisp->fdDisp, DISPIO_SET_PRI_ENABLE, 1);	
		printf("init index %d\n", pDisp->fbIndex);
		//pDisp->fbIndex = pDisp->fbIndex ^ 1;
	}
	else {
		ioctl(pDisp->fdDisp, DISPIO_SET_OSD_BITMAP(0), &pDisp->fb[pDisp->fbIndex]);

		/* Set osd layer 0 scale */
		gp_disp_scale_t osdScale;
		osdScale.x = 0;
		osdScale.y = 0;
		osdScale.width = pDisp->resolution.width;
		osdScale.height = pDisp->resolution.height;
		ioctl(pDisp->fdDisp, DISPIO_SET_OSD_SCALEINFO(0), &osdScale);

		/* Set osd alpha & color key */
		gp_disp_osdalpha_t osdAlphs;
		//osdAlphs.consta = SP_DISP_ALPHA_CONSTANT;
		//osdAlphs.ppamd = SP_DISP_ALPHA_PERPIXEL_ONLY;
		ioctl(pDisp->fdDisp, DISPIO_SET_OSD_KEY(0), TRANSPARENT_COLOR);
		osdAlphs.consta = SP_DISP_ALPHA_PERPIXEL;
		osdAlphs.ppamd = SP_DISP_ALPHA_COLORKEY_ONLY;
		osdAlphs.alpha = 100;
		ioctl(pDisp->fdDisp, DISPIO_SET_OSD_ALPHA(0), &osdAlphs);
		
		/* Enable osd layer 0 */
		ioctl(pDisp->fdDisp, DISPIO_SET_OSD_ENABLE(0), 1);
	}

	*pHandle = (HANDLE)pDisp;
	
	
	/* Enable backlight */
	ioctl(pDisp->fdDisp, DISPIO_SET_BACKLIGHT, 1);

	return SP_OK;
}

UINT32 dispLayerEnable(HANDLE handle)
{
	dispManager_t *pDisp = (dispManager_t *)handle;
	if(pDisp == NULL) return -1;
	ioctl(pDisp->fdDisp, DISPIO_SET_PRI_ENABLE, 1);	
		gp_disp_scale_t osdScale;
		osdScale.x = 0;
		osdScale.y = 0;
		osdScale.width = pDisp->resolution.width;
		osdScale.height = pDisp->resolution.height;
		ioctl(pDisp->fdDisp, DISPIO_SET_OSD_SCALEINFO(0), &osdScale);

		/* Set osd alpha & color key */
		gp_disp_osdalpha_t osdAlphs;
		//osdAlphs.consta = SP_DISP_ALPHA_CONSTANT;
		//osdAlphs.ppamd = SP_DISP_ALPHA_PERPIXEL_ONLY;
		ioctl(pDisp->fdDisp, DISPIO_SET_OSD_KEY(0), TRANSPARENT_COLOR);
		osdAlphs.consta = SP_DISP_ALPHA_PERPIXEL;
		osdAlphs.ppamd = SP_DISP_ALPHA_COLORKEY_ONLY;
		osdAlphs.alpha = 100;
		ioctl(pDisp->fdDisp, DISPIO_SET_OSD_ALPHA(0), &osdAlphs);
		
		/* Enable osd layer 0 */
		//ioctl(pDisp->fdDisp, DISPIO_SET_OSD_ENABLE(0), 1);
	ioctl(pDisp->fdDisp, DISPIO_SET_OSD_ENABLE(0), 1);
	ioctl(pDisp->fdDisp, DISPIO_SET_OSD_ENABLE(1), 0);
	return 0;
}

UINT32
dispDestroy(
	HANDLE handle
)
{
	dispManager_t *pDisp = (dispManager_t *)handle;
	if(pDisp == NULL) return -1;

	close(pDisp->fdDisp);
	ioctl(pDisp->fdMem, CHUNK_MEM_FREE, &pDisp->memBlock);
	close(pDisp->fdMem);	
	free(pDisp);

	return SP_OK;
}

void 
displayResetScale(
	HANDLE handle
)
{
	dispManager_t *pDisp = (dispManager_t *)handle;
	gp_disp_scale_t osdScale;
	if(pDisp == NULL) return;
	memset(&osdScale, 0, sizeof(osdScale));
	osdScale.x = 0;
	osdScale.y = 0;
	osdScale.width = pDisp->resolution.width;
	osdScale.height = pDisp->resolution.height;
	osdScale.blankcolor = 0x00;
	if(ioctl(pDisp->fdDisp, DISPIO_SET_PRI_SCALEINFO, &osdScale) < 0) {
		printf("DISPIO_SET_PRI_SCALEINFO Fail!\n");
	}
}

void
dispGetResolution(
	HANDLE handle,
	gp_size_t *resolution
)
{
	dispManager_t *pDisp = (dispManager_t *)handle;
	if(pDisp == NULL) return;

	resolution->width = pDisp->resolution.width;
	resolution->height = pDisp->resolution.height;
}
void
dispGetPixelSize(
	HANDLE handle,
	int disp,
	gp_disp_pixelsize_t *pixelSize
)
{
	dispManager_t *pDisp = (dispManager_t *)handle;
	int ret = -1;
	if(pDisp == NULL) return;
	if(disp == 0) {
   		ret = ioctl(pDisp->fdDisp, DISPIO_GET_PANEL_PIXELSIZE, pixelSize);
	}

	if(ret < 0) {
		pixelSize->width = DAR_W;
		pixelSize->height = DAR_H;
	}
	return;
}
void dispCleanFramebuffer(HANDLE handle)
{
	
	dispManager_t *pDisp = (dispManager_t *)handle;
	int fbIndex;
	if(pDisp == NULL) return;

	fbIndex = pDisp->fbIndex ^ 1;
	printf("clean index %d\n", fbIndex);
	memset((UINT8*) pDisp->fb[fbIndex].pData, 0, pDisp->fb[fbIndex].width*pDisp->fb[fbIndex].height*2);
}

UINT8*
dispGetFramebuffer(
	HANDLE handle
)
{
	dispManager_t *pDisp = (dispManager_t *)handle;
	int fbIndex;
	if(pDisp == NULL) return -1;

	fbIndex = pDisp->fbIndex ^ 1;
	printf("get draw index %d\n", fbIndex);
	memset((UINT8*) pDisp->fb[fbIndex].pData, 0, pDisp->fb[fbIndex].width*pDisp->fb[fbIndex].height*2);
	return (UINT8*) pDisp->fb[fbIndex].pData;
}

void dispReUpdatePrimary(
	HANDLE handle
)
{
	dispManager_t *pDisp = (dispManager_t *)handle;
	if(pDisp == NULL) return;
	if (pDisp->layer == DISP_LAYER_PRIMARY) {
		displayResetScale(pDisp);
		//ioctl(pDisp->fdDisp, DISPIO_CHANGE_PRI_BITMAP_BUF, &pDisp->fb[pDisp->fbIndex]);
		ioctl(pDisp->fdDisp, DISPIO_SET_PRI_BITMAP, &pDisp->fb[pDisp->fbIndex]);
        ioctl(pDisp->fdDisp, DISPIO_SET_PRI_ENABLE, 1);	
		ioctl(pDisp->fdDisp, DISPIO_SET_UPDATE, 0);
		ioctl(pDisp->fdDisp, DISPIO_WAIT_FRAME_END, 0);
	}
}
void dispDisablePrimary(HANDLE handle)
{
	dispManager_t *pDisp = (dispManager_t *)handle;
	if(pDisp == NULL) return;
    ioctl(pDisp->fdDisp, DISPIO_SET_PRI_ENABLE, 0);	
}
void
dispFlip(
	HANDLE handle
)
{
	dispManager_t *pDisp = (dispManager_t *)handle;
	if(pDisp == NULL) return;

	pDisp->fbIndex = pDisp->fbIndex ^ 1;

	if (pDisp->layer == DISP_LAYER_PRIMARY) {
		//ioctl(pDisp->fdDisp, DISPIO_CHANGE_PRI_BITMAP_BUF, &pDisp->fb[pDisp->fbIndex]);
		ioctl(pDisp->fdDisp, DISPIO_SET_PRI_BITMAP, &pDisp->fb[pDisp->fbIndex]);
        ioctl(pDisp->fdDisp, DISPIO_SET_PRI_ENABLE, 1);	
		ioctl(pDisp->fdDisp, DISPIO_SET_UPDATE, 0);
		ioctl(pDisp->fdDisp, DISPIO_WAIT_FRAME_END, 0);
	}
	else {
		ioctl(pDisp->fdDisp, DISPIO_SET_OSD_BITMAP(0), &pDisp->fb[pDisp->fbIndex]);
		ioctl(pDisp->fdDisp, DISPIO_SET_OSD_ENABLE(0), 1);
		ioctl(pDisp->fdDisp, DISPIO_SET_UPDATE, 0);
		ioctl(pDisp->fdDisp, DISPIO_WAIT_FRAME_END, 0);		
	}
}

