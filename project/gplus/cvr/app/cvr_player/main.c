#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mach/typedef.h"
#include "cvr_pipe.h"
#include "photo_manager.h"
#include "playback_demo.h"
#include "gp_VideoEngineApi.h"
#include <mach/gp_display.h>
#include "typedef.h"
#include "gp_mux.h"
#include "libMcpThread/mcp_thread.h"
#include "libMcpThread/mcp_queue.h"

#define PLAYER_FILE_TYPE_VIDEO 0
#define PLAYER_FILE_TYPE_JPEG 1

int g_bQuit=0;	   
int g_DispMode = 0;

display_buffer_t DispBufManage;
static pthread_t BufferThread = 0;

extern int g_VideoEnd;
extern float play_speed[4];
extern char fileMode;

void buffer_Thread(void *arg)
{

	printf("%s:%d ======= start============\n", __FUNCTION__, __LINE__);
	unsigned char err;
	int idlBuf = -1;
	unsigned int msgId;
	void *msgPara = NULL;
	int idx = 0;
	while (!g_bQuit) {
		if(!buffer_pipemsg_receive(&msgId, &msgPara)) {
			usleep(10*1000);
			//printf("%s:%d get\n", __FUNCTION__, __LINE__);
			continue;	
		}
		//printf("%s:%d get %d\n", __FUNCTION__, __LINE__, msgId);
		if(msgId == DISP_BUF_READY) {
			idx = (int)*(unsigned int *)msgPara;
			//printf("%s:%d get DISP_BUF_READY %d\n", __FUNCTION__, __LINE__, idx);
			switch(idx) {
				case 0:
					idlBuf = DispBufManage.DispBuffer[0];
					break;
				case 1:
					idlBuf = DispBufManage.DispBuffer[1];
					break;
				case 2:
					idlBuf = DispBufManage.DispBuffer[2];
					break;
				default:
					idlBuf = -1;
					break;
			}

			if(idlBuf != -1) {
				err = (unsigned int)OSQPost(&DispBufManage.BufferMBox, (void *)idlBuf);
				if(err != OS_NO_ERR) {
					printf("buffer mbox post error!!\n");
				}
			}
		}
		usleep(30*1000);
	}
	return ;
}

int get_idle_buffer(int arg)
{
	//printf("%s:%d\n", __FUNCTION__, __LINE__);
	unsigned char err;
	int idlBuf = -1;
	unsigned int msgId;
	void *msgPara = NULL;
	int idx = 0;

	idlBuf = (int)OSQPend(&DispBufManage.BufferMBox, 300, &err);
	return idlBuf;
}

/*
 * send ready buffer idx to pipe host. 
 * param arg: buffer address.
 */
int send_ready_buffer(int arg)
{
	//printf("%s:%d 0x%x\n", __FUNCTION__, __LINE__, arg);
	unsigned int msgId;
	void *magPara = NULL;
	int idx = 0;

	if(arg == DispBufManage.DispBuffer[0]) {
		idx = 0;
	}
	else if(arg == DispBufManage.DispBuffer[1]) {
		idx = 1;
	}
	else if(arg == DispBufManage.DispBuffer[2]) {
		idx = 2;
	}
	else {
		idx = -1;
	}

	msgId = DISP_BUF_READY;
	buffer_pipemsg_send(msgId, 4, (unsigned char *)&idx);

	//printf("%s:%d send ok\n", __FUNCTION__, __LINE__);
	return 0;
}

/*
 * buf:clean buffer  address
 * type: 0: rgb565, 1, yuyv
 */
int clean_buffer(unsigned char *buf, int type)
{
	unsigned short *psBuf = (unsigned short*)buf;

	int i = 0;

	if(type) {
		for(i=0; i< DispBufManage.Width*DispBufManage.Height; i++) {
			*psBuf++ = 0x8000;
		}
	}
	else {
		memset((char *)buf, 0, DispBufManage.Width * DispBufManage.Height * 2);
	}
	

	return 0;
}

