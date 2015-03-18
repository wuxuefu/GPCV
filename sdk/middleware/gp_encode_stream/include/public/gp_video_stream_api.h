#ifndef __GP_VIDEO_STREAM_API_H__
#define __GP_VIDEO_STREAM_API_H__

#include "mach/typedef.h"
/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/
#define CFG_LDW			0x1		//enable LDW
#define CFG_RAW			0x100	//no effect, debug only

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/

 //vidoe mode 
typedef enum stream_mode_E
{
	MODE_H264=0,	//video mode for h.264 stream recording
	MODE_JPG,		//video mode for JPEG image snapshot
}stream_mode_e;

//image scaler type
typedef enum gpIPC_SCALER_MODE_E{
    IPC_SCALE_NORMAL = 0,	//best image quality but slower
    IPC_SCALE_BILINEAR,		//normal image quality and fast
    IPC_SCALE_BILINEAR_2,	//good image quality but slowest
}IPC_Scale_mode_e;

//video stream initial parameters for initial sensor
typedef struct {
	int width;						//prefered video input width, set 0 for default (1920px)
	int height;						//prefered video input height, set 0 for default (1080px)
	int fps;						//prefered video input framerate, set 0 for default (30fps)
	stream_mode_e mode;				//initial video mode MODE_H264/MODE_JPG (default: MODE_H264), can be changed later
	unsigned int flag;				//function flag
	void* sensor_calibrate;			//user define sensor lense calibrate info
	void* cdsp_user_preference;		//user define sensor image quality info
} gpCVR_VStream_Cfg;

//snap shot image frame
typedef struct gpIPC_SNAP_IMAGE_S {
    unsigned char *pImage;      /*image data address*/
    unsigned int size;        /*image data size*/
	void *priv;					//reserved for internal use
}IPC_SNAP_Image_s;

//video frame type
enum vid_frame_type_e
{
	vid_FRAME_I=0,			//I-Frame
	vid_FRAME_P,			//P-Frame
	vid_FRAME_START_I,		//I-Frame contain H.264 stream Header 
};

//video stream resolutions
typedef enum gpIPC_RESOLUTION_E {
    IPC_RESOLUTION_1080P = 0,		//1920x1080
    IPC_RESOLUTION_720P,			//1280x720
    IPC_RESOLUTION_960P,			//reserved
    IPC_RESOLUTION_WVGA,			//848x480
    IPC_RESOLUTION_VGA,				//640x480
    IPC_RESOLUTION_QVGA,			//320x240
}IPC_Resolution_e;

//video stream bitrate mode
typedef enum gpIPC_BITRATE_MODE_E{
    IPC_BITRATE_CBR = 0,			//constant bit rate
    IPC_BITRATE_VBR,				//variable bit rate
}IPC_Bitrate_mode_e;

//video stream info 
typedef struct gpIPC_CHN_ATTR_S {
    SINT32 chn;             /*channel number, reserved*/
    IPC_Resolution_e rsl;     /*channel resolution*/
    SINT32 bitrate;         /*channel bit rate in kbps*/
    SINT32 framerate;       /*channel frame rate*/
    SINT32 target_fps;       /*target fps, can be changed at runtime, cannot larger than framrate*/
    IPC_Bitrate_mode_e bitrateMode;     /*channel bitrate mode: CBR or VBR*/
    SINT32 ratio;           /*channel I/P GOP Len */
	SINT32 cache_buf_num;	//cache buffer number for encoded frames. 
	SINT32 scaler_type; 	//reserved 
	SINT32 video_stb_offset; //reserved
	SINT32 enable_thumb;	//enable thumb image for video recording
	UINT32 source;			//reserved, must set 0
}IPC_Chn_Attr_s;

//output vodie frame info
typedef struct gpIPC_VIDEO_FRAME_S {
    UINT8 *pFrameAddr;             /*frame address*/
    UINT32 size;                    /*frame size*/
    UINT32 width;
    UINT32 height;
    UINT32 dur;                     /*reserved*/
    enum vid_frame_type_e frameType;/*I frame or P frame*/
    UINT64  pts;                    /*frame pts in ms*/
    UINT64  seq;                    /*frame sequence, internal use*/
    //UINT32 fromGetKeyFrame;
    UINT8 readByUser;				//internal use
	void *priv;						//internal use
	UINT8* thumb;					//thumb image data address
	UINT32 thumb_size;				//thumb image data size
}IPC_Video_Frame_s;

//display info for extern display (PPU UI), for internal use
typedef struct display_param_s
{
	UINT32 width;				//display width
	UINT32 height;				//display height
	UINT32 address;				//display frame buffer starting address in physical address
	UINT32 buffer_num;			//display buffer number
	int (*disp_in_f)(int);		//function pointer for recevie display frame
	int (*disp_out_f)(int);		//function pointer for send display frame
} display_param_t;

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

 /********************************************************************
 * description: Open/Close/Start/Stop Video Stream
 * param: cfg: video stream initial config 
 * return: NULL: return 0 when success. negative value when fail.
 * *****************************************************************/
