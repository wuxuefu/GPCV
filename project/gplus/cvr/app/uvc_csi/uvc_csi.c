#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <pthread.h>
#include <signal.h>

#include "mach/gp_cdsp.h"
#include "mach/gp_mipi.h"
#include "mach/gp_display.h"
#include "mach/gp_display2.h"
#include "mach/gp_scale.h"
#include "mach/gp_scale2.h"
#include "mach/gp_usb.h"
#include <chunkmem.h>

#include "mach/gp_hdmi.h"
#include "gp_on2.h"
#include "ceva.h"
#include "cdsp_config.h"

#include <mach/gp_usb.h>
#include <mach/audiodrv/soundcard.h>
#include <mach/audiodrv/audio_util.h>
#include "uvc_pipe.h"
#include "cdsp_calibrate.h"
#include "cdsp_preference.h"
#include "config/sysconfig.h"

/* define */
#define PANEL_ENABLE	0
#define USE_SCALER		1
#define BUFFER_NO		4
#define SCAL_BUF_NO		2
#define JPEG_BUF_NO		2
#define AUD_BUF_NO		2
#define PCM_FRM_SIZE	2048
#define PCM_SAMPLE_RATE	22050

#define ENC_WIDTH_640	640
#define ENC_WIDTH_1024	1024
#define ENC_WIDTH_1280	1280
#define ENC_WIDTH_1920	1920

#define ENC_HEIGHT_480	480
#define ENC_HEIGHT_768	768
#define ENC_HEIGHT_720	720
#define ENC_HEIGHT_1080	1080

#define MIN_JPEG_Q		10
#define MAX_JPEG_Q		50
#define MIN_JPEG_SIZE	(160*1024)
#define MAX_JPEG_SIZE	(200*1024)

/* data type */
typedef struct ScalBuf_s
{
	unsigned short width;
	unsigned short height;
	unsigned int output_addr;
} ScalBuf_t;

typedef struct JpegBuf_s
{
	unsigned short width;
	unsigned short height;
	unsigned int output_addr;
	unsigned char *pOutputRemap;
	unsigned int size;
} JpegBuf_t;

typedef struct BufCtrl_s
{
	ScalBuf_t ScalBuf[SCAL_BUF_NO];
	ScalBuf_t *ScalEmpty[SCAL_BUF_NO];
	ScalBuf_t *ScalReady[SCAL_BUF_NO];
	pthread_mutex_t scal_epy_mutex;
	pthread_mutex_t scal_rdy_mutex;
	
	JpegBuf_t JpegBuf[JPEG_BUF_NO];
	JpegBuf_t *JpegEmpty[JPEG_BUF_NO];
	JpegBuf_t *JpegReady[JPEG_BUF_NO];
	pthread_mutex_t jpeg_epy_mutex;
	pthread_mutex_t jpeg_rdy_mutex;

	unsigned int AudBuf[AUD_BUF_NO];
	unsigned int AudEmpty[AUD_BUF_NO];
	unsigned int AudReady[AUD_BUF_NO];
	pthread_mutex_t aud_epy_mutex;
	pthread_mutex_t aud_rdy_mutex;
} BufCtrl_t;

typedef struct fmt_s
{
	char *sensor_name;
	int mipi_format;
	int sensor_format;
	int sensor_width;
	int sensor_height;
	int sensor_format_no;
} fmt_t;

static char* uvc_state_str[6] = 
{
	"IDLE",
	"Sending video",
	"Send video done",
	"Sending audio",
	"Send audio done",
	"State not ready",
};	

int disp_fd = -1;
int usb_fd = -1;
int csi_fd = -1;
unsigned int Qvalue = 50;
unsigned int g_scale_flag = 0;
unsigned int g_disp_addr = 0;
unsigned int g_sensor_addr = 0;
unsigned int g_uvc_addr = 0;
unsigned short buf_width = 0;
unsigned short buf_height = 0;
unsigned short crop_width = 0;
unsigned short crop_height = 0;
unsigned short sensor_width = 0;
unsigned short sensor_height = 0;
unsigned short jpeg_buf_num = JPEG_BUF_NO;
unsigned short fmt_num = 0;
gp_bitmap_t disp_bitmap;
BufCtrl_t buf_ctrl;

pthread_t pcm_thread;
pthread_t sensor_thread;
pthread_t jpeg_thread;
pthread_t uvc_thread;
pthread_t pipe_thread;
	
static short uvc_width = ENC_WIDTH_640;
static short uvc_height = ENC_HEIGHT_480;
static int loop = 0;

const fmt_t sensor_table[] =
{
	{
		.sensor_name = "sensor_ov3640",
		.mipi_format = (-1),
		.sensor_format = V4L2_PIX_FMT_UYVY,
		.sensor_width = 1024,
		.sensor_height = 768,
		.sensor_format_no = 0,
	},
	{
		.sensor_name = "sensor_gc0308",
		.mipi_format = (-1),
		.sensor_format = V4L2_PIX_FMT_VYUY,
		.sensor_width = 640,
		.sensor_height = 480,
		.sensor_format_no = 0,
	},
	{
		.sensor_name = "sensor_ov3640_mipi",
		.mipi_format = MIPI_YUV422,
		.sensor_format = V4L2_PIX_FMT_YVYU,
		.sensor_width = 640,
		.sensor_height = 480,
		.sensor_format_no = 0,
	},
	{
		.sensor_name = "sensor_ov3640_mipi",
		.mipi_format = MIPI_YUV422,
		.sensor_format = V4L2_PIX_FMT_YVYU,
		.sensor_width = 2048,
		.sensor_height = 1536,
		.sensor_format_no = 1,
	},
	{
		.sensor_name = "sensor_ov3640_mipi",
		.mipi_format = MIPI_YUV422,
		.sensor_format = V4L2_PIX_FMT_SBGGR8,
		.sensor_width = 640,
		.sensor_height = 480,
		.sensor_format_no = 3,
	},
	{
		.sensor_name = "sensor_ov3640_mipi",
		.mipi_format = MIPI_YUV422,
		.sensor_format = V4L2_PIX_FMT_SBGGR8,
		.sensor_width = 2048,
		.sensor_height = 1536,
		.sensor_format_no = 4,
	},
	{
		.sensor_name = "sensor_ov5650_mipi",
		.mipi_format = MIPI_RAW10,
		.sensor_format = V4L2_PIX_FMT_SBGGR8,
		.sensor_width = 640,
		.sensor_height = 480,
		.sensor_format_no = 0,
	},
	{
		.sensor_name = "sensor_ov5650_mipi",
		.mipi_format = MIPI_RAW10,
		.sensor_format = V4L2_PIX_FMT_SBGGR8,
		.sensor_width = 1280,
		.sensor_height = 720,
		.sensor_format_no = 0,
	},	
	{
		.sensor_name = "sensor_ov2710_mipi",
		.mipi_format = MIPI_RAW10,
		.sensor_format = V4L2_PIX_FMT_SBGGR8,
		.sensor_width = 1280,
		.sensor_height = 720,
		.sensor_format_no = 0,
	},
	{
		.sensor_name = "sensor_ov2710_mipi",
		.mipi_format = MIPI_RAW10,
		.sensor_format = V4L2_PIX_FMT_SBGGR8,
		.sensor_width = 1920,
		.sensor_height = 1082,
		.sensor_format_no = 1,
	},
	{
		.sensor_name = "sensor_ar0330_mipi",
		.mipi_format = MIPI_RAW10,
		.sensor_format = V4L2_PIX_FMT_SGRBG8,
		.sensor_width = 1280,
		.sensor_height = 720,
		.sensor_format_no = 0,
	},
	{
		.sensor_name = "sensor_ar0330_mipi",
		.mipi_format = MIPI_RAW10,
		.sensor_format = V4L2_PIX_FMT_SGRBG8,
		.sensor_width = 1920,
		.sensor_height = 1084,
		.sensor_format_no = 1,
	},
};


void get_cdsp_preference(gp_cdsp_user_preference_t *preference)
{
	struct v4l2_control ctl;
	gpCdspSatHue_t sat_hue;
	gpCdspEdge_t edge;
	
	ctl.id = MSG_CDSP_TARGET_AE;
	ctl.value = &(preference->ae_target);
	ioctl(csi_fd, VIDIOC_G_CTRL, &ctl);

	ctl.id = MSG_CDSP_SAT_HUE_DAY;
	ctl.value = &sat_hue;
	ioctl(csi_fd, VIDIOC_G_CTRL, &ctl);
	preference->y_scale_day = sat_hue.y_scale;
	preference->u_scale_day = sat_hue.u_scale;
	preference->v_scale_day = sat_hue.v_scale;
	preference->y_offset_day = sat_hue.y_offset;
	preference->u_offset_day = sat_hue.u_offset;
	preference->v_offset_day = sat_hue.v_offset;
	
	ctl.id = MSG_CDSP_SAT_HUE_NIGHT;
	ctl.value = &sat_hue;
	ioctl(csi_fd, VIDIOC_G_CTRL, &ctl);
	preference->y_scale_night = sat_hue.y_scale;
	preference->u_scale_night = sat_hue.u_scale;
	preference->v_scale_night = sat_hue.v_scale;
	preference->y_offset_night = sat_hue.y_offset;
	preference->u_offset_night = sat_hue.u_offset;
	preference->v_offset_night = sat_hue.v_offset;
	
	ctl.id = MSG_CDSP_EDGE;
	ctl.value = &edge;
	ioctl(csi_fd, VIDIOC_G_CTRL, &ctl);
	preference->edge_day = edge.ampga;
	preference->edge_night = 0;
	
	ctl.id = MSG_CDSP_WB_OFFSET_DAY;
	ctl.value = &preference->wb_offset_day;
	ioctl(csi_fd, VIDIOC_G_CTRL, &ctl);
	preference->wb_offset_night = 0;
}

