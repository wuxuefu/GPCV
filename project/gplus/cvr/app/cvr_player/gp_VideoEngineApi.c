#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <linux/soundcard.h>
#include <fcntl.h>

#include "media_shell.h"
#include <mach/gp_display.h>

#include "video_decoder_api.h"

#define success 	0
#define fail		1
#if 1
#define ext_printf printf
#else
#define ext_printf(...)
#endif


//1:success, -1:error
int ExtGpVideoEngineOpen(int disp, int width, int height, void (*callback_fun)(unsigned int id, UINT32 data), gp_disp_buffer_func get_disp_buf, gp_disp_buffer_func send_disp_buf )
{
	ext_printf("%s:%d\n", __FUNCTION__, __LINE__);
	//int ret = video_decode_entrance(0,SP_BITMAP_RGB565);
	int ret = video_decode_entrance(disp,SP_BITMAP_YCbYCr);
	video_decode_set_play_callback(callback_fun);
	ret = video_decode_set_display_aspect(1);
	if(disp == C_DISP_BUFFER) {
		video_decode_set_display_buffer_param(width, height, get_disp_buf, send_disp_buf);
	}
	printf("%s:%d aspect ret = %d\n", __FUNCTION__, __LINE__, ret);
	return ret;
}
UINT32 ExtGpVideoEngineSetUrl(char *pUrl)
{	
	UINT32 ret = -1;
	ext_printf("%s:%d\n", __FUNCTION__, __LINE__);

	ret = video_decode_parser_start(pUrl);
	return ret;
	
}

UINT32 ExtGpVideoEnginePlay(UINT32 Stamp)
{
	ext_printf("%s:%d\n", __FUNCTION__, __LINE__);
	int ret = video_decode_start();
	if(ret<0) {
		video_decode_parser_stop();
	}
	return ret;
}

void ExtGpVideoEngineStop()
{
	int ret = 0;
	ext_printf("%s:%d\n", __FUNCTION__, __LINE__);
	ret = video_decode_stop();
	ext_printf("%s:%d ret = %d\n", __FUNCTION__, __LINE__, ret);
	ret = video_decode_parser_stop();
	ext_printf("%s:%d ret = %d\n", __FUNCTION__, __LINE__, ret);
}

void ExtGpVideoEngineExit()
{
	ext_printf("%s:%d\n", __FUNCTION__, __LINE__);
	ExtGpVideoEngineStop();
	video_decode_exit();	
	printf("exit mcplayer\n");
}


void ExtGpVideoEnginePause()
{
	printf("video engine pause!\n");
	video_decode_pause();
}

void ExtGpVideoEngineResume()
{
	printf("video engine resume!\n");
	video_decode_resume();
	
	printf("video engine resume! ok\n");
}

SINT32 ExtGpVideoEngineGetInfo(gp_size_t *size)
{
	printf("video engine get info!\n");
	SINT32 ret = -1;
	UINT16 w, h;
	ret = video_decode_get_video_info(&w, &h);
	printf("ret = %d\n", ret);
	if(ret != 0) {
		size->width=size->height=0;
	}
	else {
		size->width=w;
		size->height=h;
	}
	return ret;
}


unsigned long ExtGpVideoEngineSeek(unsigned long nGotoTime)
{
	ext_printf("%s:%d\n", __FUNCTION__, __LINE__);
	return video_decode_set_play_time(nGotoTime);
}


SINT32 ExtGpVideoEngineGetStatus()
{
	return video_decode_status();
}

SINT32 ExtGpVideoEngineGetPlayingStatus()
{
	return video_decode_playing_status();
}

UINT32 ExtGpVideoEngineSetSpeed(float spd)
{
	printf("video set speed %f\n", spd);
	return video_decode_set_play_speed(spd);
}

//1: start reversed play, 0: stop reversed play
//reversed play just play I frame.
int ExtGpVideoEngineReversePlay(int revers)
{
	return video_decode_set_reverse_play(revers);
}

unsigned long ExtGpVideoEngineGetTotalTime()
{
	unsigned long total_time=video_decode_get_total_time();
	return total_time;
}

unsigned long ExtGpVideoEngineGetTotalSample()
{
	unsigned long total_sample = video_decode_get_total_samples();
	return total_sample;
}

SINT32 ExtGpVideoEngineGetCurTime()
{
	return video_decode_get_current_time();
}


void ExtGpVideoEngineSetVolume(int nGotoVol)
{
	printf("mmmmm set vol %d\n", nGotoVol);
	if(nGotoVol<1) {	
		nGotoVol=1; 
	}
	//using audio_mixer
	SINT32 volume = (nGotoVol * 100) / 9;
	volume |= volume<<8;
	//videoPlayerSetAudVolume( nGotoVol );
    SINT32 MixHandle = open("/dev/mixer", O_RDONLY);

    ioctl(MixHandle, SOUND_MIXER_WRITE_VOLUME, &volume);
    close(MixHandle);
}

SINT32 ExtGpVideoEngineGetVolume()
{
    SINT32 MixHandle;
    SINT32 volume = 0;
    MixHandle = open("/dev/mixer", O_RDONLY);

    ioctl(MixHandle, SOUND_MIXER_READ_VOLUME, &volume);
    close(MixHandle);
	ext_printf("%s:%d get: %d\n", __FUNCTION__, __LINE__, volume&0xff);

    volume = ((volume&0xff)*9)/100;
	
	ext_printf("%s:%d get: %d\n", __FUNCTION__, __LINE__, volume);
    return volume;
}

int ExtGpVideoEngineGetThumbnail(gp_bitmap_t *bitmap)
{

	return video_decode_get_thumbnail(bitmap);
}


int ExtGpVideoEngineQtffGetThumbnail(char *filename, gp_bitmap_t *bitmap)
{
	//return qtff_get_thumbnail(filename, bitmap->type, bitmap->validRect.width, bitmap->validRect.height, bitmap->pData);
	return qtff_get_thumbnail(filename, bitmap);
}

int ExtGpVideoEngineGetFrameRate(void)
{
	return video_decode_get_frame_rate();
}
