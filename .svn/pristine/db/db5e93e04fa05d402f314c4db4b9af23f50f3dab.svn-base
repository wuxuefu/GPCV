#include "gp_mux.h"
#include "gp_avcodec.h"
#include <sys/types.h>
#include <dirent.h>
#include <sys/vfs.h>  
#include <stdlib.h>
#include "stdio.h"
#include "string.h"
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <mqueue.h>
#include <pthread.h>
#include "gp_video_stream_api.h"
#include "folder.h"
#include "storage.h"

#define NALU_TYPE_IDR           (5)
#define NALU_TYPE_SLC           (1)
#define NALU_PRIO_HIGHEST       (3)
#define NALU_IDR                (NALU_TYPE_IDR|(NALU_PRIO_HIGHEST<<5))
#define NALU_SLC                (NALU_TYPE_SLC|(NALU_PRIO_HIGHEST<<5))

#define NEW_FOLDER_FUNC 1
#define LOCK_TIME_STAMP 0
#if 0
#define DEBUG printf
#else
#define DEBUG
#endif
/********************************
system define
*********************************/
#define Stream_MAX 3
#define limit_mini_space 0x20000000  //512M
#define minimum_delete_size 0x20000000  //512M
//#define WORK_DIR "/media/sdcardc1"
#define default_file_lenth 20

 typedef enum vidVideoFrm
{
    VID_I_FRM = 0x0,
    VID_P_FRM = 0x1,    
//    VID_D_FRM = 0x2,
    VID_START_I_FRM = 0x2
}vidVideoFrm_m;

/***************************************
struct info
****************************************/
typedef struct frame_info_s
{
	int frame_time;
	int P_frame_num;
	int record_p_frame_num;
}frame_info_t;

typedef struct stream_info_s
{
	char *path;
	char *pre_name;
	char *post_name;
	char *ext;
	int record_arg[MUX_PARAM_END+1];
	int handle_packer; //mp4 hander
	gpMuxPkt_t key_mux; //record the key_frame for H264
	FILE *cur_file;
	char *filename;
	char *last_filename;
	struct frame_info_s frame_info;
	int min,max;
	int pkg_size;
	unsigned int avg_file_size;
	int total_file_num;
	int lock_current_file;
	int lock_next_file;
	int change_file_notify;
	int request_stream_header;
	int wait_stream_header;
	struct tm lock_file_time;
	struct gpMux_s* packer;
	void* fh;
	int reserve_size;
	pthread_mutex_t stream_lock;
	int disable_delete_file;
}stream_info_t;

typedef struct loop_packer_info_s
{
	mqd_t loop_packer_mq;
	pthread_t async_thread;
	pthread_mutex_t packer_lock;
	int stream_num;
	struct stream_info_s *stream[Stream_MAX];
	UINT64 space_left;
	int loop;
	char* work_path;
	int name_format;
}loop_packer_info_t;

enum async_command{
	SYNC_FILE,
	RM_FILE,
	SYNC_LOCK_FILE,
	AYSNC_CLOSE,
};
typedef struct loop_command_s{
	enum async_command command;
	int handle_packer;
	FILE* file;
	struct gpMux_s* packer;
	char *filename;
	int size;
	struct tm t;
}loop_command_t;

static struct mq_attr mq_attr = {
	.mq_maxmsg = 8,
	.mq_msgsize = sizeof(loop_command_t)
};
/******************************************************
g_packer_info
******************************************************/
struct loop_packer_info_s  *g_packer_info=NULL;

extern struct gpMux_s MP4_packer;
extern struct gpMux_s MOV_packer;
extern struct gpMux_s AVI_packer;

static int del_file(loop_packer_info_t *ptr_loop_info);
/**********************************************************
static interface function
**********************************************************/
static UINT64 find_free_space(const char *path)
{
	struct statfs infoDisk;
	UINT64 freeDiskSize = 0;
	if( statfs((char *)path, &infoDisk) != 0 ) {
	   printf("[%s][%d]statfs fail\n",__FUNCTION__,__LINE__);
	}
	DEBUG("the current disk <%s> info:\n", path);
	DEBUG("f_type:    %d\n", infoDisk.f_type);
	DEBUG("f_bsize:   %d\n", infoDisk.f_bsize);
	DEBUG("f_blocks:  %d\n", (int)infoDisk.f_blocks);
	DEBUG("f_bfree:   %d\n", (int)infoDisk.f_bfree);
	DEBUG("f_bavail:  %d\n", (int)infoDisk.f_bavail);
	DEBUG("f_files:   %d\n", (int)infoDisk.f_files);
	DEBUG("f_ffree:   %d\n", (int)infoDisk.f_ffree);

	freeDiskSize = (UINT64)infoDisk.f_bavail * (UINT64)infoDisk.f_bsize;

	DEBUG("freeDiskSize= %lld\n",freeDiskSize);
	return freeDiskSize;
}
static int index_file_name(char *filename,int index,char *path,char *tag, char* ext)
{
	sprintf(filename,"%s/%s%04d.%s",path,tag,index, ext);
	DEBUG("[index_file_name] %s\n",filename);
	return 0;
}

