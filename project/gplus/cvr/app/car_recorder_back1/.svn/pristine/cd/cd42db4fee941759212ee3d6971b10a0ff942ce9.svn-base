#include "ap_state_config.h"
#include "font.h"
#include "disp.h"
#include "playback_pipe.h"

extern iconManager_t Icon_Manager[];
extern menuManager_t DC_MENU[];
extern UINT8 setup_date_time[];
extern char dt_str[];
extern UINT8 flip_flag1;
extern HANDLE hDisp;
UINT8 Playmode_flag;
extern UINT8 cur_draw_flag;
extern UINT8 HDMIxNum;
#define STR_RES_POS_X -100
#define STR_RES_POS_Y 45
#define STR_DATETIME_POS_X 120
#define STR_DATETIME_POS_Y -22

#define STR_FILENAME_POX_X -100
#define STR_FILENAME_POX_Y 30

#define STR_FILESIZE_POX_X -100
#define STR_FILESIZE_POX_Y 45

#define STR_FILEINFO_POX_X -100
#define STR_FILEINFO_POX_Y -100 

#define STR_FILENUM_POX_X -120
#define STR_FILENUM_POX_Y -20

#define STR_SCALE_POX_X 32
#define STR_SCALE_POX_Y 32

#define STR_JPEG_SIZE

static char str_resolution[][10] = {"1080FHD","720P60FPS","720P30FPS","WVGA","VGA"};
static char date_str[][15] = {"2050 / 02 / 27", "02 / 27 / 2048", "27 / 02 / 2048"};
static char date1_str[][10] = {"YY/MM/DD","MM/DD/YY ","DD/MM/YY"};
static char time_str[] = "00 : 00 : 00";
static char scale_str[][8] = {"x1.4", "x1.9","x2.4", "x2.8", "x3.3", "x3.8", "x4.2", "x4.7", "x5.2", "x5.6", "x6.1", "x6.6", "x7.0", "x7.5", "x8.0"}; 
extern mqd_t menumodeMsgQ;
extern void DV_pipe_init(void);

extern dv_set_t dv_set;

extern int playback_menu_mode;
extern char slide_show_flag;

static cdvr_thmb_t *pdvr = NULL;
int draw_time = 2;

void PLAYBACK_UI_foreground(void);
extern void UI_Menu_draw(UINT8 mode);
extern void  DV_UI_Setting(void);
/*==========================================================
dv background UI draw function
===========================================================*/
#define SCALE_RECT_X 32
#define SCALE_RECT_Y 32
#define SCALE_RECT_W 96
#define SCALE_RECT_H 72
char scaleTable[SCALE_TAB] = {10, 14, 19, 24, 28, 33, 38, 42, 47, 52, 56, 61, 66, 70, 75, 80};
void playback_ui_DrawScale( UINT16 *fb, cdvr_thmb_t *pdvr)
{
	gp_size_t resolution;
	gp_rect_t rect;
	gp_rect_t rect2;
	char scl_x=1, scl_y=1;
	if(dv_set.change_disp != 0)
		return;	
	dispGetResolution(hDisp, &resolution);	
	scl_x = resolution.width/320;
	scl_y = resolution.height/240;

	rect.x = resolution.width - SCALE_RECT_X*scl_x;
	rect.y = SCALE_RECT_Y*scl_y;
	rect.width = SCALE_RECT_W*HDMIxNum;
	rect.height = SCALE_RECT_H*HDMIxNum;
	rect.x -= rect.width;
	//printf("{%dx%d} (%dx%d) %d %d\n", rect.x, rect.y, rect.width, rect.height, scl_x, scl_y);
	draw_rect(fb, &resolution, &rect, 2*HDMIxNum, 0x0, 0xffff);
	rect.x += 2*HDMIxNum;
	rect.y += 2*HDMIxNum;
	rect.width -= 2*2*HDMIxNum;
	rect.height -= 2*2*HDMIxNum;
	draw_rect(fb, &resolution, &rect, 2*HDMIxNum, 0xffff, 0xffff);

	rect2.width = (rect.width*10)/scaleTable[pdvr->sclIdx];
	rect2.height= (rect.height*10)/scaleTable[pdvr->sclIdx];

	rect2.x = rect.x + (rect.width-rect2.width)/2;
	rect2.y = rect.y + (rect.height-rect2.height)/2;
	//printf("{%dx%d} (%dx%d)\n", rect2.x, rect2.y, rect2.width, rect2.height);
	//draw_rect(fb, &resolution, &rect2, 1*HDMIxNum, 0xff00, 0xff00);
	fill_rectangle(fb,rect2.x,rect2.y,resolution.width,resolution.height,rect2.width,rect2.height,0xff00);
}

