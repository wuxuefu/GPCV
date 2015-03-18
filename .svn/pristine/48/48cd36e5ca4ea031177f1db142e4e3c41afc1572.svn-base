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

#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <errno.h>

#include "mach/typedef.h"
#include "cdsp_calibrate.h"
#include "cdsp_preference.h"

#include "gp_video_stream_api.h"
#include "dvr_pipe.h"
#include "config/sysconfig.h"

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/
#define WAIT_MODE 0
#define DV_MODE 1
#define DC_MODE 2
#define IDLE_MODE 3
/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/


/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/
extern int dv_main(int argc, char *argv[]);
extern int dc_main(int argc, char *argv[]);
extern int dv_parse_command(int argc, char *argv[]);
extern int dc_parse_command(int argc, char *argv[]);
/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
int g_bQuit = 0;
int powerOff = 0;
static int mode = 0;
char cam_working_path[256];
int global_ev = -1;
int global_dv_reset = 0;
int global_dc_reset = 0;
int global_time_format = -1;
int global_ldw = 0;
int global_60fps = 0;
int global_frequency = 0;
int global_flip = 0;
int g_flip_on, g_flip_off;
display_param_t disp_info;

sensor_calibration_t ar0330_cdsp_calibration;

/**************************************************************************
 *             F U N C T I O N    I M P L E M E N T A T I O N S           *
 **************************************************************************/
void update_flag(unsigned int* flag, unsigned int value, int on)
{
	if (on)
		*flag |= value;
	else
		*flag &= ~value;
}

int display_in(int arg)
{
	unsigned int msgId;
	void* msgPara = NULL;
	int idx;
//	static int ii = -1;
//	
//	ii++;
//	
//	if (ii == 3)
//		ii = 0;
//
//	printf("%s (%d)\n", __FUNCTION__, ii);
//		
//	return ii;
	
	if (!disp_pipemsg_receive(&msgId, &msgPara))
		return -1;

	if (msgId == DISP_BUF_READY)
	{
		idx = (int)*(unsigned int*)msgPara;
		return idx;
	}
	return -1;
}

int display_out(int arg)
{
	unsigned int msgId;
	void* msgPara = NULL;
	unsigned int idx = arg;
//	printf("%s\n", __FUNCTION__);
//	return 0;
	
	msgId = DISP_BUF_READY;
	
	disp_pipemsg_send(msgId, 4, (unsigned char*)&idx);

	return 0;
}

void sigint( int signo )
{
	g_bQuit = 1;
	powerOff = 1;
	printf("[camcorder] receive power off signal\n");
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

void get_signal_user1(int No)
{
	mode = DV_MODE;
}

void get_signal_user2(int No)
{
	mode = DC_MODE;
}

void get_signal_term(int No)
{
	mode = IDLE_MODE;
}
int main(int argc, char *argv[])
{
	int i;
	int sensor_on = 0;
	int ldw = 0;
	int ret = 0;
	int initial_err = 0;
	int sensor60fps = 0;
	gpCVR_VStream_Cfg cfg;
	
	if(	strcmp(SYSCONFIG_PLATFORM, "gplus.cvr_turnkey_demo1") == 0 )
	{
		g_flip_on = 0;
		g_flip_off = 1;
	}
	else
	{
		g_flip_on = 1;
		g_flip_off = 0;
	}
	global_flip = g_flip_off;
	
	memset(&cfg, 0, sizeof(gpCVR_VStream_Cfg));
	cfg.sensor_calibrate = 	&ar0330_cdsp_calibration;
	cfg.cdsp_user_preference = &cdsp_user_preference;
	
	memset(cam_working_path, 0, 256);
	
	if (strcmp(argv[1], "-dv") == 0)
		mode = DV_MODE;
	else if (strcmp(argv[1], "-dc") == 0)
		mode = DC_MODE;
	else
		return -1;
		
	dv_parse_command(argc, argv);
	dc_parse_command(argc, argv);

	
	signal(SIGUSR1,get_signal_user1);
	signal(SIGUSR2,get_signal_user2);
	signal(SIGTERM,get_signal_term);
				
	ldw = global_ldw;
	sensor60fps = global_60fps;
	
	if (mode == DV_MODE)
	{
		cfg.mode = MODE_H264;
		update_flag(&cfg.flag, CFG_LDW, ldw);
		if (sensor60fps)
		{
			cfg.width = 1280;
			cfg.height = 720;
			cfg.fps = 60;
		}
		ret = gp_IPC_VStream_Open(&cfg);
	}
	else
	{
		cfg.mode = MODE_JPG;
		ret = gp_IPC_VStream_Open(&cfg);
	}
	
	if (ret < 0)
	{
		initial_err = 1;
		powerOff = 1;
	}
		
	sensor_on = 1;
	RegisterSigint();
	
	while(!powerOff)
	{
		//unsigned int msgId;
		//void* msgPara = NULL;

		if (mode == DV_MODE)
		{
			cfg.mode = MODE_H264;
			update_flag(&cfg.flag, CFG_LDW, global_ldw);
			cfg.width = cfg.height = cfg.fps = 0;
			if (global_60fps)
			{
				cfg.width = 1280;
				cfg.height = 720;
				cfg.fps = 60;
			}
			if (!sensor_on)
			{
				ret = gp_IPC_VStream_Open(&cfg);
			}
			else if (global_ldw != ldw || global_60fps != sensor60fps)
			{
				gp_IPC_VStream_Close();
				ret = gp_IPC_VStream_Open(&cfg);
			}
			if (ret < 0) 
			{
				initial_err = 1;
				powerOff = 1;
			}
			
			sensor_on = 1;
			gp_IPC_VStream_Start();
			printf("Start DV Mode\n");
			ret = dv_main(argc, argv);
			if (ret < 0)
			{
				powerOff = 1;
				continue;
			}
			mode = WAIT_MODE;
			ldw = global_ldw;
			sensor60fps = global_60fps;
		}
		else if (mode == DC_MODE)
		{
			cfg.mode = MODE_JPG;
			update_flag(&cfg.flag, CFG_LDW, 0);
			cfg.width = cfg.height = cfg.fps = 0;
			if (!sensor_on)
			{
				ret = gp_IPC_VStream_Open(&cfg);
			}
			else if (ldw || sensor60fps)
			{
				gp_IPC_VStream_Close();
				ret = gp_IPC_VStream_Open(&cfg);
			}
			if (ret < 0) 
			{
				initial_err = 1;
				powerOff = 1;
				continue;
			}
			
			ldw = 0;
			sensor60fps = 0;
			sensor_on = 1;
			gp_IPC_VStream_Start();
			printf("Start DC Mode\n");	
			ret = dc_main(argc, argv);
			if (ret < 0)
			{
				powerOff = 1;
				continue;
			}
			mode = WAIT_MODE;
		}
		else if (mode == IDLE_MODE)
		{
			if (sensor_on)
			{
				printf("Sensor Stop\n");
				//gp_IPC_VStream_Stop();
				gp_IPC_VStream_Close();
				sensor_on = 0;
			}
			mode = WAIT_MODE;
		}

		g_bQuit = 0;
									
		usleep(5000);
	}

	if (initial_err)
	{
		printf("[camcorder] Initialize Error\n");
		dvr_pipe_init();
		dvr_pipemsg_send(CMD_READY_KEY, 0, NULL);
		dvr_pipe_close();
	}
	else
	{
		gp_IPC_VStream_Close();
	}
	printf("[camcorder] program exit\n");

	return ret;
}