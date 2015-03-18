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
#include "cvr_pipe.h"


static int g_CmdOut;
static int g_CmdIn;
static int g_DISP_CmdOut = 0; 
static int g_DISP_CmdIn = 0; 
static unsigned int pCmdBuf[BUF_MAX_SIZE_WRITE / 4];
static unsigned int pDispBuf[BUF_MAX_SIZE_WRITE / 4];


pthread_mutex_t mutex_pipewrite;
pthread_mutex_t buffer_pipe_mutex;

int cvr_pipe_init(void)
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

void cvr_pipe_uninit()
{

	if(g_CmdIn){
		close(g_CmdIn);
		g_CmdIn = 0;
	}
	if(g_CmdOut) {
		close(g_CmdOut);
		g_CmdOut = 0;
	}
	pthread_mutex_destroy(&mutex_pipewrite);
}

void cvr_pipemsg_send(unsigned int msgId, unsigned int len, unsigned char *cmddata)
{
	unsigned char *pData = NULL;
	cvrCmdPacket_t *pCmdPacket = NULL;
	int writesize = sizeof(cvrCmdPacket_t) + len;
	int ret = 0;
	
	pthread_mutex_lock(&mutex_pipewrite);
	memset(pCmdBuf, 0, sizeof(pCmdBuf));

	if (len > (sizeof(pCmdBuf)- sizeof(cvrCmdPacket_t)))
	{
		DEBUG("datasize too large\n");
		len = (sizeof(pCmdBuf)-sizeof(cvrCmdPacket_t));
	}	

	pData = (unsigned char *)pCmdBuf;
	pCmdPacket = (cvrCmdPacket_t *)pData;
	pCmdPacket->infoID = msgId;
	pCmdPacket->dataSize = len;
	memcpy(pData + sizeof(cvrCmdPacket_t), cmddata, len);
	ret = write(g_CmdOut, pData, writesize);
	pthread_mutex_unlock(&mutex_pipewrite);
}

int cvr_pipemsg_receive(unsigned int *msgId, void **para)
{
	unsigned int dataSize;
	int readSize = 0;
	unsigned char *pData = NULL;
	cvrCmdPacket_t *pCmdPacket = NULL;

	fd_set fds;
	struct timeval timeout = {0,100*1000}; // timeout 100ms

	FD_ZERO(&fds);
	FD_SET(g_CmdIn, &fds);
	
	if(select(g_CmdIn+1,&fds, NULL, NULL, &timeout)>0)
	{
		pCmdPacket = (cvrCmdPacket_t *)pCmdBuf;
		pData = (unsigned char *)(((unsigned char *)pCmdBuf) + sizeof(cvrCmdPacket_t));
		dataSize = sizeof(cvrCmdPacket_t);

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

void cvr_response_cmd(unsigned int msgId)
{
	cvr_pipemsg_send(msgId, 0, NULL);
}



int buffer_pipe_init(void)
{
	int err = 0;
	
	g_DISP_CmdIn= open(DISP_FIFO1, O_RDWR, 0666);
	if (g_DISP_CmdIn< 0) {
		DEBUG("Open FIFO[%s] Error\n", DISP_FIFO1);
		err = 1;
	}
	g_DISP_CmdOut = open(DISP_FIFO2, O_RDWR, 0666);
	if (g_DISP_CmdOut < 0) {
		DEBUG("Open FIFO[%s] Error\n", DISP_FIFO2);
		err = 1;
	}
	
	if(err){
		if(g_DISP_CmdIn){
			close(g_DISP_CmdIn);
			g_DISP_CmdIn= 0;
		}
		if(g_DISP_CmdOut) {
			close(g_DISP_CmdOut);
			g_DISP_CmdOut= 0;
		}
	}
	
	pthread_mutex_init(&buffer_pipe_mutex, NULL);
	
	return 0;	
}
void buffer_pipe_uninit()
{

	if(g_DISP_CmdIn){
		close(g_DISP_CmdIn);
		g_DISP_CmdIn= 0;
	}
	if(g_DISP_CmdOut) {
		close(g_DISP_CmdOut);
		g_DISP_CmdOut= 0;
	}
	pthread_mutex_destroy(&buffer_pipe_mutex);
}
void buffer_pipemsg_send(unsigned int msgId, unsigned int len, unsigned char *cmddata)
{
	unsigned char *pData = NULL;
	cvrCmdPacket_t *pCmdPacket = NULL;
	int writesize = sizeof(cvrCmdPacket_t) + len;
	int ret = 0;
	
	pthread_mutex_lock(&buffer_pipe_mutex);
	memset(pDispBuf, 0, sizeof(pDispBuf));

	if (len > (sizeof(pDispBuf)- sizeof(cvrCmdPacket_t)))
	{
		DEBUG("datasize too large\n");
		len = (sizeof(pDispBuf)-sizeof(cvrCmdPacket_t));
	}	

	pData = (unsigned char *)pDispBuf;
	pCmdPacket = (cvrCmdPacket_t *)pData;
	pCmdPacket->infoID = msgId;
	pCmdPacket->dataSize = len;
	memcpy(pData + sizeof(cvrCmdPacket_t), cmddata, len);
	ret = write(g_DISP_CmdOut, pData, writesize);
	pthread_mutex_unlock(&buffer_pipe_mutex);
}
int buffer_pipemsg_receive(unsigned int *msgId, void **para)
{
	unsigned int dataSize;
	int readSize = 0;
	unsigned char *pData = NULL;
	cvrCmdPacket_t *pCmdPacket = NULL;

	fd_set fds;
	struct timeval timeout = {0,100*1000}; // timeout 100ms

	FD_ZERO(&fds);
	FD_SET(g_DISP_CmdIn, &fds);
	
	if(select(g_DISP_CmdIn+1,&fds, NULL, NULL, &timeout)>0)
	{
		pCmdPacket = (cvrCmdPacket_t *)pDispBuf;
		pData = (unsigned char *)(((unsigned char *)pDispBuf) + sizeof(cvrCmdPacket_t));
		dataSize = sizeof(cvrCmdPacket_t);

		readSize = read(g_DISP_CmdIn, (unsigned char*)pCmdPacket, dataSize);		
		if (readSize <= 0)
			return readSize;	// no cmd

		if (pCmdPacket->dataSize > 0) 
		{
			readSize = read(g_DISP_CmdIn, pData, pCmdPacket->dataSize);
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

void buffer_response_cmd(unsigned int msgId)
{
	buffer_pipemsg_send(msgId, 0, NULL);
}
