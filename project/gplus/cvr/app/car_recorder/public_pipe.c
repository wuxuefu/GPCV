#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/stat.h>
#include <errno.h>
#include <linux/types.h>
#include <sys/time.h>
#include "public_pipe.h"
#define FIFO1 "/tmp/.public-cmdin"
#define FIFO2 "/tmp/.public-cmdout"

#define BUF_MAX_SIZE_READ 256
//#define DEBUG printf
#define DEBUG
/**************************************************************************
*    		globle	   D A T A	    			*
**************************************************************************/
static pthread_mutex_t public_mutex_pipe;
static pthread_mutex_t public_mutex_resp;
static publicResponse_t *gp_responseRoot = NULL;
static pthread_t g_TimeThread;
static pthread_t g_pipeReadThread;
static int public_CmdOut = 0,public_CmdIn = 0;
static int g_isOpen = 0;// 0: is not open, 1: is open
static int g_ideoReadThread=0;
//static struct timeval b_tv;
static unsigned long start_time = 0;

void (*parse_callback)(publicPlayerCmdPacket_t *CmdPacket,UINT8 *pData,pthread_mutex_t *mutex_resp );
/**************************************************************************
*    		INTER	   FUNCTION		*
**************************************************************************/
#define GP_EPOCH 1156000000
#define MCPLAYER_START_INTERVAL  9   /*5 unit seconds*/
static volatile unsigned long getRawTime(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return ((tv.tv_sec- GP_EPOCH)*1000+tv.tv_usec/1000)-start_time;
}

static int isTimeOut( publicResponse_t *p )
{
	pthread_mutex_lock(&public_mutex_resp);
	unsigned long now = getRawTime();
	//printf("isTimeOut: cmd: %d. p=0x%x,start_time=%d \n", p->Cmd,p,start_time);
	if ( (now - p->beginTime) > (MCPLAYER_START_INTERVAL * 1000UL)) {
		pthread_mutex_unlock(&public_mutex_resp);
		printf("timeout: cmd: %d btime: %ld etime: %ld :(%ld)\n", p->Cmd, p->beginTime, now, now-p->beginTime);
		return 1;
	}
	pthread_mutex_unlock(&public_mutex_resp);
	return 0;
}

static void responseAdd( publicResponse_t *p )
{
	
	if(!p)
		return;
	pthread_mutex_lock(&public_mutex_resp);
	p->next = NULL;
	
	if( gp_responseRoot == NULL ) {
		gp_responseRoot = p;
	}
	else {
		publicResponse_t *w;  
		for(w=gp_responseRoot; w->next!=NULL; w=w->next); //find last
		w->next = p;
	}
	pthread_mutex_unlock(&public_mutex_resp);
}

static void responseRemove( publicResponse_t *p )
{

	if(!p)
		return;
	pthread_mutex_lock(&public_mutex_resp);
	if( gp_responseRoot == p ) {
			gp_responseRoot = p->next;
	}
	else {
		publicResponse_t *w;
		for(w=gp_responseRoot; w!=NULL && w->next != p; w = w->next);
		if(w) {
			w->next = p->next;
		}
	}
	free(p);  // free resp;
	pthread_mutex_unlock(&public_mutex_resp);
}

static int responseWait( publicResponse_t *p )
{
	
	int res;
	while(1){
		pthread_mutex_lock(&public_mutex_resp);
		res = p->result;
		pthread_mutex_unlock(&public_mutex_resp);
		
		if( res != RESPONSE_WAIT) {
			break;
		}
		 usleep(20*1000);
	}
	
	return res;
}
static void *public_checkTimeSrv( void *arg )
{
	publicResponse_t *w;
	struct timeval b_tv;

	gettimeofday(&b_tv, NULL);
	start_time = ((b_tv.tv_sec- GP_EPOCH)*1000+b_tv.tv_usec/1000);

	while( g_isOpen ) // chack time out if it's not stop 
	{
		for(w=gp_responseRoot; w!=NULL; w=w->next) {
			//printf("@@@@public_checkTimeSrv@@@@[%d]\n",w->Cmd);
			if(isTimeOut(w)) {
				printf("Time out ->>> id[%d][0x%x]\n",w->Cmd, w);
				pthread_mutex_lock(&public_mutex_resp);
				w->result =RESPONSE_TIMEOUT;
				pthread_mutex_unlock(&public_mutex_resp);
			}
		}
		usleep(30*1000);
	}

	memset(&b_tv, 0, sizeof(struct timeval));
		for(w=gp_responseRoot; w!=NULL; w=w->next) {
				//if(isTimeOut(w)) {
				printf("Time out ->>> out id[%d][0x%x]\n",w->Cmd, w);
				pthread_mutex_lock(&public_mutex_resp);
				w->result =RESPONSE_TIMEOUT;
				pthread_mutex_unlock(&public_mutex_resp);
				//}
		}	
//	pthread_detach(pthread_self());
//	pthread_exit(NULL);
	return (void *)0;
}
static int SendToPipe( UINT8 *pData, int dataSize )
{
	int writeSize;
	pthread_mutex_lock(&public_mutex_pipe);
	writeSize = write(public_CmdOut, pData, dataSize);
	if (writeSize == dataSize) {
		
		pthread_mutex_unlock(&public_mutex_pipe);
		
		return 0;
	}
	else if (writeSize == -1) {
		printf("\n========== send to mcplayer writesize = -1==========\n");
		pthread_mutex_unlock(&public_mutex_pipe);
		return -1;
	}
}