int buffer_init(display_buffer_t *pBuf)
{
	int i;
	unsigned char err;
	unsigned int DispBuf;

	printf("%s:%d\n", __FUNCTION__, __LINE__);
	buffer_pipe_init();

	for(i=0; i< pBuf->DispNum; i++) {
		pBuf->DispBuffer[i] = pBuf->address + pBuf->Width*pBuf->Height*2*i;
		printf("%s:%d buffer: 0x%x\n", __FUNCTION__, __LINE__, pBuf->DispBuffer[i]);

	}

	if(mcp_mbox_init(&pBuf->BufferMBox) == 0) {
		printf("buffer mbox init error!!\n");
		return -1;
	}
	for(i=0; i<pBuf->DispNum; i++) {
		printf("buffer mbox pend %d!!\n", i);
		DispBuf = (unsigned int)OSQPend(&pBuf->BufferMBox, 1, &err);
		if(DispBuf == 0) {
			printf("buffer mbox pend error!!\n");
			break;
		}
	}
	for(i=0; i<pBuf->DispNum; i++) {
		printf("buffer mbox post %d!!\n", i);
		err = (unsigned int)OSQPost(&pBuf->BufferMBox, (void *)pBuf->DispBuffer[i]);
		if(err != OS_NO_ERR) {
			printf("buffer mbox post error!!\n");
			break;
		}
	}
	if(BufferThread == 0)
		pthread_create(&BufferThread, NULL, buffer_Thread, 0);
	return 0;
}

int buffer_uninit(display_buffer_t *pBuf)
{
	printf("%s:%d\n", __FUNCTION__, __LINE__);
	buffer_pipe_uninit();
	if(BufferThread) {		
		pthread_join(BufferThread, NULL);		
		BufferThread = 0;	
	}	
	mcp_mbox_free(&pBuf->BufferMBox);
}

 void sigint( int signo )
{
	g_bQuit = 1;
}

int RegisterSigint()
{
	struct sigaction act;
	memset(&act, 0, sizeof(act));
	act.sa_handler = sigint;
	act.sa_flags   = SA_RESTART;
	if(sigaction(SIGINT, &act, NULL) != 0)
	{
		return -1;
	}

	return 1;
}

int command_help( void )
{
	printf("./cvrplayer [-p -d -d2]\n");
	printf("	-p:	cvrplayer search file path\n");
	printf("	-d:	cvrplayer display 0 device, 0or other:LCD, 4:TV, 6:HDMI 7:Buffer\n");
	printf("	-d2:	cvrplayer display output to buffer: -d2 PAddress width height buffer_number(3)\n");
	//printf("	-fifo: using pipe\n");
	return 0;
}

static cdvr_thmb_t *pdvr = NULL;