void playback_ui_wait(int str_idx)
{
	STRING_INFO sub_str;
	t_STRING_TABLE_STRUCT str_res;
	gp_size_t resolution;
	UINT16 *fb;
	UINT16* addr1;
	if(dv_set.change_disp != 0)
		return;
	dispGetResolution(hDisp, &resolution);	
	addr1 = fb = (UINT16*)dispGetFramebuffer(hDisp);
	for (int y = 0; y <resolution.height; y++) {		
		for (int x = 0; x < resolution.width; x++) {			
			*addr1++ = TRANSPARENT_COLOR;		
		}	
	}

    gp_Icon_draw(fb, (resolution.width-Icon_Manager[ICON_NOFILE].icon_width*HDMIxNum)/2,(resolution.height-Icon_Manager[ICON_NOFILE].icon_height*HDMIxNum)/2,resolution.width, resolution.height,ICON_NOFILE,Icon_Manager[ICON_NOFILE].tran_color);         
    sub_str.font_type = 0;        
   	sub_str.font_color = FONT_COLOR;
    sub_str.buff_w = resolution.width;
    sub_str.buff_h = resolution.height;        
    sub_str.language = setting_config_get(SET_LANGUAGE);
    sub_str.str_idx = str_idx;                
    ap_state_resource_string_resolution_get(&sub_str,&str_res);
    sub_str.pos_x = (resolution.width -str_res.string_width) /2;
    sub_str.pos_y = (resolution.height -str_res.string_height) /2;
    ap_state_resource_string_draw((INT16U *)fb, &sub_str);  

    return ;
}
void PLAYBACK_UI_foreground(void)
{
	STRING_ASCII_INFO ascii_str;
	STRING_INFO sub_str;
	t_STRING_TABLE_STRUCT str_res;	
	gp_size_t resolution;
	UINT16 *fb;
	UINT16* addr1;
	UINT8 buf[512];
	int dispMode;
	gp_size_t size;
	if(dv_set.change_disp != 0)
		return;
	dispGetResolution(hDisp, &resolution);	
	addr1 = fb = (UINT16*)dispGetFramebuffer(hDisp);
	dispMode = dvr_thmb_getDispMode(pdvr);
	if((dispMode != DISP_FULL_SCREEN) &&( dispMode != DISP_VIDEO_SCREEN))
	{
		for (int y = 0; y <resolution.height; y++) {		
			for (int x = 0; x < resolution.width; x++) {			
				*addr1++ = TRANSPARENT_COLOR;		
			}	
		}
	}
	if(dv_set.ui_show == 0)
	{
		dispFlip(hDisp);
		return;
	}
	if(dv_set.no_power_flag == 2)	
	{
		gp_Icon_draw(fb, (resolution.width-Icon_Manager[ICON_NOFILE].icon_width*HDMIxNum)/2,(resolution.height-Icon_Manager[ICON_NOFILE].icon_height*HDMIxNum)/2,resolution.width, resolution.height,ICON_NOFILE,Icon_Manager[ICON_NOFILE].tran_color);
		sub_str.font_type = 0;	
		sub_str.font_color = FONT_COLOR;
		sub_str.buff_w = resolution.width;
		sub_str.buff_h = resolution.height;	
		sub_str.language = setting_config_get(SET_LANGUAGE);
		sub_str.str_idx = STR_NO_POWER;		
		ap_state_resource_string_resolution_get(&sub_str,&str_res);
		sub_str.pos_x = (resolution.width -str_res.string_width) /2;
		sub_str.pos_y = (resolution.height -str_res.string_height) /2;
		ap_state_resource_string_draw((INT16U *)fb, &sub_str);		
		dispFlip(hDisp);
		return;		
	}

	ascii_str.font_color = 0xFFFF;
	ascii_str.font_type = 0;
	ascii_str.buff_w = resolution.width;
	ascii_str.buff_h = resolution.height;
	
	
	if(dispMode == DISP_THMB_SCREEN) {
		gp_rect_t rect;
		rect.x = pdvr->disp_thmb[pdvr->cur_idx].x-5;
		rect.y = pdvr->disp_thmb[pdvr->cur_idx].y-5;
		rect.width = pdvr->disp_thmb[pdvr->cur_idx].bitmap.width+5;
		rect.height = pdvr->disp_thmb[pdvr->cur_idx].bitmap.height+10;
		//printf("UI x %d y %d w %d h %d\n", rect.x, rect.y, rect.width,rect.height);
		draw_rect(fb, &resolution, &rect, 5, 0xffff, TRANSPARENT_COLOR);
		int i = 0;
		for(i=0; i<pdvr->cur_pageNum;i++) {
			if(pdvr->disp_thmb[i].lock == GP_FILE_LOCK) { //show lock icon
				//printf("index %d is lock \n", i)
				gp_Icon_draw(fb, pdvr->disp_thmb[i].x+pdvr->disp_thmb[i].bitmap.width-Icon_Manager[ICON_PROTECT_F].icon_width*2*HDMIxNum-5,pdvr->disp_thmb[i].y,resolution.width, resolution.height,ICON_PROTECT_F,Icon_Manager[ICON_PROTECT_F].tran_color);
			}

			if(pdvr->disp_thmb[i].type == GP_FILE_TYPE_VIDEO) { //show video icon
				
				gp_Icon_draw(fb, pdvr->disp_thmb[i].x+pdvr->disp_thmb[i].bitmap.width-Icon_Manager[ICON_PLAYBACK_P].icon_width*HDMIxNum-5,pdvr->disp_thmb[i].y,resolution.width, resolution.height,ICON_PLAYBACK_P,Icon_Manager[ICON_PLAYBACK_P].tran_color);
			}
			
		}
		ascii_str.pos_x = resolution.width + STR_FILENUM_POX_X*HDMIxNum;
		ascii_str.pos_y = resolution.height + STR_FILENUM_POX_Y*HDMIxNum;
		sprintf(buf, "%d/%d", dvr_thmb_getCurIdxOfFile(pdvr)+1, pdvr->file_number);
		ascii_str.str_ptr = buf;
		ap_state_resource_string_ascii_draw((UINT16 *)fb, &ascii_str);	
		draw_time = 2;
		//
		// show file name
		sprintf(buf, "%s", FilelistGetFileName(dvr_thmb_getCurIdxOfFile(pdvr)));
		//char *pExt = strchr(buf, '.');
		//*pExt = '\0';
		ascii_str.str_ptr = buf; 
		t_STRING_TABLE_STRUCT str_res;
		ap_state_resource_string_ascii_resolution_get(buf, &str_res);
		//printf("get width %d\n", str_res.string_width);
		ascii_str.pos_x = resolution.width - str_res.string_width - ICON_LEFT_X_POS*HDMIxNum + STR_FILENAME_POX_X* HDMIxNum - 60 ;
		ascii_str.pos_y = resolution.height + STR_FILENUM_POX_Y*HDMIxNum;
		fill_rectangle(fb,ascii_str.pos_x,ascii_str.pos_y ,resolution.width,resolution.height,str_res.string_width,str_res.string_height,TRANSPARENT_COLOR);
		ap_state_resource_string_ascii_draw((UINT16 *)fb, &ascii_str);	

	}
	else if(dispMode == DISP_FULL_SCREEN || dispMode == DISP_VIDEO_SCREEN){
	//	if(dispMode == DISP_FULL_SCREEN)
	   	{
			if(pdvr->pVideoInfo->videoMode == VIDEO_OPEN)
				draw_time = 3;

			if(draw_time > 0)
			{
				addr1 = fb;	
				for (int y = 0; y <resolution.height; y++) {		
					for (int x = 0; x < resolution.width; x++) {			
						*addr1++ = TRANSPARENT_COLOR;		
					}	
				}
				draw_time --;
			}
			//charge and battery 0: no power  1: zero battery 	2: one battery 3:two battery 4:three battery 5:charge  
			if(dv_set.battery_state == 0)
			{
				gp_Icon_draw(fb, resolution.width-Icon_Manager[ICON_BATTERYX].x_pos*HDMIxNum,resolution.height-Icon_Manager[ICON_BATTERYX].y_pos*HDMIxNum,resolution.width, resolution.height,ICON_BATTERYX,Icon_Manager[ICON_BATTERYX].tran_color);		
			}
			else if(dv_set.battery_state == 5)
			{
				if(flip_flag1 == 0)
				{
					gp_Icon_draw(fb, resolution.width-Icon_Manager[ICON_CHARGE].x_pos*HDMIxNum,resolution.height-Icon_Manager[ICON_CHARGE].y_pos*HDMIxNum,resolution.width, resolution.height,ICON_CHARGE,Icon_Manager[ICON_CHARGE].tran_color);
	
				}
				else
				{
					fill_rectangle(fb,resolution.width-Icon_Manager[ICON_CHARGE].x_pos*HDMIxNum,resolution.height-Icon_Manager[ICON_CHARGE].y_pos*HDMIxNum ,resolution.width,resolution.height,Icon_Manager[ICON_CHARGE].icon_width*HDMIxNum,Icon_Manager[ICON_CHARGE].icon_height*HDMIxNum,TRANSPARENT_COLOR);
				}		
			}
			else
			{
				if(dv_set.battery_state > 5)
				gp_Icon_draw(fb, resolution.width-Icon_Manager[ICON_CHARGE+4].x_pos*HDMIxNum,resolution.height-Icon_Manager[ICON_CHARGE+4].y_pos*HDMIxNum,resolution.width, resolution.height,ICON_CHARGE+4,Icon_Manager[ICON_CHARGE+4].tran_color);
				else
				gp_Icon_draw(fb, resolution.width-Icon_Manager[ICON_CHARGE+dv_set.battery_state].x_pos*HDMIxNum,resolution.height-Icon_Manager[ICON_CHARGE+dv_set.battery_state].y_pos*HDMIxNum,resolution.width, resolution.height,ICON_CHARGE+dv_set.battery_state,Icon_Manager[ICON_CHARGE+dv_set.battery_state].tran_color);
			}
		}	
		//resolution string
		//
		// show file name
		sprintf(buf, "%s", FilelistGetFileName(dvr_thmb_getCurIdxOfFile(pdvr)));
		//char *pExt = strchr(buf, '.');
		//*pExt = '\0';
		ascii_str.str_ptr = buf; 
		t_STRING_TABLE_STRUCT str_res;
		ap_state_resource_string_ascii_resolution_get(buf, &str_res);
		//printf("get width %d\n", str_res.string_width);
		ascii_str.pos_x = resolution.width - str_res.string_width - ICON_LEFT_X_POS*HDMIxNum;
		ascii_str.pos_y = ICON_HEIGHT*HDMIxNum - str_res.string_height;
		fill_rectangle(fb,ascii_str.pos_x,ascii_str.pos_y ,resolution.width,resolution.height,str_res.string_width,str_res.string_height,TRANSPARENT_COLOR);
		ap_state_resource_string_ascii_draw((UINT16 *)fb, &ascii_str);	

		//show file info
		dvr_thmb_getFileInfo(pdvr, buf);

		char *pTime = strchr(buf, ' ');
		*pTime = '\0';
		pTime++;

		//date
		ap_state_resource_string_ascii_resolution_get(buf, &str_res);
		ascii_str.pos_x = resolution.width - str_res.string_width - ICON_LEFT_X_POS*HDMIxNum;
		ascii_str.pos_y = resolution.height - ICON_HEIGHT*HDMIxNum*2 - str_res.string_height - ICON_LEFT_Y_POS;
		ascii_str.str_ptr = buf; 
		fill_rectangle(fb,ascii_str.pos_x,ascii_str.pos_y ,resolution.width,resolution.height,str_res.string_width,str_res.string_height,TRANSPARENT_COLOR);		
		ap_state_resource_string_ascii_draw((UINT16 *)fb, &ascii_str);	

		//time
		ap_state_resource_string_ascii_resolution_get(pTime, &str_res);
		ascii_str.pos_x = resolution.width - str_res.string_width - ICON_LEFT_X_POS*HDMIxNum;
		ascii_str.pos_y = resolution.height - ICON_HEIGHT*HDMIxNum*1 - str_res.string_height - ICON_LEFT_Y_POS;
		ascii_str.str_ptr = pTime; 
		fill_rectangle(fb,ascii_str.pos_x,ascii_str.pos_y ,resolution.width,resolution.height,str_res.string_width,str_res.string_height,TRANSPARENT_COLOR);
		ap_state_resource_string_ascii_draw((UINT16 *)fb, &ascii_str);	

		if(FilelistGetLock(pdvr->cur_idx)== GP_FILE_LOCK) { //show lock icon
			//printf("index %d is lock \n", i);
			gp_Icon_draw(fb, (ICON_LEFT_X_POS+ICON_WIDTH*2)*HDMIxNum, ICON_LEFT_Y_POS*HDMIxNum,resolution.width, resolution.height,ICON_PROTECT_F,Icon_Manager[ICON_PROTECT_F].tran_color);
		}

#if CVR_DUAL
		ascii_str.pos_x = resolution.width + STR_FILENUM_POX_X*HDMIxNum;
		ascii_str.pos_y = resolution.height + STR_FILENUM_POX_Y*HDMIxNum - ICON_LEFT_Y_POS;
		if(pdvr->fileStat == 1) {
			sprintf(buf, "B %d/%d", dvr_thmb_getCurIdxOfFile(pdvr)+1, pdvr->file_number);
		}
		else {
			sprintf(buf, "A %d/%d", dvr_thmb_getCurIdxOfFile(pdvr)+1, pdvr->file_number);
		}
		ascii_str.str_ptr = buf;
		ap_state_resource_string_ascii_draw((UINT16 *)fb, &ascii_str);	
#endif

	
		if(pdvr->fileType == GP_FILE_TYPE_JPEG) {
			dvr_thmb_getFileResourse(pdvr, &size);
			sprintf(buf, "%dx%d", size.width, size.height);
			if(dv_set.display_mode == SP_DISP_OUTPUT_HDMI)
			ascii_str.pos_x = resolution.width + STR_FILESIZE_POX_X*HDMIxNum - 40;
			else
			ascii_str.pos_x = resolution.width + STR_FILESIZE_POX_X;	
			ascii_str.pos_y = STR_FILESIZE_POX_Y*HDMIxNum;
			ascii_str.str_ptr = buf;
			ap_state_resource_string_ascii_resolution_get(buf,&str_res);
			fill_rectangle(fb,ascii_str.pos_x,ascii_str.pos_y ,resolution.width,resolution.height,str_res.string_width,str_res.string_height,TRANSPARENT_COLOR);
			ap_state_resource_string_ascii_draw((UINT16 *)fb, &ascii_str);	
			gp_Icon_draw(fb, ICON_LEFT_X_POS*HDMIxNum, ICON_LEFT_Y_POS*HDMIxNum,resolution.width, resolution.height,ICON_PLAYBACK_F,Icon_Manager[ICON_PLAYBACK_F].tran_color);

		}
		else if(pdvr->fileType == GP_FILE_TYPE_VIDEO) {
			dispMode = dvr_thmb_getDispMode(pdvr);
			if(dispMode == DISP_FULL_SCREEN) {
				dvr_thmb_getFileResourse(pdvr, &size);
				int rsl = 0;
				if(size.height == 1080) {
					rsl = 0;
				}
				else if(size.height == 720 && dvr_thmb_getFrameRate(pdvr) > 40){ //p60
					rsl = 1;
				}
				else if(size.height == 720) { //p30
					rsl = 2;
				}
				else if(size.height == 480 && size.width > 640) {
					rsl = 3;
				}
				else if(size.height == 480 && size.width == 640) {
					rsl = 4;
				}
				ascii_str.pos_x = resolution.width + STR_RES_POS_X*HDMIxNum;
				ascii_str.pos_y = STR_RES_POS_Y*HDMIxNum;
				ascii_str.str_ptr = str_resolution[rsl];
				ap_state_resource_string_ascii_resolution_get(&str_resolution[rsl],&str_res);
				fill_rectangle(fb,ascii_str.pos_x,ascii_str.pos_y ,resolution.width,resolution.height,str_res.string_width,str_res.string_height,TRANSPARENT_COLOR);
				ap_state_resource_string_ascii_draw((UINT16 *)fb, &ascii_str);	
			}
			else if(pdvr->pVideoInfo->play_time >= 0){ //show play time
				//printf("%s:%d show time %ld\n", __FUNCTION__, __LINE__, pdvr->pVideoInfo->play_time);
				int h, m, s;
				long t = pdvr->pVideoInfo->play_time;
				if(t<0) t=0;
				h = t/(60*60);
				m = (t - h*60*60)/60;
				s = (t-h*60*60)%60;
				//printf("%d%d:%d%d:%d%d\n", h/10, h%10, m/10, m%10, s/10, s%10);
				sprintf(buf, "%d%d:%d%d:%d%d", h/10, h%10, m/10, m%10, s/10, s%10);
				ascii_str.pos_x = resolution.width + STR_RES_POS_X*HDMIxNum;
				ascii_str.pos_y = STR_RES_POS_Y*HDMIxNum;
				ascii_str.str_ptr = buf;
				ap_state_resource_string_ascii_resolution_get(buf,&str_res);
				fill_rectangle(fb,ascii_str.pos_x,ascii_str.pos_y ,resolution.width,resolution.height,str_res.string_width,str_res.string_height,TRANSPARENT_COLOR);
				ap_state_resource_string_ascii_draw((UINT16 *)fb, &ascii_str);	
			}

			gp_Icon_draw(fb, ICON_LEFT_X_POS*HDMIxNum, ICON_LEFT_Y_POS*HDMIxNum,resolution.width, resolution.height,ICON_PLAYBACK_P,Icon_Manager[ICON_PLAYBACK_P].tran_color);
			
			//play status
			if(pdvr->pVideoInfo->videoMode == VIDEO_OPEN) {
				gp_Icon_draw(fb, ICON_LEFT_X_POS*HDMIxNum, resolution.height-ICON_HEIGHT*HDMIxNum-ICON_LEFT_Y_POS,resolution.width, resolution.height,ICON_PLAY_START,Icon_Manager[ICON_PLAY_START].tran_color);
			}
			else {
				if(pdvr->pVideoInfo->videoMode == VIDEO_PAUSE) {
					gp_Icon_draw(fb, ICON_LEFT_X_POS*HDMIxNum, resolution.height-ICON_HEIGHT*HDMIxNum-ICON_LEFT_Y_POS,resolution.width, resolution.height,ICON_PLAY_PAUSE,Icon_Manager[ICON_PLAY_PAUSE].tran_color);
				}
				else {
					gp_Icon_draw(fb, ICON_LEFT_X_POS*HDMIxNum, resolution.height-ICON_HEIGHT*HDMIxNum-ICON_LEFT_Y_POS,resolution.width, resolution.height,ICON_PLAY_START,Icon_Manager[ICON_PLAY_START].tran_color);
				}
				
				//show volume icon
				gp_Icon_draw(fb, ICON_LEFT_X_POS*HDMIxNum, resolution.height-ICON_HEIGHT*2*HDMIxNum-ICON_LEFT_Y_POS,resolution.width, resolution.height,ICON_VOLUME,Icon_Manager[ICON_VOLUME].tran_color);
				#define VO_START 40
				#define VO_OFFSET 11
				int j;
				for(j=0; j<pdvr->pVideoInfo->volume; j++) {
					gp_Icon_draw(fb, (ICON_LEFT_X_POS+VO_START-13+j*VO_OFFSET)*HDMIxNum, resolution.height-ICON_HEIGHT*2*HDMIxNum-ICON_LEFT_Y_POS-1*HDMIxNum,resolution.width, resolution.height,ICON_VOLUME1,Icon_Manager[ICON_VOLUME1].tran_color-1);
				}
				
				ascii_str.pos_x = ICON_LEFT_Y_POS +Icon_Manager[ICON_VOLUME].icon_width*HDMIxNum + ICON_WIDTH;
				ascii_str.pos_y = resolution.height-ICON_HEIGHT*2*HDMIxNum-ICON_LEFT_Y_POS;
				sprintf(buf, "-%dX", 1<<pdvr->pVideoInfo->speed);
				ascii_str.str_ptr = buf;
				ap_state_resource_string_ascii_resolution_get(buf,&str_res);				
				fill_rectangle(fb,ascii_str.pos_x,ascii_str.pos_y ,resolution.width,resolution.height,str_res.string_width,str_res.string_height,TRANSPARENT_COLOR);
				if((pdvr->pVideoInfo->videoMode == VIDEO_SPEED ||(pdvr->pVideoInfo->videoMode == VIDEO_PAUSE&&
						pdvr->pVideoInfo->lastMode == VIDEO_SPEED)) && pdvr->pVideoInfo->speed > 0) {
					ascii_str.pos_x = ICON_LEFT_Y_POS +Icon_Manager[ICON_VOLUME].icon_width*HDMIxNum + ICON_WIDTH;
					ascii_str.pos_y = resolution.height-ICON_HEIGHT*2*HDMIxNum-ICON_LEFT_Y_POS;
					sprintf(buf, "%dX", 1<<pdvr->pVideoInfo->speed);
					ascii_str.str_ptr = buf;
					ap_state_resource_string_ascii_draw((UINT16 *)fb, &ascii_str);	
				}
				else if((pdvr->pVideoInfo->videoMode == VIDEO_REVERSE ||(pdvr->pVideoInfo->videoMode == VIDEO_PAUSE&&
						pdvr->pVideoInfo->lastMode == VIDEO_REVERSE)) && pdvr->pVideoInfo->speed > 0) {
					ascii_str.pos_x = ICON_LEFT_Y_POS +Icon_Manager[ICON_VOLUME].icon_width*HDMIxNum + ICON_WIDTH;
					ascii_str.pos_y = resolution.height-ICON_HEIGHT*2*HDMIxNum-ICON_LEFT_Y_POS;
					sprintf(buf, "-%dX", 1<<pdvr->pVideoInfo->speed);
					ascii_str.str_ptr = buf;
					ap_state_resource_string_ascii_draw((UINT16 *)fb, &ascii_str);	
				}
			}

		}
	}
	else if(dispMode == DISP_SCALE_SCREEN) {
		if(pdvr->sclIdx > 0) {
			ascii_str.pos_x = STR_SCALE_POX_X*HDMIxNum;
			ascii_str.pos_y = resolution.height - STR_SCALE_POX_Y*HDMIxNum;
			ascii_str.str_ptr = scale_str[pdvr->sclIdx-1];
			ap_state_resource_string_ascii_draw((UINT16 *)fb, &ascii_str);	
			playback_ui_DrawScale(fb, pdvr);
		}
		draw_time = 2;
	}
	else if(dispMode == DISP_NO_FILE) {
		playback_ui_wait(STR_NO_FILE);
		draw_time = 2;
	}
	if((cur_draw_flag ==DV_SETTING_MODE)||(cur_draw_flag ==DV_EXIT_MODE))
	{
		disp_OSD0_Set_Alpha(hDisp,100);
		cur_draw_flag =DV_EXIT_MODE+1;
	}	
	dispFlip(hDisp);
	
	
}
static int playback_menu_active( cdvr_thmb_t *pdvr)
{
	printf("get menu mode: %d\n", playback_menu_mode);
	if(playback_menu_mode == PLAYBACK_MENU_UNKNOWN) {
		return -1;
	}
	if(playback_menu_mode == PLAYBACK_MENU_DELETE_NONE) {
		return -1;
	}
	switch(playback_menu_mode) {
		case PLAYBACK_MENU_DELETE_ALL:
		{
			playback_ui_wait(STR_PLEASE_WAIT);
			dispFlip(hDisp);
			playback_menu_mode = PLAYBACK_MENU_WAIT;
			FilelistSetDeleteAll();
		}
		break;
		case PLAYBACK_MENU_DELETE_CUR:
		{
			if(FilelistGetLock(pdvr->cur_idx)== GP_FILE_LOCK) { //show lock icon
				playback_ui_wait(STR_FILE_PROTECT);
				dispFlip(hDisp);
				playback_menu_mode = PLAYBACK_MENU_UNKNOWN;
				sleep(1);
			}
			else {
				playback_ui_wait(STR_PLEASE_WAIT);
				dispFlip(hDisp);
				playback_menu_mode = PLAYBACK_MENU_UNKNOWN;
				FilelistSetDelete();
				dvr_thmb_UpdateAllInfo(pdvr);
			}
		}
		break;
		case PLAYBACK_MENU_LOCK_ALL:
		{
			FilelistSetLockAll();
			playback_menu_mode = PLAYBACK_MENU_UNKNOWN;
		}
		break;
		case PLAYBACK_MENU_UNLOCK_ALL:
		{
			FilelistSetUnLockAll();
			playback_menu_mode = PLAYBACK_MENU_UNKNOWN;
		}
		break;
		case PLAYBACK_MENU_LOCK_CUR:
		{
			FilelistSetLock(dvr_thmb_getCurIdxOfFile(pdvr));
			playback_menu_mode = PLAYBACK_MENU_UNKNOWN;
		}
		break;
		case PLAYBACK_MENU_UNLOCK_CUR:
		{
			FilelistSetUnLock(dvr_thmb_getCurIdxOfFile(pdvr));
			playback_menu_mode = PLAYBACK_MENU_UNKNOWN;
		}
		break;
		default:
			break;
	}

	return 0;
}
/*==========================================================
dv task entry function
===========================================================*/
int
PLAYBACK_task_entry(
	int argc,
	char **argv
)
{
	UINT16 i;
	gp_size_t resolution;
	UINT8* addr;
	UINT16* addr1;
	STRING_ASCII_INFO ascii_str;
	STRING_INFO str;
	UINT16 charge_flag = 1;
	int ret;
	UINT16 *fb;
	UINT8 msg_id,msg_prio;
	UINT8 playback_end = 0;
	UINT8 playback_mode;
	int drawMenu = -1;
	int modeKey = -1;
	int key = -1;
	char format_flag = 0;
	struct timespec timeout;
	
	//run dv and init pipe
	//DV_pipe_init();	
	draw_time = 2;
	while(dv_set.change_disp == 1);
	dv_set.zoom_flag = 0;
	dv_set.zoom_num = 0;	
	dispGetResolution(hDisp, &resolution);
	printf("@%d %s() resolution.width=%d,resolution.height=%d\n",__LINE__,__func__,resolution.width,resolution.height);	
	if(dv_set.display_mode == SP_DISP_OUTPUT_HDMI) { //select video decoder display output
		ret = 6;
	}
	else if(dv_set.display_mode == SP_DISP_OUTPUT_TV) {
		ret = 4;
	}
	else {
		ret = 0;
	}
	//init UI for DV
	playback_mode = DV_FOREGROUND_MODE;
	pdvr = playback_init(ret, resolution.width, resolution.height);
	do
	{
		clock_gettime(CLOCK_REALTIME, &timeout);
		timeout.tv_nsec += 10000;
		ret = mq_timedreceive(menumodeMsgQ, (char *)&msg_id, 1, (unsigned int *)&msg_prio,&timeout);
	}
	while(ret != -1);
	dv_set.dv_UI_flag = DV_FOREGROUND_MODE;
	PLAYBACK_UI_foreground();
	printf("%s:%d\n", __FUNCTION__, __LINE__);
	if(dv_set.display_mode == SP_DISP_OUTPUT_LCD)
	{
		LCD_Backlight_Set(1);
	}
	while(!playback_end)
	{

		ret = mq_receive(menumodeMsgQ, (char *)&msg_id, 1, (UINT32 *)&msg_prio);
		switch(msg_id){
			case DV_FOREGROUND_MODE:
				dv_set.draw_start = 1;
				playback_mode = DV_FOREGROUND_MODE;
				PLAYBACK_UI_foreground();
				dv_set.draw_start = 0;
			break;
			case DV_MENU_MODE:
		
				printf("%s:%d drawMenu %d\n", __FUNCTION__, __LINE__, drawMenu);
				if(drawMenu == 0){
					if((key != DV_UP_KEY && key != DV_DOWN_KEY && key != DV_MENU_KEY) && dvr_thmb_getDispMode(pdvr) == DISP_NO_FILE
							&& dv_set.menu_select_count != 2) {
						playback_ui_wait(STR_NO_FILE);
						dispFlip(hDisp);
						dv_set.menu_select_flag = 0; // not enter sub window
						usleep(500*1000);
					}
					if(dvr_thmb_getDispMode(pdvr) == DISP_FULL_SCREEN || 
							dvr_thmb_getDispMode(pdvr) == DISP_NO_FILE) {
						playback_mode = DV_MENU_MODE;
						playback_menu_active(pdvr);
						if(playback_menu_mode != PLAYBACK_MENU_WAIT) {
							disp_OSD0_Set_Alpha(hDisp,65);
							UI_Menu_draw(Playmode_flag);
						}
					}
				}
				else {
					printf("%s:%d\n", __FUNCTION__, __LINE__);
					dv_set.dv_UI_flag = DV_FOREGROUND_MODE;
					PLAYBACK_UI_foreground();
				}
			break;
			case DV_SETTING_MODE:
				if(dvr_thmb_getDispMode(pdvr) == DISP_FULL_SCREEN || 
						dvr_thmb_getDispMode(pdvr) == DISP_NO_FILE) {
					playback_mode = DV_SETTING_MODE;
					printf("%s:%d\n", __FUNCTION__, __LINE__);
					if(dv_set.sd_check == 3) {//sd format flag.
						printf("%s:%d SD Format!!!\n", __FUNCTION__, __LINE__);
						format_flag = 1;
						
					}
					if(format_flag == 1 && dv_set.sd_check == 1) {
						format_flag = 0;
						FilelistRebuild();
						dvr_thmb_get_info();
						FilelistGetPageLock();
						pdvr->pUrl = pdvr->pPath = NULL;
						pdvr->pUrl = FilelistGetFileName(0);
						pdvr->pPath = FilelistGetFilePath(0);
					}
					DV_UI_Setting();
				}
				else {
					dv_set.dv_UI_flag = DV_FOREGROUND_MODE;
				}
			break;
			case DV_EXIT_MODE:
				if((playback_mode_checkVideoStatus(pdvr) == 0 && modeKey == 0) || 
					Playmode_flag >= CMD_USB_MODE||dv_set.change_disp == 1){
					printf("%s:%d\n", __FUNCTION__, __LINE__);
					playback_mode = DV_EXIT_MODE;
					playback_end = 1;
					dvr_thmb_mutex_lock();
					slide_show_flag = 0;
					dvr_thmb_mutex_unlock();
					if(dv_set.sd_check == 0) {
						printf("sdcard had remove! send msg to stop cvr_play!\n");
						
					}
					printf("%s:%d exit slide mode!!\n", __FUNCTION__, __LINE__);
				}
				else {
					printf("%s:%d\n", __FUNCTION__, __LINE__);
					dv_set.dv_UI_flag = DV_FOREGROUND_MODE;
					Playmode_flag = CMD_PLAYBACK_MODE;
					PLAYBACK_UI_foreground();
				}
			break;
			default:
			break;	
		}
		key = msg_id;
		if(!playback_end && playback_mode == DV_FOREGROUND_MODE) {

			switch(msg_id) {
				case DV_UP_KEY:
				{
					if(slide_show_flag) {
						break;
					}
					printf("%s:%d\n", __FUNCTION__, __LINE__);
					playback_up_key_active(pdvr);
				}
				dvr_thmb_printfInfo(pdvr);
			break;
				case DV_UP_L_KEY:
				{
					if(slide_show_flag) {
						break;
					}
					printf("%s:%d\n", __FUNCTION__, __LINE__);
					playback_up_L_key_active(pdvr);
				}
				dvr_thmb_printfInfo(pdvr);
				break;
				case DV_DOWN_KEY:
					if(slide_show_flag) {
						break;
					}
					printf("%s:%d\n", __FUNCTION__, __LINE__);
					playback_down_key_active(pdvr);
					dvr_thmb_printfInfo(pdvr);
				break;
				case DV_DOWN_L_KEY:
					if(slide_show_flag) {
						break;
					}
					printf("%s:%d\n", __FUNCTION__, __LINE__);
					playback_down_L_key_active(pdvr);
					dvr_thmb_printfInfo(pdvr);
				break;
				case DV_MENU_KEY:
					if(slide_show_flag) {
						dvr_thmb_mutex_lock();
						slide_show_flag = 0;
						dvr_thmb_mutex_unlock();
						printf("%s:%d exit slide mode!!\n", __FUNCTION__, __LINE__);
						drawMenu = -1;
						break;
					}
					printf("%s:%d\n", __FUNCTION__, __LINE__);
					drawMenu = playback_menu_key_active(pdvr);
					dvr_thmb_printfInfo(pdvr);
				break;
				case DV_ENTER_KEY:
					if(slide_show_flag) {
						dvr_thmb_mutex_lock();
						slide_show_flag = 0;
						dvr_thmb_mutex_unlock();
						printf("%s:%d exit slide mode!!\n", __FUNCTION__, __LINE__);
						break;
					}
					printf("%s:%d\n", __FUNCTION__, __LINE__);
					playback_enter_key_active(pdvr);
					dvr_thmb_printfInfo(pdvr);
				break;
				case DV_MODE_KEY:
					printf("%s:%d\n", __FUNCTION__, __LINE__);
					modeKey = playback_mode_key_active(pdvr);
				dvr_thmb_printfInfo(pdvr);
				break;
				default:
				break;	
			}
		}
		else if(playback_mode == DV_MENU_MODE || playback_mode == DV_SETTING_MODE) {
					printf("%s:%d\n", __FUNCTION__, __LINE__);
			if(msg_id == DV_MODE_KEY) {
					printf("%s:%d\n", __FUNCTION__, __LINE__);
				modeKey = 0;
			}
		}
		/*if(msg_id == DV_EXIT_MODE)
		{
	printf("%s:%d\n", __FUNCTION__, __LINE__);
			break;
		}*/
					//pend message from TimeKeyThread and update UI/send message to ipcam
	}
	printf("%s:%d\n", __FUNCTION__, __LINE__);
	//pipeDelete();	
	playback_uninit(pdvr);
	pdvr = NULL;
	return 0;
}

