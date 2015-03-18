#include "ap_state_config.h"
#include "font.h"
#include "disp.h"
#include "cvr_pipe.h"
extern iconManager_t Icon_Manager[];
extern menuManager_t DC_MENU[];
extern UINT8 setup_date_time[];
extern char dt_str[];
extern HANDLE hDisp;
extern UINT8 Playmode_flag;
static UINT32 pic_size[3][8]={
		{1410,1220,1030,753,529,385,88,227}, //fine
		{1120,996,844,603,433,310,79,195}, //normal
		{810,692,577,396,273,142,43,95}//economy
};
static char str_dc_resolution[][5] = {"12M","10M","8M","5M","3M","2MHD","VGA","1.3M"};
static char pic_free[] = "00000";
static char zoom[] = "x0.0";
extern UINT8 flip_flag1;
extern mqd_t menumodeMsgQ;
extern void DV_pipe_init(void);
extern UINT32 time_num;
static UINT8 cap_time[2];
extern dv_set_t dv_set;
extern UINT8 cur_draw_flag;
UINT32 free_size;
UINT32 free_size_flag = 0;
extern UINT8 HDMIxNum;
void DC_UI_foreground(void);
extern void UI_Menu_draw(UINT8 mode);
extern void DV_UI_Setting(void);
/*==========================================================
dv background UI draw function
===========================================================*/
void DC_UI_foreground(void)
{
	STRING_INFO sub_str;
	STRING_ASCII_INFO ascii_str;
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
	//printf("flip_flag = %d\n",flip_flag);
	if(dv_set.ui_show == 0)
	{
		printf("dv_set.no_power_flag = %d\n",dv_set.no_power_flag);
		dispFlip(hDisp);
		return;
	}
	if(dv_set.no_power_flag == 2)	
	{
		printf("dv_set.no_power_flag = %d\n",dv_set.no_power_flag);
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
	else if(dv_set.sd_check == 2)
	{	
		gp_Icon_draw(fb, (resolution.width-Icon_Manager[ICON_NOFILE].icon_width*HDMIxNum)/2,(resolution.height-Icon_Manager[ICON_NOFILE].icon_height*HDMIxNum)/2,resolution.width, resolution.height,ICON_NOFILE,Icon_Manager[ICON_NOFILE].tran_color);
		sub_str.font_type = 0;	
		sub_str.font_color = FONT_COLOR;
		sub_str.buff_w = resolution.width;
		sub_str.buff_h = resolution.height;	
		sub_str.language = setting_config_get(SET_LANGUAGE);
		sub_str.str_idx = STR_INSERT_SDC;		
		ap_state_resource_string_resolution_get(&sub_str,&str_res);
		sub_str.pos_x = (resolution.width -str_res.string_width) /2;
		sub_str.pos_y = (resolution.height -str_res.string_height) /2;
		ap_state_resource_string_draw((INT16U *)fb, &sub_str);		
		dispFlip(hDisp);
		return;		
	}	
	else if(dv_set.sdc_full == 2)
	{	
		gp_Icon_draw(fb, (resolution.width-Icon_Manager[ICON_NOFILE].icon_width*HDMIxNum)/2,(resolution.height-Icon_Manager[ICON_NOFILE].icon_height*HDMIxNum)/2,resolution.width, resolution.height,ICON_NOFILE,Icon_Manager[ICON_NOFILE].tran_color);
		sub_str.font_type = 0;	
		sub_str.font_color = FONT_COLOR;
		sub_str.buff_w = resolution.width;
		sub_str.buff_h = resolution.height;	
		sub_str.language = setting_config_get(SET_LANGUAGE);
		sub_str.str_idx = STR_SD_FULL;		
		ap_state_resource_string_resolution_get(&sub_str,&str_res);
		sub_str.pos_x = (resolution.width -str_res.string_width) /2;
		sub_str.pos_y = (resolution.height -str_res.string_height) /2;
		ap_state_resource_string_draw((INT16U *)fb, &sub_str);		
		dispFlip(hDisp);
		return;		
	}
	else if(dv_set.sdc_error== 1)
	{	
		gp_Icon_draw(fb, (resolution.width-Icon_Manager[ICON_NOFILE].icon_width*HDMIxNum)/2,(resolution.height-Icon_Manager[ICON_NOFILE].icon_height*HDMIxNum)/2,resolution.width, resolution.height,ICON_NOFILE,Icon_Manager[ICON_NOFILE].tran_color);
		sub_str.font_type = 0;	
		sub_str.font_color = FONT_COLOR;
		sub_str.buff_w = resolution.width;
		sub_str.buff_h = resolution.height;	
		sub_str.language = setting_config_get(SET_LANGUAGE);
		sub_str.str_idx = STR_SDC_ERROR;		
		ap_state_resource_string_resolution_get(&sub_str,&str_res);
		sub_str.pos_x = (resolution.width -str_res.string_width) /2;
		sub_str.pos_y = (resolution.height -str_res.string_height) /2;
		ap_state_resource_string_draw((INT16U *)fb, &sub_str);		
		dispFlip(hDisp);
		return;		
	}	
	//resolution string
	ascii_str.font_color = FONT_COLOR;
	ascii_str.font_type = 0;
	ascii_str.buff_w = resolution.width;
	ascii_str.buff_h = resolution.height;
	ascii_str.str_ptr = str_dc_resolution[DC_config_get(DC_RESOLUTION)];
	ap_state_resource_string_ascii_resolution_get(ascii_str.str_ptr,&str_res);
	if(dv_set.display_mode == SP_DISP_OUTPUT_HDMI)
	{
		ascii_str.pos_x = resolution.width - ICON_LEFT_X_POS -str_res.string_width-Y_STEP ;
		ascii_str.pos_y = ICON_LEFT_Y_POS+Y_STEP*HDMIxNum;			
	}
	else
	{
		ascii_str.pos_x = resolution.width - ICON_LEFT_X_POS -str_res.string_width ;
		ascii_str.pos_y = ICON_LEFT_Y_POS+Y_STEP;			
	}	
	ap_state_resource_string_ascii_draw((UINT16 *)fb, &ascii_str);
	

	//pic FREE
	ascii_str.font_color = FONT_COLOR;
	ascii_str.font_type = 0;
	ascii_str.buff_w = resolution.width;
	ascii_str.buff_h = resolution.height;
	if(dv_set.sd_check == 0||dv_set.sdc_full != 0)
	{
		pic_free[0] = 0 + 0x30;
		pic_free[1] = 0 + 0x30;
		pic_free[2] = 0 + 0x30;
		pic_free[3] = 0 + 0x30;
		pic_free[4] = 0 + 0x30;
	}
	else
	{
		if(free_size_flag == 1)
		{
			free_size = SDC_FreeSize()-5*1024;
			free_size = free_size/pic_size[DC_config_get(DC_QUALITY)][DC_config_get(DC_RESOLUTION)];
			free_size_flag = 0;
		}
		pic_free[4] = free_size%10+0x30;
		pic_free[3] = (free_size/10)%10+0x30;
		pic_free[2] = (free_size/100)%10+0x30;
		pic_free[1] = (free_size/1000)%10+0x30;
		pic_free[0] = (free_size/10000)%10+0x30;
	}
	ascii_str.str_ptr = pic_free;
	ap_state_resource_string_ascii_resolution_get(&pic_free[0],&str_res);
	if(dv_set.display_mode == SP_DISP_OUTPUT_HDMI)
	{
		ascii_str.pos_x = resolution.width - str_res.string_width - ICON_LEFT_X_POS-Y_STEP;
		ascii_str.pos_y = ICON_LEFT_Y_POS;			
	}
	else
	{
		ascii_str.pos_x = resolution.width - str_res.string_width - ICON_LEFT_X_POS;
		ascii_str.pos_y = ICON_LEFT_Y_POS;		
	}		
	ap_state_resource_string_ascii_draw((UINT16 *)fb, &ascii_str);		

	//quality
	gp_Icon_draw(fb, resolution.width-Icon_Manager[ICON_STAR_3-DC_config_get(DC_QUALITY)].x_pos*HDMIxNum,Icon_Manager[ICON_STAR_3-DC_config_get(DC_QUALITY)].y_pos*HDMIxNum,resolution.width, resolution.height,ICON_STAR_3-DC_config_get(DC_QUALITY),Icon_Manager[ICON_STAR_3-DC_config_get(DC_QUALITY)].tran_color);
	
	//play mode icon
	gp_Icon_draw(fb, Icon_Manager[ICON_CAPTURE].x_pos*HDMIxNum,Icon_Manager[ICON_CAPTURE].y_pos*HDMIxNum,resolution.width, resolution.height,ICON_CAPTURE,Icon_Manager[ICON_CAPTURE].tran_color);
	
	//capture mode
	if(DC_config_get(DC_CAPTURE_MODE)>0)
	{
		if(dv_set.dc_ctrl == 0)
		gp_Icon_draw(fb, Icon_Manager[ICON_CAP_2S_TIMER+DC_config_get(DC_CAPTURE_MODE)-1].x_pos*HDMIxNum,Icon_Manager[ICON_CAP_2S_TIMER+DC_config_get(DC_CAPTURE_MODE)-1].y_pos*HDMIxNum,resolution.width, resolution.height,ICON_CAP_2S_TIMER+DC_config_get(DC_CAPTURE_MODE)-1,Icon_Manager[ICON_CAP_2S_TIMER+DC_config_get(DC_CAPTURE_MODE)-1].tran_color);
		else if(dv_set.dc_ctrl == 1)
		{
			if(time_num !=  10)
			{
				ascii_str.str_ptr = (char *)(&time_num);
				time_num += 0x30;
				ap_state_resource_string_ascii_resolution_get(&time_num,&str_res);
				ascii_str.pos_x = (ICON_LEFT_X_POS + ICON_WIDTH + ICON_WIDTH/2)*HDMIxNum - str_res.string_width/2;
				ascii_str.pos_y = (ICON_LEFT_Y_POS + ICON_WIDTH/2)*HDMIxNum - str_res.string_height/2;
				ap_state_resource_string_ascii_draw((UINT16 *)fb, &ascii_str);
				time_num -= 0x30;	
			}
			else
			{
				ascii_str.str_ptr = &cap_time[0];
				cap_time[0] = 1 + 0x30;
				cap_time[1] = 0x30;
				ap_state_resource_string_ascii_resolution_get(&cap_time,&str_res);
				ascii_str.pos_x = (ICON_LEFT_X_POS + ICON_WIDTH + ICON_WIDTH/2)*HDMIxNum - str_res.string_width/2;
				ascii_str.pos_y = (ICON_LEFT_Y_POS + ICON_WIDTH/2)*HDMIxNum - str_res.string_height/2;
				ap_state_resource_string_ascii_draw((UINT16 *)fb, &ascii_str);
			}
		
		}
	}
	
	//anti shaking
	gp_Icon_draw(fb, Icon_Manager[ICON_NO_ANTI_SHAKING-DC_config_get(DC_ANTI_SHAKING)].x_pos*HDMIxNum,Icon_Manager[ICON_NO_ANTI_SHAKING-DC_config_get(DC_ANTI_SHAKING)].y_pos*HDMIxNum,resolution.width, resolution.height,ICON_NO_ANTI_SHAKING-DC_config_get(DC_ANTI_SHAKING),Icon_Manager[ICON_NO_ANTI_SHAKING-DC_config_get(DC_ANTI_SHAKING)].tran_color);

	//sequence
	if(DC_config_get(DC_SEQUENCE)>0)
	gp_Icon_draw(fb, Icon_Manager[ICON_SEQUENCE_F].x_pos*HDMIxNum,Icon_Manager[ICON_SEQUENCE_F].y_pos*HDMIxNum,resolution.width, resolution.height,ICON_SEQUENCE_F,Icon_Manager[ICON_SEQUENCE_F].tran_color);

	//exporse
	gp_Icon_draw(fb, Icon_Manager[ICON_EXPOSURE_P2_0+DC_config_get(DC_EXPOSURE)].x_pos*HDMIxNum,(Icon_Manager[ICON_EXPOSURE_P2_0+DC_config_get(DC_EXPOSURE)].y_pos-2*ICON_HEIGHT)*HDMIxNum,resolution.width, resolution.height,ICON_EXPOSURE_P2_0+DC_config_get(DC_EXPOSURE),Icon_Manager[ICON_EXPOSURE_P2_0+DC_config_get(DC_EXPOSURE)].tran_color);

	//awb
	gp_Icon_draw(fb, Icon_Manager[ICON_AWB_F+DC_config_get(DC_WHITE_BALANCE)].x_pos*HDMIxNum,Icon_Manager[ICON_AWB_F+DC_config_get(DC_WHITE_BALANCE)].y_pos*HDMIxNum,resolution.width, resolution.height,ICON_AWB_F+DC_config_get(DC_WHITE_BALANCE),Icon_Manager[ICON_AWB_F+DC_config_get(DC_WHITE_BALANCE)].tran_color);

	//iso
	gp_Icon_draw(fb, Icon_Manager[ICON_ISO_AUTO+DC_config_get(DC_ISO)].x_pos*HDMIxNum,Icon_Manager[ICON_ISO_AUTO+DC_config_get(DC_ISO)].y_pos*HDMIxNum,resolution.width, resolution.height,ICON_ISO_AUTO+DC_config_get(DC_ISO),Icon_Manager[ICON_ISO_AUTO+DC_config_get(DC_ISO)].tran_color);
	
	//sdc
	gp_Icon_draw(fb, resolution.width-Icon_Manager[ICON_INT-dv_set.sd_check].x_pos*HDMIxNum,resolution.height -Icon_Manager[ICON_INT-dv_set.sd_check].y_pos*HDMIxNum,resolution.width, resolution.height,ICON_INT-dv_set.sd_check,Icon_Manager[ICON_INT-dv_set.sd_check].tran_color);

	//charge and battery 0: no power  1: zero battery 	2: one battery 3:two battery 4:three battery 5:charge  
	if(dv_set.battery_state == 0)
	{
			printf("No show charge\n");
			gp_Icon_draw(fb, resolution.width-Icon_Manager[ICON_BATTERYX].x_pos*HDMIxNum,resolution.height-Icon_Manager[ICON_BATTERYX].y_pos*HDMIxNum,resolution.width, resolution.height,ICON_BATTERYX,Icon_Manager[ICON_BATTERYX].tran_color);		
	}
	else if(dv_set.battery_state == 5)
	{
		if(flip_flag1 == 0)
		{
			gp_Icon_draw(fb, resolution.width-Icon_Manager[ICON_CHARGE].x_pos*HDMIxNum,resolution.height-Icon_Manager[ICON_CHARGE].y_pos*HDMIxNum,resolution.width, resolution.height,ICON_CHARGE,Icon_Manager[ICON_CHARGE].tran_color);
			//printf("show charge\n");
		}
		else
		{
			fill_rectangle(fb,resolution.width-Icon_Manager[ICON_CHARGE].x_pos*HDMIxNum,resolution.height-Icon_Manager[ICON_CHARGE].y_pos*HDMIxNum ,resolution.width,resolution.height,Icon_Manager[ICON_CHARGE].icon_width*HDMIxNum,Icon_Manager[ICON_CHARGE].icon_height*HDMIxNum,TRANSPARENT_COLOR);
			//printf("No show charge\n");
		}		
	}
	else
	{
		if(dv_set.battery_state > 5)
		gp_Icon_draw(fb, resolution.width-Icon_Manager[ICON_CHARGE+4].x_pos*HDMIxNum,resolution.height-Icon_Manager[ICON_CHARGE+4].y_pos*HDMIxNum,resolution.width, resolution.height,ICON_CHARGE+4,Icon_Manager[ICON_CHARGE+4].tran_color);
		else
		gp_Icon_draw(fb, resolution.width-Icon_Manager[ICON_CHARGE+dv_set.battery_state].x_pos*HDMIxNum,resolution.height-Icon_Manager[ICON_CHARGE+dv_set.battery_state].y_pos*HDMIxNum,resolution.width, resolution.height,ICON_CHARGE+dv_set.battery_state,Icon_Manager[ICON_CHARGE+dv_set.battery_state].tran_color);
	}


	if(dv_set.zoom_num != 0)
	{
		zoom[1] = dv_set.zoom_num/10+1+0x30;
		zoom[3] =  dv_set.zoom_num%10+0x30;
		ascii_str.str_ptr = zoom;
		ap_state_resource_string_ascii_resolution_get(&zoom[0],&str_res);	
		ascii_str.pos_x = Icon_Manager[ICON_WDR_F].x_pos*HDMIxNum;
		ascii_str.pos_y = resolution.height - str_res.string_height - ICON_LEFT_Y_POS*HDMIxNum-Y_STEP*HDMIxNum;
		ap_state_resource_string_ascii_draw((UINT16 *)fb, &ascii_str);		
	}
	/*//date and time
	//RTC_get();
	ap_setting_date_time_string_process(0xFF);
	ascii_str.str_ptr = dt_str;
	ap_state_resource_string_ascii_resolution_get(&dt_str[0],&str_res);	
	ascii_str.pos_x = resolution.width/2 -  str_res.string_width/2;
	ascii_str.pos_y = resolution.height - str_res.string_height + STR_DATETIME_POS_Y;
	//printf("x=%d,y=%d\n",str_res.string_width,str_res.string_height);
	ap_state_resource_string_ascii_draw((UINT16 *)fb, &ascii_str);	*/

	if((cur_draw_flag ==DV_SETTING_MODE)||(cur_draw_flag ==DV_EXIT_MODE))
	{
		disp_OSD0_Set_Alpha(hDisp,100);
		cur_draw_flag =DV_EXIT_MODE+1;
	}	
	dispFlip(hDisp);

}
/*==========================================================
dv task entry function
===========================================================*/
int
DC_task_entry(
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
	struct timespec timeout;
	//run dv and init pipe
	free_size_flag = 1;
	DC_Set_Color();
	while(dv_set.change_disp == 1);
	dv_set.zoom_flag = 0;
	dv_set.zoom_num = 0;
	DC_pipe_init();	
	if(dv_set.display_mode == SP_DISP_OUTPUT_LCD)
		CVR_Set_PIPE(CMD_SET_DISPLAY_MODE,1);
	else
		CVR_Set_PIPE(CMD_SET_DISPLAY_MODE,0);	
	dispGetResolution(hDisp, &resolution);
	printf("resolution.width=%d,resolution.height=%d\n",resolution.width,resolution.height);	
	//init UI for DV
	DC_UI_foreground();
	do
	{
		clock_gettime(CLOCK_REALTIME, &timeout);
		timeout.tv_nsec += 10000;
		ret = mq_timedreceive(menumodeMsgQ, (char *)&msg_id, 1, (unsigned int *)&msg_prio,&timeout);
		if(ret != -1)	
		DC_UI_foreground();
	}while((dv_set.dv_UI_flag == DV_EXIT_MODE)||(ret != -1));	
	dv_set.dv_UI_flag = DV_FOREGROUND_MODE;
	if(dv_set.display_mode == SP_DISP_OUTPUT_LCD)
	{
		LCD_Backlight_Set(1);
	}
	if(dv_set.default_setting_flag == 1)
	{	
		dv_set.default_setting_flag = 0;
		CVR_Set_PIPE_With_CMD(CMD_SET_DEFAULT_SETTING,0);
	}
	if(dv_set.date_type_flag == 1)
	{
		CVR_Set_PIPE(CMD_SET_DATE_TYPE,setting_config_get(SET_DATA_FORMAT));
		dv_set.date_type_flag = 0;
	}	
	while(1)
	{

		ret = mq_receive(menumodeMsgQ, (char *)&msg_id, 1, (UINT32 *)&msg_prio);
		switch(msg_id){
			case DV_FOREGROUND_MODE:
				dv_set.draw_start = 1;
				DC_UI_foreground();
				dv_set.draw_start = 0;
			break;
			case DV_MENU_MODE:
				dv_set.draw_start = 1;
				disp_OSD0_Set_Alpha(hDisp,65);
				UI_Menu_draw(Playmode_flag);
				dv_set.draw_start = 0;
			break;
			case DV_SETTING_MODE:
				dv_set.draw_start = 1;
				DV_UI_Setting();
				dv_set.draw_start = 0;
			break;			
			default:
			break;	
		}
		if(msg_id == DV_EXIT_MODE)
		{
			CVR_Set_PIPE_With_CMD(CMD_SET_EXIT,1);
			break;
		}	
					//pend message from TimeKeyThread and update UI/send message to ipcam
	}
	pipeDelete();	
	return 0;
}

