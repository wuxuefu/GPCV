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
static unsigned int pCmdBuf[BUF_MAX_SIZE_WRITE / 4];


pthread_mutex_t mutex_pipewrite;

int dvr_pipe_init(void)
{
	int err = 0;
	
	g_CmdIn = open(FIFO1, O_RDWR, 0666);
	if (g_CmdIn < 0) {
		DEBUG("Open FIFO[%s] Error\n", FIFO1);
		err = 1;
	}
	g_CmdOut= open(FIFO2, O_RDWR, 0666);
	if (g_CmdOut < 0) {
		DEBUG("Open FIFO[%s] Error\n", FIFO2);
		err = 1;
	}
	
	if(err){
		if(g_CmdIn){
			close(g_CmdIn);
			g_CmdIn = 0;
		}
		if(g_CmdOut) {
			close(g_CmdOut);
			g_CmdOut = 0;
		}
	}
	
	pthread_mutex_init(&mutex_pipewrite, NULL);
	
	return 0;	
}

void dvr_pipemsg_send(unsigned int msgId, unsigned int len, unsigned char *cmddata)
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
	ret = write(g_CmdOut, pData, writesize);
	pthread_mutex_unlock(&mutex_pipewrite);
}

int dvr_pipemsg_receive(unsigned int *msgId, void **para)
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
	FD_SET(g_CmdIn, &fds);
	
	if(select(g_CmdIn+1,&fds, NULL, NULL, &timeout)>0)
	{
		pCmdPacket = (dvrCmdPacket_t *)pCmdBuf;
		pData = (unsigned char *)(((unsigned char *)pCmdBuf) + sizeof(dvrCmdPacket_t));
		dataSize = sizeof(dvrCmdPacket_t);

		readSize = read(g_CmdIn, (unsigned char*)pCmdPacket, dataSize);		
		if (readSize <= 0)
			return readSize;	// no cmd

		if (pCmdPacket->dataSize > 0) 
		{
			readSize = read(g_CmdIn, pData, pCmdPacket->dataSize);
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

void dvr_response_cmd(unsigned int msgId)
{
	dvr_pipemsg_send(msgId, 0, NULL);
}