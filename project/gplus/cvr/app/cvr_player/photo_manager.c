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
#include "scaler.h"
#include "image_decode.h"
#include <mach/gp_chunkmem.h>
#include "photo_manager.h"
#include "playback_demo.h"


int scaleTable[SCALE_TAB] = {100, 140, 190, 240, 280, 330, 380, 420, 470, 520, 560, 610, 660, 700, 750, 800};


/**
 * load jpeg image, and using hw to decode jpeg file. if hw decode error, using 
 * soft jpeg decode. using need to call free_image() to free buffer.
 * param: @filename, jpeg file name.
 * param: @rect, to save jpeg width and height.
 * return: jpeg data point.
 **/
static UINT8* load_image(void *filename, gp_size_t *rect)
{
	return (UINT8*)Gdjpegdecode(filename, NULL, 0, 0, rect, 0);
}

int load_imageThumbnail(void *filename,cdvr_thmb_t *pdvr , disp_thmb_t *pthmb, UINT8* disp_pData)
{
	gp_size_t size;
	
	pthmb->bitmap.pData = disp_pData;
	UINT8 *pTbData = (UINT8*)Gdjpegdecode(filename, &pthmb->bitmap, pthmb->bitmap.width, pthmb->bitmap.height, &size, 1);
	if(!pTbData) {
		printf("jpeg get thumbnail error!\n");
		return -1;
	}

	return 0;	
}

/**
 * free image data malloc by load_image().
 * param: @pSrcData, jpeg data point.
 * return 0;
 **/
int free_image(void *pSrcData) 
{
	gpChunkMemFree(pSrcData);
	return 0;
}

/**
 * photo info init.
 * param: @phInfo, photo_Info_t point.
 * return ;
 **/
photo_Info_t *photo_init(char *filename, gp_bitmap_t *pbitmap)
{
	photo_Info_t*pInfo = malloc(sizeof(photo_Info_t));
	if(!pInfo) {
		printf("%s:%d\n", __FUNCTION__, __LINE__);
		return NULL;
	}
	memset((char *)pInfo, 0, sizeof(photo_Info_t));
	pInfo->pSrcData = load_image(filename, &pInfo->srcSize);
	if(!pInfo->pSrcData) {
		printf("%s:%d\n", __FUNCTION__, __LINE__);
		printf("load_image error!!!!\n");
		free(pInfo);
		return NULL;
	}
	printf("load_image ok!!!! src w %d h %d \n", pInfo->srcSize.width, pInfo->srcSize.height);

	pInfo->bitmap = pbitmap;

	pInfo->sclIdx = 0;

	return pInfo;
}

/**
 * photo uninit.
 **/
int photo_uninit(photo_Info_t *pInfo)
{
	if(!pInfo) {
		return -1;
	}

	if(pInfo->pSrcData) {
		gpChunkMemFree(pInfo->pSrcData);
	}

	pInfo->bitmap->pData = NULL;
	free(pInfo);
	return 0;
}

/**
 * image scale function. it will malloc a chunk memory to save data.
 * param: @phInfo, photo_Info_t point.
 * return: 0: success, -1: fail
 **/
int image_scale(photo_Info_t *pInfo)
{
	//UINT8* scl_data = NULL;
	int size = 0;
	if(!pInfo) {
		return -1;
	}
	printf("src:0x%x, dst 0x%x w %d h %d\n", pInfo->pSrcData, pInfo->bitmap->pData, pInfo->srcSize.width, pInfo->srcSize.height);
	printf("image scale display x %d y %d w %d h %d\n", pInfo->bitmap->validRect.x, pInfo->bitmap->validRect.y, pInfo->bitmap->width, pInfo->bitmap->height);

	if (scaler_process_zoom((unsigned int)pInfo->pSrcData, pInfo->srcSize.width, pInfo->srcSize.height, (unsigned int)pInfo->bitmap->pData, pInfo->bitmap->width, pInfo->bitmap->height, SP_BITMAP_RGB565, SP_BITMAP_RGB565, scaleTable[pInfo->sclIdx])){
		return -1;
	}

	return 0;
}

#define MOVE_STEMP (10)
/**
 * move scale point up.
 * param: @phInfo, photo_Info_t point.
 **/