int main(int argc, char *argv[])
{
	unsigned int msgId;
	void* msgPara = NULL;
	unsigned int param = 0;
	unsigned char *pRespData = NULL;
	int iRespLen = 0;
	char sendBuf[512];
	gp_size_t size;
	cdvr_info_t cvrInfo;
	cdvr_info_t *pInfo;
	int i = 0;
	int ret = 0;
	UINT32 *p = NULL;

	printf("cvrplayer start!!!!!\n");
	memset(&DispBufManage, 0, sizeof(display_buffer_t));
	fileMode = 0;

	for(i=1; i<argc; i++) {
		printf("[CVR]%s:%d %d %s\n", __FUNCTION__, __LINE__, i, argv[i]);
		if(strcmp(argv[i], "-p") == 0) {
			FilelistSetFilePath(argv[++i]);
		}
		else if(strcmp(argv[i], "-d") == 0) {
			printf("%s\n", argv[i]);
			param = atoi(argv[++i]);
			DispBufManage.DispDev = param;
			printf("%s\n", argv[i]);
		}
		else if(strcmp(argv[i], "-d2") == 0) {
			param = DispBufManage.DispDev = 7;
			printf("%s\n", argv[i]);
			sscanf(argv[++i],"0x%x", &DispBufManage.address);
			printf("%s\n", argv[i]);
			DispBufManage.Width = atoi(argv[++i]);
			printf("%s\n", argv[i]);
			DispBufManage.Height = atoi(argv[++i]);
			printf("%s\n", argv[i]);
			DispBufManage.DispNum = atoi(argv[++i]);
			printf("%s\n", argv[i]);
			DispBufManage.DispNum = DISP_BUFFRER_NUM;
			
		}
		else if(strcmp(argv[i], "-m") == 0) {
			fileMode = atoi(argv[++i]);
		}
	}
	printf("[CVR]%s:%d\n", __FUNCTION__, __LINE__);

	if(param == 4) {
		g_DispMode = SP_DISP_OUTPUT_TV;
	}
	else if(param == 6) {
		g_DispMode = SP_DISP_OUTPUT_HDMI;
	}
	else if(param == 7) {
		g_DispMode = SP_DISP_OUTPUT_LCM;
	}
	else {
		g_DispMode = SP_DISP_OUTPUT_LCD;
		param = 0;
	}
		
	printf("[CVR]%s:%d\n", __FUNCTION__, __LINE__);
	RegisterSigint();
	cvr_pipe_init();
	printf("[CVR]%s:%d\n", __FUNCTION__, __LINE__);
	if(param == 7) {
		DispBufManage.address = (int)gpChunkMemShare(DispBufManage.address, NULL);
		buffer_init(&DispBufManage);
		printf("[CVR]%s:%d\n", __FUNCTION__, __LINE__);
	}
	pdvr = playback_init(param);
	printf("[CVR]%s:%d\n", __FUNCTION__, __LINE__);
	if(DispBufManage.DispDev == C_DISP_BUFFER && pdvr->file_number == 0) {
		printf("[CVR]%s:%d enter no file mode\n", __FUNCTION__, __LINE__);
		pdvr->upFrame =  (char *)get_idle_buffer(0);
		clean_buffer(pdvr->upFrame, 0);
		dvr_thmb_dispFlip(pdvr);
	}
	while(1) {

		pRespData = NULL;
		iRespLen = 0;
		msgPara = NULL;
		
		if (g_bQuit) {

			printf("get Quit!\n");
			break;
		}
		
		if((ret = cvr_pipemsg_receive(&msgId, &msgPara)) > 0)
		{
			//printf("playback msgId=[%d] size: %d\n", msgId, ret);
			if (msgPara)
			{
				//printf("%s:%d\n", __FUNCTION__, __LINE__);
				param = *(unsigned int*)msgPara;
				//printf("param = %d\n", param);
			}
			switch(msgId)
			{
				case CMD_SET_FULLSCREEN:
					pRespData = (char *)&ret;
					iRespLen = sizeof(int);
					ret = dvr_thmb_dispFullScreen(pdvr, pdvr->upFrame);
				break;
				case CMD_SET_FULLSCREEN_REPLACE:
					dispReUpdatePrimary(pdvr->hDisp);
				break;
				case CMD_SET_THMBSCREEN:
					if(DispBufManage.DispDev == C_DISP_BUFFER) {
						pdvr->upFrame =  (char *)get_idle_buffer(0);
						clean_buffer(pdvr->upFrame, 0);
					}
					else {
						pdvr->upFrame = (char *)dispGetFramebuffer(pdvr->hDisp);
						dispCleanFramebuffer(pdvr->hDisp);
					}

					dvr_thmb_dispPage(pdvr, pdvr->cur_page, pdvr->upFrame);
					pdvr->pUrl = FilelistGetFilePath(pdvr->pFile, dvr_thmb_getCurIdxOfFile(pdvr));
					dvr_thmb_dispFlip(pdvr);
				break;
				case CMD_SET_PRE_INDEX:
					dvr_thmb_preIdx(pdvr);
				break;
				case CMD_SET_NEXT_INDEX:
					dvr_thmb_nextIdx(pdvr);
				break;
				case CMD_SET_PRE_PAGE:
					dvr_thmb_prePage(pdvr);
				break;
				case CMD_SET_NEXT_PAGE:
					dvr_thmb_nextPage(pdvr);
				break;
				case CMD_SET_SCALE_UP:
					if(DispBufManage.DispDev == C_DISP_BUFFER) {
						pdvr->upFrame =  (char *)get_idle_buffer(0);
						clean_buffer(pdvr->upFrame, 0);
					}
					else {
						pdvr->upFrame = (char *)dispGetFramebuffer(pdvr->hDisp);
					}

					pdvr->pPhotoInfo->bitmap->pData = (unsigned char*)pdvr->upFrame;
					ret = image_scale_up(pdvr->pPhotoInfo);
					if(ret == 0) {
						dvr_thmb_dispFlip(pdvr);
					}
				break;
				case CMD_SET_SCALE_DOWN:
					if(DispBufManage.DispDev == C_DISP_BUFFER) {
						pdvr->upFrame =  (char *)get_idle_buffer(0);
						clean_buffer(pdvr->upFrame, 0);
					}
					else {
						pdvr->upFrame = (char *)dispGetFramebuffer(pdvr->hDisp);
					}
					pdvr->pPhotoInfo->bitmap->pData = (unsigned char*)pdvr->upFrame;
					ret = image_scale_down(pdvr->pPhotoInfo);
					if(ret == 0) {
						dvr_thmb_dispFlip(pdvr);
					}
				break;
				case CMD_SET_PLAY:
					g_VideoEnd = 0;
					pRespData = (char *)&ret;
					iRespLen = sizeof(int);
					ret = ExtGpVideoEngineSetUrl(pdvr->pUrl);
					/*if(ret < 0) { // parse start error.
						break;
					}*/
					if(ret >= 0 && (ExtGpVideoEngineGetTotalTime() != 0 || 
							ExtGpVideoEngineGetTotalSample() > 2)) {
						ret = ExtGpVideoEnginePlay(0);
						pdvr->pVideoInfo->total_time = ExtGpVideoEngineGetTotalTime();
					}
					else {
						ret = -1;
						ExtGpVideoEngineStop();
						//pdvr->upFrame = (char *)dispGetFramebuffer(pdvr->hDisp);
						//dispCleanFramebuffer(pdvr->hDisp);
						if(DispBufManage.DispDev == C_DISP_BUFFER) {
							pdvr->upFrame =  (char *)get_idle_buffer(0);
							clean_buffer(pdvr->upFrame, 0);
						}
						else {
							pdvr->upFrame = (char *)dispGetFramebuffer(pdvr->hDisp);
							dispCleanFramebuffer(pdvr->hDisp);
						}
						dvr_thmb_dispFlip(pdvr);
					}
				break;
				case CMD_SET_PAUSE:
					ret = -1;
					pRespData = (char *)&ret;
					iRespLen = sizeof(int);
					ExtGpVideoEnginePause();
					ret = 0;
				break;
				case CMD_SET_RESUME:
					ret = 0;
					pRespData = (char *)&ret;
					iRespLen = sizeof(int);
					ExtGpVideoEngineResume();
				break;
				case CMD_SET_STOP:
					ExtGpVideoEngineStop();
					g_VideoEnd = 0;
					pdvr->pVideoInfo->total_time = 0;

				break;
				case CMD_SET_SPEED:
					pRespData = (char *)&ret;
					iRespLen = sizeof(int);
					pdvr->pVideoInfo->speed = param;
					ret = dvr_thmb_getPlayingTime(pdvr);
					if(pdvr->pVideoInfo->total_time < 3 || (pdvr->pVideoInfo->play_time > pdvr->pVideoInfo->total_time-1)
							|| (pdvr->pVideoInfo->play_time <= 1)) {
						ret = -2;
						printf("%s:%d play time: %d can not chang speed!\n", __FUNCTION__, __LINE__, pdvr->pVideoInfo->play_time);
					}
					else if(ExtGpVideoEngineGetPlayingStatus() == 0 ) {//video is playing 
						ret = ExtGpVideoEngineSetSpeed(play_speed[pdvr->pVideoInfo->speed]);
					}
					else {
						printf("%s:%d play thread is exit g_VideoEnd: %d !\n", __FUNCTION__, __LINE__, g_VideoEnd);
						if(!g_VideoEnd) {
							g_VideoEnd = 1;
						}
						ret = -1;
					}
				break;
				case CMD_SET_VOLUME:
					pdvr->pVideoInfo->volume = param;
					ExtGpVideoEngineSetVolume(pdvr->pVideoInfo->volume);
				break;
				case CMD_GET_VOLUME:
					iRespLen = sizeof(pdvr->pVideoInfo->volume);
					pRespData = (unsigned char *)&pdvr->pVideoInfo->volume;
				break;
				case CMD_SET_REVERSE_ENABLE:
					pRespData = (char *)&ret;
					iRespLen = sizeof(int);
					ret = dvr_thmb_getPlayingTime(pdvr);
					if(pdvr->pVideoInfo->total_time < 3 || (pdvr->pVideoInfo->play_time > pdvr->pVideoInfo->total_time-1)
							|| (pdvr->pVideoInfo->play_time <= 1)) {
						ret = -2;
						printf("%s:%d play time: %d can not chang to reversed mode!\n", __FUNCTION__, __LINE__, pdvr->pVideoInfo->play_time);
					}
					else if(ExtGpVideoEngineGetPlayingStatus() == 0 ) {//video is playing 
						ret = ExtGpVideoEngineReversePlay(1); //enable reversed play
						ret = 0;
					}
					else {
						printf("%s:%d play thread is exit g_VideoEnd: %d !\n", __FUNCTION__, __LINE__, g_VideoEnd);
						if(!g_VideoEnd) {
							g_VideoEnd = 1;
						}
						ret = -1;
					}
				break;
				case CMD_SET_REVERSE_DISABLE:
					pRespData = (char *)&ret;
					iRespLen = sizeof(int);
					ret = dvr_thmb_getPlayingTime(pdvr);
					if(pdvr->pVideoInfo->total_time < 3 || (pdvr->pVideoInfo->play_time >= pdvr->pVideoInfo->total_time-1)
							|| (pdvr->pVideoInfo->play_time <= 1)) {
						ret = -2;
						printf("%s:%d play time: %d can not chang to reversed mode!\n", __FUNCTION__, __LINE__, pdvr->pVideoInfo->play_time);
					}
					else if(ExtGpVideoEngineGetPlayingStatus() == 0 ) {//video is playing 
						ret = ExtGpVideoEngineReversePlay(0); //disable reversed play
						ret = 0;
					}
					else {
						printf("%s:%d play thread is exit g_VideoEnd: %d !\n", __FUNCTION__, __LINE__, g_VideoEnd);
						if(!g_VideoEnd) {
							g_VideoEnd = 1;
						}
						ret = -1;
					}
				break;
				case CMD_GET_PLAYINT_TIME:
					dvr_thmb_getPlayingTime(pdvr);
					p = (UINT32 *)sendBuf;
					*(UINT32 *)p = g_VideoEnd;
					p++;
					*(SINT32*)p = pdvr->pVideoInfo->play_time;
					iRespLen = sizeof(pdvr->pVideoInfo->play_time) + sizeof(UINT32);
					pRespData = sendBuf;
				break;
				case CMD_GET_JPEG_RESOURSE:
					dvr_thmb_getFileResourse(pdvr, &size);
					printf("jpeg resourse: %dx%d\n", size.width, size.height);
					iRespLen = sizeof(gp_size_t);
					pRespData = (unsigned char *)&size;
				break;
				case CMD_GET_JPEG_SCLIDX:
					iRespLen = sizeof(int);
					if(pdvr->pPhotoInfo) {
						param = pdvr->pPhotoInfo->sclIdx;
					}
					else {
						param = 0;
					}
					pRespData = (unsigned char *)&param;
				break;
				case CMD_GET_FILE_NAME: // file name 
					pRespData = (unsigned char *)dvr_thmb_getFileName(pdvr, pdvr->pFile);
					iRespLen = strlen((char *)pRespData);
				break;
				case CMD_GET_FILE_PATH: // path and name
					pRespData = getPath(pdvr->pFile, dvr_thmb_getCurIdxOfFile(pdvr));
					iRespLen = strlen((char *)pRespData);
				break;
				case CMD_GET_FILE_ALL_TYPE: // path and name
					param = 0;
					for(i=0; i<pdvr->cur_pageNum; i++) {
						param |= (pdvr->disp_thmb[i].type&0x01);
						param = (param<<1);
					}
					//printf("%s:%d get all file type: 0x%x\n", __FUNCTION__, __LINE__, param);
					pRespData = (char *)&param;
					iRespLen = sizeof(int);
				break;
				case CMD_GET_CVR_INFO:
					dvr_thmb_getCvrInfo(pdvr, &cvrInfo);
					dvr_sdvr_printfInfo(&cvrInfo);
					pRespData = (unsigned char *)&cvrInfo;
					iRespLen = sizeof(cvrInfo);
				break;
				case CMD_SET_CVR_INFO:
					pInfo = (cdvr_info_t *)msgPara;
					pdvr->disp_number = pInfo->disp_number;
					pdvr->disp_page = pInfo->disp_page;
					pdvr->file_number = pInfo->file_number;
					pdvr->cur_idx = pInfo->cur_idx;
					pdvr->cur_page = pInfo->cur_page;
					pdvr->cur_pageNum = pInfo->cur_pageNum;
					pdvr->dispMode = pInfo->dispMode;
					pdvr->fileType = pInfo->fileType;
				break;
				case CMD_SET_DELETE_FILE:
					dvr_thmb_deleteFile(pdvr, &pdvr->pFile);
				break;
				case CMD_SET_DELETE_ALL_FILE:
					dvr_thmb_deleteAllFile(pdvr, &pdvr->pFile);
				break;
				case CMD_SET_LOCK_FILE_PB:
					FilelistSetLock(pdvr->pFile, dvr_thmb_getCurIdxOfFile(pdvr));
				break;
				case CMD_SET_LOCK_ALL_FILE:
					FilelistSetLockAll(pdvr->pFile);
				break;
				case CMD_SET_UNLOCK_FILE:
					FilelistSetUnLock(pdvr->pFile, dvr_thmb_getCurIdxOfFile(pdvr));
				break;
				case CMD_SET_UNLOCK_ALL_FILE:
					FilelistSetUnLockAll(pdvr->pFile);
				break;
				case CMD_SET_REBUILD_FILELIST:
					pdvr->pFile = FilelistGetFileNum(pdvr, pdvr->pFile, 1);
					dvr_thmb_setFileNumber(pdvr, pdvr->file_number, 1);
				break;
				case CMD_GET_PAGE_LOCK:
					param = 0;
					param = 0;
					for(i=0; i<pdvr->cur_pageNum; i++) {
						pdvr->disp_thmb[i].lock = FilelistGetLock(pdvr->pFile, pdvr->cur_page*pdvr->disp_number+i);
						param |= (pdvr->disp_thmb[i].lock&0x01);
						param = (param<<1);
					}
					//printf("%s:%d get all file lock: 0x%x\n", __FUNCTION__, __LINE__, param);
					//param = FilelistGetLock(dvr_thmb_getCurIdxOfFile(pdvr));
					pRespData = (unsigned char *)&param;
					iRespLen = sizeof(unsigned int);
				break;
				case CMD_SET_DISP_MODE:
					param = *(unsigned char*)msgPara;
					dvr_thmb_setDispMode(pdvr, (int)param);
				break;
				case CMD_GET_FRAME_RATE:
					pRespData = (char *)&pdvr->pVideoInfo->frameRate;
					iRespLen = sizeof(int);
				break;
				case CMD_SET_RECOVER_FILE:
				{
					printf("======================recover file:%s==================\n", pdvr->pUrl);
					FILE *fd;
					ret = -1;
					pRespData = (char *)&ret;
					iRespLen = sizeof(int);
					if(pdvr->fileType == GP_FILE_TYPE_VIDEO) {
						fd = fopen(pdvr->pUrl, "rb+");
						if(fd) {
							ret = muxMP4CheckRecover(fd, RECOVER_MODE_MOV);
							if(ret < 0) {
								ret = -1;
								printf("recover file:%s error!!!!!!!!\n", pdvr->pUrl);
							}
							else {
								printf("recover file:%s OK!!!!!!!!\n", pdvr->pUrl);
								fflush(fd);
								fsync(fd);
							}
							fclose(fd);
						}
					}
				}
				break;
				case CMD_GET_FILE_DUAL_IDX:
				{
					printf("%s:%d file stat: %s\n", __FUNCTION__, __LINE__, pdvr->pUrl);
					if(strstr(pdvr->pUrl, "MB")) {
						pdvr->fileStat = 1;
					}
					else {
						pdvr->fileStat = 0;
					}
					ret = pdvr->fileStat;
					printf("%s:%d file stat: %d\n", __FUNCTION__, __LINE__, pdvr->fileStat);
					pRespData = (char *)&ret;
					iRespLen = sizeof(int);
					break;
				}
				case CMD_SET_PLAYBACK_EXIT:
					g_bQuit = 1;
				break;
				default:
					printf("get default: %d\n", msgId);
				break;
			}
		//	if (!g_bQuit) 
			{
				//printf("send %d size: %d\n", msgId, iRespLen);
				cvr_pipemsg_send(msgId, iRespLen, pRespData);
			}
			//	cvr_response_cmd(msgId);
		}
	}
	playback_uninit(pdvr);
	buffer_uninit(&DispBufManage);
	pdvr = NULL;
	printf("%s:%d exit cvr_player\n", __FUNCTION__, __LINE__);
	return 0;
}
