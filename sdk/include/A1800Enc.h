#ifndef __A1800ENC_H__
#define __A1800ENC_H__



#define	A18_ENC_FRAMESIZE			320		// input pcm size per frame
#define	A18_ENC_MEMORY_SIZE			5784//660		// 20 + 320*2

#define A18_ENC_NO_ERROR			0
/*#define A18_E_NO_MORE_SRCDATA		0x80000000
#define A18_E_READ_IN_BUFFER		0x80000001	
#define A18_CODE_FILE_FORMAT_ERR	0x80000002
#define A18_E_FILE_END				0x80000003*/
#define A18_E_MODE_ERR				0x80000004

extern int A18_enc_run(unsigned char *p_workmem, const short *p_pcmbuf, unsigned char *p_bsbuf);
extern int A18_enc_init(unsigned char *p_workmem);
//extern int A18_enc_get_ri(char *A18enc_workmem);
//extern int A18_enc_get_channel(char *A18enc_workmem);
//extern int A18_enc_get_samplerate(char *A18enc_workmem);
extern int A18_enc_get_BitRate(unsigned char *p_workmem);
extern int A18_enc_get_PackageSize(unsigned char *p_workmem);
//extern int A18_enc_get_bitspersample(char *A18enc_workmem);
extern int A18_enc_get_errno(char *A18enc_workmem);
extern const char* A18_enc_get_version(void);
extern int A18_enc_get_mem_block_size(void);
extern void A18_enc_set_BitRate(unsigned char *p_workmem, int BitRate);

#endif //!__A1800ENC_H__