static SINT32 file_to_index( UINT8 *pName, char *tag)
{
	SINT32 fileNum=-1;
	int lenth=strlen(pName);
	int len_tag =strlen(tag);
	int i;
	
	if(lenth - 4 < len_tag)
	{
		return -1;
	}
	else
	{
		if(strncmp(pName,tag,len_tag)==0)
		{
			char* tmp;
			tmp = strdup(&(pName[len_tag]));
			if (strlen(tmp) == 8)
			{
				tmp[4] = 0;
				fileNum = atoi(tmp);
			}
			else 
				fileNum = -1;
			free(tmp);
		}
		else
			return -1;
	}
	return fileNum;
}
static int find_file_index( char *path,int *index_min,int *index_max,char *tag)
{
	DIR *dir;
	int fNum,hdMux;
	struct dirent *ptr;
	int min=0,max=0,first=0;
	short idx[1000];
	int i = 0, j, swap = 0;

	dir=opendir((char *)path);
	if ( dir == NULL ) {
	    printf("open directory failed...\n");
	    return -1;
	} 
	fNum = 0;
	while((ptr=readdir(dir)) != NULL){    
	    if(ptr->d_name[0] == '.'){
	        continue;
	    }
	    hdMux = file_to_index((UINT8 *)ptr->d_name,tag);
		if(hdMux<0)
			continue;
		fNum++;
	    if(first==0)
	    {
	    	idx[i++] = hdMux;
	    	//min=max=hdMux;
			first=1;
	    }
		else
		{
			if(idx[i-1] > hdMux)
			{
				idx[i] = idx[i-1];
				idx[i-1] = hdMux;
				swap = 1;
			}
			else
				idx[i] = hdMux;
			i++;
			//if(hdMux>max)
			//	max=hdMux;
			//if(hdMux<min)
			//	min =hdMux;
		}
	}
	hdMux = 0;
	closedir(dir);
	
	if (fNum)
	{
		j = fNum;
		//perform bubble sort if necessary
		while (swap)
		{
			swap = 0;
			for(i = 1; i < j-1; i++)
			{
				if (idx[i] < idx[i-1])
				{
					int tmp = idx[i];
					idx[i] = idx[i-1];
					idx[i-1] = tmp;
					swap = 1;
				}
			}
			j--;
		}
		max = idx[fNum-1];
		min = idx[0];
		if (max == 9999 && min == 0)
		{
			i = 0;
			while(idx[i+1] - idx[i] == 1) i++;
			max = idx[i];
			
			i = fNum-1;
			while(idx[i] - idx[i-1] == 1) i--;
			min = idx[i];
		}
	}

	*index_max =max;
	*index_min =min;
	printf("index_min %d, index max %d\n",min,max);
	return 0;
}

static int parse_filename(char* filename, char **path, char **prefix, char **postfix, char** ext)
{
	char string[256];
	char temp[256];
	char *pch, *pch1;
	int len =strlen(filename);
	int i=0,pos=0;
	
	strcpy(string, filename);
	temp[0] = 0;
	
	pch = strtok(string, "/");
	pch1 = NULL;
	
	*path = *prefix = *postfix = *ext = NULL;
	
	while(pch != NULL)
	{
		if (pch1)
			sprintf(temp, "%s%s/", temp, pch1);
		//strcpy(temp, pch);
		pch1 = pch;
		pch = strtok (NULL, "/");
	}
	
	if (strlen(temp))
		*path = strdup(temp);
	else
		*path = strdup("./");
	
	if (pch1)
	{
		pch = strtok(pch1, ".");
		
		if (pch != NULL && *pch != ' ')
			*prefix = strdup(pch);
			
		pch = strtok(NULL,".");
		
		if (pch != NULL && *pch != ' ')
			*postfix = strdup(pch);
			
		pch = strtok(NULL,".");
		
		if (pch != NULL && *pch != ' ')
			*ext = strdup(pch);
	}
	
	printf("filename path = %s, prefix = %s, postfix = %s, ext = %s\n", *path, *prefix, *postfix, *ext);
	return 0;
}

static int get_path(char **path, char* filename)
{
	int i;
	char* out_path;
	
	out_path = strdup(filename);
	
	for (i = strlen(out_path)-1; i >= 0; i--)
	{
		if (out_path[i] == '/')
		{
			out_path[i+1] = 0;
			break;
		}
	}
	
	if (i < 0)
	{
		free(out_path);
		out_path = strdup("./");
	}
	
	*path = out_path;
	
	return 0;
}

static int get_pre_name(char **tag,char* filename)
{
	char string[256];
	char temp[256];
	char* pch;
	int len =strlen(filename);
	int i=0,pos=0;
	
	strcpy(string, filename);
	temp[0] = 0;
	
	pch = strtok(string, "/");
	
	while(pch != NULL)
	{
		strcpy(temp, pch);
		pch = strtok (NULL, "/");
	}
	
	if (strlen(temp) > 0)
	{
		pch = strtok(temp, ".");
	}
	if (pch)
		*tag = strdup(pch);
	//for(i=0;i<len;i++)
	//{
	//	if(filename[i]=='/')
	//		pos=0;
	//	if(filename[i]=='.')
	//	{
	//		temp[pos]='\0';
	//		break;
	//	}
	//	temp[pos]=filename[i];
	//	pos++;
	//}
	//*tag =strdup(temp);
	DEBUG("tag =%s\n",*tag);
	
	return 0;
}

