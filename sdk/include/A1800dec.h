#ifndef __A1800DEC_H__
#define __A1800DEC_H__

/*typedef struct
{
	long WaveLen;
	short BitRate;	//added by zgq on 20060807
} A1800_INFO;*/

#define A1800DEC_MEMORY_BLOCK_SIZE		5824//1676

#define A18_DEC_FRAMESIZE        320
#define A18_DEC_BITSTREAM_BUFFER_SIZE 4096


#define A18_OK						0x00000001
#define A18_E_NO_MORE_SRCDATA		0x80040005

extern int  A18_dec_SetRingBufferSize(void *obj, int size);
//extern void A18_dec_set_file_end(void *obj, int write_index);
extern int  a1800dec_run(void *obj, int write_index, short * pcm_out);
extern int  a1800dec_init(void *obj, const unsigned char* bs_buf);
extern int  a1800dec_parsing(void *obj, int write_index);
extern int  a1800dec_read_index(void *obj);
extern int  a1800dec_GetMemoryBlockSize(void);
extern int  a1800dec_errno(void *obj);

extern const char* A18_dec_get_version(void);
extern int  A18_dec_get_bitrate(void *obj);
extern int  A18_dec_get_samplerate(void *obj);
extern int	A18_dec_get_channel(void *obj);
extern int	A18_dec_get_bitspersample(void *obj);
//extern int  A18_dec_get_PackageSize(void *obj);	// byte


#endif //__A1800DEC_H__
