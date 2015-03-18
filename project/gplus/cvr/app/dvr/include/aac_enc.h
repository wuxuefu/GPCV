/***************************************************************************
 * Name: aacplus_dec.h
 *
 * Purpose:
 *
 * Developer:
 *     zhoulu, 2010-10-11
 *
 * Copyright (c) 2010-2011 by Sunplus mMobile Inc.
 ***************************************************************************/
#ifndef __AAC_ENC_H__
#define __AAC_ENC_H__
	 
/***************************************************************************
 * Header Files
 ***************************************************************************/

/***************************************************************************
 * Constants
 ***************************************************************************/
#define AAC_INPUT_FRAME_LEN			1024	// samples per channel
#define AAC_OUTPUT_FRAME_MAX_LEN	800     // bytes per channel
#define AUD_AAC_SAMPLE_TIMES           1

SINT32 aac_encoder_instance_size(void);


// bit_rate: in "bits per second"
// sample_rate: in "Hz"
// format : 0(ADTS), 1(ADIF) or 2(RAW)
SINT32 aac_encoder_init(void *pWorkMem,
					 UINT32 bit_rate, UINT32 sample_rate, UINT32 nchannels,
					 UINT32 format, char *ExtData, SINT32 *ExtDataLen);

// PCM: input buffer
// BS: output buffer
// cbBSLen: the length of output buffer (count in byte)
SINT32 aac_encoder_run(void *pWorkMem, const short *PCM,
					UINT8 *BS, SINT32 cbBSLen);
					
SINT32 aac_encoder_uninit(void *pWorkMem);

const char *aac_encoder_version_string(void);


#endif  // __AAC_ENC_H__