static int get_ext(char **ext, char* filename)
{
	char string[256];
	char temp[256];
	char* pch;
	
	strcpy(string, filename);
	temp[0] = 0;
	
	pch = strtok(string, ".");
	while(pch != NULL)
	{
		strcpy(temp, pch);
		pch = strtok (NULL, ".");
	}
	
	if (strlen(temp) > 0)
		*ext = strdup(temp);
	else
		*ext = NULL;
	
	return 0;
}
#if 0
/***********************************************************
function about PSP
***********************************************************/
static int find_key_SPS_PPS(char *pBuf)
{
	UINT32 uiVolStart;
	char *pHBuf=pBuf;
	int i;
	
	UINT32 uiSize, uiHSize;
	do{
		uiVolStart = LREAD32(pHBuf);
		if (uiVolStart == (0x00000100|NALU_IDR) ||uiVolStart == (0x00000100|NALU_SLC)||(uiVolStart==0x00000001)) {
			if(i==0)
			{
				pHBuf++;
				DEBUG("I have find SPS\n");
			}	
			else if(i==1)
			{
				pHBuf++;
				DEBUG("I have get PPS\n");
			}	
			else if(i==2)
			{
				DEBUG("I get the frist frame\n");
				break;
			}	
			i++;
			} 
			else {
				pHBuf++;
			}
	}
	while (1);
	uiHSize =pHBuf - pBuf;
	DEBUG("uiHSize %d\n",uiHSize);
	return uiHSize;
}

static int save_video_key_frame(stream_info_t *stream_info,gpMuxPkt_t *ptr_key_mux)
{
	char *keyframe;
	int sps_pps_size=0;
	sps_pps_size=find_key_SPS_PPS(ptr_key_mux->data); 
	DEBUG("size of sps_pps_size is %d\n",sps_pps_size);
	keyframe=(char*)malloc(sps_pps_size);
	if(keyframe==NULL)
	{
		printf("[%s][%d]malloc %d fail\n",__FUNCTION__,__LINE__,sizeof(struct loop_packer_info_s));
		return -1;
	}
	memcpy((char *)keyframe,ptr_key_mux->data,sps_pps_size);
	stream_info->key_mux.data =keyframe;
	stream_info->key_mux.size = sps_pps_size;
	return 0;
}

static int sent_video_key_frame(stream_info_t *stream_info,gpMuxPkt_t *ptr_key_mux)
{
	int size=stream_info->key_mux.size+ptr_key_mux->size;
	unsigned char *data= (unsigned char *)malloc(size);
	int ret;
	if(data ==NULL)
	{
		printf("[%s][%d]malloc size fail %d\n",__FUNCTION__,__LINE__,size);
		return -1;
	}
	memcpy(data,stream_info->key_mux.data,stream_info->key_mux.size);
	memcpy(data+stream_info->key_mux.size,ptr_key_mux->data,ptr_key_mux->size);
	ptr_key_mux->data=data;
	ret = stream_info->packer->pack(stream_info->handle_packer,ptr_key_mux,GP_ES_TYPE_VIDEO);
	free(data);
	return ret;
}
#endif
static int change_file(stream_info_t *stream_info)
{
	char filename[256];
	int index;
	int i;
	struct loop_command_s command;
	UINT64 free_space;
	
	stream_info->packer->close(stream_info->handle_packer);
	
	if (stream_info->lock_current_file)
	{
		command.command =SYNC_LOCK_FILE;
		command.t = stream_info->lock_file_time;
	}
	else
		command.command =SYNC_FILE;
	command.file =stream_info->cur_file; 
	command.handle_packer=stream_info->handle_packer;
	command.filename =strdup(stream_info->filename);
	//command.stream = stream_info;
	//command.packer = stream_info->packer;
	if (stream_info->reserve_size)
		command.size = ftell(stream_info->cur_file);
	else
		command.size = 0;
	mq_send(g_packer_info->loop_packer_mq, (const char *) &command, sizeof(struct loop_command_s), 0);
		
	stream_info->max++;
#if NEW_FOLDER_FUNC
	stream_info->max = cam_get_new_file_name(stream_info->fh, filename, stream_info->ext, stream_info->max);
#else	
	if (stream_info->max > 9999)
		stream_info->max = 0;
	index_file_name(filename,stream_info->max,stream_info->path,stream_info->pre_name, stream_info->ext); 
#endif	
	stream_info->cur_file=fopen(filename, "w+b");
	if(stream_info->cur_file==NULL) {
		perror("[change file]open fail\n");
		return -1;
	}
	debug_log_open(filename);
	if(stream_info->last_filename)
	{
		free(stream_info->last_filename);
		stream_info->last_filename = NULL;
	}
	if (stream_info->lock_current_file)
		free(stream_info->filename);
	else
		stream_info->last_filename = stream_info->filename;
	stream_info->filename = strdup(filename);

	stream_info->lock_current_file = 0;
	
	if (stream_info->lock_next_file)
	{
		stream_info->lock_next_file = 0;
		stream_info->lock_current_file = 1;
	}
	
	printf("begin to record the %s\n",filename);
	stream_info->handle_packer = stream_info->packer->open((HANDLE)stream_info->cur_file);
	if(stream_info->handle_packer==0)
	{
		printf("=======why open MP4 fail=========\n");
		return -1;
	}
	for(i=0;i<MUX_PARAM_END;i++)
	{
		stream_info->packer->set(stream_info->handle_packer,i,stream_info->record_arg[i]);
	}
	pthread_mutex_lock(&g_packer_info->packer_lock);
	g_packer_info->space_left = find_free_space(g_packer_info->work_path);

	if (stream_info->reserve_size)
	{
		printf("reserve file size = %dKB\n", stream_info->reserve_size/1024);
		
		if (g_packer_info->space_left < stream_info->reserve_size)
			del_file(g_packer_info);
		truncate(stream_info->filename, stream_info->reserve_size);
	}

	free_space = g_packer_info->space_left;
	if (free_space > limit_mini_space)
		free_space -= limit_mini_space;
	else
		free_space = 0;
	printf("free disk space = %llu\n", free_space);
		
	if (free_space > MP4_MAX_FILE_SIZE)
		free_space = MP4_MAX_FILE_SIZE;
	stream_info->packer->set(stream_info->handle_packer, MUX_MAX_SIZE, (int)free_space);
	pthread_mutex_unlock(&g_packer_info->packer_lock);
	
	//cal agv file size
	if(stream_info->total_file_num == 0)
	{
		stream_info->total_file_num++; 
		stream_info->avg_file_size = stream_info->pkg_size;
	}
	else
	{
		unsigned int total_size;
		
		total_size = stream_info->pkg_size + stream_info->avg_file_size * stream_info->total_file_num;
		stream_info->avg_file_size= total_size / (stream_info->total_file_num + 1);
		
		if (stream_info->total_file_num < 4)
			stream_info->total_file_num ++;
	}
	stream_info->pkg_size =0;
	printf("avg size of %s is %d\n",stream_info->pre_name,stream_info->avg_file_size);
	memset(&stream_info->frame_info,0,sizeof(struct frame_info_s ));
	
	stream_info->change_file_notify = 1;
}
static int find_stream_index(stream_info_t *stream)
{
	char *tag =stream->pre_name;
	stream_info_t *temp;
	int index=-1;
	int i=0;
	for(i=0;i<Stream_MAX;i++)
	{
		temp=g_packer_info->stream[i];
		if((temp!=NULL)&&(strcmp(temp->pre_name,stream->pre_name)==0))
		{
			index = i;
			break;
		}
	}
	return index;
}
static int find_ng_index()
{
	int index=-1;
	int i=0;
	stream_info_t *temp;
	
	for(i=0;i<Stream_MAX;i++)
	{
		temp=g_packer_info->stream[i];
		if(temp==NULL)
		{
			index =i;
			break;
		}
	}
	return index;
}

