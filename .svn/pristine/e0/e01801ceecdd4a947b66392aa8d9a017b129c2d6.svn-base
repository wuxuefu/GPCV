#ifndef __PHOTO_MANAGER_H__
#define __PHOTO_MANAGER_H__

#include "playback_demo.h"



int load_imageThumbnail(void *filename, cdvr_thmb_t *pdvr, disp_thmb_t *pthmb, UINT8* disp_pData);
photo_Info_t *photo_init(char *filename, gp_bitmap_t *pbitmap);
int free_image(void *pSrcData);
photo_Info_t *photo_init(char *filename, gp_bitmap_t *pbitmap);
int photo_uninit(photo_Info_t *pInfo);

int image_scale(photo_Info_t *pInfo);
int image_move_up(photo_Info_t *pInfo);
int image_move_down(photo_Info_t *pInfo);
int image_move_right(photo_Info_t *pInfo);
int image_move_left(photo_Info_t *pInfo);
int image_scale_up(photo_Info_t *pInfo);
int image_scale_down(photo_Info_t *pInfo);

#endif
