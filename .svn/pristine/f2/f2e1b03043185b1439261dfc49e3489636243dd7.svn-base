/**************************************************************************
 *                                                                        *
 *         Copyright (c) 2010 by Generalplus Inc.                         *
 *                                                                        *
 *  This software is copyrighted by and is the property of Generalplus    *
 *  Inc. All rights are reserved by Generalplus Inc.                      *
 *  This software may only be used in accordance with the                 *
 *  corresponding license agreement. Any unauthorized use, duplication,   *
 *  distribution, or disclosure of this software is expressly forbidden.  *
 *                                                                        *
 *  This Copyright notice MUST not be removed or modified without prior   *
 *  written consent of Generalplus Technology Co., Ltd.                   *
 *                                                                        *
 *  Generalplus Inc. reserves the right to modify this software           *
 *  without notice.                                                       *
 *                                                                        *
 *  Generalplus Inc.                                                      *
 *  No.19, Industry E. Rd. IV, Hsinchu Science Park,                      *
 *  Hsinchu City 30078, Taiwan, R.O.C.                                    *
 *                                                                        *
 **************************************************************************/
/**
 * @file main.c
 * @brief main function
 * @author Anson Chuang
 */

#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include "gp_video_stream_api.h"
#include "string.h"
#include "gp_csi_attr.h"
#include "dvr_pipe.h"
//#include "mach/aeawb.h"
#include "mach/gp_cdsp.h"
#include "folder.h"
#include "config/sysconfig.h"

extern sensor_calibration_t ar0330_cdsp_calibration;
extern gp_cdsp_user_preference_t cdsp_user_preference;

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/

//#define DISP_SCALE_TYPE	IPC_SCALE_BILINEAR
#define DISP_SCALE_TYPE	IPC_SCALE_NORMAL
/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/


/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/

typedef struct option_s {
	int width;
	int height;
	int quality;
	int ev;
	int sharpness;
	int iso;
	int color;
	int white_balance;
	int time_stamp;
	int time_format;
	int sequence;
	int raw;
	int display;
	char outpath[128];
} dc_option_t;

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

extern int g_bQuit;
extern char cam_working_path[256];
extern int global_ev;
extern int global_dv_reset;
extern int global_dc_reset;
extern int global_time_format;
extern int global_frequency;
extern int global_flip;
extern int g_flip_on, g_flip_off;
extern display_param_t disp_info;
/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/
extern int display_in(int arg);
extern int display_out(int arg);
/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static dc_option_t opt;
int _file_idx;
void* fh = NULL;
/**************************************************************************
 *             F U N C T I O N    I M P L E M E N T A T I O N S           *
 **************************************************************************/

static int get_quality(int mode)
{
	int value;
	switch(mode)
	{
	case 0: value = 90; break;
	case 1: value = 80; break;
	case 2: value = 70; break;
	defaut: value = 80; break;
	}
	return value;
}

static int get_resolution(int mode, int* w, int* h)
{
	int width, height;
	
	switch(mode)
	{
	case 0:	width = 4032; height = 3024; break;
	case 1: width = 3648; height = 2736; break;
	case 2: width = 3264; height = 2448; break;
	case 3: width = 2592; height = 1944; break;
	case 4: width = 2048; height = 1536; break;
	case 5: width = 1920; height = 1080; break;
	case 6: width = 640;  height = 480;  break;
	case 7: width = 1280; height = 960;  break;
	default:
		return -1;
	}
	
	*w = width;
	*h = height;
	return 0;
}

