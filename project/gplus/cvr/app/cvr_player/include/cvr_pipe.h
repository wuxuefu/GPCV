#ifndef _DC_PIPT_H_
#define _DC_PIPT_H_

//#define DSC_OK 0

#define FIFO1 "/tmp/.public-cmdin"
#define FIFO2 "/tmp/.public-cmdout"
#define DISP_FIFO1 "/tmp/.disp-cmdin"
#define DISP_FIFO2 "/tmp/.disp-cmdout"

#define MAX_DATASIZE 64 
#define BUF_MAX_SIZE_WRITE (sizeof(cvrCmdPacket_t) + MAX_DATASIZE)

#define DEBUG printf

enum{
        DISP_CMD_MIN=0,
        DISP_BUF_READY,
        DISP_BUF_FREE,
        DISP_CMD_MAX
};

enum responseResult{
	RESPONSE_WAIT=-1,
	RESPONSE_OK = 0,
	RESPONSE_FAIL=1,
	RESPONSE_TIMEOUT=2
};

typedef struct cvrResponse_s {
	unsigned int Cmd;					/*the cmd need response*/
	unsigned long beginTime;	/*cmd begin time*/
	volatile enum responseResult result;				/*cmd ok or fail*/
	struct cvrResponse_s *next;
} cvrResponse_t;

typedef struct cvrCmdPacket_s {
	unsigned int infoID;						/*ID of Message, Setup or Command*/
	unsigned int dataSize;					/*Size of variable lenth data*/
	unsigned char data[0];						/*Variable length data*/
} cvrCmdPacket_t;

enum{
	CVR_CMD_MIN=0,
	CMD_SET_FULLSCREEN,
	CMD_SET_THMBSCREEN,
	CMD_SET_PRE_INDEX,
	CMD_SET_NEXT_INDEX,
	CMD_SET_PRE_PAGE,
	CMD_SET_NEXT_PAGE,
	CMD_SET_SCALE_UP,
	CMD_SET_SCALE_DOWN,
	CMD_SET_PLAY,
	CMD_SET_PAUSE, //10
	CMD_SET_RESUME,
	CMD_SET_STOP,
	CMD_SET_SPEED,
	CMD_SET_VOLUME,
	CMD_GET_VOLUME,
	CMD_SET_REVERSE_ENABLE,
	CMD_SET_REVERSE_DISABLE,
	CMD_GET_PLAYINT_TIME,
	CMD_GET_JPEG_RESOURSE,
	CMD_GET_JPEG_SCLIDX, //20

	CMD_GET_FILE_NAME,
	CMD_GET_FILE_PATH,
	CMD_GET_FILE_ALL_TYPE,
	CMD_GET_CVR_INFO,
	CMD_SET_CVR_INFO,
	CMD_SET_DELETE_FILE,
	CMD_SET_DELETE_ALL_FILE,
	CMD_SET_LOCK_FILE_PB,
	CMD_SET_LOCK_ALL_FILE,
	CMD_SET_UNLOCK_FILE, //30
	CMD_SET_UNLOCK_ALL_FILE,
	CMD_GET_PAGE_LOCK, 
	CMD_SET_DISP_MODE,
	CMD_SET_PLAYBACK_EXIT,
	CMD_SET_REBUILD_FILELIST,
	CMD_SET_FULLSCREEN_REPLACE,
	CMD_GET_FRAME_RATE,
	CMD_SET_RECOVER_FILE,
CMD_GET_FILE_DUAL_IDX, //for cvr dual for check file is A or B sensor.
	CVR_CMD_MAX
};

int cvr_pipe_init(void);
int cvr_pipemsg_receive(unsigned int *msgId, void **para);
void cvr_pipemsg_send( unsigned int msgId, unsigned int len, unsigned char *cmddata);
void cvr_response_cmd(unsigned int msgId);

// for buffer
int buffer_pipe_init(void);
void buffer_pipe_uninit();
void buffer_pipemsg_send(unsigned int msgId, unsigned int len, unsigned char *cmddata);
int buffer_pipemsg_receive(unsigned int *msgId, void **para);
void buffer_response_cmd(unsigned int msgId);
#endif