static int lock_file(char* filename, struct tm t)
{
	char string[256];
	char* newfilename;
	int len;
	
	newfilename = strdup(filename);
	len = strlen(newfilename);
	if (g_packer_info->name_format == TIME_IDX_FILE_NAME)
	{
		newfilename[len-12] = 'S';
		newfilename[len-11] = 'O';
		newfilename[len-10] = 'S';
	}
	else
	{
		newfilename[len-4] = 0;
		sprintf(string, "%s_LOCK.MOV", newfilename);
		free(newfilename);
		newfilename = strdup(string);
	}
#if LOCK_TIME_STAMP
	newfilename[len-4] = 0;
	sprintf(string, "%s_%02d%02d%02d.MOV", newfilename, t.tm_hour, t.tm_min, t.tm_sec);
	free(newfilename);
	newfilename = strdup(string);
#endif	
	
	sprintf(string, "chmod a-w %s &", filename);
	system(string);
	sprintf(string, "mv %s %s &", filename, newfilename);
	system(string);
	free(newfilename);
	printf("lock file: %s\n", filename);
}
/**********************************************
ASYNC FUNCTION
***********************************************/

static int del_file(loop_packer_info_t *ptr_loop_info)
{
	int i=0;
	UINT64 del_space = 0;
	char filename[256];
	struct timeval t1, t2,t3;
	unsigned int free_space;
	int do_delete = 0;
	
	//gettimeofday(&t1,NULL);
	DEBUG("stream num %d\n",ptr_loop_info->stream_num);
	for(i=0;i<ptr_loop_info->stream_num;i++)
	{
		stream_info_t *stream;
		stream=ptr_loop_info->stream[i];
		if(stream!=NULL && !stream->disable_delete_file)
		{
			do_delete = 1;
#if NEW_FOLDER_FUNC
			int ret;
			ret = cam_delete_file(stream->fh, stream->avg_file_size + 0x3200000);
			del_space += (ret < 0 ? 0: ret);
#else				
			do
			{
				struct stat stbuf;
				if(stream->min == stream->max)
					break;
				index_file_name(filename, stream->min, stream->path, stream->pre_name, stream->ext);
				stream->min++;
				if (stream->min > 9999)
					stream->min = 0;
				
				stat(filename, &stbuf);
				if (stbuf.st_mode & S_IWUSR)
				{
					printf("del the file %s\n",filename);
					if(unlink(filename) == 0)
						del_space += stbuf.st_size;
				}
				if (del_space >= 0x3200000) //minimum 50MB 
					break;
			}while(1);
#endif
		}
	}
	//gettimeofday(&t3,NULL);
	if (del_space == 0)
	{
		if (do_delete)
			return -1;
		else
			return 0;
	}
		
	//system("sync");
	//gettimeofday(&t2,NULL);
	DEBUG("del  time  %d ms\n",(t3.tv_sec-t1.tv_sec)*1000+(t3.tv_usec-t1.tv_usec)/1000);
	DEBUG("del all time  %d ms\n",(t2.tv_sec-t1.tv_sec)*1000+(t2.tv_usec-t1.tv_usec)/1000);
	printf("space left %lld k, del %lld k\n", ptr_loop_info->space_left>>10, del_space>>10);
	
	if (ptr_loop_info->work_path)
	{
		ptr_loop_info->space_left =find_free_space(ptr_loop_info->work_path);
		
		for(i = 0; i < ptr_loop_info->stream_num; i++)
		{
			ptr_loop_info->stream[i]->packer->get(ptr_loop_info->stream[i]->handle_packer, MUX_MAX_SIZE, &free_space);
			free_space += del_space;
			free_space = (free_space > MP4_MAX_FILE_SIZE) ? MP4_MAX_FILE_SIZE : free_space;
			ptr_loop_info->stream[i]->packer->set(ptr_loop_info->stream[i]->handle_packer, MUX_MAX_SIZE, free_space);
		}
	}
	
	return 0;
	
}

