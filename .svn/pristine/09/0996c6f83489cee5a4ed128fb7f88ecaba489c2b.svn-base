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
#include "dvr_pipe.h"


static int g_CmdOut;
static int g_CmdIn;
static int g_DISP_CmdOut = 0; 
static int g_DISP_CmdIn = 0; 

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

int dvr_pipe_init(void)
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

int dvr_pipe_close()
{
	pipe_close(g_CmdIn);
	pipe_close(g_CmdOut);
	
	g_CmdIn = g_CmdOut = 0;
	
	pthread_mutex_destroy(&mutex_pipewrite);
}

int disp_pipe_init(void)
{
	int err = 0;

	g_DISP_CmdIn = pipe_init(DISP_FIFO1);
	if (!g_DISP_CmdIn)
		goto init_fail;

	g_DISP_CmdOut = pipe_init(DISP_FIFO2);
	if (!g_DISP_CmdOut)
		goto init_fail;
				
	return 0;	
	
init_fail:
	pipe_close(g_DISP_CmdIn);
	pipe_close(g_DISP_CmdOut);

	g_DISP_CmdIn = g_DISP_CmdOut = 0;

	return -1;
}

int disp_pipe_close()
{
	pipe_close(g_DISP_CmdIn);
	pipe_close(g_DISP_CmdOut);
	
	g_DISP_CmdIn = g_DISP_CmdOut = 0;	
}

static void pipemsg_send(int handle, unsigned int msgId, unsigned int len, unsigned char *cmddata)
{
	unsigned char *pData = NULL;
	dvrCmdPacket_t *pCmdPacket = NULL;
	int writesize = sizeof(dvrCmdPacket_t) + len;
	int ret = 0;
	
	pthread_mutex_lock(&mutex_pipewrite);
	memset(pCmdBuf, 0, sizeof(pCmdBuf));

	if (len > (sizeof(pCmdBuf)- sizeof(dvrCmdPacket_t)))
	{
		DEBUG("datasize too large\n");
		len = (sizeof(pCmdBuf)-sizeof(dvrCmdPacket_t));
	}	

	pData = (unsigned char *)pCmdBuf;
	pCmdPacket = (dvrCmdPacket_t *)pData;
	pCmdPacket->infoID = msgId;
	pCmdPacket->dataSize = len;
	memcpy(pData + sizeof(dvrCmdPacket_t), cmddata, len);
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
	dvrCmdPacket_t *pCmdPacket = NULL;

	fd_set fds;
	struct timeval timeout = {0,100*1000}; // timeout 100ms
	int	maxfdp;

	FD_ZERO(&fds);
	FD_SET(handle, &fds);
	
	if(select(handle+1,&fds, NULL, NULL, &timeout)>0)
	{
		pCmdPacket = (dvrCmdPacket_t *)pCmdBuf;
		pData = (unsigned char *)(((unsigned char *)pCmdBuf) + sizeof(dvrCmdPacket_t));
		dataSize = sizeof(dvrCmdPacket_t);

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
void dvr_pipemsg_send(unsigned int msgId, unsigned int len, unsigned char *cmddata)
{
	return pipemsg_send(g_CmdOut, msgId, len, cmddata);
}

int dvr_pipemsg_receive(unsigned int *msgId, void **para)
{
	return pipemsg_receive(g_CmdIn, msgId, para);
}

void disp_pipemsg_send(unsigned int msgId, unsigned int len, unsigned char *cmddata)
{
	return pipemsg_send(g_DISP_CmdOut, msgId, len, cmddata);
}

int disp_pipemsg_receive(unsigned int *msgId, void **para)
{
	return pipemsg_receive(g_DISP_CmdIn, msgId, para);
}

void dvr_response_cmd(unsigned int msgId)
{
	dvr_pipemsg_send(msgId, 0, NULL);
}