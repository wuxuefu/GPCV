#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include "uvc_pipe.h"


static int g_CmdOut;
static int g_CmdIn;

static unsigned int pCmdBuf[BUF_MAX_SIZE_WRITE / 4];

pthread_mutex_t mutex_pipewrite;

static int pipe_init(char* name)
{
	int handle;
	
	handle = open(name, O_RDWR, 0666);
	if (handle < 0)
	{
		handle = 0;
		DEBUG("Open FIFO[%s] Error\n", name);
	}
		
	return handle;
}

static void pipe_close(int handle)
{
	if (handle)
		close(handle);
}

int uvc_pipe_init(void)
{
	int err = 0;

	g_CmdIn = pipe_init(FIFO1);
	if (!g_CmdIn)
		goto init_fail;

	g_CmdOut = pipe_init(FIFO2);
	if (!g_CmdOut)
		goto init_fail;
			
	pthread_mutex_init(&mutex_pipewrite, NULL);
	
	return 0;	
	
init_fail:
	pipe_close(g_CmdIn);
	pipe_close(g_CmdOut);
	
	return -1;
}

int uvc_pipe_close()
{
	pipe_close(g_CmdIn);
	pipe_close(g_CmdOut);
	
	g_CmdIn = g_CmdOut = 0;
	
	pthread_mutex_destroy(&mutex_pipewrite);
}

static void pipemsg_send(int handle, unsigned int msgId, unsigned int len, unsigned char *cmddata)
{
	unsigned char *pData = NULL;
	uvcCmdPacket_t *pCmdPacket = NULL;
	int writesize = sizeof(uvcCmdPacket_t) + len;
	int ret = 0;
	
	pthread_mutex_lock(&mutex_pipewrite);
	memset(pCmdBuf, 0, sizeof(pCmdBuf));

	if (len > (sizeof(pCmdBuf)- sizeof(uvcCmdPacket_t)))
	{
		DEBUG("datasize too large\n");
		len = (sizeof(pCmdBuf)-sizeof(uvcCmdPacket_t));
	}	

	pData = (unsigned char *)pCmdBuf;
	pCmdPacket = (uvcCmdPacket_t *)pData;
	pCmdPacket->infoID = msgId;
	pCmdPacket->dataSize = len;
	memcpy(pData + sizeof(uvcCmdPacket_t), cmddata, len);
	ret = write(handle, pData, writesize);
	pthread_mutex_unlock(&mutex_pipewrite);
}

static int pipemsg_receive(int handle, unsigned int *msgId, void **para)
{
	unsigned int dataSize;
	int readSize = 0;
	int writeSize = 0;
	unsigned char *pData = NULL;
	unsigned int cmddata = 0;
	uvcCmdPacket_t *pCmdPacket = NULL;

	fd_set fds;
	struct timeval timeout = {0,100*1000}; // timeout 100ms
	int	maxfdp;

	FD_ZERO(&fds);
	FD_SET(handle, &fds);
	
	if(select(handle+1,&fds, NULL, NULL, &timeout)>0)
	{
		pCmdPacket = (uvcCmdPacket_t *)pCmdBuf;
		pData = (unsigned char *)(((unsigned char *)pCmdBuf) + sizeof(uvcCmdPacket_t));
		dataSize = sizeof(uvcCmdPacket_t);

		readSize = read(handle, (unsigned char*)pCmdPacket, dataSize);		
		if (readSize <= 0)
			return readSize;	// no cmd

		if (pCmdPacket->dataSize > 0) 
		{
			readSize = read(handle, pData, pCmdPacket->dataSize);
			if (readSize <= 0)
				DEBUG("cmd %d error, no data\n", pCmdPacket->infoID);
		}
		else
			pData = NULL;
	
		*msgId = pCmdPacket->infoID;

		if(pData != NULL)
		{
			*para = (void *)pData;
		}
	}
	return readSize;
}
void uvc_pipemsg_send(unsigned int msgId, unsigned int len, unsigned char *cmddata)
{
	return pipemsg_send(g_CmdOut, msgId, len, cmddata);
}

int uvc_pipemsg_receive(unsigned int *msgId, void **para)
{
	return pipemsg_receive(g_CmdIn, msgId, para);
}

void uvc_response_cmd(unsigned int msgId)
{
	uvc_pipemsg_send(msgId, 0, NULL);
}