static unsigned int get_all_stream_avg_size(loop_packer_info_t *ptr_loop_info)
{
	unsigned int next_file_size=0;
	int i;
	for(i=0;i<ptr_loop_info->stream_num;i++)
	{
		stream_info_t *stream;
		stream=ptr_loop_info->stream[i];
		if(stream!=NULL)
		{
			next_file_size =stream->avg_file_size+next_file_size;
		}
	}

	return next_file_size;
	
}
static void *Async(void *argv)
{
	loop_packer_info_t *ptr_loop_info = (loop_packer_info_t *)argv;
	struct loop_command_s  command_buf;
	struct loop_command_s *command=&command_buf;
	int index;
	int msg_prio;
	int ret;
	char filename[256];
	struct timespec t1;
	unsigned int free_space;
	int cannot_delete = 0;
	struct timeval tv1,tv2;

	//while(ptr_loop_info->loop)
	while(1)
	{
		clock_gettime(CLOCK_REALTIME, &t1);
		t1.tv_nsec += 10000;
		ret = mq_timedreceive(ptr_loop_info->loop_packer_mq, (char *) command, sizeof(struct loop_command_s), &msg_prio,&t1);
		//ret = mq_receive(ptr_loop_info->loop_packer_mq, (char *) command, sizeof(struct loop_command_s), &msg_prio);
		if (ret < 0) {
			ret = 0;
			if (!cannot_delete)
			{
				pthread_mutex_lock(&ptr_loop_info->packer_lock);
				if(g_packer_info->work_path && g_packer_info->space_left < limit_mini_space)
					ret = del_file(ptr_loop_info);
				pthread_mutex_unlock(&ptr_loop_info->packer_lock);
				if (ret == 0)
					sync();

				if (ret < 0)
				{
					gettimeofday(&tv1, NULL);
					cannot_delete = 1;
				}
			}
			
			if (cannot_delete)
			{
				gettimeofday(&tv2, NULL);
				if (tv2.tv_sec - tv1.tv_sec > 3)
				{
					cannot_delete = 0;
					tv1 = tv2;
				}
			}
			usleep(1000);
			continue;
		}	
		switch(command->command)
		{
			case SYNC_FILE:
			case SYNC_LOCK_FILE:
				sleep(1);
				//command->packer->close((SINT32)command->handle_packer);

				fflush(command->file);
				fsync(fileno(command->file));
				fclose(command->file);
				if (command->size)
					truncate(command->filename, command->size);
				if (command->command == SYNC_LOCK_FILE)
					lock_file(command->filename, command->t);

				if(command->filename)
				{
					printf("======close file  %s=======\n",command->filename);
					free(command->filename);
				}
				//system("sync ");
				//sync();
				//ptr_loop_info->space_left = find_free_space(ptr_loop_info->work_path);
				DEBUG("========left space %lld k==========\n",ptr_loop_info->space_left>>10);
				
				cannot_delete = 0;
				break;
			case AYSNC_CLOSE:
				  mq_close(g_packer_info->loop_packer_mq);
				  return 0;
			default:
				    printf("unknow the command\n");
		}
	
	}
}