int image_move_up(photo_Info_t *pInfo) 
{
	if(!pInfo) {
		return -1;
	}
	if(!pInfo->bitmap->pData) {
		return -1;
	}

	return 0;
}
/**
 * move scale point down.
 * param: @phInfo, photo_Info_t point.
 **/
int image_move_down(photo_Info_t *pInfo) 
{
	if(!pInfo) {
		return -1;
	}
	if(!pInfo->bitmap->pData) {
		return -1;
	}

	return 0;
}
/**
 * move scale point right.
 * param: @phInfo, photo_Info_t point.
 **/
int image_move_right(photo_Info_t *pInfo) 
{
	if(!pInfo) {
		return -1;
	}
	if(!pInfo->bitmap->pData) {
		return -1;
	}

	return 0;
}
/**
 * move scale point left.
 * param: @phInfo, photo_Info_t point.
 **/
int image_move_left(photo_Info_t *pInfo) 
{
	if(!pInfo) {
		return -1;
	}
	if(!pInfo->bitmap->pData) {
		return -1;
	}


	return 0;
}

/**
 * image scale up.
 * param: @phInfo, photo_Info_t point.
 **/
int image_scale_up(photo_Info_t *pInfo)
{
	gp_rect_t rect;
	int ret = -1;
	if(!pInfo) {
		return -1;
	}

	pInfo->sclIdx++;
	if(pInfo->sclIdx >= SCALE_TAB) {
		pInfo->sclIdx = SCALE_TAB-1;
	}
    printf("%s:%d\n", __FUNCTION__, __LINE__);
/*
	memcpy(&rect, &pInfo->scl_rect, sizeof(gp_rect_t));

	pInfo->scl_rect.width = scaleTable[pInfo->sclIdx] * pInfo->bitmap->width/10;
	pInfo->scl_rect.height = scaleTable[pInfo->sclIdx] * pInfo->bitmap->height/10;
	pInfo->scl_rect.x = (pInfo->scl_rect.width - pInfo->bitmap->width)/2;
	pInfo->scl_rect.y = (pInfo->scl_rect.height- pInfo->bitmap->height)/2;

	pInfo->scl_rect.x = (pInfo->scl_rect.x>>1)<<1;
	pInfo->scl_rect.y = (pInfo->scl_rect.y>>1)<<1;
	pInfo->scl_rect.width = (pInfo->scl_rect.width+0x07)&~0x07;
	pInfo->scl_rect.height= (pInfo->scl_rect.height+0x07)&~0x07;*/

	ret = image_scale(pInfo);
	if(ret != 0) {
		pInfo->sclIdx--;
		//memcpy(&pInfo->scl_rect, &rect, sizeof(gp_rect_t));
	}

	return ret;

}

/**
 * image scale down.
 * param: @phInfo, photo_Info_t point.
 **/
int image_scale_down(photo_Info_t *pInfo)
{
	gp_rect_t rect;
	int ret = -1;
	if(!pInfo) {
		return -1;
	}

	pInfo->sclIdx--;
	if(pInfo->sclIdx < 0 ) {
		pInfo->sclIdx = 0;
	}
    printf("%s:%d\n", __FUNCTION__, __LINE__);
	/*memcpy(&rect, &pInfo->scl_rect, sizeof(gp_rect_t));

	pInfo->scl_rect.width = scaleTable[pInfo->sclIdx] * pInfo->bitmap->width/10;
	pInfo->scl_rect.height = scaleTable[pInfo->sclIdx] * pInfo->bitmap->height/10;
	pInfo->scl_rect.x = (pInfo->scl_rect.width - pInfo->bitmap->width)/2;
	pInfo->scl_rect.y = (pInfo->scl_rect.height- pInfo->bitmap->height)/2;

	pInfo->scl_rect.x = (pInfo->scl_rect.x>>1)<<1;
	pInfo->scl_rect.y = (pInfo->scl_rect.y>>1)<<1;
	pInfo->scl_rect.width = (pInfo->scl_rect.width+0x07)&~0x07;
	pInfo->scl_rect.height = (pInfo->scl_rect.height+0x07)&~0x07;*/

	ret = image_scale(pInfo);
	if(ret != 0) {
		pInfo->sclIdx++;
		//memcpy(&pInfo->scl_rect, &rect, sizeof(gp_rect_t));
	}

	return ret;
}

