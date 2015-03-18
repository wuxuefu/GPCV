#ifndef _DVR_PIPT_H_
#define _DVR_PIPT_H_

//#define DSC_OK 0

#define FIFO1 "/tmp/.uvc-cmdin"
#define FIFO2 "/tmp/.uvc-cmdout"

#define MAX_DATASIZE 512 
#define BUF_MAX_SIZE_WRITE (sizeof(uvcCmdPacket_t) + MAX_DATASIZE)

#define DEBUG printf

enum responseResult{
	RESPONSE_WAIT=-1,
	RESPONSE_OK = 0,
	RESPONSE_FAIL=1,
	RESPONSE_TIMEOUT=2
};

typedef struct uvcResponse_s {
	unsigned int Cmd;					/*the cmd need response*/
	unsigned long beginTime;	/*cmd begin time*/
	volatile enum responseResult result;				/*cmd ok or fail*/
	struct uvcResponse_s *next;
} uvcResponse_t;

typedef struct uvcCmdPacket_s {
	unsigned int infoID;						/*ID of Message, Setup or Command*/
	unsigned int dataSize;					/*Size of variable lenth data*/
	unsigned char data[0];						/*Variable length data*/
} uvcCmdPacket_t;

enum{ 
        UVC_CMD_MIN=0, 
        UVC_G_PREFERENCE,
		UVC_G_PREFERENCE_READY,
        UVC_CMD_MAX 
}; 

int uvc_pipe_init(void);
int uvc_pipe_close();
int uvc_pipemsg_receive(unsigned int *msgId, void **para);
void uvc_pipemsg_send( unsigned int msgId, unsigned int len, unsigned char *cmddata);
void uvc_response_cmd(unsigned int msgId);


#endif