/***************************************************
interface for extern
***************************************************/
static SINT32 loop_packer_close(SINT32 hd)
{
	struct stream_info_s *stream =(struct stream_info_s *)hd;
	int index;
	
	if(stream->pre_name!=NULL)
		printf("close the stream %s\n",stream->pre_name);

	pthread_mutex_lock(&g_packer_info->packer_lock);
	index=find_stream_index(stream);
	if(index < 0)
	{
		pthread_mutex_unlock(&g_packer_info->packer_lock);
		return -1;
	}
	g_packer_info->stream[index]=NULL;
	g_packer_info->stream_num--;
	pthread_mutex_unlock(&g_packer_info->packer_lock);
	
	
	if(stream->handle_packer)
		stream->packer->close(stream->handle_packer);

	if(stream->cur_file)
	{
		int fsize = ftell(stream->cur_file);
		fflush(stream->cur_file);
		fsync(fileno(stream->cur_file));
		fclose(stream->cur_file);
		if (stream->reserve_size)
			truncate(stream->filename, fsize);
		printf("close and sync finished\n");
		debug_log_close();
		
		if (stream->lock_current_file)
			lock_file(stream->filename, stream->lock_file_time);

		//printf("wait sync \n");
		//system("sync");
		//printf("sync end \n");
	}
#if NEW_FOLDER_FUNC
	cam_folder_close(stream->fh);
#endif	
	
	if(stream->key_mux.data)
		free(stream->key_mux.data);
	
	if(stream->path)
		free(stream->path);
	if(stream->pre_name)
		free(stream->pre_name);
	if(stream->ext)
		free(stream->ext);
	if(stream->filename)
		free(stream->filename);
	if(stream->last_filename)
		free(stream->last_filename);

	pthread_mutex_destroy(&stream->stream_lock);
	free(stream);
	
	return 0;
	
}

