/**************************************************************************
 *                                                                        *
 *         Copyright (c) 2004 by Sunplus Technology Co., Ltd.             *
 *                                                                        *
 *  This software is copyrighted by and is the property of Sunplus        *
 *  Technology Co., Ltd. All rights are reserved by Sunplus Technology    *
 *  Co., Ltd. This software may only be used in accordance with the       *
 *  corresponding license agreement. Any unauthorized use, duplication,   *
 *  distribution, or disclosure of this software is expressly forbidden.  *
 *                                                                        *
 *  This Copyright notice MUST not be removed or modified without prior   *
 *  written consent of Sunplus Technology Co., Ltd.                       *
 *                                                                        *
 *  Sunplus Technology Co., Ltd. reserves the right to modify this        *
 *  software without notice.                                              *
 *                                                                        *
 *  Sunplus Technology Co., Ltd.                                          *
 *  19, Innovation First Road, Science-Based Industrial Park,             *
 *  Hsin-Chu, Taiwan, R.O.C.                                              *
 *                                                                        *
 *                                                                        *
 **************************************************************************/
#ifndef __BIT_OPERATION_H__
#define __BIT_OPERATION_H__



/******************************************************************
                         C O N S T A N T S                            
******************************************************************/

#define NEG_SSR32(a,s) (((SINT32)(a))>>(32-(s)))
#define NEG_USR32(a,s) (((UINT32)(a))>>(32-(s)))

#define FFMAX(a,b) ((a) > (b) ? (a) : (b))
#define FFMIN(a,b) ((a) > (b) ? (b) : (a))

#define AV_RB16(x)  ((((UINT8*)(x))[0] << 8) | ((UINT8*)(x))[1])
#define AV_RB32(x)  ((((UINT8*)(x))[0] << 24) | \
	                 (((UINT8*)(x))[1] << 16) | \
	                 (((UINT8*)(x))[2] <<  8) | \
                      ((UINT8*)(x))[3])

#define OPEN_READER(name, gb)\
	    SINT32 name##_index= (gb)->index;\
        SINT32 name##_cache= 0;\

#define UPDATE_CACHE(name, gb)\
        name##_cache= AV_RB32( ((const UINT8 *)(gb)->buffer)+(name##_index>>3) ) << (name##_index&0x07);\

#define SHOW_UBITS(name, gb, num)     NEG_USR32(name##_cache, num)

#define SKIP_COUNTER(name, gb, num)	  name##_index += (num);	
		
#define LAST_SKIP_BITS(name, gb, num) SKIP_COUNTER(name, gb, num)

#define CLOSE_READER(name, gb)        (gb)->index= name##_index;
/******************************************************************
              S T R U C T U R E   D E C L A R A T I O N                
******************************************************************/

/* bit input */
/* buffer, buffer_end and size_in_bits must be present and used by every reader */
typedef struct GetBitContext_s
{
	const UINT8 *buffer;
	const UINT8 *buffer_end;
	SINT32 index;
	SINT32 size_in_bits;
} GetBitContext_t;
/******************************************************************
           V A R I A B L E   D E C L A R A T I O N S 
******************************************************************/



/******************************************************************
        F U N C T I O N    D E C L A R A T I O N S
******************************************************************/

UINT16 read12(void *, UINT32);
UINT16 read16(void *);
UINT32 LREAD32(void *);
UINT32 BREAD32(void *);
UINT64 read64(void *);
void   write12(void *, UINT16, UINT32);
void   write16(void *, UINT16);
void   write32(void *, UINT32);
void   write64(void *, UINT64);

void   lwrite(void *, UINT32, UINT32);
UINT32 lread(void *, UINT32);

UINT32 parser_get_bits(GetBitContext_t *s, SINT32 n);
UINT32 parser_get_bits1(GetBitContext_t *s);
void   parser_skip_bits(GetBitContext_t *s, int n);
void   init_get_bits(GetBitContext_t *s,
						  const UINT8 *buffer, SINT32 bit_size);
SINT32  get_bits_count(GetBitContext_t *s);
void   align_get_bits(GetBitContext_t *s);

#endif