int gp_IPC_VStream_Open(gpCVR_VStream_Cfg* cfg); //Open and allocate resource for video stream
int gp_IPC_VStream_Close(void);						//Close all resource 

int gp_IPC_VStream_Start(void);		//signal to start video stream
int gp_IPC_VStream_Stop(void);		//signal to stop video stream

/********************************************************************
 * description: enable/disable display image to LCD panel
 * param: skip [in]: n:skip 1 frame for every n frame 0:no skip display frame, set this for better performance
 * param: type [in]: set scaler type for display.
 * param: crop4x3 [in]: 0:original display size 1:display cropped left and right to 4:3 aspect ratio
 * param: param [in]: initial info for external display.
 * return: NULL: return 0 when success. negative value when fail.
 * *****************************************************************/
SINT32 gp_IPC_Disable_Display();	//disable display
SINT32 gp_IPC_Enable_Display(int skip, IPC_Scale_mode_e type, int crop4x3);	//enable display to screen
SINT32 gp_IPC_Enable_Display2(int skip, IPC_Scale_mode_e type, int crop4x3, display_param_t* param);	//enable external display

/********************************************************************
 * description: enable timeStamp on video image
 * param: enable [in]: 0: disable 1: enable
 * param: format [in]: 0: YY/MM/DD 1:MM/DD/YY 2:DD/MM/YY
 * return: NULL: return 0 when success. negative value when fail.
 * *****************************************************************/
SINT32 gp_IPC_Enable_TimeStamp(int enable, int format);

/********************************************************************
 * description: set zoom value
 * param: zoom [in]: 10~40  (1.0~4.0) or 0 to disable
 * return: NULL: return 0 when success. negative value when fail.
 * *****************************************************************/
SINT32 gp_IPC_Set_Zoom(int zoom);//EXPERIMENTAL FUNCTION, reserved

/********************************************************************
 * description: add video stream channel 
 * param: chn_attr [in]: IPC_Chn_Attr_s,
 * return: video channel handle for later use NULL: open fail
 * *****************************************************************/
void *gp_IPC_VChn_Open(IPC_Chn_Attr_s *chn_attr);

/*******************************************************************
 * description: close video channel.
 * param: CHandle [in]: channel handle.
 * return: GP_SUCCESS: sucess, GP_FAIL: fail
 ******************************************************************/
SINT32 gp_IPC_VChn_close(void *CHandle);

/******************************************************************
 * description: enable channel, the channel will start streaming
 * param: CHandle [in]: channel handle.
 * return: GP_SUCCESS: sucess,other value: error
 * ***************************************************************/
SINT32 gp_IPC_VChn_Enable(void *CHandle);

/******************************************************************
 * description: disable channel. the channel will stop streaming
 * param: CHandle [in]: channel handle.
 * return: GP_SUCCESS: sucess,other value: error
 * ***************************************************************/
SINT32 gp_IPC_VChn_Disable(void *CHandle);

/*****************************************************************
 * description: get channel frame, wait and block until channel ready or error occur.
 * param: CHandle [in]: channel handle.
 * param: frame [out]: IPC_Video_Frame_s ** output frame data
 * return: GP_SUCCESS: sucess,other value: error
 * **************************************************************/
SINT32 gp_IPC_VChn_Get_Frame(void *CHandle, IPC_Video_Frame_s **frame);

/*****************************************************************
 * description: get channel frame, wait and block until channel ready or error or timeout
 * param: CHandle [in]: channel handle.
 * param: frame [out]: IPC_Video_Frame_s ** output frame data
 * param: timeout [in]: UINT32, timeout in ms
 * return: GP_SUCCESS: sucess,other value: error
 * **************************************************************/
SINT32 gp_IPC_VChn_Get_FrameTimeOut(void *CHandle, IPC_Video_Frame_s **frame, UINT32 timeout);

/*****************************************************************
 * description: free channel frame, when done use a frame, must call this API to release it.
 * param: CHandle [in]: channel handle.
 * param: frame [in]: IPC_Video_Frame_s * frame to release
 * return: GP_SUCCESS: sucess,other value: error
 * **************************************************************/
SINT32 gp_IPC_VChn_FrameRelease(void *CHandle, IPC_Video_Frame_s *frame);

/*****************************************************************
 * description: set channel bitrate (when channel disabled), only for VBR mode,
 * param: CHandle [in]: channel handle.
 * param: bitrate [in]: bitrat in kbps
 * return: GP_SUCCESS: sucess,other value: error.
 * **************************************************************/
SINT32 gp_IPC_VChn_Set_Bitrate(void *CHandle, int bitrate);

/*****************************************************************
 * description: get current channel bitrat.
 * param: CHandle [in]: channel handle.
 * param: bitrat [out]: int *,  output bitrate
 * return: GP_SUCCESS: sucess,other value: error.
 * **************************************************************/