static SINT32 loop_packer_open(HANDLE out)
{
	struct stream_info_s  *stream;
	int index;
	char filename[256];
	char* ext;
	int try_cnt=0;
	if(g_packer_info->stream_num==Stream_MAX)
	{
		printf("stream is enough \n");
		return -1;
	}


	index = find_ng_index();
	stream = malloc(sizeof(struct stream_info_s));
	memset(stream,0,sizeof(struct stream_info_s));
	//find pre name
	parse_filename((char*)out, &stream->path, &stream->pre_name, &stream->post_name, &stream->ext);
	//get_path(&stream->path, (char*)out);
	//get_pre_name(&stream->pre_name,(char*)out);
	//get_ext(&stream->ext, (char*)out);
	
	if (stream->ext)
	{
		if (strncasecmp(stream->ext, "mp4", 3) == 0)
			stream->packer = &MP4_packer;
		else if (strncasecmp(stream->ext, "mov", 3) == 0)
			stream->packer = &MOV_packer;
		else if (strncasecmp(stream->ext, "avi", 3) == 0)
			stream->packer = &AVI_packer;
	}
	else
	{
		stream->ext = strdup("MP4");
		stream->packer = &MP4_packer;
	}
#if NEW_FOLDER_FUNC
	stream->fh = cam_folder_init(stream->path, stream->pre_name, stream->post_name, &stream->max, g_packer_info->name_format);

	if (stream->max < 0)
	{
		fprintf(stderr, "[%s]folder init fail\n", __FUNCTION__);
		return -1;
	}
	stream->max = cam_get_new_file_name(stream->fh, filename, stream->ext, stream->max);
	if (stream->max < 0)
	{
		fprintf(stderr, "[%s]file name init fail\n", __FUNCTION__);
		return -1;
	}
#else	
	find_file_index(stream->path,&stream->min,&stream->max,stream->pre_name);
	stream->max++;
	if (stream->max > 9999)
		stream->max = 0;
	index_file_name(filename,stream->max,stream->path,stream->pre_name, stream->ext);
#endif	
	stream->cur_file =fopen(filename, "w+b");
	if(!stream->cur_file) {
		fprintf(stderr, "[%s]open fail\n", __FUNCTION__);
		return -1;
	}
	debug_log_open(filename);
	truncate(filename, 0x3200000);
	stream->handle_packer = stream->packer->open(stream->cur_file);
	stream->record_arg[MUX_FILE_TIME] =default_file_lenth;
	stream->filename=strdup(filename);
	stream->avg_file_size = 0x3200000; //50 MB
	pthread_mutex_init(&stream->stream_lock,NULL);
	stream->disable_delete_file = 1;
	
	pthread_mutex_lock(&g_packer_info->packer_lock);
	g_packer_info->stream[index]=stream;
	g_packer_info->stream_num++;
	
	if (!g_packer_info->work_path)
	{
		g_packer_info->work_path = strdup(stream->path);
		g_packer_info->space_left = find_free_space(g_packer_info->work_path);
	}
	
	pthread_mutex_unlock(&g_packer_info->packer_lock);
	
	//while(g_packer_info->space_left < limit_mini_space)
	//{	
	//	del_file(g_packer_info);
	//	if(try_cnt++>10)
	//		break;
	//}
	//if(g_packer_info->space_left<limit_mini_space)
	//{
	//	printf("space is limit \n");
	//	loop_packer_close((int)stream);
	//	return -1;
	//}
	printf("begin to record the %s\n",filename);
	return (SINT32)stream;
	
}

		
static SINT32 loop_paker_pack(SINT32 hd, gpMuxPkt_t *pkt, gpEsType_t type)
{
	stream_info_t *stream_info=(stream_info_t *)hd;
	int ret=0;

	pthread_mutex_lock(&stream_info->stream_lock);
	
	if (type == GP_ES_TYPE_VIDEO)
	{
		if (pkt->frameType == VID_START_I_FRM)
		{
			if (stream_info->wait_stream_header)
			{
				change_file(stream_info);
				stream_info->wait_stream_header = 0;
			}
			pkt->frameType = VID_I_FRM;
		}

		ret = stream_info->packer->pack(stream_info->handle_packer,pkt,type);
		
		if (ret == MUX_MEM_FULL)
		{
			int delete_size = 0;
			pthread_mutex_lock(&g_packer_info->packer_lock);
			//try delete file and retry pack.
			delete_size = del_file(g_packer_info);
			pthread_mutex_unlock(&g_packer_info->packer_lock);
			if (0 == delete_size)
				ret = stream_info->packer->pack(stream_info->handle_packer,pkt,type);
		}
		
		if (ret >= 0)
		{
			stream_info->frame_info.frame_time += pkt->pts;
		
			if(!stream_info->wait_stream_header && stream_info->frame_info.frame_time > stream_info->record_arg[MUX_FILE_TIME]*1000)
				stream_info->request_stream_header = 1;
		}
	}
	else if (type == GP_ES_TYPE_AUDIO)
	{
		ret = stream_info->packer->pack(stream_info->handle_packer, pkt, type);
		
		if (ret == MUX_MEM_FULL)
		{
			int delete_size = 0;
			pthread_mutex_lock(&g_packer_info->packer_lock);
			//try delete file and retry pack.
			delete_size = del_file(g_packer_info);
			pthread_mutex_unlock(&g_packer_info->packer_lock);
			if (0 == delete_size)
				ret = stream_info->packer->pack(stream_info->handle_packer,pkt,type);
		}
	}
	else if (type == GP_ES_TYPE_THUMB)
	{
		if (stream_info->wait_stream_header)
		{
			change_file(stream_info);
			stream_info->wait_stream_header = 0;
		}
		
		ret = stream_info->packer->pack(stream_info->handle_packer,pkt,type);
	}

//	if((stream_info->frame_info.frame_time==0)&&(type==GP_ES_TYPE_VIDEO))//use for first video frame
//	{
//		if(pkt->frameType==VID_I_FRM)
//		{
//			if(stream_info->key_mux.data!=NULL)
//			{
//				
//				DEBUG("sent the frist packet\n");
//				ret=sent_video_key_frame(stream_info,pkt);
//			}
//			else
//			{
//				save_video_key_frame(stream_info,pkt);
//				ret = stream_info->packer->pack(stream_info->handle_packer,pkt,type);
//				if(pkt->pts==0) pkt->pts=1;
//				
//			}
//		}
//		else if(pkt->frameType!=VID_I_FRM)
//		{
//			static int i=0;
//			i++;
//			printf("the first key is not VID_I_FRM %d\n",i);
//			 return 0;
//		}	
//	}
//	else
//		ret = stream_info->packer->pack(stream_info->handle_packer,pkt,type);
//
//	if(type==GP_ES_TYPE_VIDEO)
//	{
//		stream_info->frame_info.frame_time+=pkt->pts;
//		if(pkt->frameType==VID_I_FRM)
//		{
//			
//			if(stream_info->frame_info.record_p_frame_num==0)
//				stream_info->frame_info.record_p_frame_num=stream_info->frame_info.P_frame_num;
//			stream_info->frame_info.P_frame_num=0;
//		//	printf("dumy frame is %d\n",stream_info->frame_info.record_p_frame_num);
//		}
//		else
//		{
//			stream_info->frame_info.P_frame_num++;
//		}
//		
//		if((stream_info->frame_info.frame_time>stream_info->record_arg[MUX_FILE_TIME]*1000) &&
//			(stream_info->frame_info.record_p_frame_num==stream_info->frame_info.P_frame_num))
//		{
//			//struct timeval t1,t2;
//			//gettimeofday(&t1,NULL);
//			change_file(stream_info);
//			//gettimeofday(&t2,NULL);
//			changeFileNotify = 1;
//		}
//	}

	if (ret == MUX_RET_OK)
	{
		pthread_mutex_lock(&g_packer_info->packer_lock);
		g_packer_info->space_left-=pkt->size;
		stream_info->pkg_size +=pkt->size;
		pthread_mutex_unlock(&g_packer_info->packer_lock);
	}
	pthread_mutex_unlock(&stream_info->stream_lock);
	
	return ret;
}

