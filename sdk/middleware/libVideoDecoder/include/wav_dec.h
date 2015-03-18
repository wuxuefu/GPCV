#ifndef __WAVE_DEC_H__
#define __WAVE_DEC_H__

#define WAVE_OUT_INTERLEAVED		//added by gaoxin 2008.1.4
//=======Constant Definition================
#define WAV_DEC_FRAMESIZE				1024
#define WAV_DEC_BITSTREAM_BUFFER_SIZE   4096
#define WAV_DEC_MEMORY_SIZE				96


//=======wave format tag====================
#define	WAVE_FORMAT_PCM				(0x0001) 
#define	WAVE_FORMAT_ADPCM			(0x0002)
#define	WAVE_FORMAT_ALAW			(0x0006)
#define	WAVE_FORMAT_MULAW			(0x0007)
#define WAVE_FORMAT_IMA_ADPCM		(0x0011)


//========wave decoder error NO.====================
#define WAV_DEC_NO_ERROR    			0
#define WAV_DEC_INBUFFER_NOT_ENOUGH		0x80000001
#define WAV_DEC_RIFF_HEADER_NOT_FOUND	0x80000002
#define WAV_DEC_HEADER_NOT_FOUND		0x80000003
#define WAV_DEC_CHUNK_ALIGN_ERROR		0x80000004
#define WAV_DEC_DATA_CHUNK_NOT_FOUND	0x80000005
#define WAV_DEC_FMT_CHUNK_TOO_SHORT		0x80000006
#define WAV_DEC_CHANNEL_ERROR			0x80000007
#define WAV_DEC_BIT_DEPTH_ERROR			0x80000008
#define WAV_DEC_CHUNK_EXTEND_ERROR		0x80000009
#define WAV_DEC_MSADPCM_COEF_ERROR		0x8000000A
#define WAV_DEC_UNKNOWN_FORMAT			0x8000000B


int wav_dec_init(unsigned char *p_workmem, const unsigned char *p_bsbuf);
// p_bsbuf:      pointer to bit-stream buffer
// p_workmem:    pointer to working memory
// return value: 0 success


#ifndef WAVEFORMATEX
typedef struct{
	short wFormatTag;
	unsigned short wChannels;
	unsigned long dwSamplesPerSecond;
	unsigned long dwAvgBytesPerSec;
	unsigned short wBlockAlign;
	unsigned short wBitsPerSample;
	unsigned short cbSize;
	unsigned short nSamplesPerBlock;
	unsigned short nNumCoef;
	short aCoeff[14];
} WAVE_FORMATEX;
#endif // WAVEFORMATEX


int wav_dec_parsing(unsigned char *p_workmem, unsigned int wi);//, WAVEFORMATEX *p_wave_format
// p_workmem:    pointer to working memory
// wi:           write index of bitstream ring buffer
// return value: 0 success
int wav_dec_set_param(unsigned char *p_workmem, const unsigned char *p_WaveFormatEx);
//this function is only used for AVI

int wav_dec_run(unsigned char *p_workmem, short *p_pcmbuf, unsigned int wi, int pcmbuf_size);
// p_ pcmbuf:    pointer to PCM buffer
// p_workmem:    pointer to working memory
// wi:           write index of bitstream ring buffer
// return value: 
//	positive : samples of PCM
//  zero:      not enough bitstream data
//  negtive:   error


int wav_dec_get_ri(unsigned char *p_workmem);
// p_workmem:    pointer to working memory
// return value: read index of bitstream ring buffer


int wav_dec_get_mem_block_size(void);
// return value: Wave Decoder Working memory size


int wav_dec_get_wFormatTag(unsigned char *p_workmem);
// p_workmem:    pointer to working memory
// return value: wave format tag


int wav_dec_get_nChannels(unsigned char *p_workmem);
// p_workmem:    pointer to working memory
// return value: number of channels


int wav_dec_get_SampleRate(unsigned char *p_workmem);
// p_workmem:    pointer to working memory
// return value: sample rate


int wav_dec_get_nAvgBytesPerSec(unsigned char *p_workmem);
// p_workmem:    pointer to working memory
// return value: average bytes per second


int wav_dec_get_nBlockAlign(unsigned char *p_workmem);
// p_workmem:    pointer to working memory
// return value: block align


int wav_dec_get_wBitsPerSample(unsigned char *p_workmem);
// p_workmem:    pointer to working memory
// return value: bits per sample


const char * wav_dvr_get_version(void);



///////  20110310
void wav_dec_set_bs_ptr(char* mp3dec_workmem, unsigned char *ptr);
void wav_dec_set_ri(unsigned char *wave_dec_workmem, unsigned int ri);
void wav_dec_set_ring_buf_size(unsigned char *wave_dec_workmem, unsigned long size);


#endif //__WAVE_DEC_H__