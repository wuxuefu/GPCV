#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "gp_video_channel.h"
#include "gp_jpg_stream.h"
#include "string.h"
#include "gp_csi_attr.h"
#include "osd_resource.h"
#include "dc_pipe.h"
#include "mach/aeawb.h"

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
	int sequence;
	int raw;
	char outpath[128];
} option_t;

int g_bQuit=0;	   
 void sigint( int signo )
{
	g_bQuit = 1;
}

int RegisterSigint()
{
	struct sigaction act;
	memset(&act, 0, sizeof(act));
	act.sa_handler = sigint;
	act.sa_flags   = SA_RESTART;
	if(sigaction(SIGINT, &act, NULL) != 0)
	{
		return -1;
	}
	return 1;
}

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
int parse_command(int argc, char *argv[], option_t* opt)
{
	int i;

	for (i =1; i < argc; i++)
	{
		if (strcmp(argv[i], "-rel") == 0)
		{
			if (0 > get_resolution(atoi(argv[++i]), &opt->width, &opt->height))
				return -1;
		}
		else if (strcmp(argv[i], "-q") == 0)
		{
			opt->quality = get_quality(atoi(argv[++i]));
		}
		else if (strcmp(argv[i], "-w") == 0)
		{
			opt->white_balance = atoi(argv[++i]);
		}
		else if (strcmp(argv[i], "-s") == 0)
		{
			opt->sharpness = atoi(argv[++i]);
		}
		else if (strcmp(argv[i], "-iso") == 0)
		{
			opt->iso = atoi(argv[++i]);
		}
		else if (strcmp(argv[i], "-ev") == 0)
		{
			opt->ev = atoi(argv[++i]);
		}
		else if (strcmp(argv[i], "-c") == 0)
		{
			opt->color = atoi(argv[++i]);
		}
		else if (strcmp(argv[i], "-timestamp") == 0)
		{
			opt->time_stamp = atoi(argv[++i]);
		}
		else if (strcmp(argv[i], "-o") == 0)
		{
			strcpy(opt->outpath, argv[++i]);
			printf("outpath = %s\n", opt->outpath);
		}
		else if (strcmp(argv[i], "-seq") == 0)
		{
			opt->sequence = atoi(argv[++i]);
			printf("sequence = %d\n", opt->sequence);
		}
	}
	return 0;
}


int snap_shot(int width,int height,int QType, char* path, int sequence, int raw)
{
	void *handle;
	struct gpIPC_SNAP_IMAGE_S image_info;
	static int i=0;
	char filename[128], ext[4];
	FILE *file;
	int j;

	if (raw)
		sprintf(ext, "raw");
	else
		sprintf(ext, "jpg");
		
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

	handle=gp_capture_stream_create(width,height);
	gp_capture_stream_Enable(handle);
			
	if (!sequence)
		sequence = 1;
	
	for (j = 0; j < sequence; j++)
	{
		if (sequence > 1 && j > 0)
			dc_pipemsg_send(CMD_SET_SEQUENCE_INT, 0, NULL);
			
		if(gp_capture_picture(handle,&image_info,QType)==0)
		{
			sprintf(filename, "%s/DSC%05d.%s", path, i, ext);
			printf("save image to %s\n", filename);
			i++;
			file=fopen(filename,"w+");
			fwrite(image_info.pImage,1,image_info.size,file);
			fflush(file);
			fclose(file);
			gp_capture_picture_Free(handle,&image_info);
		}
	}
	sync();
	gp_capture_stream_disable(handle);
	gp_capture_stream_close(handle);
	printf("snpshot over\n");
	
}

static int set_sharpness(void* csi_attr, int mode)
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

static int set_awb(void* csi_attr, int mode)
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

static int set_color(void* csi_attr, int mode)
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

static int set_iso(void* csi_attr, int mode)
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

int main(int argc,char *argv[])
{
	void *handle;
	option_t opt;
	void* csi_attr;
	int restart = 0;
	
	RegisterSigint();
	
	memset(&opt, 0, sizeof(option_t));
	opt.ev = 6;
	opt.sharpness = 1;
	strcpy(opt.outpath, "./");
	if (0 > parse_command(argc, argv, &opt))
		return -1;
		
	dc_pipe_init();
	
//	opt.raw = 1;
start:
	g_bQuit = 0;
	restart = 0;
	gp_IPC_VStream_Open(IPC_COLOR_YUYV, 0, 0, ON2_JPG);

	csi_attr = gp_IPC_VDev_Open(NULL);
	
	gp_IPC_VDev_Set_Exposure(csi_attr, opt.ev);
	set_sharpness(csi_attr, opt.sharpness);
	set_awb(csi_attr, opt.white_balance);
	set_color(csi_attr, opt.color);
	set_iso(csi_attr,opt.iso);
	
	gp_IPC_Enable_Display(1, 0, IPC_SCALE_BILINEAR, (opt.width == 1920) ? 0:1);
	
	gp_IPC_Enable_TimeStamp(opt.time_stamp);
	
	dc_pipemsg_send(CMD_READY_KEY, 0, NULL);
	
	while(1)
	{
		unsigned int msgId;
		void* msgPara;
		unsigned int mode;
		
		if (g_bQuit)
			break;
		
		if(dc_pipemsg_receive(&msgId, &msgPara) > 0)
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
					if(opt.raw)
					{
						gp_IPC_VDev_Close(csi_attr);
						gp_IPC_VStream_Close();
						gp_IPC_VStream_Open(IPC_COLOR_RAW, 0, 0, ON2_JPG);
						csi_attr = gp_IPC_VDev_Open(NULL);
						snap_shot(opt.width, opt.height, opt.quality, opt.outpath, opt.sequence, 1);
						restart = 1;
						g_bQuit = 1;
						dc_response_cmd(msgId);
					}
					else
						snap_shot(opt.width, opt.height, opt.quality, opt.outpath, opt.sequence, 0);
				break;
				case CMD_SET_DC_RESOLUTION:
					get_resolution(mode, &opt.width, &opt.height);
					gp_IPC_Enable_Display(0,0,0,0);
					gp_IPC_Enable_Display(1, 2, IPC_SCALE_BILINEAR, (opt.width == 1920) ? 0:1);
				break;
				case CMD_SET_QUALITY:
					opt.quality = get_quality(mode);
				break;
				case CMD_SET_EXPOSURE:
					gp_IPC_VDev_Set_Exposure(csi_attr, mode);
				break;
				case CMD_SET_SHARPNESS:
					set_sharpness(csi_attr, mode);
				break;
				case CMD_SET_AWB:
					set_awb(csi_attr, mode);	
				break;
				case CMD_SET_COLOR:
					set_color(csi_attr, mode);
				break;
				case CMD_SET_ISO:
					set_iso(csi_attr, mode);
				break;
				case CMD_SET_DC_DATE_STAMP:
					opt.time_stamp = mode;
					gp_IPC_Enable_TimeStamp(opt.time_stamp);
				break;
				case CMD_SET_SEQUENCE:
					opt.sequence = mode ? 3:0;
				break;
				case CMD_SET_ZOOM:
					mode = mode ? mode + 10: 0;
					gp_IPC_Set_Zoom(mode);
				break;
				case CMD_SET_EXIT:
					g_bQuit = 1;
				break;
			}
			if (!g_bQuit)
				dc_response_cmd(msgId);
		}
	}
	printf("++++++++++dc exit++++++++++\n");
	gp_IPC_VDev_Close(csi_attr);
	gp_IPC_VStream_Close();
	
	if(restart)
		goto start;

	dc_response_cmd(CMD_SET_EXIT);
}