#ifndef __mp3_dec_h__
#define __mp3_dec_h__

/////////////////////////////////////////////////////////////////////////////
//		Constant Definition
/////////////////////////////////////////////////////////////////////////////
#define MP3_DEC_FRAMESIZE					1152	// ???
#define MP3_DEC_BITSTREAM_BUFFER_SIZE   	2048    // size in bytes
#define MP3_DEC_MEMORY_SIZE 				14020	//(20632-8)	// 18456



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
#define MP3_DEC_ERR_BADMPEGID		0x80000106	//for error mpegid add by zgq on 20080508

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

// MP3 Decoder Version
// @return  return version of mp3 decoder library
const char * mp3_dec_get_version(void);

// MP3 Decoder Initial
// @param  *p_workmem: pointer to working memory
// @param  *p_bsbuf: pointer to bitstream buffer
// @return  return 0 (success), others(fail)
int mp3_dec_init(void *p_workmem, void *p_bsbuf);

// tell the size of bitstream buffer to decoder
// @param   *p_workmem: pointer to working memory
// @param   size: size of bitstream
// @return  return size of bitstream buffer, it must be the mutiple of 512
int mp3_dec_set_ring_size(void *p_workmem, int size);

// MP3 header parsing
// @param   *p_workmem: pointer to working memory
// @param   wi: write index of bitstream buffer
// @return  return error number
int mp3_dec_parsing(void *p_workmem, int wi);

// MP3 Decoder
// @param   *p_workmem: pointer to working memory
// @param   *p_pcmbuf: pointer to PCM buffer
// @param   wi: write index of bitstream buffer
// @param   granule: if set(1), it will output the last granule from decoder
// @return  return the number of sample. if return 0, decoder need more bitstream
int mp3_dec_run(void *p_workmem, short *p_pcmbuf, int wi, int granule);

// Get Read Index
// @param  *p_workmem: pointer to working memory
// @return  return read index of bitstream buffer
int mp3_dec_get_ri(void *p_workmem);



// Get mpeg id
// @param   *p_workmem: pointer to working memory
// @return  return MPEG id: 1 or 2
const char *mp3_dec_get_mpegid(void *p_workmem);


// Get the size of MP3 Decoder Working memory
// @return   return size of MP3 Decoder Working memory
int mp3_dec_get_mem_block_size (void);


// get error number.
// @param   *p_workmem: pointer to working memory
// @return  return error number 
int mp3_dec_get_errno(void *p_workmem);

// get layer.
// @param   *p_workmem: pointer to working memory
// @return  return number of layer
int mp3_dec_get_layer(void *p_workmem);

// get channel.
// @param   *p_workmem: pointer to working memory
// @return  return 1(mono), 2(stereo)
int mp3_dec_get_channel(void *p_workmem);

// get bitrate in kbps.
// @param   *p_workmem: pointer to working memory
// @return  return bitrate in kbps
int mp3_dec_get_bitrate(void *p_workmem);

// get sampling rate in Hz.
// @param   *p_workmem: pointer to working memory
// @return  return sampling rate in Hz
int mp3_dec_get_samplerate(void *p_workmem);

// to check the end of decoder (it's not really good)
// @param   *p_workmem: pointer to working memory
// @param   wi: write index of bitstream buffer
// @return  return 1(end), 0(not end)
int mp3_dec_end(void *p_workmem, int wi);

// set EQ table
// @param   *p_workmem: pointer to working memory
// @param   *eqtable: pointer to EQ Table (short EQ_Tab[12])
void mp3_dec_set_eq_table(void *p_workmem, short *eqtable);

// set EQ band out
// @param   *p_workmem: pointer to working memory
// @param   *band: pointer to EQ Band (short EQ_Band[32])
void mp3_dec_set_band_addr(void *p_workmem, short *band);


#endif  // __mp3_dec_h__

