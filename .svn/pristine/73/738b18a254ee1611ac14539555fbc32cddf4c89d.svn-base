#ifndef _DVR_PIPT_H_
#define _DVR_PIPT_H_

//#define DSC_OK 0

#define FIFO1 "/tmp/.public-cmdin"
#define FIFO2 "/tmp/.public-cmdout"
#define DISP_FIFO1 "/tmp/.disp-cmdin" 
#define DISP_FIFO2 "/tmp/.disp-cmdout" 

#define MAX_DATASIZE 64 
#define BUF_MAX_SIZE_WRITE (sizeof(dvrCmdPacket_t) + MAX_DATASIZE)

#define DEBUG printf

enum responseResult{
	RESPONSE_WAIT=-1,
	RESPONSE_OK = 0,
	RESPONSE_FAIL=1,
	RESPONSE_TIMEOUT=2
};

typedef struct dvrResponse_s {
	unsigned int Cmd;					/*the cmd need response*/
	unsigned long beginTime;	/*cmd begin time*/
	volatile enum responseResult result;				/*cmd ok or fail*/
	struct dvrResponse_s *next;
} dvrResponse_t;

typedef struct dvrCmdPacket_s {
	unsigned int infoID;						/*ID of Message, Setup or Command*/
	unsigned int dataSize;					/*Size of variable lenth data*/
	unsigned char data[0];						/*Variable length data*/
} dvrCmdPacket_t;

enum{ 
        DISP_CMD_MIN=0, 
        DISP_BUF_READY, 
        DISP_BUF_FREE, 
        DISP_CMD_MAX 
}; 

enum{
	DV_CMD_MIN=0,
	CMD_START_DV,	//开始录像
	CMD_STOP_DV,	//结束录像
	CMD_DO_CAPTURE,	//拍照
	CMD_SET_DIR,	//设置路径及文件名
	CMD_SET_DV_RESOLUTION,
	CMD_SET_LOOPRECORDING,
	CMD_SET_WDR,
	CMD_SET_EXPOSURE,
	CMD_SET_MD,
	CMD_SET_RECORD_AUDIO,
	CMD_SET_DV_DATE_STAMP,
	CMD_SET_DC_RESOLUTION,
	CMD_SET_SEQUENCE,
	CMD_SET_QUALITY,
	CMD_SET_SHARPNESS,
	CMD_SET_AWB,
	CMD_SET_COLOR,
	CMD_SET_ISO,
	CMD_SET_ANTI_SHAKING,
	CMD_SET_DC_DATE_STAMP,
	CMD_SET_TVMODE,
	CMD_SET_FREQUENCY,
	CMD_SET_DISPLAY_MODE,
	CMD_SET_MD_DV_START,
	CMD_SET_MD_DV_STOP,
	CMD_SET_EXIT,
	CMD_SET_LOOPRECORDING_INT,
	CMD_SET_ZOOM,
	CMD_SET_LOCK_FILE,
	CMD_SET_SEQUENCE_INT,
	CMD_READY_KEY,
	CMD_SET_DEFAULT_SETTING,
	CMD_SET_DATE_TYPE,
	CMD_SET_SDC_ERROR,
	CMD_SET_LDW_EN,
	CMD_SET_LDW_LOW_SPEED,
	CMD_SET_LDW_HIGH_SPEED,
	CMD_SET_LDW_INT,
	CMD_SET_TIME_LAPSE,
	CMD_GET_LDW_LINE,
	CMD_GET_LDW_LINE2DISP,
	CMD_SET_SENSOR_FLIP,

	CMD_SET_LDW_FCW,
	CMD_SET_LDW_STOPALRAM,
	CMD_SET_LDW_GOALRAM,

	CMD_SET_LDW_FCW_FLAG,//ldw set fcw flag.UI disp fcw icon
	CMD_SET_LDW_STOPandGOALRAM_FLAG,////ldw set StopAndGo flag.UI disp S&G icon
	DV_CMD_MAX
};

int dvr_pipe_init(void);
int dvr_pipe_close();
int dvr_pipemsg_receive(unsigned int *msgId, void **para);
void dvr_pipemsg_send( unsigned int msgId, unsigned int len, unsigned char *cmddata);
void dvr_response_cmd(unsigned int msgId);
int disp_pipe_init(void);
int disp_pipe_close();
int disp_pipemsg_receive(unsigned int *msgId, void **para);
void disp_pipemsg_send( unsigned int msgId, unsigned int len, unsigned char *cmddata);


typedef struct ldw_items_s {
 unsigned char  LDW_Enable;
 unsigned char 	LDW_Sensitivity;
 unsigned char	LDW_Area_Choice;
 unsigned char 	LDW_Speed;
 unsigned char 	LDW_SFCW;
 unsigned char 	LDW_StopAndGo;
} LDW_ITEMS;

#endif