/*crate pipe*/
static int pipeCreate(void)
{
	printf("pipe create begin\n");
	unlink(FIFO1);
	if ((mkfifo(FIFO1, O_CREAT | O_EXCL) < 0) && (errno != EEXIST)) {
	//if ((mkfifo(FIFO1, 0777) < 0) && (errno != EEXIST)) {
		printf("can't create fifo server\n");
		printf("error: %s!\n", strerror(errno));	//wwj add
		return -1;
	}
	unlink(FIFO2);
	if ((mkfifo(FIFO2, O_CREAT | O_EXCL) < 0) && (errno != EEXIST)) {
	//if ((mkfifo(FIFO2, 0777) < 0) && (errno != EEXIST)) {
		printf("can't create fifoserver\n");
		printf("error: %s!\n", strerror(errno));	//wwj add
		return -1;
	}
	printf("pipe create end\n");
	return 0;
}
static int
pipeRead(
	UINT32 fd,
	UINT8 *buf,
	UINT32 dataSize
)
{
	int readSize = 0;
	int temSize = 0;
	fd_set fds;
	struct timeval timeout = {0, 100*1000};// timeout 100ms
	int maxfdp;
	temSize = dataSize;
	FD_ZERO(&fds);
	FD_SET(fd, &fds);
	
	switch( select(fd+1, &fds, NULL,NULL , &timeout) ) {

	case -1:
		printf("daemon:select error!\n");
		break;
	case 0:
		//printf("daemon:selcet time out!\n"); // test
		break;
	default:
		while(1) {
			DEBUG("pipe have data \n");
			readSize= read(fd, buf, temSize);
			DEBUG("readSize %d \n",readSize);
			if (readSize == temSize) {
				break;
				//return;
			}
			else if (readSize == 0) {	/*child process is over then parent exit*/
				printf(" \n===========pipe read readsize = 0===========\n");
				//sleep(1);
				//exit(0);
				//return ;
				break;
			}
			else if (readSize == -1) {
				printf(" daemon exit1");
				printf("\n========== pipe read readsize  = -1==========\n");
				//exit(1);
				//return ;
				break;
			}
			else if (readSize != temSize) {
				temSize = temSize - readSize;
				buf += readSize;
				continue;
			}
		}
		break;
	}
	return readSize;
}

 void *
