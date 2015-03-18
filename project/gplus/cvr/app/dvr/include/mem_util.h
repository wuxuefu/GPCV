/**************************************************************************
 *                                                                        *
 *         Copyright (c) 2002 by Sunplus Technology Co., Ltd.             *
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
 *  Author: Jenghung Luo                                                  *
 *                                                                        *
 **************************************************************************/
#ifndef  _MEM_UTIL_H_
#define _MEM_UTIL_H_

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/
UINT16 read12(void *, UINT32);
UINT16 read16(void *);
UINT32 read32(void *);
UINT64 read64(void *);
void   write12(void *, UINT16, UINT32);
void   write16(void *, UINT16);
void   write32(void *, UINT32);
void   write64(void *, UINT64);

void   lwrite(void *, UINT32, UINT32);
UINT32 lread(void *, UINT32);

void   yuv420to422Conv(UINT32, UINT32, UINT32);
UINT32 bitsRead(UINT8 *, UINT32, UINT32);
void   bitsWrite(UINT8 *, UINT32, UINT32, UINT32);

UINT32	bitsShow(UINT32, UINT32);
void	bitsFlush(UINT8 **, UINT32 *, UINT32 *, UINT32 *, UINT32);
void memDmaCpy(UINT8 *, UINT8 *, UINT32);
/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
#define FILL_AND_MOVE(ptr, val, type) \
	*(type *)(ptr) = (type)(val); \
	(ptr) = (ptr) + sizeof(type);

#define MEMCPY_AND_MOVE(dst, src, size) \
	memcpy((dst), (src), (size)); \
	(dst) = (dst) + size;
#define MEMSET_AND_MOVE(dst, val, size) \
	memset((dst), (val), (size)); \
	(dst) = (dst) + size;

#define WRITE16_AND_MOVE(ptr, val) \
	write16((ptr), (val)); (ptr) = (ptr) + 2;

#define WRITE32_AND_MOVE(ptr, val) \
	write32((ptr), (val)); (ptr) = (ptr) + 4;

#define WRITE64_AND_MOVE(ptr, val) \
	write64((ptr), (val)); (ptr) = (ptr) + 8;

#define PHY_WADDR_TO_NONCACHE_VIR_BADDR(phyAddr) ((UINT32 )((UINT32 )(phyAddr) << 1) | 0xac000000)

#define LWRITE32_AND_MOVE(ptr, val) \
	lwrite((ptr), (val), (4)); (ptr) = (ptr) + 4;

#define LWRITE24_AND_MOVE(ptr, val) \
	lwrite((ptr), (val), (3)); (ptr) = (ptr) + 3;

#define LWRITE16_AND_MOVE(ptr, val) \
	lwrite((ptr), (val), (2)); (ptr) = (ptr) + 2;

#define LREAD32(ptr) \
	lread((ptr), (4));

#define LREAD24(ptr) \
	lread((ptr), (3));

#define LREAD16(ptr) \
	lread((ptr), (2));

#endif /* _MEM_UTIL_H_ */
 