static int scale1_act(scale_content_t *pSct)
{
	int nRet, scale_handle;
	
	/* scale1 */
	scale_handle = open("/dev/scalar", O_RDONLY);
	if (scale_handle < 0) {
		printf("scale_handle fail\n");
		return -1;
	}

	/* scale1 start */	
	nRet = ioctl(scale_handle, SCALE_IOCTL_TRIGGER, pSct);
	if (nRet < 0) {
		printf("scale_start fail\n");
		goto __exit;
	}

__exit:
	if (scale_handle >= 0) {
		close(scale_handle);
	}
	return 0;
}

static int scale2_act(gpScale2Format_t *pScale)
{
	int nRet, scale_handle;

	/* scale2 */
	scale_handle = open("/dev/scale2", O_RDWR);
	if (scale_handle < 0) {
		printf("scale2_handle fail\n");
		return -1;
	}

	/* scale2 start */
#if 1
	nRet = ioctl(scale_handle, SCALE2_IOCTL_S_START, pScale);
#else
	nRet = ioctl(scale_handle, SCALE2_IOCTL_S_START_WITHOUT_WAIT, pScale);
#endif
	if (nRet == C_SCALE2_STATUS_DONE || nRet == C_SCALE2_STATUS_STOP) {
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
	if (nRet < 0) {
		printf("SCALE2_IOCTL_S_STOP fail\n");
		goto __exit;
	}
	
__exit:	
	if (scale_handle >= 0) {
		close(scale_handle);
	}
	return nRet;
}

static int Panel_Init(int DispDev)
{
	int disp_handle;
	gp_disp_res_t panel_res;
	gp_disp_output_t dispOutput;

	switch(DispDev)
	{
	case 0:
		//display 0, Panel out.
		printf("use display 0 TFT Out\n");
		disp_handle = open("/dev/disp0", O_RDWR);
		if (disp_handle <= 0) {
			printf("disp_handle fail!\n");
			return -1;
		}

		/* Setup display */	
		if (ioctl(disp_handle, DISPIO_SET_INITIAL, 0) < 0) {
			printf("disp initial fail\n");
			return -1;
		}	
		
		if (ioctl(disp_handle, DISPIO_GET_PANEL_RESOLUTION, &panel_res) < 0) {
			printf("disp get panel resolution fail\n");
			return -1;
		}
		
		printf("PanelSize = [%d,%d]\n", panel_res.width, panel_res.height);	

		/* set primary layer bitmap */
		memset((void *)&disp_bitmap, 0x0, sizeof(gp_bitmap_t));
		disp_bitmap.width = panel_res.width;
		disp_bitmap.height = panel_res.height;
		disp_bitmap.bpl = panel_res.width * 2;
		disp_bitmap.type = SP_BITMAP_RGB565;
		//disp_bitmap.type = SP_BITMAP_YUYV;
		disp_bitmap.pData = 0;
		break;

	case 1:
		//display 1, Panel out.
		printf("use display 1 TFT Out\n");
		disp_handle = open("/dev/disp1", O_RDWR);
		if (disp_handle <= 0) {
			printf("disp_handle fail!\n");
			return -1;
		}

		/* Setup display */	
		if (ioctl(disp_handle, DISPIO_SET_INITIAL, 0) < 0) {
			printf("disp initial fail\n");
			return -1;
		}	
		
		if (ioctl(disp_handle, DISPIO_GET_PANEL_RESOLUTION, &panel_res) < 0) {
			printf("disp get panel resolution fail\n");
			return -1;
		}
		
		printf("PanelSize = [%d,%d]\n", panel_res.width, panel_res.height);	

		/* set primary layer bitmap */
		memset((void *)&disp_bitmap, 0x0, sizeof(gp_bitmap_t));
		disp_bitmap.width = panel_res.width;
		disp_bitmap.height = panel_res.height;
		disp_bitmap.bpl = panel_res.width * 2;
		disp_bitmap.type = SP_BITMAP_RGB565;
		//disp_bitmap.type = SP_BITMAP_YUYV;
		disp_bitmap.pData = 0;
		break;

	case 2:
		//display 2, TV out.
		printf("use display 2 TV out\n");
		disp_handle = open("/dev/disp2", O_RDWR);
		if (disp_handle <= 0) {
			printf("disp_handle fail!\n");
			return -1;
		}
		
		/* Setup display */	
		if (ioctl(disp_handle, DISPIO_SET_INITIAL, 0) < 0) {
			printf("disp initial fail\n");
			return -1;
		}	
		
		if (ioctl(disp_handle, DISPIO_GET_PANEL_RESOLUTION, &panel_res) < 0) {
			printf("disp get panel resolution fail\n");
			return -1;
		}
		
		printf("PanelSize = [%d,%d]\n", panel_res.width, panel_res.height);	

		/* set primary layer bitmap */
		memset((void *)&disp_bitmap, 0x0, sizeof(gp_bitmap_t));
		disp_bitmap.width = panel_res.width;
		disp_bitmap.height = panel_res.height;
		disp_bitmap.bpl = panel_res.width * 2;
		disp_bitmap.type = SP_BITMAP_RGB565;
		//disp_bitmap.type = SP_BITMAP_YUYV;
		disp_bitmap.pData = 0;
		break;

	case 3:
		//display 0, HDMI out.
		printf("use display 0 HDMI Out\n");
		disp_handle = open("/dev/disp0", O_RDWR);
		if (disp_handle <= 0) {
			printf("disp_handle fail!\n");
			return -1;
		}
		
		dispOutput.type = -1;
		if (ioctl(disp_handle, DISPIO_GET_OUTPUT, &dispOutput) < 0) {
			printf("disp get output fail\n");
			return -1;
		}
		
		if (dispOutput.type != SP_DISP_OUTPUT_HDMI) {
			dispOutput.type = SP_DISP_OUTPUT_HDMI;
			dispOutput.mode = HDMI_1280X720P60;
			if (ioctl(disp_handle, DISPIO_SET_OUTPUT, &dispOutput) < 0) {
				printf("disp set output fail\n");
				return -1;
			}
		
			if (ioctl(disp_handle, DISPIO_SET_INITIAL, 0) < 0) {
				printf("disp initial fail\n");
				return -1;
			}
		}

		memset((void *)&disp_bitmap, 0x0, sizeof(gp_bitmap_t));
		disp_bitmap.width = 1280;
		disp_bitmap.height = 720;
		disp_bitmap.bpl = 1280 * 2;
		disp_bitmap.type = SP_BITMAP_RGB565;
		//disp_bitmap.type = SP_BITMAP_YUYV;
		disp_bitmap.pData = 0;
		break;

	default:
		printf("Display Device Error\n");
		return -1;
	}
	
	g_disp_addr = (unsigned int) gpChunkMemAlloc(disp_bitmap.bpl * disp_bitmap.height * BUFFER_NO);
	if (g_disp_addr == 0) {
		printf("disp mem alloc fail!\n");
		return -1;
	}
	
	return disp_handle;
}

static int Panel_Close(int disp_handle)
{
	if (disp_handle >= 0) {
		close(disp_handle);
	}

	if (g_disp_addr) {
		gpChunkMemFree((void *)g_disp_addr);
	}
	return 0;
}

static int Panel_Display(int disp_handle, unsigned int frame_addr, unsigned short input_w, unsigned short input_h)
{
	static unsigned char nIndex = 0;
	struct timeval oldTimeVal, newTimeVal, TimeVal;
	struct timezone oldTimeZone, newTimeZone;
#if USE_SCALER == 1
	scale_content_t sct;
#elif USE_SCALER == 2
	gpScale2Format_t scale;
#endif

	disp_bitmap.pData = (void *)(g_disp_addr + disp_bitmap.bpl * disp_bitmap.height * nIndex);
	nIndex++;
	if (nIndex >= BUFFER_NO) {
		nIndex = 0;
	}
	
#if 1 //use scale
	if (gettimeofday(&oldTimeVal, &oldTimeZone) < 0) {
		printf("line%d:gettimeerror\n", __LINE__);
	}
	
#if USE_SCALER == 1
	memset(&sct, 0, sizeof(sct));
	sct.src_img.pData = (void *)frame_addr;
	sct.src_img.width = input_w;
	sct.src_img.height = input_h;
	sct.src_img.bpl = input_w*2;
	sct.src_img.type = SP_BITMAP_YUYV;
		
	sct.clip_rgn.x = 0;
	sct.clip_rgn.y = 0;
	sct.clip_rgn.width = input_w;
	sct.clip_rgn.height = input_h;
		
	sct.scale_rgn.x = 0;
	sct.scale_rgn.y = 0;
	sct.scale_rgn.width = disp_bitmap.width;
	sct.scale_rgn.height = disp_bitmap.height;

	sct.dst_img.pData = (void *)disp_bitmap.pData;
	sct.dst_img.width = disp_bitmap.width;
	sct.dst_img.height = disp_bitmap.height;
	sct.dst_img.bpl = disp_bitmap.width*2;
	sct.dst_img.type = SP_BITMAP_RGB565;
	
	scale1_act(&sct);
#elif USE_SCALER == 2
	memset(&scale, 0, sizeof(gpScale2Format_t));
	scale.input_format = C_SCALE2_CTRL_IN_VYUY;
	scale.input_width = input_w;
	scale.input_height = input_h;
	scale.input_visible_width = 0;
	scale.input_visible_height = 0;
	scale.input_x_offset = 0;
	scale.input_y_offset = 0;
	scale.input_y_addr = frame_addr;
	scale.input_u_addr = 0;
	scale.input_v_addr = 0;
	
	scale.output_format = C_SCALE2_CTRL_OUT_RGB565;
	scale.output_width = disp_bitmap.width;
	scale.output_height = disp_bitmap.height;
	scale.output_buf_width = disp_bitmap.width;
	scale.output_buf_height = disp_bitmap.height;
	scale.output_x_offset = 0;
	scale.output_y_addr = (unsigned int)disp_bitmap.pData;
	scale.output_u_addr = 0;
	scale.output_v_addr = 0;	
	
	scale.fifo_mode = C_SCALE2_CTRL_FIFO_DISABLE;
	scale.scale_mode = C_SCALE2_BY_RATIO;
	scale.digizoom_m = 10;
	scale.digizoom_n = 10;

	scale2_act(&scale);
#endif

	if (gettimeofday(&newTimeVal, &newTimeZone) < 0) {
		printf("line%d:gettimeerror\n", __LINE__);
	}

	TimeVal.tv_sec = newTimeVal.tv_sec - oldTimeVal.tv_sec;
	TimeVal.tv_usec = newTimeVal.tv_usec - oldTimeVal.tv_usec;
	if(TimeVal.tv_usec < 0) {
		TimeVal.tv_sec -= 1;
		TimeVal.tv_usec += 1000000;
	}

	//printf("Scale[%ds,%dus]\n", TimeVal.tv_sec, TimeVal.tv_usec);
#else
	disp_bitmap.pData = (void *)frame_addr;
#endif

	ioctl(disp_handle, DISPIO_SET_PRI_BITMAP, &disp_bitmap);
	ioctl(disp_handle, DISPIO_SET_PRI_ENABLE, 1);
	ioctl(disp_handle, DISPIO_SET_UPDATE, 0);
	ioctl(disp_handle, DISPIO_WAIT_FRAME_END, 0);
	return 0;
};

static int jpeg_encode_init(void)
{
	int ret;
	
	ret = gpOn2Codec_Load(CEVA_CODEC_TYPE_JPG_ENCODE);
	if (ret < 0) {
		printf("CEVA Init Fail\n");
        return -1;
    }	

	return 0;
}

static void jpeg_encode_uninit(void)
{
	gpOn2Codec_Unload();
}

static int jpeg_encode_once(unsigned char *inbuf, unsigned char *pOutput, int width, int height)
{
	int ret;
    cevaEncode_t vdt;
			
	memset(&vdt, 0x00, sizeof(vdt));
	vdt.pInBuf[0] = inbuf;
	vdt.width     = width;
    vdt.height    = height;
    vdt.QType     = Qvalue;
    vdt.flags     = 0; //VFLAG_YUV422IN;
   	vdt.pOutBuf   = pOutput;
   	vdt.nBytes	  = width * height;
	vdt.inputFormat = SP_BITMAP_YUYV;

	ret = gpOn2Codec_Init(&vdt);
	if (ret < 0) {
		printf("On2 Init Fail\n");
		goto __exit;
	}

	ret = gpOn2Codec_Exec(&vdt);
	if (ret < 0) {
		printf("On2 Exec Fail\n");
        goto __exit;
    }
	
	ret = vdt.nBytes;
	if(ret >= width * height) {
		printf("jpeg = 0x%x, 0x%x\n", vdt.pOutBuf, 	vdt.nBytes);
	}

#if 0
	Panel_Display(disp_fd, (unsigned int)inbuf, width, height);
#endif
#if 0
	{
		static int index = 0;
		char jpeg[32] = "/media/sdcardc1/";
		char file[32];
		int fd;
			
		sprintf(file, "test%04d.jpg", index++);					
		strcat(jpeg, file);					
		printf("File = %s\n", jpeg);
		fd = open(jpeg, O_CREAT|O_WRONLY|O_TRUNC);
		write(fd, pOutput, ret);
		close(fd);
		sync();
	}
#endif	
	
__exit:	
	gpOn2Codec_Uninit(&vdt);	
	return ret;
}

int csi1_init(char *sensor_name, int sensor_format, int format_no)
{
	char *p;
	int i, fd, index;
	struct v4l2_capability cap;
	struct v4l2_input input;
	struct v4l2_fmtdesc fmtdest;
	struct v4l2_format fmt;
	struct v4l2_control ctrl;
	struct v4l2_requestbuffers req;
	struct v4l2_buffer buf;
	
	/* init csi */
	fd = open("/dev/csi1", O_RDWR);
	if (fd < 0) {
		printf("csi open fail!!!\n");
		goto __exit;
	}
	
	/* capacity */
	if (ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0) {
		printf("VIDIOC_QUERYCAP fail!!!\n");
		goto __exit;
	}
	
	if (cap.capabilities != V4L2_CAP_VIDEO_CAPTURE) {
		printf("V4L2_CAP_VIDEO_CAPTURE fail!!!\n");
		goto __exit;
	}
	
	/* Find sensor driver */
	for (i=0; i<4; i++) {
		memset((void *)&input, 0x0, sizeof(input));
		input.index = i;
		ioctl(fd, VIDIOC_ENUMINPUT, &input);
		printf("sensor[%d] = %s\n", i, input.name);
		if (strcmp(input.name, sensor_name) == 0) {
			index = i;
			break;
		}
	}

	if (i == 4) {
		printf("Not Find device!!!\n");
		goto __exit;
	}
	
	/* set input source */
	if (ioctl(fd, VIDIOC_S_INPUT, index) < 0) {
		printf("VIDIOC_S_INPUT fail!!!\n");
		goto __exit;
	}

	/* get sensor size */
	memset((void *)&fmtdest, 0x0, sizeof(fmtdest));
	fmtdest.index = format_no;
	if (ioctl(fd, VIDIOC_ENUM_FMT, &fmtdest) < 0) {
		printf("VIDIOC_ENUM_FMT fail!!!\n");
		goto __exit;
	}

	if (fmtdest.pixelformat != sensor_format) {
		sensor_format = fmtdest.pixelformat;
	}

	printf("%s\n", fmtdest.description);
	p = strchr((char *)(fmtdest.description), '=');
	if (p) {
		int ret;
		
		ret = sscanf(p, "=%d*%d,crop=%d*%d", &buf_width, &buf_height, &crop_width, &crop_height);		
		if (ret == 2) {
			crop_width = buf_width;
			crop_height = buf_height;
		} else if (ret == 4) {
			
		} else {
			buf_width = sensor_table[fmt_num].sensor_width;
			buf_height = sensor_table[fmt_num].sensor_height;
			crop_width = sensor_table[fmt_num].sensor_width;
			crop_height = sensor_table[fmt_num].sensor_height;
		}
	} else {
		buf_width = sensor_table[fmt_num].sensor_width;
		buf_height = sensor_table[fmt_num].sensor_height;
		crop_width = sensor_table[fmt_num].sensor_width;
		crop_height = sensor_table[fmt_num].sensor_height;
	}

	printf("buf_size = %dx%d\n", buf_width, buf_height);
	printf("crop_size = %dx%d\n", crop_width, crop_height);

	/* set input format */
	memset((void *)&fmt, 0, sizeof(struct v4l2_format));
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = buf_width;
	fmt.fmt.pix.height = buf_height;
	fmt.fmt.pix.pixelformat = sensor_format;
	fmt.fmt.pix.priv = format_no;
	fmt.fmt.pix.field = V4L2_FIELD_NONE;
	if (ioctl (fd, VIDIOC_S_FMT, &fmt) < 0) {
		printf("VIDIOC_S_FMT fail!!!\n");
		goto __exit;
	}

	/* get input format */
	if (ioctl (fd, VIDIOC_G_FMT, &fmt) < 0) {
		printf("VIDIOC_S_FMT fail!!!\n");
		goto __exit;
	}

	sensor_width = fmt.fmt.pix.width;
	sensor_height = fmt.fmt.pix.height;
	printf("sensor_size = %dx%d\n", sensor_width, sensor_height);
	
	/* alloc buffer */
	g_sensor_addr = (unsigned int) gpChunkMemAlloc(buf_width * buf_height * 2 * BUFFER_NO);
	printf("g_sensor_addr = 0x%x\n", g_sensor_addr);
	if (g_sensor_addr == 0) {
		printf("mem alloc fail!\n");
		goto __exit;
	}
	
	/* set frame buffer number */
	memset((void *)&req, 0, sizeof(struct v4l2_requestbuffers));
	req.count = BUFFER_NO;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_USERPTR;
	if (ioctl(fd, VIDIOC_REQBUFS, &req) < 0) {
		printf("VIDIOC_REQBUFS fail!!!\n");
		goto __exit;
	}
	
	/* set frame buffer address */
	for (i=0; i<BUFFER_NO; i++) {
		memset((void *)&buf, 0, sizeof(struct v4l2_buffer));
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_USERPTR;
		buf.index = i;
		buf.m.userptr = (unsigned long)g_sensor_addr + buf_width*buf_height*2*i;;
		buf.length = buf_width*buf_height*2;
	
		if (ioctl(fd, VIDIOC_QBUF, &buf) < 0) {
			printf("VIDIOC_QBUF fail!!!\n");
			goto __exit;
		}
	}
	
	/* set start */
	printf("csi stream on.\n");
	if (ioctl(fd, VIDIOC_STREAMON, NULL) < 0) {
		printf("VIDIOC_STREAMON fail!!!\n");
		goto __exit;
	}

	return fd;
__exit:
	if (g_sensor_addr) {
		gpChunkMemFree((void *)g_sensor_addr);
		g_sensor_addr = 0;
	}

	if (fd >= 0) {
		close(fd);
	}
	return -1;
}

int cdsp_init(char *sensor_name, int sensor_format, int format_no)
{
	char *p;
	int i, fd, index;
	struct v4l2_capability cap;
	struct v4l2_input input;
	struct v4l2_fmtdesc fmtdest;
	struct v4l2_format fmt;
	struct v4l2_control ctrl;
	struct v4l2_requestbuffers req;
	struct v4l2_buffer buf;
	
	/* init csi */
	fd = open("/dev/cdsp", O_RDWR);
	if (fd < 0) {
		printf("csi open fail!!!\n");
		goto __exit;
	}
	
	/* capacity */
	if (ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0) {
		printf("VIDIOC_QUERYCAP fail!!!\n");
		goto __exit;
	}
	
	if (cap.capabilities != V4L2_CAP_VIDEO_CAPTURE) {
		printf("V4L2_CAP_VIDEO_CAPTURE fail!!!\n");
		goto __exit;
	}
	
	/* Find sensor driver */
	for (i=0; i<4; i++) {
		memset((void *)&input, 0x0, sizeof(input));
		input.index = i;
		ioctl(fd, VIDIOC_ENUMINPUT, &input);
		printf("sensor[%d] = %s\n", i, input.name);
		if(strcmp(input.name, sensor_name) == 0) {
			index = i;
			break;
		}
	}

	if (i == 4) {
		printf("Not Find device!!!\n");
		goto __exit;
	}
	
	ioctl(fd, VIDIOC_S_CALIBRATE, &ar0330_cdsp_calibration);
	
	/* set input source */
	if (ioctl(fd, VIDIOC_S_INPUT, index) < 0) {
		printf("VIDIOC_S_INPUT fail!!!\n");
		goto __exit;
	}

	/* get sensor size */
	memset((void *)&fmtdest, 0x0, sizeof(fmtdest));
	fmtdest.index = format_no;
	if (ioctl(fd, VIDIOC_ENUM_FMT, &fmtdest) < 0) {
		printf("VIDIOC_ENUM_FMT fail!!!\n");
		goto __exit;
	}

	if (fmtdest.pixelformat != sensor_format) {
		sensor_format = fmtdest.pixelformat;
	}

	printf("%s\n", fmtdest.description);
	p = strchr((char *)(fmtdest.description), '=');
	if (p) {
		int ret;
		
		ret = sscanf(p, "=%d*%d,crop=%d*%d", &buf_width, &buf_height, &crop_width, &crop_height);		
		if (ret == 2) {
			crop_width = buf_width;
			crop_height = buf_height;
		} else if (ret == 4) {
			
		} else {
			buf_width = sensor_table[fmt_num].sensor_width;
			buf_height = sensor_table[fmt_num].sensor_height;
			crop_width = sensor_table[fmt_num].sensor_width;
			crop_height = sensor_table[fmt_num].sensor_height;
		}
	} else {
		buf_width = sensor_table[fmt_num].sensor_width;
		buf_height = sensor_table[fmt_num].sensor_height;
		crop_width = sensor_table[fmt_num].sensor_width;
		crop_height = sensor_table[fmt_num].sensor_height;
	}

	printf("buf_size = %dx%d\n", buf_width, buf_height);
	printf("crop_size = %dx%d\n", crop_width, crop_height);

	/* set input format */	
	memset((void *)&fmt, 0, sizeof(struct v4l2_format));
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = buf_width;
	fmt.fmt.pix.height = buf_height;
	fmt.fmt.pix.pixelformat = sensor_format;
	fmt.fmt.pix.priv = format_no;
	fmt.fmt.pix.field = V4L2_FIELD_NONE;
	if (ioctl (fd, VIDIOC_S_FMT, &fmt) < 0) {
		printf("VIDIOC_S_FMT fail!!!\n");
		goto __exit;
	}

	/* get input format */
	if (ioctl (fd, VIDIOC_G_FMT, &fmt) < 0) {
		printf("VIDIOC_S_FMT fail!!!\n");
		goto __exit;
	}

	sensor_width = fmt.fmt.pix.width;
	sensor_height = fmt.fmt.pix.height;
	printf("sensor_size = %dx%d\n", sensor_width, sensor_height);

	if(strcmp(sensor_name, "sensor_ar0330_mipi") == 0) {
		/* cdsp enable AE/AWB and scale down */
		gp_cdsp_enable(fd, buf_width, buf_height, &cdsp_user_preference);
	} else {
		/* scale down */
		if(sensor_width > buf_width || sensor_height > buf_height) {
			gpCdspScalePara_t  cdspScale;
			struct v4l2_control ctrl;

			memset(&cdspScale, 0x00, sizeof(cdspScale));
			cdspScale.yuvhscale_en = 1;
			cdspScale.yuvhscale_mode = 1;
			cdspScale.yuvvscale_en = 1;
			cdspScale.yuvvscale_mode = 1;
			cdspScale.yuv_dst_hsize = buf_width;
			cdspScale.yuv_dst_vsize = buf_height;

			cdspScale.img_rb_h_size = buf_width;
			cdspScale.img_rb_v_size = buf_height;

			memset(&ctrl, 0x00, sizeof(ctrl));
			ctrl.id = MSG_CDSP_SCALE_CROP;
			ctrl.value = (int)&cdspScale;

			if (ioctl(fd, VIDIOC_S_CTRL, &ctrl) < 0) {
				printf("VIDIOC_S_CTRL fail!\n");
				goto __exit;
			}
		}
	}

	/* alloc buffer */
	g_sensor_addr = (unsigned int) gpChunkMemAlloc(buf_width * buf_height * 2 * BUFFER_NO);
	printf("g_sensor_addr = 0x%x\n", g_sensor_addr);
	if (g_sensor_addr == 0) {
		printf("mem alloc fail!\n");
		goto __exit;
	}
	
	/* set frame buffer number */
	memset((void *)&req, 0, sizeof(struct v4l2_requestbuffers));
	req.count = BUFFER_NO;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_USERPTR;
	if (ioctl(fd, VIDIOC_REQBUFS, &req) < 0) {
		printf("VIDIOC_REQBUFS fail!!!\n");
		goto __exit;
	}
	
	/* set frame buffer address */
	for (i=0; i<BUFFER_NO; i++) {
		memset((void *)&buf, 0, sizeof(struct v4l2_buffer));
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_USERPTR;
		buf.index = i;
		buf.m.userptr = (unsigned long)g_sensor_addr + buf_width*buf_height*2*i;;
		buf.length = buf_width*buf_height*2;
	
		if(ioctl(fd, VIDIOC_QBUF, &buf) < 0) {
			printf("VIDIOC_QBUF fail!!!\n");
			goto __exit;
		}
	}
	
	if(	strcmp(SYSCONFIG_PLATFORM, "gplus.cvr_turnkey_demo1") == 0 ) {
		ctrl.id = V4L2_CID_VFLIP;
		ctrl.value = 1;
		ioctl(fd, VIDIOC_S_CTRL, &ctrl);
		ctrl.id = V4L2_CID_HFLIP;
		ctrl.value = 1;
		ioctl(fd, VIDIOC_S_CTRL, &ctrl);
	}		
	
	/* set start */
	printf("csi stream on.\n");
	if (ioctl(fd, VIDIOC_STREAMON, NULL) < 0) {
		printf("VIDIOC_STREAMON fail!!!\n");
		goto __exit;
	}

	return fd;
__exit:
	if (g_sensor_addr) {
		gpChunkMemFree((void *)g_sensor_addr);
		g_sensor_addr = 0;
	}

	if (fd >= 0) {
		close(fd);
	}
	return -1;
}


int mipi_init(char *sensor_name, int mipi_format, int width, int height)
{
	int fd = -1;
	int io = 0;		
	gpMipiCCIR601_t mipi_ccir601;	

	/* mipi1 to cdsp open */
	printf("open mipi1\r\n");	
	fd = open("/dev/mipi1", O_RDWR);	
	if (fd < 0) {		
		printf("mipi1 open fail\n");		
		return -1;	
	}

	memset(&mipi_ccir601, 0x00, sizeof(mipi_ccir601));
	mipi_ccir601.data_from_mmr = 0;	// auto detect input format
	mipi_ccir601.data_type = mipi_format;	
	mipi_ccir601.data_type_to_cdsp = ENABLE;	
	mipi_ccir601.h_size = width;	
	mipi_ccir601.v_size = height;	
	mipi_ccir601.h_back_porch = 0;
	mipi_ccir601.h_front_porch = 4;
	mipi_ccir601.blanking_line_en = ENABLE;	
	if (ioctl(fd, MIPI_IOCTL_S_CCIR601, &mipi_ccir601) < 0) {		
		printf("MIPI_IOCTL_S_CCIR601 fail!\n");		
		return -1;	
	}

#if 1	
	/* IOC27 ~ IOC24 */	
	printf("IO_IS_MIPI_PIN, IOC27-IOC24\n");	
	io = C_IO_IS_MIPI_PIN;	
#else	
	/* IOC3 ~ IOC0 */	
	printf("IO_IS_CMOS_PIN, IOC3-IOC0\n");	
	io = C_IO_IS_CMOS_PIN;
#endif	
	if (ioctl(fd, MIPI_IOCTL_S_IO_PIN, &io) < 0) {		
		printf("MIPI_IOCTL_S_IO_PIN fail!\n");		
		return -1;	
	}

	/* set mipi start */	
	if (ioctl(fd, MIPI_IOCTL_S_START, sensor_name) < 0) {		
		printf("MIPI_IOCTL_S_START fail!\n");		
		return -1;	
	}
	
	return fd;
}

void sighandler(int sig)
{
	printf("Got signal 0x%x\n", sig);
	/* leave main loop */
	loop = 0;

	// exit and wait thread stop  
	pthread_join(pipe_thread, NULL);
	pthread_join(uvc_thread, NULL);
#if (AUDIO_SUPPORT == 1)	
	pthread_join(pcm_thread, NULL);
#endif
	pthread_join(sensor_thread, NULL);
	
	pthread_mutex_destroy(&buf_ctrl.scal_epy_mutex);
	pthread_mutex_destroy(&buf_ctrl.scal_rdy_mutex);
	pthread_mutex_destroy(&buf_ctrl.jpeg_epy_mutex);
	pthread_mutex_destroy(&buf_ctrl.jpeg_rdy_mutex);
	pthread_mutex_destroy(&buf_ctrl.aud_epy_mutex);
	pthread_mutex_destroy(&buf_ctrl.aud_rdy_mutex);
	printf("sighandler finish...\n");
}	

ScalBuf_t *Scale_Get_Ready_Buf(void)
{
	int i;
	ScalBuf_t *pScal;

	pthread_mutex_lock(&buf_ctrl.scal_rdy_mutex);
	pScal = buf_ctrl.ScalReady[0];	
	if(pScal == 0) {
		pthread_mutex_unlock(&buf_ctrl.scal_rdy_mutex);
		return 0;
	}

	// clear scale ready buffer
	for (i=0; i<(SCAL_BUF_NO-1); i++) {
		buf_ctrl.ScalReady[i] = buf_ctrl.ScalReady[i+1];
	}
		
	buf_ctrl.ScalReady[SCAL_BUF_NO-1] = 0;
	pthread_mutex_unlock(&buf_ctrl.scal_rdy_mutex);
	return pScal;
}

ScalBuf_t *Scale_Get_Empty_Buf(void)
{
	int i;
	ScalBuf_t *pScal;

	pthread_mutex_lock(&buf_ctrl.scal_epy_mutex);	
	pScal = buf_ctrl.ScalEmpty[0];
	if(pScal == 0) {
		pthread_mutex_unlock(&buf_ctrl.scal_epy_mutex);
		return 0;
	}

	// clear scale empty buffer	
	for (i=0; i<(SCAL_BUF_NO-1); i++) {
		buf_ctrl.ScalEmpty[i] = buf_ctrl.ScalEmpty[i+1];
	}
		
	buf_ctrl.ScalEmpty[SCAL_BUF_NO-1] = 0;
	pthread_mutex_unlock(&buf_ctrl.scal_epy_mutex);
	return pScal;
}

void Scale_Set_Ready_Buf(ScalBuf_t *pScal)
{
	int i;
	
	// set scale ready buffer
	pthread_mutex_lock(&buf_ctrl.scal_rdy_mutex);	
	for (i=0; i<SCAL_BUF_NO; i++) {
		if (buf_ctrl.ScalReady[i] == 0) {
			buf_ctrl.ScalReady[i] = pScal;
			break;
		}
	}

	if (i == SCAL_BUF_NO) {
		printf("Error: ScalReady Full\n");
	}
	
	pthread_mutex_unlock(&buf_ctrl.scal_rdy_mutex);
}

void Scale_Set_Empty_Buf(ScalBuf_t *pScal)
{
	int i;
	
	// set scale empty buffer
	pthread_mutex_lock(&buf_ctrl.scal_epy_mutex);	
	for (i=0; i<SCAL_BUF_NO; i++) {
		if (buf_ctrl.ScalEmpty[i] == 0) {
			buf_ctrl.ScalEmpty[i] = pScal;
			break;
		}
	}

	if (i == SCAL_BUF_NO) {
		printf("Error: ScalEmptyBuf Full\n");
	}
	
	pthread_mutex_unlock(&buf_ctrl.scal_epy_mutex);
}

JpegBuf_t *Jpeg_Get_Ready_Buf(void)
{
	int i;
	JpegBuf_t *pJpeg;

	pthread_mutex_lock(&buf_ctrl.jpeg_rdy_mutex);
	pJpeg = buf_ctrl.JpegReady[0];	
	if(pJpeg == 0) {
		pthread_mutex_unlock(&buf_ctrl.jpeg_rdy_mutex);
		return 0;
	}

	// clear scale ready buffer
	for (i=0; i<(JPEG_BUF_NO-1); i++) {
		buf_ctrl.JpegReady[i] = buf_ctrl.JpegReady[i+1];
	}
		
	buf_ctrl.JpegReady[JPEG_BUF_NO-1] = 0;
	pthread_mutex_unlock(&buf_ctrl.jpeg_rdy_mutex);
	return pJpeg;
}

JpegBuf_t *Jpeg_Get_Empty_Buf(void)
{
	int i;
	JpegBuf_t *pJpeg;

	pthread_mutex_lock(&buf_ctrl.jpeg_epy_mutex);	
	pJpeg = buf_ctrl.JpegEmpty[0];
	if(pJpeg == 0) {
		pthread_mutex_unlock(&buf_ctrl.jpeg_epy_mutex);
		return 0;
	}

	// clear scale ready buffer	
	for (i=0; i<(JPEG_BUF_NO-1); i++) {
		buf_ctrl.JpegEmpty[i] = buf_ctrl.JpegEmpty[i+1];
	}
		
	buf_ctrl.JpegEmpty[JPEG_BUF_NO-1] = 0;
	pthread_mutex_unlock(&buf_ctrl.jpeg_epy_mutex);
	return pJpeg;
}

void Jpeg_Set_Ready_Buf(JpegBuf_t *pJpeg)
{
	int i;
	
	// set jpeg ready buffer
	pthread_mutex_lock(&buf_ctrl.jpeg_rdy_mutex);
	for (i=0; i<JPEG_BUF_NO; i++) {
		if (buf_ctrl.JpegReady[i] == 0) {
			buf_ctrl.JpegReady[i] = pJpeg;
			break;
		}
	}

	if (i == JPEG_BUF_NO) {
		printf("Error: JpegReady Full\n");
	}

	pthread_mutex_unlock(&buf_ctrl.jpeg_rdy_mutex);
}

void Jpeg_Set_Empty_Buf(JpegBuf_t *pJpeg)
{
	int i;
	
	// set jpeg empty buffer
	pthread_mutex_lock(&buf_ctrl.jpeg_epy_mutex);
	for (i=0; i<JPEG_BUF_NO; i++) {
		if (buf_ctrl.JpegEmpty[i] == 0) {
			buf_ctrl.JpegEmpty[i] = pJpeg;
			break;
		}
	}

	if (i == JPEG_BUF_NO) {
		printf("Error: JpegEmpty Full\n");
	}

	pthread_mutex_unlock(&buf_ctrl.jpeg_epy_mutex);
}

void *pipe_receive_thread(void *param)
{
	unsigned int msgId;
	void* msgPara = NULL;
	int idx;
	gp_cdsp_user_preference_t preference;

	while(1) {
		if (!uvc_pipemsg_receive(&msgId, &msgPara))
			usleep(100000);
		else {
			if (msgId == UVC_G_PREFERENCE)
			{
				get_cdsp_preference(&preference);
				uvc_pipemsg_send(UVC_G_PREFERENCE_READY, sizeof(gp_cdsp_user_preference_t), (char *)&preference);
			} else
				usleep(100000);
		}
	}
	return NULL;
}

void *pcm_encode_thread(void *param)
{
	char pcm_temp[PCM_FRM_SIZE];
	int ch, sample_rate, format;
	int input_src, volume;
	int dsp_fd = -1;
	int mixer_fd = -1;
	int i, ret;
	unsigned int audio_addr = 0;
	unsigned int pcm_buf;
	audio_buf_info audio_info;
	BufCtrl_t *Ctrl = (BufCtrl_t *)param;
	
	printf("%s entry.\n", __func__);

	/* audio pcm buffer */
	audio_addr = (unsigned int) gpChunkMemAlloc(PCM_FRM_SIZE * AUD_BUF_NO);
	if (audio_addr == 0) {
		printf("audio buffer alloc fail\n");
		goto __exit;
	}

	for (i=0; i<AUD_BUF_NO; i++) {
		Ctrl->AudBuf[i] = audio_addr + (PCM_FRM_SIZE * i);
		Ctrl->AudEmpty[i] = Ctrl->AudBuf[i];
		Ctrl->AudReady[i] = 0x00;
	}

	// open audio
	dsp_fd = open("/dev/dsp", O_RDONLY);
	if (dsp_fd < 0) {
		printf("dsp open fail\n");
		goto __exit;
	}

    ret = ioctl(dsp_fd, SNDCTL_DSP_NONBLOCK, NULL);
	if (ret < 0) {
		printf("SNDCTL_DSP_NONBLOCK fail\n");
		goto __exit;
	}
	
    ch = 1; //mono
    ret = ioctl(dsp_fd, SNDCTL_DSP_CHANNELS, &ch);
	if (ret < 0) {
		printf("SNDCTL_DSP_CHANNELS fail\n");
		goto __exit;
	}

	sample_rate = PCM_SAMPLE_RATE;
    ret = ioctl(dsp_fd, SNDCTL_DSP_SPEED, &sample_rate);
	if (ret < 0) {
		printf("SNDCTL_DSP_SPEED fail\n");
		goto __exit;
	}

	format = AFMT_S16_LE;
    ret = ioctl(dsp_fd, SNDCTL_DSP_SETFMT, &format);
	if (ret < 0) {
		printf("SNDCTL_DSP_SETFMT fail\n");
		goto __exit;
	}

	/* input source */	
	mixer_fd = open("/dev/mixer", O_WRONLY);
	if (mixer_fd < 0) {
		printf("mixer open fail\n");
		goto __exit;
	}

	input_src = SOUND_MASK_MIC;
	ret = ioctl(mixer_fd, SOUND_MIXER_WRITE_RECSRC, &input_src); 
	if (ret < 0) {
		printf("SOUND_MIXER_WRITE_RECSRC fail\n");
		goto __exit;
	}
	
    ret = ioctl(mixer_fd, SOUND_MIXER_READ_MIC, &volume);
	if (ret < 0) {
		printf("SOUND_MIXER_READ_MIC fail\n");
		goto __exit;
	}
	
	printf("default volume: 0x%x\n", volume);
    volume = 100;
	ret = ioctl(mixer_fd, SOUND_MIXER_WRITE_MIC, &volume);
	if (ret < 0) {
		printf("SOUND_MIXER_WRITE_MIC fail\n");
		goto __exit;
	}
	
	ret = ioctl(mixer_fd, SOUND_MIXER_READ_MIC, &volume);
	if (ret < 0) {
		printf("SOUND_MIXER_READ_MIC fail\n");
		goto __exit;
	}
	printf("set volume: 0x%x\n", volume);

	// start audio
	ret = read(dsp_fd,(SINT8 *)audio_addr, PCM_FRM_SIZE);
	
	while (loop) {
		ret = ioctl(dsp_fd, SNDCTL_DSP_GETISPACE, &audio_info);	
		if (ret < 0) {
			printf("SNDCTL_DSP_GETISPACE fail\n");
			goto __exit;
		}

		//printf("fragments = %d\n", audio_info.fragments);
		//printf("fragstotal = %d\n", audio_info.fragstotal);
		//printf("fragsize = %d\n", audio_info.fragsize);
		//printf("bytes = %d\n", audio_info.bytes);	
		if (audio_info.bytes >= PCM_FRM_SIZE) {
			pcm_buf = Ctrl->AudEmpty[0];
			if(pcm_buf == 0) {
				ret = read(dsp_fd, (SINT8 *)pcm_temp, PCM_FRM_SIZE);
				//printf("Lost Pcm = %d\n", ret);	
				continue;
			}

			//printf("bytes = %d\n", audio_info.bytes);	
			ret = read(dsp_fd,(SINT8 *)pcm_buf, PCM_FRM_SIZE);
			if(ret < 0) {
				goto __exit;
			}

			// clear audio empty buffer
			pthread_mutex_lock(&Ctrl->aud_epy_mutex);
			for (i=0; i<(AUD_BUF_NO-1); i++) {
				Ctrl->AudEmpty[i] = Ctrl->AudEmpty[i+1];
			}
			
			Ctrl->AudEmpty[AUD_BUF_NO-1] = 0;
			pthread_mutex_unlock(&Ctrl->aud_epy_mutex);
			
			// set audio ready buffer
			pthread_mutex_lock(&Ctrl->aud_rdy_mutex);
			for (i=0; i<AUD_BUF_NO; i++) {
				if (Ctrl->AudReady[i] == 0) {
					Ctrl->AudReady[i] = pcm_buf;
					break;
				}
			}

			pthread_mutex_unlock(&Ctrl->aud_rdy_mutex);
			if (i == AUD_BUF_NO) {
				printf("Error: AudReadyBuf Full\n");
			}
		} else {
			usleep(1000);
		}
	}

__exit:
	if (dsp_fd >= 0) {
		ret = 0;
		ioctl(dsp_fd, SNDCTL_DSP_SYNC, &ret);
		close(dsp_fd);
	}

	if (mixer_fd >= 0) {
		close(mixer_fd);
	}

	if (audio_addr) {
		gpChunkMemFree((void *)audio_addr);
	}

	printf("%s exit.\n", __func__);
	pthread_exit(NULL);
	return NULL;
}

void *jpeg_encode_thread(void *param)
{
	int ret;
	ScalBuf_t *pScal;
	JpegBuf_t *pJpeg;
	BufCtrl_t *Ctrl = (BufCtrl_t *)param;
	
	printf("%s entry.\n", __func__);
	
	while(loop) 
	{
		pScal = Scale_Get_Ready_Buf();
		if (pScal == 0) {
			usleep(5000);
			continue;
		}
		
	__loop:
		pJpeg = Jpeg_Get_Empty_Buf();
		if (pJpeg == 0) {
			usleep(5000);
			goto __loop;
		}

		// jpeg encode
		ret = jpeg_encode_once((unsigned char *)pScal->output_addr, (unsigned char *)pJpeg->pOutputRemap, pJpeg->width, pJpeg->height);
		if (ret < 0) {
			printf("jpeg encode fail\n");
			Jpeg_Set_Empty_Buf(pJpeg);
			Scale_Set_Empty_Buf(pScal);
			continue;
		} else {
			pJpeg->size = ret;
			Jpeg_Set_Ready_Buf(pJpeg);
			Scale_Set_Empty_Buf(pScal);
		}				
	}

__exit:
	printf("%s exit.\n", __func__);
	pthread_exit(NULL);
	return NULL;
}

void *sensor_run_thread(void *param)
{	
	int ret, usec, FpsCnt;
	ScalBuf_t *pScal;
	JpegBuf_t *pJpeg;
	BufCtrl_t *Ctrl = (BufCtrl_t *)param;
	fd_set fds;
	struct timeval tv, TimeVal;
	struct v4l2_buffer buf;
	
	printf("%s entry.\n", __func__);
	
	FpsCnt = 0;
	gettimeofday(&TimeVal, NULL);
	usec = (TimeVal.tv_sec * 1000000) + TimeVal.tv_usec;

	/* poll & wait */	
	while(loop)
	{
		/* fds set. */
		FD_ZERO (&fds);
		FD_SET (csi_fd, &fds);
		
		/* Timeout. */
		tv.tv_sec = 30;
		tv.tv_usec = 0;
		if (select(csi_fd + 1, &fds, NULL, NULL, &tv) == 0) {
			printf ("select timeout\n");
			break;
		}
		
		/* measure sensor frame rate */
		FpsCnt++;
		gettimeofday(&TimeVal, NULL);
		ret = (TimeVal.tv_sec * 1000000) + TimeVal.tv_usec;
		if((ret - usec) >= 5000000) {
			float fps = (float)(FpsCnt*1000000)/(ret - usec);
			int fps0 = (int)fps;
			int fps1 = (float)(fps - fps0) * 100;
			printf("SensorFps:%d.%d\n", fps0, fps1);
			FpsCnt = 0;
			usec = ret; 
		}
				
		/* get ready buffer */
		memset((void *)&buf, 0, sizeof(struct v4l2_buffer));
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_USERPTR;
		if (ioctl (csi_fd, VIDIOC_DQBUF, &buf) < 0) {
			printf ("VIDIOC_DQBUF fail\n");
			break;
		}

		if (g_scale_flag) {
		#if USE_SCALER == 1
			scale_content_t sct;
		#elif USE_SCALER == 2
			gpScale2Format_t scale;
		#endif
		
			pScal = Scale_Get_Empty_Buf();
			if (pScal == 0) {
				goto __DQBuf;
			}
			
		#if USE_SCALER == 1
			memset(&sct, 0, sizeof(sct));
			sct.src_img.pData = (void *)buf.m.userptr;
			sct.src_img.width = crop_width;
			sct.src_img.height = crop_height;
			sct.src_img.bpl = crop_width*2;
			sct.src_img.type = SP_BITMAP_YUYV;
				
			sct.clip_rgn.x = 0;
			sct.clip_rgn.y = 0;
			sct.clip_rgn.width = crop_width;
			sct.clip_rgn.height = crop_height;
				
			sct.scale_rgn.x = 0;
			sct.scale_rgn.y = 0;
			sct.scale_rgn.width = pScal->width;
			sct.scale_rgn.height = pScal->height;

			sct.dst_img.pData = (void *)pScal->output_addr;
			sct.dst_img.width = pScal->width;
			sct.dst_img.height = pScal->height;
			sct.dst_img.bpl = pScal->width*2;
			sct.dst_img.type = SP_BITMAP_YUYV;
			
			ret = scale1_act(&sct);	
		#elif USE_SCALER == 2
			memset(&scale, 0, sizeof(gpScale2Format_t));
			scale.input_format = C_SCALE2_CTRL_IN_VYUY;
			scale.input_width = crop_width;
			scale.input_height = crop_height;
			scale.input_visible_width = 0;
			scale.input_visible_height = 0;
			scale.input_x_offset = 0;
			scale.input_y_offset = 0;
			scale.input_y_addr = buf.m.userptr;
			scale.input_u_addr = 0;
			scale.input_v_addr = 0;
			
			scale.output_format = C_SCALE2_CTRL_OUT_VYUY;
			scale.output_width = pScal->width;
			scale.output_height = pScal->height;
			scale.output_buf_width = pScal->width;
			scale.output_buf_height = pScal->height;
			scale.output_x_offset = 0;
			scale.output_y_addr = pScal->output_addr;
			scale.output_u_addr = 0;
			scale.output_v_addr = 0;
			
			scale.fifo_mode = C_SCALE2_CTRL_FIFO_DISABLE;
			scale.scale_mode = C_SCALE2_BY_RATIO;
			scale.digizoom_m = 10;
			scale.digizoom_n = 10;
	
			ret = scale2_act(&scale);
		#endif
			if(ret < 0) {
				printf("Scale fail\n");
				Scale_Set_Empty_Buf(pScal);
				goto __DQBuf;
			}
			
			Scale_Set_Ready_Buf(pScal);		
		} else {
			pJpeg = Jpeg_Get_Empty_Buf();
			if (pJpeg == 0) {
				goto __DQBuf;
			}
			
			ret = jpeg_encode_once((unsigned char *)buf.m.userptr, (unsigned char *)pJpeg->pOutputRemap, pJpeg->width, pJpeg->height);
			if (ret < 0) {
				printf("Jpeg encode fail\n");
				Jpeg_Set_Empty_Buf(pJpeg);
				goto __DQBuf;
			} else {
				pJpeg->size = ret;
				Jpeg_Set_Ready_Buf(pJpeg);
			}
		}
__DQBuf:
	#if PANEL_ENABLE == 1			
		/* display */
		Panel_Display(disp_fd, buf.m.userptr, crop_width, crop_height);
	#endif	
	
		/* send empty buffer */
		if(ioctl (csi_fd, VIDIOC_QBUF, &buf) < 0) {
			printf("VIDIOC_QBUF fail\n");
			break;
		}
	}
	
__exit:
	printf("%s exit.\n", __func__);
	pthread_exit(NULL);
	return NULL;
}


static JpegBuf_t *pVIDJpeg = NULL;
int send_video(BufCtrl_t *Ctrl)
{
	JpegBuf_t *pJpeg;
	gp_uvc_parameter_t uvc_param;
	int ret, i;
	
	if(jpeg_buf_num > 1) {
		if(pVIDJpeg) {
			Jpeg_Set_Empty_Buf(pVIDJpeg);
			pVIDJpeg = 0;
		}
		
		pJpeg = Jpeg_Get_Ready_Buf();
		if (pJpeg) {
			/* send this jpeg to USB device */
			uvc_param.buf = (void*)pJpeg->pOutputRemap;
			uvc_param.len = pJpeg->size;
			ioctl(usb_fd, USBDEVFS_SET_UVC_PARAM, &uvc_param);
			//printf("Send video, cnt = %d\n", timecnt);

			pVIDJpeg = pJpeg;
			return 0;
		}
	} else if(jpeg_buf_num == 1) {
		pJpeg = Jpeg_Get_Ready_Buf();
		if (pJpeg) {
			static int nIndex = 0;
			unsigned int size = (pJpeg->size + 3) >> 2 << 2;
			unsigned int addr = g_uvc_addr + (nIndex * uvc_width * uvc_height);
			
			memcpy((void *)addr, (void*)pJpeg->output_addr, size);
			Jpeg_Set_Empty_Buf(pJpeg);
			//printf("size = %d\n", pJpeg->size);
			
			if(size > MAX_JPEG_SIZE) {
				if(Qvalue > MIN_JPEG_Q) {
					Qvalue -= 10;
				}
			} 
			
			if(size < MIN_JPEG_SIZE) {
				if(Qvalue < MAX_JPEG_Q) {
					Qvalue += 10;
				}
			}
			
			/* send this jpeg to USB device */
			uvc_param.buf = (void*)addr;
			uvc_param.len = pJpeg->size;
			ioctl(usb_fd, USBDEVFS_SET_UVC_PARAM, &uvc_param);
			//printf("Send video, cnt = %d\n", timecnt);
			nIndex++;
			if(nIndex >= 2) {
				nIndex = 0;
			}
			return 0;
		}	
	}

	return -1;	
}	

int send_audio(BufCtrl_t *Ctrl)
{
	gp_uvc_parameter_t uvc_param;
	int ret, i;
	unsigned int pcm_buf;
	
	pcm_buf = Ctrl->AudReady[0];
	if (pcm_buf) {
		/* send 2048 audio data to USB device */
		uvc_param.buf = (void*)pcm_buf;
		uvc_param.len = PCM_FRM_SIZE;
		ioctl(usb_fd, USBDEVFS_SET_UVC_AUDIO_PARAM, &uvc_param);
		//printf("Send audio, cnt = %d\n", timecnt);

		// clear aud ready buffer
		pthread_mutex_lock(&Ctrl->aud_rdy_mutex);
		for (i=0; i<(AUD_BUF_NO-1); i++) {
			Ctrl->AudReady[i] = Ctrl->AudReady[i+1];
		}

		Ctrl->AudReady[AUD_BUF_NO-1] = 0;
		pthread_mutex_unlock(&Ctrl->aud_rdy_mutex);

		// set aud empty buffer
		pthread_mutex_lock(&Ctrl->aud_epy_mutex);
		for (i=0; i<AUD_BUF_NO; i++) {
			if(Ctrl->AudEmpty[i] == 0) {
				Ctrl->AudEmpty[i] = pcm_buf;
				break;
			}
		}

		pthread_mutex_unlock(&Ctrl->aud_epy_mutex);
		if (i == AUD_BUF_NO) {
			printf("Error: AudEmptyBuf Full\n");
		}
		return 0;
	}
	else
	{
		return -1;
	}	
}

void *uvc_trans_thread(void *param)
{	
	int ret;
	int uvc_state;
	gp_uvc_parameter_t uvc_param;
	BufCtrl_t *Ctrl = (BufCtrl_t *)param;
	
	printf("%s entry.\n", __func__);
	
	/* Then  open /dev/usb_device node, if failed exit this porcess */
	usb_fd = open("/dev/usb_device", O_RDWR);
	if (usb_fd < 0) {	
		printf("Error occurred when open /dev/usb_device node, exit process\n");
		goto __exit;
	}

	while(loop) 
	{
		/* Check UVC gadget state */
		ret = ioctl(usb_fd, USBDEVFS_GET_UVC_STATE, &uvc_state);
		//printf("UVC state %s, timecnt = %d\n", uvc_state_str[uvc_state], timecnt);

#if (AUDIO_SUPPORT == 1)
		if(uvc_state == USB_UVC_SEND_A_DONE_STATE || uvc_state == USB_UVC_IDLE_STATE)
		{
			if(send_video(Ctrl))
			{
				/* can't get video buffer, sen audio */
				send_audio(Ctrl);
			}				
		}
		else if(uvc_state == USB_UVC_SEND_V_DONE_STATE)
		{	
			if(send_audio(Ctrl))
			{
				/* Can't get audio buffer, send video */
				send_video(Ctrl);
			}		
		}
#else
		if(uvc_state == USB_UVC_SEND_V_DONE_STATE || uvc_state == USB_UVC_IDLE_STATE)
		{
			/* only send video data */
			send_video(Ctrl);
		}	
#endif		
		usleep(1000);		
	}
__exit:
	if (usb_fd >= 0) {
		close(usb_fd);
	}

	printf("%s exit.\n", __func__);
	pthread_exit(NULL);
	return NULL;
}

int main(int argc, char *argv[])
{
	char uvc_command[64] = {0};
	int mipi_handle = -1;
	int mem_fd = -1;
	int disp_dev = 0;
	int i, j, ret, size;
	unsigned int scaler_addr = 0;
	unsigned int jpeg_addr = 0;

	for (i=1; i<argc; i++)
	{
		if (strcmp("-d", argv[i]) == 0) {
			i++;
			disp_dev = atoi(argv[i]);
			printf("display = %d\n", disp_dev);
		} else if (strcmp("-i", argv[i]) == 0) {
			i++;
			fmt_num = atoi(argv[i]);
			if(fmt_num >= sizeof(sensor_table)/sizeof(fmt_t)) {
				fmt_num = 0;
			}
			
			printf("sensor_index = %d, %s\n", fmt_num, sensor_table[fmt_num].sensor_name);
		} else if (strcmp("-s", argv[i]) == 0) {
			i++;
			if (strcmp("vga", argv[i]) == 0) {
				printf("uvc vga\n");
				uvc_width = ENC_WIDTH_640;
				uvc_height = ENC_HEIGHT_480;
			} else if (strcmp("xga", argv[i]) == 0) {
				printf("uvc xga\n");
				uvc_width = ENC_WIDTH_1024;
				uvc_height = ENC_HEIGHT_768;
			} else if (strcmp("720p", argv[i]) == 0) {
				printf("uvc 720p\n");
				uvc_width = ENC_WIDTH_1280;
				uvc_height = ENC_HEIGHT_720;
			} else if (strcmp("1080p", argv[i]) == 0) {
				printf("uvc 1080p\n");
				uvc_width = ENC_WIDTH_1920;
				uvc_height = ENC_HEIGHT_1080;
			}  
		} else if (strcmp("-h", argv[i]) == 0) {
			printf("-d display device\n");
			printf("-i sensor index\n");
			printf("-s uvc size, vga, xga, 720p\n");
			
			for (j=0; j<sizeof(sensor_table)/sizeof(fmt_t); j++) {
				printf("sensor_table[%d] %s, size %dx%d, fmt 0x%x\n", j, sensor_table[j].sensor_name, 
						sensor_table[j].sensor_width, sensor_table[j].sensor_height, sensor_table[j].sensor_format);
			}
			return 0;
		} 
	}

	/* Insert GP's UVC gadget driver first */
	sprintf(uvc_command, "modprobe gp329xxb_cam_gadget width=%d height=%d", uvc_width, uvc_height);
	printf("send command [%s]\n", uvc_command);
	system(uvc_command);

	signal(SIGINT, sighandler);
	loop = 1;
	memset(&buf_ctrl, 0x00, sizeof(buf_ctrl));
	
	uvc_pipe_init();
	
	/* init mutex */
	ret = pthread_mutex_init(&buf_ctrl.scal_epy_mutex, NULL);
	if (ret != 0) {
		printf("scal_epy_mutex init fail\n");
		goto __exit;
	}

	ret = pthread_mutex_init(&buf_ctrl.scal_rdy_mutex, NULL);
	if (ret != 0) {
		printf("scal_rdy_mutex init fail\n");
		goto __exit;
	}

	ret = pthread_mutex_init(&buf_ctrl.jpeg_epy_mutex, NULL);
	if (ret != 0) {
		printf("jpeg_epy_mutex init fail\n");
		goto __exit;
	}

	ret = pthread_mutex_init(&buf_ctrl.jpeg_rdy_mutex, NULL);
	if(ret != 0) {
		printf("jpeg_rdy_mutex init fail\n");
		goto __exit;
	}

	ret = pthread_mutex_init(&buf_ctrl.aud_epy_mutex, NULL);
	if (ret != 0) {
		printf("aud_epy_mutex init fail\n");
		goto __exit;
	}

	ret = pthread_mutex_init(&buf_ctrl.aud_rdy_mutex, NULL);
	if (ret != 0) {
		printf("aud_rdy_mutex init fail\n");
		goto __exit;
	}	
	
	/* init display */
#if PANEL_ENABLE == 1	
	disp_fd = Panel_Init(disp_dev);
	if (disp_fd < 0) {
		goto __exit;
	}
#endif

	/* init csi */
	if (sensor_table[fmt_num].mipi_format < 0) {
		/* csi1 */ 
		csi_fd = csi1_init(sensor_table[fmt_num].sensor_name,
								sensor_table[fmt_num].sensor_format,
								sensor_table[fmt_num].sensor_format_no);
		if (csi_fd < 0) {
			printf("csi1 init Fail\n");
			goto __exit;
		}
	} else {
		/* cdsp + mipi */ 
		csi_fd = cdsp_init(sensor_table[fmt_num].sensor_name,
								sensor_table[fmt_num].sensor_format,
								sensor_table[fmt_num].sensor_format_no);
		if (csi_fd < 0) {
			printf("cdsp init Fail\n");
			goto __exit;
		}	
		
		mipi_handle = mipi_init(sensor_table[fmt_num].sensor_name, 
								sensor_table[fmt_num].mipi_format,
								sensor_width,
								sensor_height);
		if (mipi_handle < 0) {
			printf("mipi init Fail\n");
			goto __exit;
		}
	}

	/* init jpeg */
	if (jpeg_encode_init() < 0) {
		printf("jpeg_encode_init Fail\n");
		goto __exit;
	}

	/* check scalar */
	if ((crop_width != uvc_width) || (crop_height != uvc_height)) {
		printf("scale enable\n");
		g_scale_flag = 1;
		jpeg_buf_num = JPEG_BUF_NO;

		/* scale buffer */
		scaler_addr = (unsigned int)gpChunkMemAlloc(uvc_width * uvc_height * 2 * SCAL_BUF_NO);
		if (scaler_addr == 0) {
			printf("scaler_addr alloc fail!\n");
			goto __exit;
		}

		printf("scaler_addr = 0x%x\n", scaler_addr);
		size = uvc_width * uvc_height * 2;
		for (i=0; i<SCAL_BUF_NO; i++) {
			buf_ctrl.ScalBuf[i].width = uvc_width;
			buf_ctrl.ScalBuf[i].height = uvc_height;
			buf_ctrl.ScalBuf[i].output_addr = scaler_addr + (size * i);
			buf_ctrl.ScalEmpty[i] = &buf_ctrl.ScalBuf[i];
		}
	} else {
		printf("scale disable\n");
		g_scale_flag = 0;
		jpeg_buf_num = 1;

		/* uvc buffer */
		g_uvc_addr = (unsigned int)gpChunkMemAlloc(uvc_width * uvc_height * 2);
		if(g_uvc_addr == 0) {
			printf("g_uvc_addr alloc fail!\n");
			goto __exit;
		}
		
		printf("g_uvc_addr = 0x%x\n", g_uvc_addr);
	}

	/* jpeg buffer */
	jpeg_addr = (unsigned int)gpChunkMemAlloc(uvc_width * uvc_height * jpeg_buf_num);
	if (jpeg_addr == 0) {
		printf("jpeg_addr alloc fail!\n");
		goto __exit;
	}

	printf("jpeg_addr = 0x%x\n", jpeg_addr);
	mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
	if (mem_fd < 0) {
		printf("mem_fd Fail\n");
		goto __exit;
	}

	/* remap */
	size = (uvc_width * uvc_height);
	for (i=0; i<jpeg_buf_num; i++) {
		buf_ctrl.JpegBuf[i].width = uvc_width;
		buf_ctrl.JpegBuf[i].height = uvc_height;
		buf_ctrl.JpegBuf[i].output_addr = jpeg_addr + (size * i);

		// remap output buffer to make them non-cache
		buf_ctrl.JpegBuf[i].pOutputRemap = mmap(NULL, size, (PROT_READ | PROT_WRITE), 
											MAP_SHARED, mem_fd, (UINT32)gpChunkMemVA2PA((void *)buf_ctrl.JpegBuf[i].output_addr));
		
		printf("pOutputRemap = 0x%x\n", buf_ctrl.JpegBuf[i].pOutputRemap);
		if ((int)buf_ctrl.JpegBuf[i].pOutputRemap == -1) {
			printf("remap Fail\n");
			goto __exit;
		}

		buf_ctrl.JpegEmpty[i] = &buf_ctrl.JpegBuf[i];
	}

	/* audio thread */
#if (AUDIO_SUPPORT == 1)
	ret = pthread_create(&pcm_thread, NULL, pcm_encode_thread, &buf_ctrl);
	if (ret != 0) {
		printf("pcm_encode_thread create fail\n");
		goto __exit;
	}
#endif

	/* uvc trans thread */
	ret = pthread_create(&uvc_thread, NULL, uvc_trans_thread, &buf_ctrl);
	if (ret != 0) {
		printf("uvc_trans_thread create fail\n");
		goto __exit;
	}
	
	/* sensor thread */
	ret = pthread_create(&sensor_thread, NULL, sensor_run_thread, &buf_ctrl);
	if (ret != 0) {
		printf("uvc_trans_thread create fail\n");
		goto __exit;
	}

	/* jpeg encode thread */
	if(g_scale_flag) {
		ret = pthread_create(&jpeg_thread, NULL, jpeg_encode_thread, &buf_ctrl);
		if (ret != 0) {
			printf("uvc_trans_thread create fail\n");
			goto __exit;
		}
	}
	
	/* pipe thread */
	ret = pthread_create(&pipe_thread, NULL, pipe_receive_thread, NULL);
	if( ret!=0 ) {
		printf("pipe_thread create fail\n");
		goto __exit;
	}
	// check exit io
	// loop = 0;
	
__exit:
	printf("wait thread exit\n");
#if (AUDIO_SUPPORT == 1)	
	pthread_join(pcm_thread, NULL);
#endif

	pthread_join(uvc_thread, NULL);
	pthread_join(sensor_thread, NULL);
	if(g_scale_flag) {
		pthread_join(jpeg_thread, NULL);
	}

	pthread_mutex_destroy(&buf_ctrl.scal_epy_mutex);
	pthread_mutex_destroy(&buf_ctrl.scal_rdy_mutex);
	pthread_mutex_destroy(&buf_ctrl.jpeg_epy_mutex);
	pthread_mutex_destroy(&buf_ctrl.jpeg_rdy_mutex);
	pthread_mutex_destroy(&buf_ctrl.aud_epy_mutex);
	pthread_mutex_destroy(&buf_ctrl.aud_rdy_mutex);

	// display close	
#if PANEL_ENABLE == 1		
	if(disp_fd >= 0) {
		Panel_Close(disp_fd);
	}

	if(g_disp_addr) {
		gpChunkMemFree((void *)g_disp_addr);
	}
#endif

	if(mipi_handle >= 0) {
		close(mipi_handle);
	}

	if(csi_fd >= 0) {
		if(ioctl(csi_fd, VIDIOC_STREAMOFF, NULL) < 0) {
			printf("VIDIOC_STREAMON fail!!!\n");
		}
		close(csi_fd);
	}

	if(g_sensor_addr) {
		gpChunkMemFree((void *)g_sensor_addr);
	}

	jpeg_encode_uninit();

	if (scaler_addr) {
		gpChunkMemFree((void *)scaler_addr);
	}

	if(g_uvc_addr) {
		gpChunkMemFree((void *)g_uvc_addr);
	}

	if(mem_fd >= 0) {
		for(i=0; i<jpeg_buf_num; i++) {
			munmap(buf_ctrl.JpegBuf[i].pOutputRemap, uvc_width * uvc_height);
		}
		
		close(mem_fd);
	}
	
	if(jpeg_addr) {
		gpChunkMemFree((void *)jpeg_addr);
	}
	
	system("rmmod gp329xxb_cam_gadget");
	printf("Exit uvc app...\n");
	return 0;
}


