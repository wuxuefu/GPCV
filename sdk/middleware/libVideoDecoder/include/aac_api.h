
#ifndef __AAC_API_H__
#define __AAC_API_H__

#define SA_SET_DTCM

#ifdef WIN32       
#define diag_printf printf
#else
extern int diag_printf(const char *fmt, ...);
#endif


#define AAC_OK				(0)
#define AAC_BITSTREAM_WAIT	(-1)
#define ERR_SYNC			(-2)
#define ERR_CRC				(-3)
#define ERR_PROFILE			(-4)
#define ERR_LFE				(-5)
#define ERR_CCE				(-6)
#define ERR_ELEID			(-7)
#define ERR_NUMCHAN			(-8)
#define ERR_SCE				(-9)
#define ERR_CPE				(-10)
#define	ERR_PCE		        (-11)
#define ERR_GAIN			(-12)
#define	ERR_HUFFMANBOOK		(-13)


#if 0
#if defined AAC_ON_6400_PLATFORM
#define AAC_BITSTREAM_BUFFER_SIZE (8 * 1024)
#else
#define AAC_BITSTREAM_BUFFER_SIZE (16 * 1024)
#endif
#else
#define AAC_BITSTREAM_BUFFER_SIZE (64 * 1024)
#endif

#define AAC_MAX_FRAME_SAMPLES	(1024)
#define AAC_PCM_BUFFER_SIZE		(1024)
#define AAC_MAX_ENCODED_BYTES	(1600)

enum aac_return_value 
{
	AAC_MORE_DATA	 = 0,
	AAC_INIT_OK		 = 0,
	AAC_PARSING_OK	 = 1,
	AAC_PARSING_FAIL = -1,

	//AAC_FILE_END	 = -1,
	AAC_FILE_END	 = 0, // modify by Comi for GP12 2011.03.29

	AAC_FATAL_ERROR	 = -2,
	AAC_RECOVERABLE  = -128
};


typedef struct 
{
	int frame_counter;
	int error;
	int sample_rate;	    // sample rate in Hz
	int ext_sample_rate;	// SBR sample rate in Hz, for HE-AAC
	int bit_rate;	        // bits per second
	int profile;
	int channels;
	int seekable;
	int adif_format;
	int adts_format;
	int aacPlus_on;
} AAC_DECODER_INFO;


int aac_get_error(void);

/* AAC ENCODER API */
const char *aac_encoder_version_string(void);
int aac_encoder_init(unsigned int bit_rate, unsigned int sample_rate, unsigned int nchannels, unsigned int format, int *Set_DTCM_Add);
int aac_encoder_run(unsigned char *OutBAddr, short *InBAddr);
int aac_encoder_frame_size(void);
int aac_decoder_get_adts(void);

/* AAC DECODER API */
const char *aac_decoder_version_string(void);
int aac_decoder_init(unsigned char *Input_Buffer_Addr, int *Set_DTCM_Add);
int aac_decoder_parsing(int wi,	AAC_DECODER_INFO **);
int aac_decoder_raw_config(AAC_DECODER_INFO **, int aac_profile, int aac_sr, int aac_nch);
int aac_decoder_run(int write_pointer, short *aacoutbuf);
int aac_decoder_bitrate(unsigned char *inbuf, int bufsize);
void aac_decoder_set_file_end(void);
int aac_decoder_read_index(void);
int aac_decoder_get_sync(void);
int aac_decoder_get_adts(void);

void aac_decoder_set_sync(int flag);
int aac_bs_avail_byte_size(int wi, int ri);


// add by Comi 2010.11.26
void aac_decoder_set_read_index(int ri);
void aac_decoder_set_bs_ptr(unsigned char *ptr);

// add by Comi 2011.03.07
void aad_decoder_set_ring_buf_size(unsigned long size);

#endif //!__AAC_API_H__