int dc_parse_command(int argc, char *argv[])
{
	int i;

	memset(&opt, 0, sizeof(dc_option_t));
	opt.display = 1;
	opt.ev = 6;
	opt.sharpness = 1;
	strcpy(opt.outpath, "./");
		
	for (i =1; i < argc; i++)
	{
		if (strcmp(argv[i], "-dc") == 0)
			break;
	}
	for (; i < argc; i++)
	{
		if (strcmp(argv[i], "-dv") == 0)
			break;
			
		if (strcmp(argv[i], "-rel") == 0)
		{
			if (0 > get_resolution(atoi(argv[++i]), &opt.width, &opt.height))
				return -1;
		}
		else if (strcmp(argv[i], "-q") == 0)
		{
			opt.quality = get_quality(atoi(argv[++i]));
		}
		else if (strcmp(argv[i], "-w") == 0)
		{
			opt.white_balance = atoi(argv[++i]);
		}
		else if (strcmp(argv[i], "-s") == 0)
		{
			opt.sharpness = atoi(argv[++i]);
		}
		else if (strcmp(argv[i], "-iso") == 0)
		{
			opt.iso = atoi(argv[++i]);
		}
		else if (strcmp(argv[i], "-ev") == 0)
		{
			opt.ev = atoi(argv[++i]);
		}
		else if (strcmp(argv[i], "-c") == 0)
		{
			opt.color = atoi(argv[++i]);
		}
		else if (strcmp(argv[i], "-timestamp") == 0)
		{
			opt.time_stamp = atoi(argv[++i]);
		}
		else if (strcmp(argv[i], "-timeformat") == 0)
		{
			opt.time_format = atoi(argv[++i]);
		}
		else if (strcmp(argv[i], "-o") == 0)
		{
			strcpy(opt.outpath, argv[++i]);
			printf("outpath = %s\n", opt.outpath);
			strcpy(cam_working_path, opt.outpath);
		}
		else if (strcmp(argv[i], "-seq") == 0)
		{
			opt.sequence = atoi(argv[++i]);
			printf("sequence = %d\n", opt.sequence);
		}
		else if (strcmp(argv[i], "-d2") == 0)
		{
			opt.display = 0;
			sscanf(argv[++i],"0x%x", &disp_info.address);
			disp_info.width = atoi(argv[++i]);
			disp_info.height = atoi(argv[++i]);
			disp_info.buffer_num = atoi(argv[++i]);
			disp_info.disp_in_f = display_in;
			disp_info.disp_out_f = display_out;
		}
	}
	return 0;
}


static int snap_shot(int width,int height,int QType, char* path, int sequence, int raw)
{
	void *handle;
	struct gpIPC_SNAP_IMAGE_S image_info;
	static int i=0;
	char filename[128], ext[4];
	FILE *file;
	int j;
	int ret = 0;

	if (raw)
		sprintf(ext, "RAW");
	else
		sprintf(ext, "JPG");

#if 0
	if (i == 0)
	{
		while(1)
		{
			sprintf(filename, "%s/DSC%05d.%s", path, i, ext);
			if(access(filename, F_OK) != 0)
				break;
				
			i++;
			if (i > 9999)
				i = 0;
		}
	}
#endif

	handle=gp_capture_stream_create(width,height);
	gp_capture_stream_Enable(handle);
			
	if (!sequence)
		sequence = 1;
	
	for (j = 0; j < sequence; j++)
	{
		if (sequence > 1 && j > 0)
			dvr_pipemsg_send(CMD_SET_SEQUENCE_INT, 0, NULL);
			
		if(gp_capture_picture(handle,&image_info,QType)==0)
		{
#if 1
			_file_idx = cam_get_new_file_name(fh, filename, ext, _file_idx);
#else
			sprintf(filename, "%s/DSC%05d.%s", path, i, ext);
#endif
			printf("save image to %s\n", filename);
			file=fopen(filename,"w+");
			if(file)
			{
				ret = fwrite(image_info.pImage,1,image_info.size,file);
				fflush(file);
				fclose(file);
			}

			gp_capture_picture_Free(handle,&image_info);

			if (ret == image_info.size)
#if 1
				_file_idx++;
#else				
				i++;
#endif				
			else
			{
				printf("save image fail: %s\n", strerror(errno));
				break;
			}
		}
	}
	sync();
	gp_capture_stream_disable(handle);
	gp_capture_stream_close(handle);
	printf("snpshot over\n");
	
}

int set_sharpness(void* csi_attr, int mode)
{
	int value;
	switch(mode)
	{
	case 0:	value = 2; break;
	case 1:	value = 1; break;
	case 2:	value = 0; break;
	default:	value = -1; break;
	}
	if (0 > value)
		return -1;
				
	return gp_IPC_VDev_Set_Sharpness(csi_attr, value);
}

int set_awb(void* csi_attr, int mode)
{
	int value;
	switch(mode)
	{
	case 0:	value = AWB_AUTO_DC;	break;
	case 1:	value = AWB_DAYLIGHT;	break;
	case 2:	value = AWB_CLOULDY;	break;
	case 3:	value = AWB_LAMP;		break;
	case 4:	value = AWB_FLUORESCENT;	break;
	default:	value = -1;	break;
	}
	
	if (0 > value)
		return -1;
		
	return gp_IPC_VDev_Set_AWB(csi_attr, value);
}

int set_color(void* csi_attr, int mode)
{
	int value;
	switch(mode)
	{
	case 0:	value = MODE_NORMAL;	break;
	case 1:	value = MODE_BLACKWHITE;	break;
	case 2:	value = MODE_SEPIA;	break;
	default:	value = -1;	break;
	}
	
	if (0 > value)
		return -1;
		
	return gp_IPC_VDev_Set_Effect(csi_attr, value);
}