public_threadRead(void)
{
	/*child read info from pipe2 and trasfer into client*/
	UINT32 dataSize;
	UINT8 *pData = NULL;
	publicPlayerCmdPacket_t *pCmdPacket = NULL;
	static UINT32 aCmdBuf[(sizeof(publicPlayerCmdPacket_t) + BUF_MAX_SIZE_READ)/4];
	publicResponse_t *w = NULL;
	
	pCmdPacket = (publicPlayerCmdPacket_t *)aCmdBuf;
	pData = (UINT8 *)(((UINT8 *)aCmdBuf) + sizeof(publicPlayerCmdPacket_t));
	dataSize = sizeof(publicPlayerCmdPacket_t);

	if(0 == pipeRead(public_CmdIn, (UINT8 *)pCmdPacket, dataSize) ) {// 修改成都不到数据就返回
		return (void *)0;
	}
	if (pCmdPacket->dataSize > 0) {
		
		pipeRead(public_CmdIn, pData, pCmdPacket->dataSize);
	}
	if(parse_callback)
		parse_callback(pCmdPacket,pData,&public_mutex_resp );
		
	usleep(10*1000);
	return (void*)0;
}
void *ReadPipeThread( void *arg )
{
	//int i=0;
	printf("start thread\n");
	//sleep(1);

	while(g_ideoReadThread)
	{
		public_threadRead();
	}

	printf("close thread\n");
//	pthread_detach(pthread_self());
//	pthread_exit(NULL);
	return (void *)0;
}
/**************************************************************************
*    		EXTERN	   FUNCTION		*
**************************************************************************/
//init pipe
int public_pipeinit( void (*callback)(publicPlayerCmdPacket_t *CmdPacket,UINT8 *pData,pthread_mutex_t *mutex_resp ))
{

	int ret;
	if(g_isOpen == 1) {
		printf("[%s:%d]ERR: public pipe is used,I can not init the public \n",__FUNCTION__,__LINE__);
		return -1;
	}
	
	g_isOpen = 1; 
	gp_responseRoot = NULL;
	ret=pipeCreate();
	if(ret <0)
	{
		printf("[%s:%d]ERR:can not creat pipe\n",__FUNCTION__,__LINE__);
		perror("create pipe err");
		g_isOpen = 0;
		return -1;
	}
	printf("begin open\n");
	public_CmdOut = open(FIFO1, O_RDWR, 0666);
	if (public_CmdOut < 0) {
		perror(" open public_CmdOut error");
		unlink(FIFO1);
		unlink(FIFO2);
		return -1;
	}
	public_CmdIn= open(FIFO2, O_RDWR, 0666);
	if (public_CmdIn < 0) {
		perror("open public_CmdIn error");
		close(public_CmdOut);
		unlink(FIFO1);
		unlink(FIFO2);
		return -1;
	}
	printf("create pipe ok!\n");
	
	
	pthread_mutex_init(&public_mutex_pipe, NULL);
	pthread_mutex_init(&public_mutex_resp, NULL);
	printf("open pipe read thread!\n");
	g_ideoReadThread = 1;
	parse_callback =callback;
	pthread_create(&g_pipeReadThread, NULL, ReadPipeThread,NULL);
	
    pthread_create(&g_TimeThread, NULL, public_checkTimeSrv, NULL);
	
	return 0;
}

void pipeDelete( void )
{
	printf("pipeDelete entry \n");
	g_ideoReadThread = 0;
	g_isOpen = 0;

	pthread_join(g_pipeReadThread,NULL);
	pthread_join(g_TimeThread,NULL);
	pthread_mutex_destroy(&public_mutex_pipe);
	pthread_mutex_destroy(&public_mutex_resp);
	close(public_CmdIn);
	close(public_CmdOut);
	unlink(FIFO1);
	unlink(FIFO2);
	printf("pipeDelete over \n");
}


publicResponse_t *public_isInlist(UINT32 cmd)
{
	publicResponse_t *w = NULL;
	//while(w == NULL ) {
		pthread_mutex_lock(&public_mutex_resp);
		for(w=gp_responseRoot; w!=NULL; w=w->next) { //
			//printf("w->Cmd %d[0x%x]\n",w->Cmd, w);
			if(w->Cmd == cmd) {
				DEBUG("I have find the resp \n");
				break;
			}
		}
		pthread_mutex_unlock(&public_mutex_resp);
	//}
	return w;
}

//sent pipe
int sendPacket(publicPlayerCmdPacket_t *pCmdPacket,int waitresp)
{
	publicResponse_t *resp;
	UINT32 dataSize;
	if(!g_isOpen) {
		return -1;
	}
	dataSize =sizeof(publicPlayerCmdPacket_t) +pCmdPacket->dataSize;
	if(waitresp)
	{
		resp = (publicResponse_t *)malloc(sizeof(publicResponse_t));
		if(resp <= 0)
		{
			printf("sendPacket malloc fail\n");
		}
		/*else
		{
			printf("resp = 0x%x\n",resp);
		}*/
		resp->beginTime = getRawTime();
		resp->Cmd = pCmdPacket->infoID;
		//printf("resp->Cmd %d",resp->Cmd);
		resp->result = RESPONSE_WAIT;
		responseAdd(resp);
	}
	if( SendToPipe((UINT8 *)pCmdPacket, dataSize) == 0 ) {
		if(waitresp)
		{
			if( responseWait(resp) != RESPONSE_OK ) {
				responseRemove(resp);
				return -1;
			}
			responseRemove(resp);
		}
		DEBUG("exit sendPacket\n");
		return 0;
	}
	else {
			printf("send cmd error!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
			if(waitresp)
				responseRemove(resp);
			
	}
	printf("err exit sendPacket\n");
	return -1;
	
}
