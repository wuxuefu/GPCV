#ifndef _DC_PIPT_H_
#define _DC_PIPT_H_

//#define DSC_OK 0

#define FIFO1 "/tmp/.public-cmdin"
#define FIFO2 "/tmp/.public-cmdout"

#define MAX_DATASIZE 64 
#define BUF_MAX_SIZE_WRITE (sizeof(dcCmdPacket_t) + MAX_DATASIZE)

#define DEBUG printf

enum responseResult{
	RESPONSE_WAIT=-1,
	RESPONSE_OK = 0,
	RESPONSE_FAIL=1,
	RESPONSE_TIMEOUT=2
};

typedef struct dcResponse_s {
	unsigned int Cmd;					/*the cmd need response*/
	unsigned long beginTime;	/*cmd begin time*/
	volatile enum responseResult result;				/*cmd ok or fail*/
	struct dcResponse_s *next;
} dcResponse_t;

typedef struct dcCmdPacket_s {
	unsigned int infoID;						/*ID of Message, Setup or Command*/
	unsigned int dataSize;					/*Size of variable lenth data*/
	unsigned char data[0];						/*Variable length data*/
} dcCmdPacket_t;

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
	DV_CMD_MAX
};

int dc_pipemsg_receive(unsigned int *msgId, void **para);
void dc_pipemsg_send( unsigned int msgId, unsigned int len, unsigned char *cmddata);

#endif