int set_iso(void* csi_attr, int mode)
{
	int value;
	switch(mode)
	{
	case 0:	value = ISO_AUTO;	break;
	case 1:	value = 100;	break;
	case 2:	value = 200;	break;
	case 3:	value = 400;	break;
	case 4:	value = 800;	break;
	default:	value = -1;	break;
	}
	
	if (0 > value)
		return -1;
		
	return gp_IPC_VDev_Set_ISO(csi_attr, value);
}

static void dc_reset(dc_option_t* opt)
{
	get_resolution(0, &opt->width, &opt->height);
	
	opt->quality = 80;
	opt->ev = 6;
	opt->sharpness = 1;
	opt->white_balance = 0;
	opt->color = 0;
	opt->iso = 0;
	opt->time_stamp = 2;
	opt->time_format = 0;
	opt->sequence = 0;
	opt->display = 1;
}
int dc_main(int argc,char *argv[])
{
	void *handle;
	void* csi_attr;
	int restart = 0;
	int disp4x3 = 0;
	
	//RegisterSigint();
	
	if (global_dv_reset)
	{
		dc_reset(&opt);
		global_dv_reset = 0;
	}
	if (global_ev >= 0)
		opt.ev = global_ev;
		
	if (global_time_format >= 0)
		opt.time_format = global_time_format;
	
	if (strlen(cam_working_path) > 0)
		strcpy(opt.outpath, cam_working_path);

	dvr_pipe_init();
	
start:
	g_bQuit = 0;
	
	gp_IPC_Set_Zoom(0);
	gp_IPC_Set_Mode(MODE_JPG);
	//gp_IPC_VStream_Open(IPC_COLOR_YUYV, 0, 0, MODE_JPG);

	csi_attr = gp_IPC_VDev_Open(NULL);
	
	gp_IPC_VDev_Set_Frequency(csi_attr, global_frequency);
	
	gp_IPC_VDev_MirrorImage(csi_attr, global_flip);
	gp_IPC_VDev_FlipImage(csi_attr, global_flip);
	
	gp_IPC_VDev_Set_Exposure(csi_attr, opt.ev);
	set_sharpness(csi_attr, opt.sharpness);
	set_awb(csi_attr, opt.white_balance);
	//set_color(csi_attr, opt.color);
	set_color(csi_attr, 0);
	set_iso(csi_attr,opt.iso);
	
	disp4x3 = (opt.width == 1920) ? 0:1;
	
	if (opt.display == 1)
		gp_IPC_Enable_Display(0, DISP_SCALE_TYPE, disp4x3);
	
	gp_IPC_Enable_TimeStamp(opt.time_stamp, opt.time_format);

	if (!restart)
		dvr_pipemsg_send(CMD_READY_KEY, 0, NULL);
	
	restart = 0;
	
	while(1)
	{
		unsigned int msgId;
		void* msgPara = NULL;
		unsigned int mode;

		if (0 > gp_IPC_VStream_Query_Status())
		{
			int ret = 0;
			gpCVR_VStream_Cfg cfg;
	
			memset(&cfg, 0, sizeof(gpCVR_VStream_Cfg));
			cfg.mode = MODE_JPG;
			cfg.sensor_calibrate = 	&ar0330_cdsp_calibration;
			cfg.cdsp_user_preference = &cdsp_user_preference;
			
			printf("[camcorder] restart DC\n");
			gp_IPC_VStream_Close();
			do {
				ret = gp_IPC_VStream_Open(&cfg);
				
				if (ret < 0)
				{
					printf("[camcorder] restart DC Fail!\n");
					usleep(500000);
				}
			}while (ret < 0);
		
			gp_IPC_VStream_Start();
			
			g_bQuit = 1;
			restart = 1;
			
		}
	
		if (g_bQuit)
			break;
		
		if(dvr_pipemsg_receive(&msgId, &msgPara) > 0)
		{
			printf("msgId=%d\n", msgId);
			if (msgPara)
			{
				mode = *(unsigned int*)msgPara;
				printf("mode = %d\n", mode);
			}
			switch(msgId)
			{
				case CMD_DO_CAPTURE:
					if (strlen(opt.outpath) > 0)
					{
						fh = cam_folder_init(opt.outpath, "DCIM", NULL, &_file_idx, TIME_IDX_FILE_NAME);
						snap_shot(opt.width, opt.height, opt.quality, opt.outpath, opt.sequence, 0);
						cam_folder_close(fh);
						fh = NULL;
					}
				break;
				case CMD_SET_DIR:
					strcpy(opt.outpath, (char*)msgPara);
					strcpy(cam_working_path, (char*)msgPara);
					printf("[dc] get output path = %s\n", opt.outpath);
				break;
				case CMD_SET_DC_RESOLUTION:
					get_resolution(mode, &opt.width, &opt.height);
					if (disp4x3 != (int)(opt.width != 1920))
					{
						disp4x3 = (opt.width == 1920) ? 0:1;
						gp_IPC_Disable_Display();
						if (opt.display == 2)
							gp_IPC_Enable_Display2(0, DISP_SCALE_TYPE, disp4x3, &disp_info);
						else if (opt.display == 1)
							gp_IPC_Enable_Display(0, DISP_SCALE_TYPE, disp4x3);
					}
				break;
				case CMD_SET_QUALITY:
					opt.quality = get_quality(mode);
				break;
				case CMD_SET_EXPOSURE:
					gp_IPC_VDev_Set_Exposure(csi_attr, mode);
					opt.ev = mode;
				break;
				case CMD_SET_SHARPNESS:
					set_sharpness(csi_attr, mode);
					opt.sharpness = mode;
				break;
				case CMD_SET_AWB:
					set_awb(csi_attr, mode);	
					opt.white_balance = mode;
				break;
				case CMD_SET_COLOR:
					set_color(csi_attr, mode);
					opt.color = mode;
				break;
				case CMD_SET_ISO:
					set_iso(csi_attr, mode);
					opt.iso = mode;
				break;
				case CMD_SET_DC_DATE_STAMP:
					opt.time_stamp = mode;
					gp_IPC_Enable_TimeStamp(opt.time_stamp, opt.time_format);
				break;
				case CMD_SET_DATE_TYPE:
					opt.time_format = mode;
					gp_IPC_Enable_TimeStamp(opt.time_stamp, opt.time_format);
				break;
				case CMD_SET_SEQUENCE:
					opt.sequence = mode ? 3:0;
				break;
				case CMD_SET_ZOOM:
					mode = mode ? mode + 10: 0;
					gp_IPC_Set_Zoom(mode);
				break;
				case CMD_SET_FREQUENCY:
					global_frequency = mode;
					gp_IPC_VDev_Set_Frequency(csi_attr, mode);
				break;
				case CMD_SET_SENSOR_FLIP:
					mode = mode ? g_flip_on : g_flip_off;
					if(global_flip != mode) {
						global_flip = mode;
						gp_IPC_VDev_MirrorImage(csi_attr, global_flip);
						gp_IPC_VDev_FlipImage(csi_attr, global_flip);
					}
				break;
				case CMD_SET_DISPLAY_MODE:
				if (opt.display != mode+1)
				{
					if (opt.display == 2)
						disp_pipe_close();

					gp_IPC_Disable_Display();

					opt.display = mode+1;

					if (opt.display == 1)
						gp_IPC_Enable_Display(0, DISP_SCALE_TYPE, disp4x3);
					else
					{
						disp_pipe_init();
						gp_IPC_Enable_Display2(0, DISP_SCALE_TYPE, disp4x3, &disp_info);
					}
				}
				break;
				case CMD_SET_DEFAULT_SETTING:
					dc_reset(&opt);

					gp_IPC_Disable_Display();
					if (opt.display == 2)
						gp_IPC_Enable_Display2(0, DISP_SCALE_TYPE, (opt.width == 1920) ? 0:1, &disp_info);
					else
						gp_IPC_Enable_Display(0, DISP_SCALE_TYPE, (opt.width == 1920) ? 0:1);
					
					gp_IPC_VDev_Set_Exposure(csi_attr, opt.ev);
					set_sharpness(csi_attr, opt.sharpness);
					set_awb(csi_attr, opt.white_balance);
					set_color(csi_attr, opt.color);
					set_iso(csi_attr,opt.iso);
					gp_IPC_Enable_TimeStamp(opt.time_stamp, opt.time_format);
					global_dc_reset = 1;
				break;
				case CMD_SET_EXIT:
					g_bQuit = 1;
				break;
			}
			if (!g_bQuit)
				dvr_response_cmd(msgId);
		}
	}
	printf("++++++++++dc exit++++++++++\n");
	gp_IPC_Enable_TimeStamp(0, 0);
	gp_IPC_Disable_Display();
	gp_IPC_VDev_Close(csi_attr);
	//gp_IPC_VStream_Close();
	
	if (opt.display == 2)
		disp_pipe_close();
	
	if(restart)
		goto start;

	dvr_response_cmd(CMD_SET_EXIT);
	
	dvr_pipe_close();
	
	global_ev = opt.ev;
	global_time_format = opt.time_format;
	
	printf("[camcorder] exit dc mode finish\n");
	
	return 0;
}