SINT32 gp_IPC_VChn_Get_Bitrate(void *CHandle, int *bitrate);


/*****************************************************************
 * description: set channel frame rate (when channel disabled), only for VBR mode
 * param: CHandle [in]: channel handle.
 * param: framerate [in]: int, framerate
 * return: GP_SUCCESS: sucess,other value: error.
 * **************************************************************/
SINT32 gp_IPC_VChn_Set_Framerate(void *CHandle, int framerate);

/*****************************************************************
 * description: get channel framerate.
 * param: CHandle [in]: channel handle.
 * param: framerate [out]: int *,  output framerate
 * return: GP_SUCCESS: sucess,other value: error.
 * **************************************************************/
SINT32 gp_IPC_VChn_Get_Framerate(void *CHandle, int *framerate);

/*****************************************************************
 * description: set bitrate mode, disbale channel before call this function
 * param: CHandle [in]: channel handle.
 * param: bitrateMode [in]: IPC_Bitrate_mode_e, VBR or CBR
 * return: GP_SUCCESS: sucess,other value: error.
 * **************************************************************/
SINT32 gp_IPC_VChn_Set_BitrateMode(void *CHandle, IPC_Bitrate_mode_e bitrateMode);

/*****************************************************************
 * description: get channel bitrateMode.
 * param: CHandle [in]: channel handle.
 * param: bitrateMode [out]: int *,  output bitrateMode.
 * return: GP_SUCCESS: sucess,other value: error.
 * **************************************************************/
SINT32 gp_IPC_VChn_Get_BitrateMode(void *CHandle, IPC_Bitrate_mode_e *bitrateMode);

/*****************************************************************
 * description: set channel frame GOP Length (disable channel first)
 * param: CHandle [in]: channel handle.
 * param: ratio[in]: int , GOP Length, recommand 1~100.
 * return: GP_SUCCESS: sucess,other value: error.
 * **************************************************************/
SINT32 gp_IPC_VChn_Set_FrameRatio(void *CHandle, int ratio);

/*****************************************************************
 * description: get channel frame GOP Length
 * param: CHandle [in]: channel handle.
 * param: ratio [out]: int *,  output GOP Length
 * return: GP_SUCCESS: sucess,other value: error.
 * **************************************************************/
SINT32 gp_IPC_VChn_Get_FrameRatio(void *CHandle, int *ratio);


/*****************************************************************
 * description: skip to channel next key frame. wait and block until key frame ready or error
 * param: CHandle [in]: channel handle.
 * param: keyframe [out]: IPC_Video_Frame_s ** output keyframe
 * return: GP_SUCCESS: sucess,other value: error.
 * **************************************************************/
SINT32 gp_IPC_VChn_Get_KeyFrame(void *CHandle, IPC_Video_Frame_s **keyframe);


/*************************************
release keyframe, identical to gp_IPC_VChn_FrameRelease right now
***************************************/
int gp_IPC_VChn_free_KeyFrame(void *CHandle, IPC_Video_Frame_s *keyframe);

/*****************************************************************
 * description: request I-Frame with h.264 stream header info
 * then repeatedly call gp_IPC_VChn_Get_Frame until get the frame with type vid_FRAME_START_I
 * param: CHandle [in]: channel handle.
 * return: GP_SUCCESS: sucess,other value: error.
 * **************************************************************/
SINT32 gp_IPC_VChn_Request_Header(void *CHandle);

/*****************************************************************
 * description: change video source resolution, it will affect all video channel
 * param: res [in]: request resolution
 * return: GP_SUCCESS: sucess,other value: error.
 * **************************************************************/
int gp_IPC_VStream_Set_Resolution(IPC_Resolution_e res);

/*****************************************************************
 * description: change video stream mode, call this when all channel closed.
 * param: mode [in]: H264/JPEG
 * return: GP_SUCCESS: sucess,other value: error.
 * **************************************************************/
int gp_IPC_Set_Mode(stream_mode_e mode);

/*****************************************************************
 * description: query video sensor status 
 * return: 0: sensor stop 1: sensor running -1: sensor error
 * **************************************************************/
int gp_IPC_VStream_Query_Status(void);


/*****************************************************************
 * description: function for DC mode
 * gp_capture_stream_create return handle for later use.
 * then call Enable->capture_picture->picture_Free->disable->close
 * param: width/height: picture dimension
 * param: image: returned image
 * param: QType: jpeg quality (1-10)
 * **************************************************************/
void * gp_capture_stream_create(int width,int height);
void gp_capture_stream_close(void *handle);
int gp_capture_stream_Enable(void *handle);
int gp_capture_stream_disable(void * handle);
int gp_capture_picture(void *handle, IPC_SNAP_Image_s *image,int QType);
int  gp_capture_picture_Free(void *handle, IPC_SNAP_Image_s *image);

#endif