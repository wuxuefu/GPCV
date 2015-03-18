#ifndef __mp3_dec_h__
#define __mp3_dec_h__

/////////////////////////////////////////////////////////////////////////////
//		Constant Definition
/////////////////////////////////////////////////////////////////////////////
#define MP3_DEC_FRAMESIZE					1152
#define MP3_DEC_BITSTREAM_BUFFER_SIZE   	(64*1024) 
#define MP3_DEC_MEMORY_SIZE 				13496
#define MP3_DEC_HW_MEMORY_SIZE 				8192 // actual size is 7176B


/////////////////////////////////////////////////////////////////////////////
//		Error Code
/////////////////////////////////////////////////////////////////////////////
#define MP3_DEC_ERR_NONE			0x00000000	/* no error */

#define MP3_DEC_ERR_BUFLEN	   	   	0x80000001	/* input buffer too small (or EOF) */
#define MP3_DEC_ERR_BUFPTR	   	   	0x80000002	/* invalid (null) buffer pointer */

#define MP3_DEC_ERR_NOMEM	   	   	0x80000031	/* not enough memory */

#define MP3_DEC_ERR_LOSTSYNC	   	0x80000101	/* lost synchronization */
#define MP3_DEC_ERR_BADLAYER	   	0x80000102	/* reserved header layer value */
#define MP3_DEC_ERR_BADBITRATE	   	0x80000103	/* forbidden bitrate value */
#define MP3_DEC_ERR_BADSAMPLERATE  	0x80000104	/* reserved sample frequency value */
#define MP3_DEC_ERR_BADEMPHASIS	   	0x80000105	/* reserved emphasis value */

#define MP3_DEC_ERR_BADCRC	   	   	0x80000201	/* CRC check failed */
#define MP3_DEC_ERR_BADBITALLOC	   	0x80000211	/* forbidden bit allocation value */
#define MP3_DEC_ERR_BADSCALEFACTOR  0x80000221	/* bad scalefactor index */
#define MP3_DEC_ERR_BADMODE         0x80000222	/* bad bitrate/mode combination */
#define MP3_DEC_ERR_BADFRAMELEN	    0x80000231	/* bad frame length */
#define MP3_DEC_ERR_BADBIGVALUES    0x80000232	/* bad big_values count */
#define MP3_DEC_ERR_BADBLOCKTYPE    0x80000233	/* reserved block_type */
#define MP3_DEC_ERR_BADSCFSI	    0x80000234	/* bad scalefactor selection info */
#define MP3_DEC_ERR_BADDATAPTR	    0x80000235	/* bad main_data_begin pointer */
#define MP3_DEC_ERR_BADPART3LEN	    0x80000236	/* bad audio data length */
#define MP3_DEC_ERR_BADHUFFTABLE    0x80000237	/* bad Huffman table select */
#define MP3_DEC_ERR_BADHUFFDATA	    0x80000238	/* Huffman data overrun */
#define MP3_DEC_ERR_BADSTEREO	    0x80000239	/* incompatible block_type for JS */




/////////////////////////////////////////////////////////////////////////////
//		Function Definition
/////////////////////////////////////////////////////////////////////////////

// MP3 Decoder Initial
int mp3_dec_init(char *p_workmem, unsigned char *p_bsbuf, char *ram);


// MP3 header parsing
int mp3_dec_parsing(char *p_workmem, unsigned int wi);

// MP3 Decoder
int mp3_dec_run(char *p_workmem, short *p_pcmbuf, unsigned int wi);


// Get Read Index
int mp3_dec_get_ri(char *p_workmem);
// p_workmem:    pointer to working memory
// return value: read index of bitstream ring buffer

// set Read Index
void mp3_dec_set_ri(char *p_workmem, int ri);


// Get mpeg id
const char *mp3_dec_get_mpegid(char *p_workmem);

int mp3_dec_get_mem_block_size (void);
// return value: MP3 Decoder Working memory size

// return error number.
int mp3_dec_get_errno(char *p_workmem);

// return layer.
int mp3_dec_get_layer(char *p_workmem);

// return channel.
int mp3_dec_get_channel(char *p_workmem);

// return bitrate in kbps.
int mp3_dec_get_bitrate(char *p_workmem);

// return sampling rate in Hz.
int mp3_dec_get_samplerate(char *p_workmem);

// get library version
const unsigned char * mp3_dec_get_version(void);

// set EQ
void mp3_dec_set_EQ(unsigned char *mp3dec_workmem, unsigned short *EQ_table);


// set volume gain, Q.14 (1 = 0x4000)
extern void mp3_dec_set_volume(char *mp3dec_workmem, unsigned short vol);


// set bit-stream buffer start addr
void mp3_dec_set_bs_ptr(char* mp3dec_workmem, unsigned char *ptr);


void mp3_dec_set_ring_buf_size(char *mp3dec_workmem, unsigned int size);


#endif  // __mp3_dec_h__