static SINT32 loop_packer_set(SINT32 hd, UINT32 param, UINT32 value)
{
  	stream_info_t *stream_info=(stream_info_t *)hd;
	
	if (param < MUX_PARAM_END)
		stream_info->record_arg[param] = value;

	if (param == MUX_FILE_TIME)
	{
		stream_info->disable_delete_file = ((value >= 18000) ? 1: 0);
		pthread_mutex_lock(&g_packer_info->packer_lock);
		
		if (!stream_info->disable_delete_file && (g_packer_info->space_left < limit_mini_space))
			del_file(g_packer_info);
		pthread_mutex_unlock(&g_packer_info->packer_lock);
	}
	else if (param == MUX_LOCK_FILE)
	{
		time_t t;
		
		pthread_mutex_lock(&stream_info->stream_lock);
		time(&t);
		stream_info->lock_current_file = 1;
		stream_info->lock_file_time = *localtime(&t);
		if (stream_info->frame_info.frame_time < 10*1000)
		{
			if (stream_info->last_filename)
			{
				lock_file(stream_info->last_filename, stream_info->lock_file_time);
				free(stream_info->last_filename);
				stream_info->last_filename = NULL;
			}
		}
		if (stream_info->frame_info.frame_time > (stream_info->record_arg[MUX_FILE_TIME]*1000 - 10*1000))
		{
			stream_info->lock_next_file = 1;
		}
		pthread_mutex_unlock(&stream_info->stream_lock);
		return 0;
	}
	else if (param == MUX_FILENAME_FORMAT)
	{
		g_packer_info->name_format = value;
		
		return 0;
	}
	else if (param == MUX_RESERVE_SIZE)
	{
		stream_info->reserve_size = value;
		if (stream_info->pkg_size < stream_info->reserve_size)
		{
			pthread_mutex_lock(&g_packer_info->packer_lock);
			if (g_packer_info->space_left < stream_info->reserve_size)
				del_file(g_packer_info);
			pthread_mutex_unlock(&g_packer_info->packer_lock);
			printf("reserve file size = %dKB\n", stream_info->reserve_size/1024);
			truncate(stream_info->filename, stream_info->reserve_size);
		}
		return 0;
	}

	stream_info->packer->set(stream_info->handle_packer,param,value);
	return 0;
}
static SINT32 loop_packer_get(SINT32 hd, UINT32 param, UINT32 *value)
{
	int ret;
	stream_info_t *stream_info=(stream_info_t *)hd;

	pthread_mutex_lock(&stream_info->stream_lock);
	
	if (param == MUX_CHANGE_FILE)
	{
		*value = stream_info->change_file_notify;
		stream_info->change_file_notify = 0;
		pthread_mutex_unlock(&stream_info->stream_lock);
		return 0;
	}
	else if (param == MUX_REQUEST_HEADER)
	{
		*value = stream_info->request_stream_header;
		stream_info->request_stream_header = 0;
		if (*value)
			stream_info->wait_stream_header = 1;
		pthread_mutex_unlock(&stream_info->stream_lock);
		return 0;
	}
	else if (param == MUX_LOCK_FILE)
	{
		*value = stream_info->lock_current_file;
		pthread_mutex_unlock(&stream_info->stream_lock);
		return 0;
	}
	pthread_mutex_unlock(&stream_info->stream_lock);
	return stream_info->packer->get(stream_info->handle_packer,  param,   value);	
}


static int loop_packer_init()
{
	g_packer_info = malloc(sizeof(struct loop_packer_info_s ));
	if(g_packer_info==NULL)
	{
		printf("[%s][%d]malloc %d size fail\n",__FUNCTION__,__LINE__,sizeof(struct loop_packer_info_s));
		return -1;
	}
	memset(g_packer_info,0,sizeof(struct loop_packer_info_s ));
	//g_packer_info->loop =1;
	//g_packer_info->space_left =find_free_space(WORK_DIR);
	
	mq_unlink("/LoopMsgQ");
	g_packer_info->loop_packer_mq= mq_open("/LoopMsgQ", O_CREAT | O_RDWR, S_IREAD | S_IWRITE, &mq_attr);
	if(g_packer_info->loop_packer_mq==(mqd_t) -1)
	{
		printf("[%s][%d] create mq fail\n",__FUNCTION__,__LINE__);
		free(g_packer_info);
		g_packer_info =NULL;
	}
	if(pthread_mutex_init(&g_packer_info->packer_lock,NULL)!=0)
	{
		printf("[%s][%d]create mutex fail\n",__FUNCTION__,__LINE__);
		mq_close(g_packer_info->loop_packer_mq);
		free(g_packer_info);
		g_packer_info =NULL;
	}

	
	if (pthread_create(&g_packer_info->async_thread, NULL, Async, g_packer_info) != 0) {
		printf("[%s:%d], storage input create thread fail.\n", __FUNCTION__, __LINE__);
		mq_close(g_packer_info->loop_packer_mq);
		pthread_mutex_destroy(&g_packer_info->packer_lock);
		free(g_packer_info);
		g_packer_info =NULL;
		return -1;
	}

	return 0;
}

static SINT32 loop_packer_uninit()
{
	int i;
	struct loop_command_s command;
	
	for(i=0;i<Stream_MAX;i++)
	{
		if(g_packer_info->stream_num==0)
			break;
		
		if(g_packer_info->stream[i]!=NULL)
			loop_packer_close((SINT32)g_packer_info->stream[i]);
	}
	//g_packer_info->loop =0;
	command.command =AYSNC_CLOSE;
	mq_send(g_packer_info->loop_packer_mq, (const char *) &command, sizeof(struct loop_command_s), 0);
	pthread_join(g_packer_info->async_thread, NULL);
	pthread_mutex_destroy(&g_packer_info->packer_lock);
	
	if (g_packer_info->work_path)
		free(g_packer_info->work_path);
	free(g_packer_info);
	g_packer_info =NULL;
	//sync();
	return 0;
}



struct gpMux_s loop_packer={
	.init= loop_packer_init,
	.uninit =loop_packer_uninit,
	.open=loop_packer_open,
	.close = loop_packer_close,
	.pack = loop_paker_pack,
	.set = loop_packer_set,
	.get =loop_packer_get,
};
