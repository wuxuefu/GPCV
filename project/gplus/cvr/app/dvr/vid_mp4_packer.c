/**************************************************************************
 *                                                                        *
 *         Copyright (c) 2006 by Sunplus Technology Co., Ltd.             *
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
 *  Author: Yi-Ming Kao                                                   *
 *                                                                        *
 *  Reviewer:                                                             *
 *                                                                        *
 **************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "ceva.h"
#include "dv_used.h"
#include "gp_avcodec.h"
#include "mem_util.h"
#include "vid_mp4_packer.h"
#include "AAC_enc.h"
#include <chunkmem.h>

 /**************************************************************************
 *                   G E N E R A L    C O N S T A N T S                   *
 **************************************************************************/
#define PACKER_MP4_MAJOR_VER   1
#define PACKER_MP4_MINOR_VER   0
#define PACKER_MP4_SERIAL_VER  1


#define VID_PACKER_PKT_SIZE         (1024 * 32)

#define VID_STTS_HEADER_SIZE        16
#define VID_STSZ_HEADER_SIZE        20
#define VID_STCO_HEADER_SIZE        16
#define VID_STSS_HEADER_SIZE        16

#define AUD_STTS_HEADER_SIZE        16
#define AUD_STSZ_HEADER_SIZE		20
#define AUD_STCO_HEADER_SIZE		16

#ifdef ONLY_ONE_HOUR_REC
#define BASE_SIZE                   (1024 * 512)
#else
#define BASE_SIZE                   (1024 * 32)
#endif

#define VID_STTS_BUF_SIZE           (BASE_SIZE * 2)
#define VID_STSZ_BUF_SIZE           BASE_SIZE
#define VID_STCO_BUF_SIZE           BASE_SIZE
#define VID_STSS_BUF_SIZE           BASE_SIZE
#define AUD_STCO_BUF_SIZE           (BASE_SIZE * 4)
#define AUD_STSZ_BUF_SIZE           (BASE_SIZE * 4)

#define ATOM_BUF_SIZE               VID_STTS_BUF_SIZE +	\
									VID_STSZ_BUF_SIZE + \
									VID_STCO_BUF_SIZE + \
									VID_STSS_BUF_SIZE + \
									AUD_STCO_BUF_SIZE + \
									AUD_STSZ_BUF_SIZE

#define ATOM_FILE_NAME              "IDX00001.TMP"
#define ATOM_FILE_NAME_LEN          15

#define HEADER_BUF_SIZE             1024
#define RESERVED_SPACE              (1024 * 256) 
#define FREE_BOX_BUF_SIZE           1024

#define VID_I_FRM                   0

// for H264 NAL
// NALU_PRIORITY
#define NALU_PRIO_DISPOSE       (0)
#define NALU_PRIO_LOW           (1)
#define NALU_PRIO_HIGH          (2)
#define NALU_PRIO_HIGHEST       (3)
// NALU_TYPE
#define NALU_TYPE_SPS           (7)
#define NALU_TYPE_PPS           (8)
#define NALU_TYPE_IDR           (5)
#define NALU_TYPE_SLC           (1)
// SLICE_TYPE
#define SLICE_P                 (0)
#define SLICE_B                 (1)
#define SLICE_I                 (2)
// NALU_HEADER
#define NALU_SPS                (NALU_TYPE_SPS|(NALU_PRIO_HIGHEST<<5))
#define NALU_PPS                (NALU_TYPE_PPS|(NALU_PRIO_HIGHEST<<5))
#define NALU_IDR                (NALU_TYPE_IDR|(NALU_PRIO_HIGHEST<<5))
#define NALU_SLC                (NALU_TYPE_SLC|(NALU_PRIO_HIGHEST<<5))
#define NALU_SLICE_IDR          ((NALU_IDR|(SLICE_I<<8))&255)
#define NALU_SLICE_I            ((NALU_SLC|(SLICE_I<<8))&255)
#define NALU_SLICE_P            ((NALU_SLC|(SLICE_P<<8))&255)
/**************************************************************************
 *                             M A C R O S                                *
 **************************************************************************/

#ifdef USE_SYSTEM_FS
#define fread(buf, size, count, fd)  read((SINT32)(fd), buf, (size)*(count))
#define fwrite(buf, size, count, fd) write((SINT32)(fd), buf, (size)*(count))
#define fseek(fd, off, pos)          lseek((SINT32)(fd), off, pos)
#define ftell(fd)                    lseek((SINT32)(fd), 0, SEEK_CUR)
#define fclose(fd)                   close((SINT32)(fd))    
#endif


/**************************************************************************
 *                         D A T A   T Y P E S                            *
 **************************************************************************/


/**************************************************************************
 *                        G L O B A L   D A T A                           *
 **************************************************************************/
struct gpMux_s MP4_packer={
	.init= NULL,
	.uninit =NULL,
	.open=muxMP4Open,
	.close = muxMP4Close,
	.pack = muxMP4Pack,
	.set = muxMP4Set,
	.get =muxMP4Get,
};

struct gpMux_s MOV_packer={
	.init= NULL,
	.uninit =NULL,
	.open=muxMOVOpen,
	.close = muxMP4Close,
	.pack = muxMP4Pack,
	.set = muxMP4Set,
	.get =muxMP4Get,
};

/**************************************************************************
 *                  E X T E R N A L    R E F E R E N C E                  *
 **************************************************************************/



/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/

static UINT32 vidMP4HeaderInit(MP4MuxInfo_t *gpPackBitsInfo);
static UINT16 vidMP4PacketAdd(MP4MuxInfo_t *gpPackBitsInfo, FILE *fd,UINT8 *pdata,UINT32 size);
static UINT16 vidMP4PacketFlush(MP4MuxInfo_t *gpPackBitsInfo, FILE *fd, UINT32* pSize);
static UINT16 vidMP4IdxFileDel(MP4MuxInfo_t *gpPackBitsInfo);
static UINT16 vidMP4IdxInfoAdd(MP4MuxInfo_t *gpPackBitsInfo, UINT32 n);
static UINT16 vidMP4VidInfoProc(MP4MuxInfo_t *gpPackBitsInfo, UINT32 sampleTime, UINT32 sampleSize, UINT32 sampleOffset, UINT8  frmType);
static UINT16 vidMP4AudInfoProc(MP4MuxInfo_t *gpPackBitsInfo, UINT32 sampleTime, UINT32 sampleSize, UINT32 sampleOffset);
static UINT32 vidMP4BoxAdd(UINT8* pBuf, UINT32 size, char *pStr);
static UINT32 vidMP4FullBoxAdd(UINT8 *pbuf, UINT32 size, char *pstr, UINT8 version, UINT32 flags);
static UINT32 vidMP4MVHDBoxAdd(MP4MuxInfo_t *gpPackBitsInfo, UINT8 *pbuf);
static UINT32 vidMP4TKHDBoxAdd(MP4MuxInfo_t *gpPackBitsInfo, UINT8 *pbuf, UINT32 trakID);
static UINT32 vidMP4MDHDBoxAdd(MP4MuxInfo_t *gpPackBitsInfo, UINT8 *pbuf, UINT32 trakID);
static UINT32 vidMP4HDLRBoxAdd(MP4MuxInfo_t *gpPackBitsInfo, UINT8 *pbuf, UINT32 trakID);
static UINT32 vidMP4VMHDBoxAdd(UINT8 *pbuf);
static UINT32 vidMP4SMHDBoxAdd(UINT8 *pbuf);
static UINT32 vidMP4DREFBoxAdd(UINT8 *pbuf);
static UINT16 vidMP4FreeBoxAdd(MP4MuxInfo_t *gpPackBitsInfo, FILE *fd, UINT32 size);
static FILE *vidMP4IdxBufLeftAdd(MP4MuxInfo_t *gpPackBitsInfo, UINT32 idxItem);
static UINT16 vidMP4ClusterAlign(MP4MuxInfo_t *gpPackBitsInfo, FILE *fd, UINT32* pSize);
static UINT16 vidMP4IdxInfoUpdate(MP4MuxInfo_t *gpPackBitsInfo, UINT8 *pdata, UINT32 dataSize, UINT32 indexItem, UINT32* pSize);
static UINT32 vidMP4STSDBoxAdd(MP4MuxInfo_t *gpPackBitsInfo, UINT8 *pbuf, UINT32 trakID);
static UINT32 vidMP4STSCBoxAdd(UINT8 *pbuf, UINT32 trakID);
static UINT16 vidMP4STTSBoxAdd(MP4MuxInfo_t *gpPackBitsInfo, FILE *fd, UINT32 trakID, UINT32* pSize);
static UINT16 vidMP4STSZBoxAdd(MP4MuxInfo_t *gpPackBitsInfo, FILE *fd, UINT32 trakID, UINT32* pSize);
static UINT16 vidMP4STCOBoxAdd(MP4MuxInfo_t *gpPackBitsInfo, FILE *fd, UINT32 trakID, UINT32 *pSize);
static UINT16 vidMP4STSSBoxAdd(MP4MuxInfo_t *gpPackBitsInfo, FILE *fd, UINT32 *pSize);
static UINT16 vidMP4HeaderSizeUpdate(MP4MuxInfo_t *gpPackBitsInfo, FILE *fd,  UINT32 offset,  UINT32 size);
static UINT16 vidMP4HeaderUpdate(MP4MuxInfo_t *gpPackBitsInfo, FILE *fd, UINT32 *pFileSize);
static UINT32 vidVoVolDataAdd(UINT8* pbuf,   MP4MuxInfo_t* pInfo);


UINT16 fsConcatefile(MP4MuxInfo_t *gpPackBitsInfo, FILE *dstHandle,FILE *srcHandle)
{
	SINT32 nSrcSize;
	SINT32 nReadSize;
	SINT32 nWriteSize;
	SINT32 nBlockSize;
	SINT32 nWriteBlock;	
	UINT8* pBuffer;		
	
	nBlockSize = gpPackBitsInfo->clusterSize;
	nWriteSize = 0;
	nSrcSize = 0;

	pBuffer = (UINT8 *)osMalloc(nBlockSize);
	if (pBuffer == NULL)
	{		
		return FAIL;
	}
	
	fseek(dstHandle, 0, SEEK_END);	
	fseek(srcHandle, 0, SEEK_END);
	nSrcSize = ftell(srcHandle);
	fseek(srcHandle, 0, SEEK_SET);
	
	while (nWriteSize < nSrcSize)
	{		
		nReadSize = fread(pBuffer, 1, nBlockSize, srcHandle);
		if (nReadSize != nBlockSize)
		{
			break;
		}
		nWriteSize += nReadSize;		
		
		nWriteBlock = fwrite(pBuffer, 1, nReadSize, dstHandle);
		if(nWriteBlock <=0 )
		{
			return FAIL;
		}		
	}	

	osFree(pBuffer);
	pBuffer = NULL;
	
	return SUCCESS;
}
/**************************************************************************

  Function Name: vidMP4HeaderInit

  Purposes: Initial the 3GP file header

  Limitations:

  Arguments: None

  Returns: The size of the header

  See also: vidMP4PackerInit

 **************************************************************************/
static UINT32
vidMP4HeaderInit(
	MP4MuxInfo_t *gpPackBitsInfo
)
{
	UINT8* pBuf;
	UINT32 size;
	UINT32 tmp;
	UINT32 ftypBoxSize = FTYP_BOX_SIZE;

	pBuf = gpPackBitsInfo->gpMP4PktBufCurr/*[gPackIdx]*/;
	/* ftyp box size */
	LWRITE32_AND_MOVE(pBuf, ftypBoxSize);
	/* ftyp token */
	MEMCPY_AND_MOVE(pBuf, "ftyp", 0x04);
	if (gpPackBitsInfo->mode == MODE_MOV)
	{
		/* major brand */
		MEMCPY_AND_MOVE(pBuf, "qt  ", 0x04);
		/* minor version */
		LWRITE32_AND_MOVE(pBuf, 0x200);
		/* compatible brands */
		MEMCPY_AND_MOVE(pBuf, "qt  ", 0x08);
	}
	else
	if(gpPackBitsInfo->vidType == VIDEO_TYPE_H264_BP) {	
		diag_printf("vidMP4HeaderInit H264\n");
		/* major brand: "avc1" */
		MEMCPY_AND_MOVE(pBuf, "avc1", 0x04);
		/* minor version : 0x00 0x00 0x00 0x00 */
		tmp = 0x0;
		MEMCPY_AND_MOVE(pBuf, &tmp, 0x04);
		/* compatible brands : "avc1isom" */
		MEMCPY_AND_MOVE(pBuf, "avc1isom", 0x08);	 
    }
    else {	
		diag_printf("vidMP4HeaderInit MP41\n");
		/* major brand: "mp41" */
		MEMCPY_AND_MOVE(pBuf, "mp41", 0x04);
		/* minor version : 0x00 0x00 0x00 0x00 */
		tmp = 0x0;
		MEMCPY_AND_MOVE(pBuf, &tmp, 0x04);
		/* compatible brands : "mp41isom" */
		MEMCPY_AND_MOVE(pBuf, "mp41isom", 0x08);	
    }
	/* mdat box size, fake size */
	LWRITE32_AND_MOVE(pBuf, 0x00);
	/* mdat token */
	MEMCPY_AND_MOVE(pBuf, "mdat", 0x04);   
	
	size = pBuf - gpPackBitsInfo->gpMP4PktBufCurr;
	gpPackBitsInfo->gpMP4PktBufCurr = pBuf;

	return size;
}



/**************************************************************************

  Function Name: vidMP4PacketAdd

  Purposes: Add data to packet. If the packet is full, it will be flushed
            into file.

  Limitations:

  Arguments:
    fd    [in] - the file descriptor
    pdata [in] - pointer to data to be written
    size  [in] - the size to be written

  Returns: SUCCESS or error code

  See also: vidMP4PacketFlush

 **************************************************************************/
static UINT16
vidMP4PacketAdd(
	MP4MuxInfo_t *gpPackBitsInfo,
	FILE  *fd,
	UINT8 *pdata,
	UINT32 size
)
{
	UINT8 *ptr;
	UINT32 nBytesRemain;
	UINT32 nBytesLeft;
	UINT32 tmpSize;	
	UINT32 writeSize;
	SINT32 bytesWrite;
	
	tmpSize = size;
	while (tmpSize > 0) 
	{
		if (tmpSize > VID_PACKER_PKT_SIZE)
		{
			writeSize = VID_PACKER_PKT_SIZE;
		}
		else 
		{
			writeSize = tmpSize;
		}
		nBytesRemain = gpPackBitsInfo->gpMP4PktBufEnd - gpPackBitsInfo->gpMP4PktBufCurr;
		nBytesLeft = writeSize;
		ptr = gpPackBitsInfo->gpMP4PktBufCurr;

		/* The remain size is smalller than the required size */
		if (nBytesRemain < nBytesLeft) 
		{
			if (nBytesRemain != 0) 
			{
				memcpy(ptr, pdata, nBytesRemain);//VIDEO_MEM_COPY

				pdata += nBytesRemain;
				nBytesLeft = nBytesLeft - nBytesRemain;
			}
			bytesWrite = fwrite(gpPackBitsInfo->gpMP4PktBufStart, 1, VID_PACKER_PKT_SIZE, fd);
			if (bytesWrite <= 0)
			{
				return FAIL;
			}

			ptr = gpPackBitsInfo->gpMP4PktBufStart;
		}

		memcpy(ptr, pdata, nBytesLeft);//VIDEO_MEM_COPY

		ptr += nBytesLeft;
		pdata += nBytesLeft;
		gpPackBitsInfo->gpMP4PktBufCurr = ptr;
		tmpSize -= writeSize;
	}

	return SUCCESS;
}


/**************************************************************************

  Function Name: vidMP4PacketFlush

  Purposes: Flush the data in memory into file.

  Limitations:

  Arguments:
    fd     [in] - file descriptor
    pSize [out] - size written

  Returns:

  See also: vidMP4PacketAdd

 **************************************************************************/
static UINT16
vidMP4PacketFlush(
	MP4MuxInfo_t *gpPackBitsInfo,
	FILE *fd,
	UINT32* pSize
)
{
	UINT32 size;
	SINT32 bytesWrite;

	size = gpPackBitsInfo->gpMP4PktBufCurr - gpPackBitsInfo->gpMP4PktBufStart;
	if ( size != 0 )
	{		
		bytesWrite = fwrite((char *)gpPackBitsInfo->gpMP4PktBufStart, 1, size, fd);
		if ( bytesWrite <= 0) 
		{
			return FAIL;
		}
		gpPackBitsInfo->gpMP4PktBufCurr = gpPackBitsInfo->gpMP4PktBufStart;
	}
	*pSize = size;

	return SUCCESS;
}


/**************************************************************************

  Function Name: vidMP4IdxFileDel

  Purposes: Delete the index file

  Limitations:

  Arguments: NONE

  Returns: SUCCESS

  See also: vidMP4IdxInfoAdd

 **************************************************************************/
static UINT16
vidMP4IdxFileDel(
	MP4MuxInfo_t *gpPackBitsInfo
)
{
	UINT32 j;
	UINT8 tmpPath[LONG_FILENAME_LEN];
	UINT8 tmpName[ATOM_FILE_NAME_LEN];

	memset(tmpName, 0, sizeof(tmpName));
	strcpy((char *)tmpName, ATOM_FILE_NAME);
	tmpName[12] = '\0';

	for ( j = 0; j < IDX_NUM; j++ ) 
	{
		if ( gpPackBitsInfo->gbIdxFd[j] != 0 ) 
		{
			tmpName[7] = (UINT8)(j + 48);
			memset(tmpPath, 0, LONG_FILENAME_LEN);
			strcpy((char *)tmpPath, (char *)gpPackBitsInfo->tempFile);
			strcat((char *)tmpPath, (char *)tmpName);
			remove((char *)tmpPath);
		}
	}

	return SUCCESS;
}



/**************************************************************************

  Function Name: vidMP4IdxInfoAdd

  Purposes: Add the index information of 3GP

  Limitations:

  Arguments:
    n [in] - the index file number (IDX_VID_STTS, etc)

  Returns: SUCCESS or error code

  See also: vid3GPVidInfoProc, vidMP4AudInfoProc

 **************************************************************************/
static UINT16
vidMP4IdxInfoAdd(
	MP4MuxInfo_t *gpPackBitsInfo,
	UINT32 n
)
{
	UINT8 tmpPath[LONG_FILENAME_LEN];
	UINT8 tmpName[ATOM_FILE_NAME_LEN];
	UINT8 *pbuf;
	FILE  *iTempFd;
	UINT32 size, sizeWrite;
	SINT32  err;


	if (gpPackBitsInfo->gbReserved == 0) 
	{
		gpPackBitsInfo->gEstSize += RESERVED_SPACE;
		gpPackBitsInfo->gbReserved = 1;
	}

	memset(tmpName, 0, sizeof(tmpName));
	strcpy((char *)tmpName, ATOM_FILE_NAME);
	tmpName[12] = '\0';
	tmpName[7] = (UINT8)(n + 48);

	memset(tmpPath, 0, LONG_FILENAME_LEN);
	strcpy((char *)tmpPath, (char *)gpPackBitsInfo->tempFile);
	strcat((char *)tmpPath, (char *)tmpName);

	/* create file */
	if ( gpPackBitsInfo->gbIdxFd[n] == 0 )
	{
		diag_printf("1 Create: %s\n", tmpPath);
        #ifdef USE_SYSTEM_FS
        iTempFd = (FILE *)open((char *)tmpPath,   O_CREAT | O_RDWR);
        if (iTempFd == (FILE *)(-1)) 
        {
			vidMP4IdxFileDel(gpPackBitsInfo);
			return FAIL;
		}
        #else
        iTempFd = (FILE *)fopen((char *)tmpPath, "wb");
        if (iTempFd == NULL)
		{
			vidMP4IdxFileDel(gpPackBitsInfo);
			return FAIL;
		}
        #endif
        
		fclose(iTempFd);
	}
	diag_printf("Open: %s\n", tmpPath);
    #ifdef USE_SYSTEM_FS
    iTempFd = (FILE *)open((char *)tmpPath,    O_APPEND | O_RDWR);
    if (iTempFd == (FILE *)(-1)) 
    {
        vidMP4IdxFileDel(gpPackBitsInfo);
        return FAIL;
    }
    #else
    iTempFd = fopen((char *)tmpPath, "w+b");
    if (iTempFd == NULL)
    {
        vidMP4IdxFileDel(gpPackBitsInfo);
        return FAIL;
    }
    #endif
    
	/* Update this variable to avoid delete recorded data */
	gpPackBitsInfo->gbIdxFd[n] = 1;

	/* Update file */
	pbuf = gpPackBitsInfo->gpMP4IdxBufStart[n];
	size = gpPackBitsInfo->gpMP4IdxBufCurr[n] - gpPackBitsInfo->gpMP4IdxBufStart[n];	
	err = fseek(iTempFd, 0, SEEK_END);
	if (err == -1)
	{
		return FAIL; 
	}
	diag_printf("Write size: %d\n", size);	
	sizeWrite = fwrite(pbuf, 1, size, iTempFd);
	if (sizeWrite <= 0)
	{
		fclose(iTempFd);
		vidMP4IdxFileDel(gpPackBitsInfo);
		return FAIL;
	}
	diag_printf("End write\n");
	fclose(iTempFd);

	gpPackBitsInfo->gpMP4IdxBufCurr[n] = gpPackBitsInfo->gpMP4IdxBufStart[n];

	return SUCCESS;
}


/**************************************************************************

  Function Name: vidMP4VidInfoProc

  Purposes: Process all the video related index information

  Limitations:

  Arguments:
    sampleTime   [in] - the time between previous sample and current one
    sampleSize   [in] - the size of this sample
    sampleOffset [in] - the file offset of this sample
    frmType      [in] - the frame type of this sample

  Returns: SUCCESS or error code

  See also: vidMP4AudInfoProc

 **************************************************************************/
static UINT16
vidMP4VidInfoProc(
	MP4MuxInfo_t *gpPackBitsInfo,
	UINT32 sampleTime,
	UINT32 sampleSize,
	UINT32 sampleOffset,
	UINT8  frmType
)
{
	UINT32 idx;
	UINT32 tmpValue;
	UINT32 data[4];
	UINT16 err;

	idx = IDX_VID_STTS;
	if (gpPackBitsInfo->gPrevSampleTime == 0) 
	{
		LWRITE32_AND_MOVE(gpPackBitsInfo->gpMP4IdxBufCurr[idx], 1);
		LWRITE32_AND_MOVE(gpPackBitsInfo->gpMP4IdxBufCurr[idx], sampleTime);
		gpPackBitsInfo->gPrevSampleTime = sampleTime;
		gpPackBitsInfo->gEstSize += 8;
		gpPackBitsInfo->gSTTSEntryCnt = 1;
	}
	else 
	{
		if (gpPackBitsInfo->gPrevSampleTime != sampleTime)
		{			
			if (gpPackBitsInfo->gpMP4IdxBufCurr[idx] == gpPackBitsInfo->gpMP4IdxBufEnd[idx]) 
			{
				err = vidMP4IdxInfoAdd(gpPackBitsInfo, idx);
				if ( err != SUCCESS ) 
				{
					return err;
				}
			}
			LWRITE32_AND_MOVE(gpPackBitsInfo->gpMP4IdxBufCurr[idx], 1);
			LWRITE32_AND_MOVE(gpPackBitsInfo->gpMP4IdxBufCurr[idx], sampleTime);
			gpPackBitsInfo->gEstSize += 8;
			gpPackBitsInfo->gSTTSEntryCnt++;
			gpPackBitsInfo->gPrevSampleTime = sampleTime;
		}
		else 
		{
			gpPackBitsInfo->gpMP4IdxBufCurr[idx] -= 8;
			tmpValue = LREAD32(gpPackBitsInfo->gpMP4IdxBufCurr[idx]);
			LWRITE32_AND_MOVE(gpPackBitsInfo->gpMP4IdxBufCurr[idx],  tmpValue + 1);
			gpPackBitsInfo->gpMP4IdxBufCurr[idx] += 4;
		}
	}

	data[IDX_VID_STSZ] = sampleSize;
	data[IDX_VID_STCO] = sampleOffset;
	for (idx = IDX_VID_STSZ; idx <= IDX_VID_STSS; idx++) 
	{		
		if (gpPackBitsInfo->gpMP4IdxBufCurr[idx] == gpPackBitsInfo->gpMP4IdxBufEnd[idx])
		{			
			err = vidMP4IdxInfoAdd(gpPackBitsInfo, idx);
			if ( err != SUCCESS )
			{
				return err;
			}
		}
		if (idx != IDX_VID_STSS)
		{
			LWRITE32_AND_MOVE(gpPackBitsInfo->gpMP4IdxBufCurr[idx], data[idx]);
			gpPackBitsInfo->gEstSize += 4;
		}
		else 
		{
			if (frmType == VID_I_FRM)
			{
				LWRITE32_AND_MOVE(gpPackBitsInfo->gpMP4IdxBufCurr[IDX_VID_STSS], gpPackBitsInfo->gVidFrmNo + 1);
		 		gpPackBitsInfo->gEstSize += 4;
				gpPackBitsInfo->gVidIFrmNo++;
			}
		}
	}
	return SUCCESS;
}


/**************************************************************************

  Function Name: vidMP4AudInfoProc

  Purposes: Process all the audio related index information

  Limitations:

  Arguments:
    sampleTime   [in] - the time between previous sample and current one
    sampleSize   [in] - the size of this sample
    sampleOffset [in] - the file offset of this sample

  Returns: SUCCESS or error code

  See also: vid3GPVidInfoProc

 **************************************************************************/
static UINT16
vidMP4AudInfoProc(
	MP4MuxInfo_t *gpPackBitsInfo,
	UINT32 sampleTime,
	UINT32 sampleSize,
	UINT32 sampleOffset
)
{
	UINT32 indexItem;
	UINT16 err;
	UINT32 data[9];

	data[IDX_AUD_STSZ] = sampleSize;
	data[IDX_AUD_STCO] = sampleOffset;

	for (indexItem = IDX_AUD_STSZ; indexItem <= IDX_AUD_STCO; indexItem++)
	{
		if ( gpPackBitsInfo->gpMP4IdxBufCurr[indexItem] == gpPackBitsInfo->gpMP4IdxBufEnd[indexItem])
		{
			err = vidMP4IdxInfoAdd(gpPackBitsInfo, indexItem);
			if ( err != SUCCESS ) 
			{
				return err;
			}
		}
		LWRITE32_AND_MOVE( gpPackBitsInfo->gpMP4IdxBufCurr[indexItem], data[indexItem] );
		gpPackBitsInfo->gEstSize += 4;
	}
	return SUCCESS;
}


/**************************************************************************

  Function Name: vidMP4BoxAdd

  Purposes: Add a simple box of 3GP file

  Limitations:

  Arguments:
    pBuf [out] - pointer to free buffer to be written
    size  [in] - the size of this box
    pStr  [in] - pointer to the name of this box

  Returns: the size this box takes

  See also: vidMP4FullBoxAdd

 **************************************************************************/
static UINT32
vidMP4BoxAdd(
	UINT8* pBuf,
	UINT32 size,
	char * pStr
)
{
	LWRITE32_AND_MOVE( pBuf, size );
	MEMCPY_AND_MOVE( pBuf, pStr, 0x04 );

	return 8;
}


/**************************************************************************

  Function Name: vidMP4FullBoxAdd

  Purposes: Add a full box of 3GP file

  Limitations:

  Arguments:
    pBuf   [out] - pointer to free buffer to be written
    size    [in] - the size of this box
    pStr    [in] - pointer to the name of this box
    version [in] - version of this box
    flags   [in] - special flags of this box

  Returns: the size this box takes

  See also: vidMP4BoxAdd

 **************************************************************************/
static UINT32
vidMP4FullBoxAdd(
	UINT8 *pbuf,
	UINT32 size,
	char  *pstr,
	UINT8 version,
	UINT32 flags
)
{
	UINT32 ret;

	ret = vidMP4BoxAdd( pbuf, size, pstr );
	pbuf += ret;

	/* version: 0 */
	*pbuf++ = version;
	/* flags */
	LWRITE24_AND_MOVE( pbuf, flags );

	return 12;
}


/**************************************************************************

  Function Name: vidMP4MVHDBoxAdd

  Purposes: Add the "mvhd" header of a 3GP file.

  Limitations:

  Arguments:
    pbuf [in] - pointer to free buffer to be written.

  Returns: the size this box takes.

  See also: vid3GPTKHDBoxAdd

 **************************************************************************/
static UINT32
vidMP4MVHDBoxAdd(
	MP4MuxInfo_t *gpPackBitsInfo,
	UINT8 *pbuf
)
{
	UINT32 ret;
	UINT32 size;
	UINT32 duration;

	size = 0x6C;
	ret = vidMP4FullBoxAdd(pbuf, size, "mvhd", 0x00, 0x00);
	pbuf += ret;
	/* creation time : a fake value */
	LWRITE32_AND_MOVE( pbuf, 0xB9E3B1CE );
	/* modification time : a fake value */
	LWRITE32_AND_MOVE( pbuf, 0xB9E3B1CE );
	/* timescale : 1s = 1000ms, self-defined */
	LWRITE32_AND_MOVE( pbuf, 1000 );

	duration = gpPackBitsInfo->gPackVidDuration;

	LWRITE32_AND_MOVE( pbuf, duration );
	/* rate : 0x00010000 */
	LWRITE32_AND_MOVE( pbuf, 0x00010000 );
	/* volume : 0x0100 */
	LWRITE16_AND_MOVE( pbuf, 0x0100 );
	/* reserved */
	LWRITE16_AND_MOVE( pbuf, 0x00 );
	/* reserved */
	LWRITE32_AND_MOVE( pbuf, 0x00 );
	LWRITE32_AND_MOVE( pbuf, 0x00 );
	/* matrix : 0x00010000,0,0,0,0x00010000,0,0,0,0x40000000 */
	LWRITE32_AND_MOVE( pbuf, 0x00010000 );
	LWRITE32_AND_MOVE( pbuf, 0x00000000 );
	LWRITE32_AND_MOVE( pbuf, 0x00000000 );
	LWRITE32_AND_MOVE( pbuf, 0x00000000 );
	LWRITE32_AND_MOVE( pbuf, 0x00010000 );
	LWRITE32_AND_MOVE( pbuf, 0x00000000 );
	LWRITE32_AND_MOVE( pbuf, 0x00000000 );
	LWRITE32_AND_MOVE( pbuf, 0x00000000 );
	LWRITE32_AND_MOVE( pbuf, 0x40000000 );
	/* pre_defined */
	LWRITE32_AND_MOVE( pbuf, 0x00 );
	LWRITE32_AND_MOVE( pbuf, 0x00 );
	LWRITE32_AND_MOVE( pbuf, 0x00 );
	LWRITE32_AND_MOVE( pbuf, 0x00 );
	LWRITE32_AND_MOVE( pbuf, 0x00 );
	LWRITE32_AND_MOVE( pbuf, 0x00 );
	/* next_track_ID : 3, This value shall be larger than the largest track ID in use */
	LWRITE32_AND_MOVE( pbuf, 0x03 );

	return size;
}


/**************************************************************************

  Function Name: vidMP4TKHDBoxAdd

  Purposes: Add the "tkhd" header of a 3GP file.

  Limitations:

  Arguments:
    pbuf   [in] - pointer to free buffer to be written.
    trakID [in] - the track id of this track header box.

  Returns: the size this box takes.

  See also: vid3GPMVHDBoxAdd

 **************************************************************************/
static UINT32
vidMP4TKHDBoxAdd(
	MP4MuxInfo_t *gpPackBitsInfo,
	UINT8 *pbuf,
	UINT32 trakID
)
{
	UINT32 duration = 0;
	UINT32 size;
	UINT32 ret;
	UINT32 width, height;

	size = 0x5C;
	ret = vidMP4FullBoxAdd(pbuf, size, "tkhd", 0x00, 0x01);
	pbuf += ret;
	/* creation time : a fake value */
	LWRITE32_AND_MOVE( pbuf, 0xB9E3B1CE );
	/* modification time : a fake value */
	LWRITE32_AND_MOVE( pbuf, 0xB9E3B1CE );
	/* track ID */
	LWRITE32_AND_MOVE( pbuf, trakID );
	/* reserved */
	LWRITE32_AND_MOVE( pbuf, 0x00 );

	if ( trakID == VIDEO_TRAK )
	{
		duration = gpPackBitsInfo->gPackVidDuration;
	}
	else 
	{
		duration = gpPackBitsInfo->gPackAudDuration;
	}
	LWRITE32_AND_MOVE( pbuf, duration );
	/* reserved */
	LWRITE32_AND_MOVE( pbuf, 0x00 );
	/* reserved */
	LWRITE32_AND_MOVE( pbuf, 0x00 );
	/* layer */
	LWRITE16_AND_MOVE( pbuf, 0x00 );
	/* alternate group */
	LWRITE16_AND_MOVE( pbuf, 0x00 );
	/* volume */
	if ( trakID == VIDEO_TRAK ) 
	{
		LWRITE16_AND_MOVE( pbuf, 0x00 );
	}
	else 
	{
		LWRITE16_AND_MOVE( pbuf, 0x0100 );
	}
	/* reserved */
	LWRITE16_AND_MOVE( pbuf, 0x00 );
	/* matrix : 0x00010000,0,0,0,0x00010000,0,0,0,0x40000000 */
	LWRITE32_AND_MOVE( pbuf, 0x00010000 );
	LWRITE32_AND_MOVE( pbuf, 0x00000000 );
	LWRITE32_AND_MOVE( pbuf, 0x00000000 );
	LWRITE32_AND_MOVE( pbuf, 0x00000000 );
	LWRITE32_AND_MOVE( pbuf, 0x00010000 );
	LWRITE32_AND_MOVE( pbuf, 0x00000000 );
	LWRITE32_AND_MOVE( pbuf, 0x00000000 );
	LWRITE32_AND_MOVE( pbuf, 0x00000000 );
	LWRITE32_AND_MOVE( pbuf, 0x40000000 );

	if ( trakID == VIDEO_TRAK ) 
	{
		width = gpPackBitsInfo->width;
		height = gpPackBitsInfo->height;
		LWRITE32_AND_MOVE( pbuf, width << 16 );
		LWRITE32_AND_MOVE( pbuf, height << 16 );
	}
	else 
	{
		LWRITE32_AND_MOVE( pbuf, 0x00 );
		LWRITE32_AND_MOVE( pbuf, 0x00 );
	}

	return size;
}


/**************************************************************************

  Function Name: vidMP4MDHDBoxAdd

  Purposes: Add the "mdhd" header of a 3GP file.

  Limitations:

  Arguments:
    pbuf   [in] - pointer to free buffer to be written.
    trakID [in] - the track id of this box.

  Returns: the size this box takes.

  See also: vid3GPMVHDBoxAdd

 **************************************************************************/
static UINT32
vidMP4MDHDBoxAdd(
	MP4MuxInfo_t *gpPackBitsInfo,
	UINT8 *pbuf,
	UINT32 trakID
)
{
	UINT32 ret;
	UINT32 size;

	size = 0x20;
	ret = vidMP4FullBoxAdd(pbuf, size, "mdhd", 0x00, 0x00);
	pbuf += ret;
	/* creation time : a fake value */
	LWRITE32_AND_MOVE( pbuf, 0xB9E3B1CE );
	/* modification time : a fake value */
	LWRITE32_AND_MOVE( pbuf, 0xB9E3B1CE );
	/* timescale */
	/* video_mdhd_timescale : 1s = 1000ms, it depends on the time unit of the timer */
	/* assume that the sound_mdhd_timescale equals mvhd_timescale which is 10000 */
	if ( trakID == VIDEO_TRAK ) 
	{
		LWRITE32_AND_MOVE( pbuf, 1000 );
		LWRITE32_AND_MOVE( pbuf, gpPackBitsInfo->gPackVidDuration );
	}
	else
	{
		/* video timescale = sample rate?? */
		if (gpPackBitsInfo->audType == AUDIO_TYPE_AMR_NB) 
		{
			LWRITE32_AND_MOVE( pbuf, 1000 );
		}
		if (gpPackBitsInfo->audType == AUDIO_TYPE_AAC)
		{
			//LWRITE32_AND_MOVE( pbuf, 0x0000AC44 );
			switch (gpPackBitsInfo->audSR)
			{
			case 12000:
				LWRITE32_AND_MOVE( pbuf, 0x00002EE0 );
				break;
			case 16000:
				LWRITE32_AND_MOVE( pbuf, 0x00003E80 );
				break;
			case 22050:
				LWRITE32_AND_MOVE( pbuf, 0x00005622 );
				break;
			case 24000:
				LWRITE32_AND_MOVE( pbuf, 0x00005DC0 );
				break;
			case 32000:
				LWRITE32_AND_MOVE( pbuf, 0x00007D00 );
				break;
			case 44100:
				LWRITE32_AND_MOVE( pbuf, 0x0000AC44 );
				break;
			case 48000:
				LWRITE32_AND_MOVE( pbuf, 0x0000BB80 );
				break;
			default:
				/* time scale, 44100 */
				LWRITE32_AND_MOVE( pbuf, 0x0000AC44 );
				break;
			}
		}
		if (gpPackBitsInfo->audType == AUD_FORMAT_PCM)
		{
			LWRITE32_AND_MOVE( pbuf, 0x00003E80 );
		}
		LWRITE32_AND_MOVE( pbuf, gpPackBitsInfo->gPackAudDurationT );
	}
	/* pad, language */
	LWRITE16_AND_MOVE( pbuf, 0x00 );
	/* pre_defined */
	LWRITE16_AND_MOVE( pbuf, 0x00 );

	return size;
}


/**************************************************************************

  Function Name: vidMP4HDLRBoxAdd

  Purposes: Add the "hdlr" header of a 3GP file.

  Limitations:

  Arguments:
    pbuf   [in] - pointer to free buffer to be written.
    trakID [in] - the track id of this box.

  Returns: the size this box takes.

  See also: vid3GPMVHDBoxAdd

 **************************************************************************/
static UINT32
vidMP4HDLRBoxAdd(
	MP4MuxInfo_t *gpPackBitsInfo, 
	UINT8 *pbuf,
	UINT32 trakID
)
{
	UINT32 ret;
	UINT32 size;
	const char* type;
	const char* descr;

	size = 0x21;
	
	/* handler type */
	if ( trakID == VIDEO_TRAK ) 
	{
		type      = "vide";
		descr     = "VideoHandler";
	}
	else if ( trakID == AUDIO_TRAK ) 
	{
		type      = "soun";
		descr     = "SoundHandler";
	}
	else
	{
		type      = "url ";
		descr     = "DataHandler";
	}
	
	if (gpPackBitsInfo->mode == MODE_MOV)
		size += strlen(descr);
	ret = vidMP4FullBoxAdd(pbuf, size, "hdlr", 0x00, 0x00);
	pbuf += ret;
	if (gpPackBitsInfo->mode == MODE_MOV)
	{
		if (trakID)
		{
			MEMCPY_AND_MOVE( pbuf, "mhlr", 0x04 );
		}
		else
		{
			MEMCPY_AND_MOVE( pbuf, "dhlr", 0x04 );
		}
	}
	else
	{
		/* pre_defined */
		LWRITE32_AND_MOVE( pbuf, 0x00 );
	}
	/* handler type */
	MEMCPY_AND_MOVE( pbuf, type, 0x04 );
	/* reserved */
	LWRITE32_AND_MOVE( pbuf, 0x00 );
	LWRITE32_AND_MOVE( pbuf, 0x00 );
	LWRITE32_AND_MOVE( pbuf, 0x00 );
	/* string name */
	if (gpPackBitsInfo->mode == MODE_MOV)
	{
		*pbuf++ = strlen(descr);
		MEMCPY_AND_MOVE( pbuf, descr, strlen(descr));
	}
	else
		*pbuf++ = 0x00;

	return size;
}


/**************************************************************************

  Function Name: vidMP4VMHDBoxAdd

  Purposes: Add the "vmhd" header of a 3GP file.

  Limitations:

  Arguments:
    pbuf   [in] - pointer to free buffer to be written.

  Returns: the size this box takes.

  See also: vid3GPMVHDBoxAdd

 **************************************************************************/
static UINT32
vidMP4VMHDBoxAdd(
	UINT8 *pbuf
)
{
	UINT32 ret;
	UINT32 size;

	size = 0x14;
	ret = vidMP4FullBoxAdd(pbuf, size, "vmhd", 0x00, 0x01);
	pbuf += ret;
	/* graphics mode */
	LWRITE16_AND_MOVE( pbuf, 0x00 );
	/* operation color */
	*pbuf++ = 0x00;
	*pbuf++ = 0x00;
	*pbuf++ = 0x00;
	*pbuf++ = 0x00;
	*pbuf++ = 0x00;
	*pbuf++ = 0x00;

	return size;
}


/**************************************************************************

  Function Name: vidMP4SMHDBoxAdd

  Purposes: Add the "smhd" header of a 3GP file.

  Limitations:

  Arguments:
    pbuf   [in] - pointer to free buffer to be written.

  Returns: the size this box takes.

  See also: vid3GPMVHDBoxAdd

 **************************************************************************/
static UINT32
vidMP4SMHDBoxAdd(
	UINT8 *pbuf
)
{
	UINT32 ret;
	UINT32 size;

	size = 0x10;
	ret = vidMP4FullBoxAdd(pbuf, size, "smhd", 0x00, 0x00);
	pbuf += ret;
	/* balance */
	LWRITE16_AND_MOVE( pbuf, 0x00 );
	/* reserved */
	LWRITE16_AND_MOVE( pbuf, 0x00 );

	return size;
}


/**************************************************************************

  Function Name: vidMP4DREFBoxAdd

  Purposes: Add the "dref" header of a 3GP file.

  Limitations:

  Arguments:
    pbuf   [in] - pointer to free buffer to be written.

  Returns: the size this box takes.

  See also: vid3GPMVHDBoxAdd

 **************************************************************************/
static UINT32
vidMP4DREFBoxAdd(
	UINT8 *pbuf
)
{
	UINT32 ret;
	UINT32 size;

	size = 0x1C;
	ret = vidMP4FullBoxAdd(pbuf, size, "dref", 0x00, 0x00);
	pbuf += ret;
	/* entry count : 0x00000001 */
	LWRITE32_AND_MOVE( pbuf, 0x01 );
	/****** Data Reference ******/
	ret = vidMP4FullBoxAdd(pbuf, 0x0C, "url ", 0x00, 0x01);
	pbuf += ret;
	/****** end of Data Reference ******/

	return size;
}


/**************************************************************************

  Function Name: vidMP4FreeBoxAdd

  Purposes:
      Add the free box of a 3GP file. Since we use cluster align method to
    achieve that concate the main file with index file, we have to add the
    free box and this helps us to achieve this.

  Limitations:

  Arguments:
    fd   [in] - file descriptor of main file.
    size [in] - size of this box.

  Returns: SUCCESS or error code

  See also: vid3GPMVHDBoxAdd

 **************************************************************************/
static UINT16
vidMP4FreeBoxAdd(
	MP4MuxInfo_t *gpPackBitsInfo,
	FILE *fd,
	UINT32 size
)
{
	UINT8 *pbuf;
	UINT8 *pbufStart;
	UINT16 err;
	//UINT8 data[512];

	//pbuf = (UINT8*)((UINT32)data | 0x3C000000);
	pbuf = gpPackBitsInfo->gFreeBuf;
	pbufStart = pbuf;
	vidMP4BoxAdd(pbuf, size, "free");
	pbuf = pbufStart;
	err = vidMP4PacketAdd(gpPackBitsInfo, fd, pbuf, 8);
	if (err != SUCCESS) 
	{
		return err;
	}
	size -= 8;

	pbuf = gpPackBitsInfo->gpMP4IdxBufStart[0];
	err = vidMP4PacketAdd(gpPackBitsInfo, fd, pbuf, size);
	if (err != SUCCESS)
	{
		return err;
	}

	return SUCCESS;
}


/**************************************************************************

  Function Name: vidMP4IdxBufLeftAdd

  Purposes:
      Open the index file and write the index buffer data to it. We are
    going to concate the main file with index file.

  Limitations:

  Arguments:
    indexFD [out] - Returns the file descriptor of specified index item.
    idxItem  [in] - the index item number (IDX_VID_STTS, etc)

  Returns: SUCCESS or error code

  See also: vidMP4ClusterAlign

 **************************************************************************/
static FILE *
vidMP4IdxBufLeftAdd(	
	MP4MuxInfo_t *gpPackBitsInfo,
	UINT32 idxItem
)
{	
	UINT8 *pbuf;
	UINT8 *pbufStart;
	UINT32 tmpSize;
	UINT8 tmpPath[LONG_FILENAME_LEN];
	UINT8 tmpName[ATOM_FILE_NAME_LEN];
	SINT32 sizeAdd;
	SINT32 err;
	FILE *iTempFd;

	pbufStart = gpPackBitsInfo->gpMP4IdxBufStart[idxItem];
	pbuf = gpPackBitsInfo->gpMP4IdxBufCurr[idxItem];
	tmpSize = pbuf - pbufStart;
	strcpy((char *)tmpName, ATOM_FILE_NAME);
	tmpName[7] = (UINT8)(idxItem + 48);

	memset(tmpPath, 0, LONG_FILENAME_LEN);
	strcpy((char *)tmpPath, (char *)gpPackBitsInfo->tempFile);
	strcat((char *)tmpPath, (char *)tmpName);
	diag_printf("IdxBufLeftAdd: tmpName: %s, size: %d\n", tmpPath, tmpSize);
    #ifdef USE_SYSTEM_FS
    iTempFd = (FILE *)open((char *)tmpPath,    O_APPEND | O_RDWR);
    if (iTempFd == (FILE *)(-1))
    {
        return NULL;
    }
    #else
    iTempFd = fopen((char *)tmpPath, "w+b");
    if (iTempFd == NULL)
    {
        return NULL;
    }
    #endif
    
	/* Write the left data to IDX file */
	if ( tmpSize > 0 )
	{
		err = fseek(iTempFd, 0, SEEK_END);
		if (err == -1)
		{
			return NULL;
		}		
		sizeAdd = fwrite(pbufStart, 1, tmpSize, iTempFd);
		if (sizeAdd <= 0)
		{
			return NULL;
		}
	}	

	return iTempFd;

}



/**************************************************************************

  Function Name: vidMP4ClusterAlign

  Purposes:
      Append free data to make the file to be cluster aligned. We have the
    make sure the main file is cluster aligned so that we can concate the
    index file with it by fsConcateFile.

  Limitations:

  Arguments:
    fd     [in] - file descriptor of the main file.
    pSize [out] - the size we consumed to make the file be cluster align

  Returns: SUCCESS or error code

  See also: vidMP4IdxBufLeftAdd, vidMP4IdxInfoUpdate

 **************************************************************************/
static UINT16
vidMP4ClusterAlign(
	MP4MuxInfo_t *gpPackBitsInfo,
	FILE   *fd,
	UINT32 *pSize
)
{
	UINT32 freeSize;
	UINT32 filePos;
	UINT32 clusterSize;
	UINT32 sizeAdd;
	SINT32  err;

	clusterSize = gpPackBitsInfo->clusterSize;	
	err = fseek(fd, 0, SEEK_END);
	if (err == -1)
	{
		return FAIL; 
	}
	filePos = ftell(fd);	
	diag_printf("filePos: 0x%x\n", filePos);
	freeSize = clusterSize - (filePos % clusterSize);
	if (freeSize < 8) 
	{
		freeSize += clusterSize;
	}
	diag_printf("freeSize: 0x%x\n", freeSize);
	if ( freeSize != 0 ) 
	{
		err = vidMP4FreeBoxAdd(gpPackBitsInfo, fd, freeSize);
		if ( err != SUCCESS ) 
		{
			return FAIL;
		}
		err = vidMP4PacketFlush(gpPackBitsInfo, fd, &sizeAdd);
		if (err != SUCCESS)
		{
			return FAIL;
		}
	}
	*pSize = freeSize;

	return SUCCESS;

}


/**************************************************************************

  Function Name: vidMP4IdxInfoUpdate

  Purposes:
      Update the index information. This function will be called when we
    stop the record operation and we've opened temporary index file to
    preserve the index information.

  Limitations:

  Arguments:
    pdata     [in] - pointer to the index information to be filled.
    dataSize  [in] - the size of index information to be filled.
    indexItem [in] - the index number (IDX_VID_STTS, etc.)
    pSize    [out] - the added file size to call this function.

  Returns: SUCCESS or error code

  See also: vid3GPSTSDBoxAdd

 **************************************************************************/
static UINT16
vidMP4IdxInfoUpdate(
	MP4MuxInfo_t *gpPackBitsInfo,
	UINT8 *pdata,
	UINT32 dataSize,
	UINT32 indexItem,
	UINT32* pSize
)
{
	UINT8 *pbuf;
	UINT8 *pbufStart;
	FILE  *indexFD;
	//UINT8 data[128];
	SINT32 size;
	SINT32  err;
	UINT32 freeSize;

	indexFD = NULL;
	/* Flush the left index data to file */
	indexFD = vidMP4IdxBufLeftAdd(gpPackBitsInfo, indexItem);
	if (indexFD == NULL) 
	{
		return FAIL;
	}
	/* Make the source file to be cluster boundary, in order to concate file */
	err = vidMP4ClusterAlign(gpPackBitsInfo, gpPackBitsInfo->gPackFd, &freeSize);
	if (err != SUCCESS) 
	{
		return FAIL;
	}
	/* Update box infomation */
	//pbuf = (UINT8*)((UINT32)data | 0x3C000000);
	pbuf = gpPackBitsInfo->gFreeBuf;
	pbufStart = pbuf;
	err = fseek(indexFD, 0, SEEK_SET);
	if (err == -1) 
	{
		return FAIL;
	}	
	size = fread(pbuf, 1, 128, indexFD);
	if (size != 128)
	{
		return FAIL;
	}

	/* Copy pdata to pbuf */
	memcpy(pbufStart, pdata, dataSize);


	/* Write to file */
	err = fseek(indexFD, 0, SEEK_SET);
	if (err == -1) 
	{
		return FAIL;
	}
	
	size = fwrite(pbufStart, 1, 128, indexFD);
	if (size != 128)
	{
		return FAIL;
	}

	/* Concate file */
	err = fsConcatefile(gpPackBitsInfo, gpPackBitsInfo->gPackFd, indexFD);
	if (err != SUCCESS)
	{
		return FAIL;
	}
	*pSize = freeSize;
	if (indexFD != NULL)
	{
		fclose(indexFD);
	}

	return SUCCESS;
}


static UINT16
vidNALHeaderSearch(
    UINT8 *pBuf,
    UINT32 usefulSize
    )
{
    UINT32 cnt = 0;
	UINT32 uiVolStart;
	
    if(usefulSize <= 4){
        return usefulSize;
    }
	
    while(cnt < usefulSize) {       
        uiVolStart = LREAD32(pBuf);
        if(0x00000001==uiVolStart)  {
            break;
        }
        else {
            cnt ++;
            pBuf++;
        }
    }
    return cnt;
}
/**************************************************************************

  Function Name: vidMP4STSDBoxAdd

  Purposes: Add the "stsd" box of a 3GP file.

  Limitations:

  Arguments:
    pbuf  [out] - pointers to the free buffer to be filled this box.
    trakID [in] - the track id of the stsd box.

  Returns: the size this box takes

  See also: vidMP4STSCBoxAdd

 **************************************************************************/
static UINT32
vidMP4STSDBoxAdd(
	MP4MuxInfo_t *gpPackBitsInfo,
	UINT8 *pbuf,
	UINT32 trakID
)
{
	UINT32 size = 0;
	UINT32 ret;
	UINT32 i;

	if ( trakID == VIDEO_TRAK ) 
	{
		/* 117 for h263, 159 for mp4v */
		switch(gpPackBitsInfo->vidType) 
		{
		case VIDEO_TYPE_H264_BP:
            if (gpPackBitsInfo->vidExtraLen != 0 && gpPackBitsInfo->pVidExtraData != NULL) 
            { 
                    size = 139 + gpPackBitsInfo->vidExtraLen; 
            }         
            else 
            { 
                    size = 159; 
            } 
            break;
		case VIDEO_TYPE_MPEG4_SP:
			if (gpPackBitsInfo->vidExtraLen != 0 && gpPackBitsInfo->pVidExtraData != NULL)
			{
				size = 139 + gpPackBitsInfo->vidExtraLen;
			} 
			else
			{
				size = 159;
			}					
			break;
		case VIDEO_TYPE_H263:
			size = 117;
			break;

        default:
            size = 0;
            break;
		}
		ret = vidMP4FullBoxAdd(pbuf, size, "stsd", 0x00, 0x00);
		pbuf += ret;
		/****** sample description ******/
		/* entry count : 0x00000001 */
		LWRITE32_AND_MOVE( pbuf, 0x01 );
		/* size : 101 for h263; 143 for mp4v */
		/* format : s263 or mp4v */
		switch(gpPackBitsInfo->vidType)
		{
		case VIDEO_TYPE_H264_BP:
			if (gpPackBitsInfo->vidExtraLen != 0 && gpPackBitsInfo->pVidExtraData != NULL)
			{
				ret = vidMP4BoxAdd(pbuf, 123 + gpPackBitsInfo->vidExtraLen, "avc1");
			}
			else
			{
			    ret = vidMP4BoxAdd(pbuf, 143, "avc1");
			}
		    break;
		case VIDEO_TYPE_MPEG4_SP:
			if (gpPackBitsInfo->vidExtraLen != 0 && gpPackBitsInfo->pVidExtraData != NULL)
			{
				ret = vidMP4BoxAdd(pbuf, 123 + gpPackBitsInfo->vidExtraLen, "mp4v");
			}
			else
			{
				ret = vidMP4BoxAdd(pbuf, 143, "mp4v");
			}
			
			break;
		case VIDEO_TYPE_H263:
			ret = vidMP4BoxAdd(pbuf, 101, "s263");
			break;

        default:
            ret = 0;
            break;
		}
		pbuf += ret;
		/* reserved 6 bytes */
		LWRITE24_AND_MOVE( pbuf, 0x00 );
		LWRITE24_AND_MOVE( pbuf, 0x00 );
		/* data reference index : 0x0001 */
		LWRITE16_AND_MOVE( pbuf, 0x01 );
		/****** visual sequence ******/
		/* reserved */
		LWRITE32_AND_MOVE( pbuf, 0x00 );
		LWRITE32_AND_MOVE( pbuf, 0x53554E50 );//vendor:"SUNP"
		if (gpPackBitsInfo->mode == MODE_MOV)
		{
			LWRITE32_AND_MOVE( pbuf, 0x200 ); //temporal quality normal
			LWRITE32_AND_MOVE( pbuf, 0x200 ); // spatial quality normal
		}
		else
		{
			LWRITE32_AND_MOVE( pbuf, 0x00 );
			LWRITE32_AND_MOVE( pbuf, 0x00 );
		}
		/* width, height T.B.D. */
		LWRITE16_AND_MOVE( pbuf, gpPackBitsInfo->width);
		LWRITE16_AND_MOVE( pbuf, gpPackBitsInfo->height );
		/* horizontal resolution : 0x00480000 */
		LWRITE32_AND_MOVE( pbuf, 0x00480000 );
		/* vertical resolution : 0x00480000 */
		LWRITE32_AND_MOVE( pbuf, 0x00480000 );
		/* reserved */
		LWRITE32_AND_MOVE( pbuf, 0x00 );
		/* frame count : 0x0001 */
		LWRITE16_AND_MOVE( pbuf, 0x01 );
		/* compressor name */
		for ( i = 0; i < 8; i++ )
		{
			LWRITE32_AND_MOVE( pbuf, 0x00 );
		}
		/* color depth : 0x0018 */
		LWRITE16_AND_MOVE( pbuf, 0x18 );
		/* pre_defined = -1 */
		LWRITE16_AND_MOVE( pbuf, 0xFFFF );
		/****** end of visual sequence ******/
		if (gpPackBitsInfo->vidType == VIDEO_TYPE_H264_BP) 
        {
            //avcC descriptor
			if (gpPackBitsInfo->vidExtraLen != 0 && gpPackBitsInfo->pVidExtraData != NULL)
			{
				ret = vidMP4BoxAdd(pbuf, 37 + gpPackBitsInfo->vidExtraLen, "avcC");
			} 
			else
			{
			    ret = vidMP4BoxAdd(pbuf, 57, "avcC");
			}
            pbuf += ret;
            //ConfigurationVersion
            *pbuf++ = 0x01;
            //AVCProfileIndication
            *pbuf++ = 0x42;
            //Profile compatibility
            *pbuf++ = 0x80;//0xC0
            //AVCLevelIndication
            *pbuf++ = 0x1E;
            //reserved = '111111'b
            //lengthSizeMinusOne = 3
            *pbuf++ = 0xff;
            //reserved = '111'b
            //numOfSequenceParameterSetLength = 1
            *pbuf++ = 0xE1;

			if (gpPackBitsInfo->vidExtraLen != 0 && gpPackBitsInfo->pVidExtraData != NULL)
			{
			    UINT8 *ptr;
                UINT16 size=0, usefulSize;
                ptr = gpPackBitsInfo->pVidExtraData;
                usefulSize = gpPackBitsInfo->vidExtraLen;
                do
                {
                    UINT32 uiVolStart;
                    uiVolStart = LREAD32(ptr);
                    if(0x00000001==uiVolStart)
                    {
                        ptr        += 4;
                        usefulSize -= 4;
                        size = vidNALHeaderSearch(ptr,usefulSize);
                        LWRITE16_AND_MOVE(pbuf, size);
                        memcpy(pbuf, ptr, size);
                        pbuf+= size;
                        ptr += size;
                        usefulSize -= size;
                        //numOfPictureParameterSetLength = 1
                        if(usefulSize>4){
                            *pbuf++ = 0x01;
                        }
                    }
                    else
                    {
                        ptr++;
                        usefulSize--;
                    }
                }
                while(usefulSize>4);

				osFree(gpPackBitsInfo->pVidExtraData);
				gpPackBitsInfo->pVidExtraData = NULL;
				gpPackBitsInfo->vidExtraLen = 0;
			}            
        }
		else if (gpPackBitsInfo->vidType == VIDEO_TYPE_MPEG4_SP) 
		{
			/****** optional description ******/
			/* size : 57 */
			if (gpPackBitsInfo->vidExtraLen != 0 && gpPackBitsInfo->pVidExtraData != NULL)
			{
				ret = vidMP4FullBoxAdd(pbuf, 37 + gpPackBitsInfo->vidExtraLen, "esds", 0x00, 0x00);
			} 
			else
			{
				ret = vidMP4FullBoxAdd(pbuf, 0x39, "esds", 0x00, 0x00);
			}			
			pbuf += ret;
			/****** ES_Descriptor ******/
			/* ES_DescrTag */
			*pbuf++ = 0x03;
			/* size : 43 , The value of size does not contain this field and last field */
			if (gpPackBitsInfo->vidExtraLen != 0 && gpPackBitsInfo->pVidExtraData != NULL)
			{
				*pbuf++ = 23 + gpPackBitsInfo->vidExtraLen;
			}
			else
			{
				*pbuf++ = 0x2B;
			}
			
			/* ES_ID : 0x0001 */
			LWRITE16_AND_MOVE( pbuf, 0x0001 );
			/* streamDependenceFlag, URL_Flag, OCRstreamFlag, streamPriority */
			*pbuf++ = 0x1F;
			/**** DecoderConfigDescriptor ****/
			/* DecoderConfigDescrTag */
			*pbuf++ = 0x04;
			/* size : 35 , The value of size does not contain this field and last field */
			if (gpPackBitsInfo->vidExtraLen != 0 && gpPackBitsInfo->pVidExtraData != NULL)
			{
				*pbuf++ = 15 + gpPackBitsInfo->vidExtraLen;
			}
			else
			{
				*pbuf++ = 0x23;
			}
			
			/* objectTypeIndication : Visual ISO/IEC 14496-2 */
			*pbuf++ = 0x20;
			/* streamType, upStream, reserved */
			*pbuf++ = 0x11;
			/* bufferSizeDB */
			LWRITE24_AND_MOVE( pbuf, 0x000249F0 );
			/* maxBitrate : 4000000 */
			LWRITE32_AND_MOVE( pbuf, 0x003D0900 );
			/* avgBitrate : 2000000 */
			LWRITE32_AND_MOVE( pbuf, 0x001E8480 );
			/** DecoderSpecificInfo **/
			/* DecoderSpecificInfoTag */
			*pbuf++ = 0x05;	
			if (gpPackBitsInfo->vidExtraLen != 0 && gpPackBitsInfo->pVidExtraData != NULL)
			{
				*pbuf++ = gpPackBitsInfo->vidExtraLen;
				memcpy(pbuf, gpPackBitsInfo->pVidExtraData, gpPackBitsInfo->vidExtraLen);
				pbuf += gpPackBitsInfo->vidExtraLen;
				osFree(gpPackBitsInfo->pVidExtraData);
				gpPackBitsInfo->pVidExtraData = NULL;
				gpPackBitsInfo->vidExtraLen = 0;
			} 
			else
			{
				/* size : 20 , The value of size does not contain this field and last field */
				*pbuf++ = 0x14;
				/* Vo and Vol header */
				pbuf += vidVoVolDataAdd(pbuf, gpPackBitsInfo);	
			}
					
			/**** SLConfigDescriptor ****/
			/* SLConfigDescriptorTag */
			*pbuf++ = 0x06;
			/* size : 1 , The value of size does not contain this field and last field */
			*pbuf++ = 0x01;
			/* content */
			*pbuf++ = 0x02;
			/****** end of ES_Descriptor ******/
		}
		else if (gpPackBitsInfo->vidType == VIDEO_TYPE_H263) 
		{
			/* size : 15 */
			ret = vidMP4BoxAdd(pbuf, 0x0F, "d263");
			pbuf += ret;
			/* company */
			MEMCPY_AND_MOVE( pbuf, "SUNP", 0x04 );
			/* decoding version */
			*pbuf++ = 0x00;
			/* level */
			*pbuf++ = 0x0A;
			/* profile */
			*pbuf++ = 0x00;
		}
		/****** end of optional description ******/
	}
	else 
	{

		/* 117 for h263, 159 for mp4v */
		if (gpPackBitsInfo->audType == AUDIO_TYPE_AMR_NB)
		{
			size = 69;
		}
		else if (gpPackBitsInfo->audType == AUDIO_TYPE_AAC) 
		{
			size = 91 + ((gpPackBitsInfo->mode == MODE_MOV) ? 8:0);
		}
		else if (gpPackBitsInfo->audType == AUD_FORMAT_PCM) 
		{
			size = 52 + ((gpPackBitsInfo->mode == MODE_MOV) ? 8:0);
		}
		ret = vidMP4FullBoxAdd(pbuf, size, "stsd", 0x00, 0x00);
		pbuf += ret;

		if (gpPackBitsInfo->audType == AUDIO_TYPE_AMR_NB) 
		{
			/****** sample description ******/
			/* entry count : 0x00000001 */
			LWRITE32_AND_MOVE( pbuf, 0x01 );
			/* size : 53 for samr */
			/* format : samr */
			ret = vidMP4BoxAdd(pbuf, 53, "samr");
			pbuf += ret;
			/* reserved */
			/* reserved 6 bytes */
			LWRITE24_AND_MOVE( pbuf, 0x00 );
			LWRITE24_AND_MOVE( pbuf, 0x00 );
			/* data reference index : 0x0001 */
			LWRITE16_AND_MOVE( pbuf, 0x01 );
			/* version */
			LWRITE16_AND_MOVE( pbuf, 0x00 );
			/* revision level */
			LWRITE16_AND_MOVE( pbuf, 0x00 );
			/* vendor */
			LWRITE32_AND_MOVE( pbuf, 0x00 );
			/* number of channels */
			LWRITE16_AND_MOVE( pbuf, 0x01 );
			/* sample size */
			LWRITE16_AND_MOVE( pbuf, 0x08 );
			/* compression ID */
			LWRITE16_AND_MOVE( pbuf, 0x00 );
			/* packet size */
			LWRITE16_AND_MOVE( pbuf, 0x00 );
			/* time scale, 1000 */
			LWRITE32_AND_MOVE( pbuf, 0x01F40000 );
			/* damr is 17 bytes*/
			ret = vidMP4BoxAdd(pbuf, 0x11, "damr");
			pbuf += ret;
			/* vendor */
			MEMCPY_AND_MOVE( pbuf, "SUNP", 0x04 );
			/* decoder_version */
			*pbuf++ = 0x00;
			/* mode_set is 0x02 for 5.15kbps */
			LWRITE16_AND_MOVE( pbuf, 0x02 );
			/* mode_change_period */
			*pbuf++ = 0x00;
			/* frames_per_sample */
			*pbuf++ = 0x01;
		}
		/* TODO: add the aac part */
		if (gpPackBitsInfo->audType == AUDIO_TYPE_AAC)
		{
			int sd_size;
			
			LWRITE32_AND_MOVE( pbuf, 0x01 );
			
			sd_size = 75;
			if (gpPackBitsInfo->mode == MODE_MOV)
				sd_size += 8;
				
			ret = vidMP4BoxAdd(pbuf, sd_size, "mp4a");
			
			pbuf += ret;
			/* reserved 6 bytes */
			LWRITE24_AND_MOVE( pbuf, 0x00 );
			LWRITE24_AND_MOVE( pbuf, 0x00 );
			/* data reference index : 0x0001 */
			LWRITE16_AND_MOVE( pbuf, 0x01 );
			/* version */
			LWRITE16_AND_MOVE( pbuf, 0x00 );
			/* revision level */
			LWRITE16_AND_MOVE( pbuf, 0x00 );
			/* vendor */
			LWRITE32_AND_MOVE( pbuf, 0x00 );
			/* number of channels */
			//LWRITE16_AND_MOVE( pbuf, 0x01 );
			LWRITE16_AND_MOVE( pbuf, gpPackBitsInfo->audChannel );
			/* sample size */
			//LWRITE16_AND_MOVE( pbuf, 0x10 );
			LWRITE16_AND_MOVE( pbuf, gpPackBitsInfo->audBits );
			/* compression ID */
			LWRITE16_AND_MOVE( pbuf, 0x00 );
			/* packet size */
			LWRITE16_AND_MOVE( pbuf, 0x00 );
			switch (gpPackBitsInfo->audSR)
			{
			case 12000:
				LWRITE32_AND_MOVE( pbuf, 0x2EE00000 );
				break;
			case 16000:
				LWRITE32_AND_MOVE( pbuf, 0x3E800000 );
				break;
			case 22050:
				LWRITE32_AND_MOVE( pbuf, 0x56220000 );
				break;
			case 24000:
				LWRITE32_AND_MOVE( pbuf, 0x5DC00000 );
				break;
			case 32000:
				LWRITE32_AND_MOVE( pbuf, 0x7D000000 );
				break;
			case 44100:
				LWRITE32_AND_MOVE( pbuf, 0xAC440000 );
				break;
			case 48000:
				LWRITE32_AND_MOVE( pbuf, 0xBB800000 );
				break;
			default:
				/* time scale, 44100 */
				LWRITE32_AND_MOVE( pbuf, 0xAC440000 );
				break;
			}
			
			/* esds is 39 bytes*/
			ret = vidMP4BoxAdd(pbuf, 0x27, "esds");
			pbuf += ret;
			/* version + flag */
			LWRITE32_AND_MOVE( pbuf, 0x00 );
			LWRITE16_AND_MOVE( pbuf, 0x0319 );
			LWRITE24_AND_MOVE( pbuf, 0x00 );
			LWRITE16_AND_MOVE( pbuf, 0x0411 );
			LWRITE16_AND_MOVE( pbuf, 0x4015 );
			LWRITE24_AND_MOVE( pbuf, 0x001800 );
			LWRITE32_AND_MOVE( pbuf, 0x00 );
			LWRITE32_AND_MOVE( pbuf, 0x00 );
			LWRITE16_AND_MOVE( pbuf, 0x0502 );
			switch (gpPackBitsInfo->audSR)
			{
			case 12000:
				LWRITE16_AND_MOVE( pbuf, 0x1488 );  
				break;
			case 16000:
				LWRITE16_AND_MOVE( pbuf, 0x1410 );  
				break;
			case 22050:
				LWRITE16_AND_MOVE( pbuf, 0x1388 );  
				break;
			case 24000:
				LWRITE16_AND_MOVE( pbuf, 0x1310 );  
				break;
			case 32000:
				LWRITE16_AND_MOVE( pbuf, 0x1290 );  
				break;
			case 44100:
				LWRITE16_AND_MOVE( pbuf, 0x1208 );  
				break;
			case 48000:
				LWRITE16_AND_MOVE( pbuf, 0x1190 );  
				break;
			default:
				/* time scale, 44100 */
				LWRITE16_AND_MOVE( pbuf, 0x1208 );  //44Khz, mono
				break;
			}
			LWRITE24_AND_MOVE( pbuf, 0x060102 );
			
			if (gpPackBitsInfo->mode == MODE_MOV)
			{
				LWRITE32_AND_MOVE( pbuf, 0x08 );
				LWRITE32_AND_MOVE( pbuf, 0x00 );
			}
		}
		if (gpPackBitsInfo->audType == AUD_FORMAT_PCM)
		{
			int sd_size;
			
			LWRITE32_AND_MOVE( pbuf, 0x01 );
			
			sd_size = 36;
			if (gpPackBitsInfo->mode == MODE_MOV)
				sd_size += 8;

			ret = vidMP4BoxAdd(pbuf, sd_size, "sowt");

			pbuf += ret;
			/* reserved 6 bytes */
			LWRITE24_AND_MOVE( pbuf, 0x00 );
			LWRITE24_AND_MOVE( pbuf, 0x00 );
			/* data reference index : 0x0001 */
			LWRITE16_AND_MOVE( pbuf, 0x01 );
			/* version */
			LWRITE16_AND_MOVE( pbuf, 0x00 );
			/* revision level */
			LWRITE16_AND_MOVE( pbuf, 0x00 );
			/* vendor */
			LWRITE32_AND_MOVE( pbuf, 0x00 );
			/* number of channels */
			//LWRITE16_AND_MOVE( pbuf, 0x01 );
			LWRITE16_AND_MOVE( pbuf, gpPackBitsInfo->audChannel );
			/* sample size */
			//LWRITE16_AND_MOVE( pbuf, 0x10 );
			LWRITE16_AND_MOVE( pbuf, gpPackBitsInfo->audBits );
			/* compression ID */
			LWRITE16_AND_MOVE( pbuf, 0x00 );
			/* packet size */
			LWRITE16_AND_MOVE( pbuf, 0x00 );
			
			//sample rate 16K
			LWRITE32_AND_MOVE( pbuf, 0x3E800000 );
						
			if (gpPackBitsInfo->mode == MODE_MOV)
			{
				LWRITE32_AND_MOVE( pbuf, 0x08 );
				LWRITE32_AND_MOVE( pbuf, 0x00 );
			}
		}
	}

	return size;
}


/**************************************************************************

  Function Name: vidMP4STSCBoxAdd

  Purposes: Add the "stsc" box of a 3GP file.

  Limitations:

  Arguments:
    pbuf  [out] - pointers to the free buffer to be filled this box.
    trakID [in] - the track id of the stsd box.

  Returns: the size this box takes

  See also: vid3GPSTSDBoxAdd

 **************************************************************************/
static UINT32
vidMP4STSCBoxAdd(
	UINT8 *pbuf,
	UINT32 trakID
)
{
	UINT32 ret;
	UINT32 size;

	if ( trakID == VIDEO_TRAK )
	{
		size = 0x1C;
		ret = vidMP4FullBoxAdd(pbuf, size, "stsc", 0x00, 0x00);
		pbuf += ret;
		/* entry count : 0x00000001 */
		LWRITE32_AND_MOVE( pbuf, 0x01 );
		/* first chunk : 0x00000001 */
		LWRITE32_AND_MOVE( pbuf, 0x01 );
		/* samples per chunk : 0x00000001 */
		LWRITE32_AND_MOVE( pbuf, 0x01 );
		/* sample description index */
		LWRITE32_AND_MOVE( pbuf, 0x01 );
	}
	else 
	{
		size = 0x1C;
		ret = vidMP4FullBoxAdd(pbuf, size, "stsc", 0x00, 0x00);
		pbuf += ret;
		/* entry count : 0x00000001 */
		LWRITE32_AND_MOVE( pbuf, 0x01 );
		/* first chunk : 0x00000001 */
		LWRITE32_AND_MOVE( pbuf, 0x01 );
		/* samples per chunk : 0x05 */
		LWRITE32_AND_MOVE( pbuf, 0x01 );
		/* sample description index */
		LWRITE32_AND_MOVE( pbuf, 0x01 );
	}

	return size;
}


/**************************************************************************

  Function Name: vidMP4STTSBoxAdd

  Purposes: Add the "stts" box of a 3GP file.

  Limitations:

  Arguments:
    fd     [in] - the file descriptor of main file.
    trakID [in] - the track id of the stts box.
    pSize [out] - the added size after invoking this function.

  Returns: the size this box takes

  See also: vid3GPSTSDBoxAdd

 **************************************************************************/
static UINT16
vidMP4STTSBoxAdd(
	MP4MuxInfo_t *gpPackBitsInfo,
	FILE *fd,
	UINT32 trakID,
	UINT32* pSize
)
{

	UINT8 *pbuf;
	UINT8 *pbufStart;
	UINT32 sizeAdd;
	UINT32 size;
	UINT32 cntEntry;
	UINT32 indexItem;
	UINT32 indexFD;
	UINT32 freeSize;
	UINT16 err;

	/* size : 	4(size of size field) +
				4(size of name field) +
				4(size of version_flag field) +
				4(size of entry count field) +
				(entry count) * 8 */
	if ( trakID == VIDEO_TRAK ) 
	{
		cntEntry = gpPackBitsInfo->gSTTSEntryCnt;
		size = VID_STTS_HEADER_SIZE + cntEntry * 8;
		indexItem = IDX_VID_STTS;
		indexFD = gpPackBitsInfo->gbIdxFd[indexItem];
		if ( indexFD == 0 ) 
		{
			pbuf = gpPackBitsInfo->gpMP4IdxBufStart[indexItem];
		}
		else 
		{
			pbuf = gpPackBitsInfo->gpMP4HeaderStart;
		}
		pbufStart = pbuf;
		sizeAdd = vidMP4FullBoxAdd(pbuf, size, "stts", 0x00, 0x00);
		pbuf += sizeAdd;
		/* entry count */
		LWRITE32_AND_MOVE( pbuf, cntEntry );
		if (indexFD == 0) 
		{
			pbuf = gpPackBitsInfo->gpMP4IdxBufStart[indexItem];
			err = vidMP4PacketAdd(gpPackBitsInfo, fd, pbuf, size);
			if (err != SUCCESS)
			{
				return err;
			}
		}
		else 
		{
			err = vidMP4IdxInfoUpdate(gpPackBitsInfo, pbufStart, pbuf - pbufStart, indexItem, &freeSize);
			if (err != SUCCESS) 
			{
				return err;
			}
			size += freeSize;
		}
	}
	else 
	{
		cntEntry = 1;
		size = AUD_STTS_HEADER_SIZE + cntEntry * 8;
		pbuf = gpPackBitsInfo->gpMP4HeaderStart;
		sizeAdd = vidMP4FullBoxAdd(pbuf, size, "stts", 0x00, 0x00);
		pbuf += sizeAdd;
		/* entry count */
		LWRITE32_AND_MOVE(pbuf, cntEntry);
		/* sample count */
		LWRITE32_AND_MOVE(pbuf, gpPackBitsInfo->gAudFrmNo);
		/* sample duration */
		/* TODO: the AAC might not be 20 ms */
		if (gpPackBitsInfo->audType == AUDIO_TYPE_AMR_NB) 
		{
			LWRITE32_AND_MOVE(pbuf, 20);
		}
		if (gpPackBitsInfo->audType == AUDIO_TYPE_AAC) 
		{
			LWRITE32_AND_MOVE(pbuf, AAC_INPUT_FRAME_LEN);
		}
		if (gpPackBitsInfo->audType == AUD_FORMAT_PCM) 
		{
			LWRITE32_AND_MOVE(pbuf, gpPackBitsInfo->gAudRecSize/(2*gpPackBitsInfo->gAudFrmNo));
		}

		err = vidMP4PacketAdd(gpPackBitsInfo, fd, gpPackBitsInfo->gpMP4HeaderStart, size);
		if (err != SUCCESS)
		{
			return err;
		}
	}

	*pSize = size;

	return SUCCESS;
}


/**************************************************************************

  Function Name: vidMP4STSZBoxAdd

  Purposes: Add the "stsz" box of a 3GP file.

  Limitations:

  Arguments:
    fd     [in] - the file descriptor of main file.
    trakID [in] - the track id of the stsz box.
    pSize [out] - the added size after invoking this function.

  Returns: the size this box takes

  See also: vid3GPSTTSBoxAdd

 **************************************************************************/
static UINT16
vidMP4STSZBoxAdd(
	MP4MuxInfo_t *gpPackBitsInfo,
	FILE *fd,
	UINT32 trakID,
	UINT32* pSize
)
{
	UINT8 *pbuf;
	UINT8 *pbufStart;
	UINT32 sizeAdd;
	UINT32 size;
	UINT32 indexItem;
	UINT32 indexFD;
	UINT16 err;

	if ( trakID == VIDEO_TRAK ) 
	{
		/* size : 	4(size of size field) +
					4(size of name field) +
					4(size of version_flag field) +
					4(sample size field) +
					4(samplecount field) +
					(sample count) * 4 */
		size = VID_STSZ_HEADER_SIZE + gpPackBitsInfo->gVidFrmNo * 4;
		indexItem = IDX_VID_STSZ;
		indexFD = gpPackBitsInfo->gbIdxFd[indexItem];
		if ( indexFD == 0 )
		{
			pbuf = gpPackBitsInfo->gpMP4IdxBufStart[indexItem];
		}
		else
		{
			pbuf = gpPackBitsInfo->gpMP4HeaderStart;
		}
		pbufStart = pbuf;
		sizeAdd = vidMP4FullBoxAdd(pbuf, size, "stsz", 0x00, 0x00);
		pbuf += sizeAdd;
		/* sample size */
		LWRITE32_AND_MOVE( pbuf, 0x00 );
		/* sample count */
		LWRITE32_AND_MOVE( pbuf, gpPackBitsInfo->gVidFrmNo );

		if ( indexFD == 0 ) 
		{
			pbuf = gpPackBitsInfo->gpMP4IdxBufStart[indexItem];
			err = vidMP4PacketAdd(gpPackBitsInfo, fd, pbuf, size);
			if (err != SUCCESS) 
			{
				return err;
			}
		}
		else 
		{
			err = vidMP4IdxInfoUpdate(gpPackBitsInfo, pbufStart, pbuf - pbufStart, indexItem, &sizeAdd);
			if (err != SUCCESS)
			{
				return err;
			}
			size += sizeAdd;
		}
	}
	else
	{
		/* modified by jerry */
		#if 0
		pbuf = gpPackBitsInfo->gpMP4HeaderStart;
		size = AUD_STSZ_HEADER_SIZE;
		sizeAdd = vidMP4FullBoxAdd(pbuf, size, "stsz", 0x00, 0x00);
		pbuf += sizeAdd;
		/* sample size */
		LWRITE32_AND_MOVE(pbuf, 14);
		/* sample count */
		LWRITE32_AND_MOVE(pbuf, gpPackBitsInfo->gAudFrmNo);
		err = vidMP4PacketAdd(gpPackBitsInfo, fd, gpPackBitsInfo->gpMP4HeaderStart, size);
		if (err != SUCCESS) 
		{
			return err;
		}
		#else
		size = AUD_STSZ_HEADER_SIZE + gpPackBitsInfo->gAudFrmNo * 4;
		indexItem = IDX_AUD_STSZ;
		indexFD = gpPackBitsInfo->gbIdxFd[indexItem];
		if ( indexFD == 0 )
		{
			pbuf = gpPackBitsInfo->gpMP4IdxBufStart[indexItem];
		}
		else 
		{
			pbuf = gpPackBitsInfo->gpMP4HeaderStart;
		}
		pbufStart = pbuf;
		sizeAdd = vidMP4FullBoxAdd(pbuf, size, "stsz", 0x00, 0x00);
		pbuf += sizeAdd;
		/* sample size */
		LWRITE32_AND_MOVE( pbuf, 0x00 );
		/* sample count */
		LWRITE32_AND_MOVE( pbuf, gpPackBitsInfo->gAudFrmNo );

		if ( indexFD == 0 ) 
		{
			pbuf = gpPackBitsInfo->gpMP4IdxBufStart[indexItem];
			err = vidMP4PacketAdd(gpPackBitsInfo, fd, pbuf, size);
			if (err != SUCCESS) 
			{
				return err;
			}
		}
		else 
		{
			err = vidMP4IdxInfoUpdate(gpPackBitsInfo, pbufStart, pbuf - pbufStart, indexItem, &sizeAdd);
			if (err != SUCCESS)
			{
				return err;
			}
			size += sizeAdd;
		}
		#endif
	}
	*pSize = size;

	return SUCCESS;
}


/**************************************************************************

  Function Name: vidMP4STCOBoxAdd

  Purposes: Add the "stco" box of a 3GP file.

  Limitations:

  Arguments:
    fd     [in] - the file descriptor of main file.
    trakID [in] - the track id of the stco box.
    pSize [out] - the added size after invoking this function.

  Returns: the size this box takes

  See also: vid3GPSTTSBoxAdd

 **************************************************************************/
static UINT16
vidMP4STCOBoxAdd(
	MP4MuxInfo_t *gpPackBitsInfo,
	FILE  *fd,
	UINT32 trakID,
	UINT32 *pSize
)
{
	UINT32 size;
	UINT32 indexFD;
	UINT8 *pbuf;
	UINT8 *pbufStart;
	UINT32 indexItem;
	UINT32 sizeAdd;
	UINT16 err;

	if ( trakID == VIDEO_TRAK )
	{

		/* size : 	4(size of size field) +
					4(size of name field) +
					4(size of version_flag field) +
					4(size of entry count field) +
					(sample count) * 4 */

		size = VID_STCO_HEADER_SIZE + gpPackBitsInfo->gVidFrmNo * 4;
		indexItem = IDX_VID_STCO;
		indexFD = gpPackBitsInfo->gbIdxFd[indexItem];
		if ( indexFD == 0 ) 
		{
			pbuf = gpPackBitsInfo->gpMP4IdxBufStart[indexItem];
		}
		else
		{
			pbuf = gpPackBitsInfo->gpMP4HeaderStart;
		}
		pbufStart = pbuf;
		sizeAdd = vidMP4FullBoxAdd(pbuf, size, "stco", 0x00, 0x00);
		pbuf += sizeAdd;
		/* entry count */
		LWRITE32_AND_MOVE( pbuf, gpPackBitsInfo->gVidFrmNo );

		if ( indexFD == 0 )
		{
			pbuf = gpPackBitsInfo->gpMP4IdxBufStart[indexItem];
			err = vidMP4PacketAdd(gpPackBitsInfo, fd, pbuf, size);
			if (err != SUCCESS) 
			{
				return err;
			}
		}
		else
		{
			err = vidMP4IdxInfoUpdate(gpPackBitsInfo, pbufStart, pbuf - pbufStart, indexItem, &sizeAdd);
			if (err != SUCCESS) 
			{
				return err;
			}
			size += sizeAdd;
		}
	}
	else 
	{
		size = AUD_STCO_HEADER_SIZE + gpPackBitsInfo->gAudFrmNo * 4;
		indexItem = IDX_AUD_STCO;
		indexFD = gpPackBitsInfo->gbIdxFd[indexItem];
		if ( indexFD == 0 )
		{
			pbuf = gpPackBitsInfo->gpMP4IdxBufStart[indexItem];
		}
		else 
		{
			pbuf = gpPackBitsInfo->gpMP4HeaderStart;
		}
		pbufStart = pbuf;
		sizeAdd = vidMP4FullBoxAdd(pbuf, size, "stco", 0x00, 0x00);
		pbuf += sizeAdd;
		/* entry count */
		LWRITE32_AND_MOVE(pbuf, gpPackBitsInfo->gAudFrmNo);
		if ( indexFD == 0 ) 
		{
			pbuf = gpPackBitsInfo->gpMP4IdxBufStart[indexItem];
			err = vidMP4PacketAdd(gpPackBitsInfo, fd, pbuf, size);
			if (err != SUCCESS) 
			{
				return err;
			}
		}
		else
		{
			err = vidMP4IdxInfoUpdate(gpPackBitsInfo, pbufStart, pbuf - pbufStart, indexItem, &sizeAdd);
			if (err != SUCCESS)
			{
				return err;
			}
			size += sizeAdd;
		}
	}
	*pSize = size;

	return SUCCESS;
}


/**************************************************************************

  Function Name: vidMP4STSSBoxAdd

  Purposes: Add the "stss" box of a 3GP file.

  Limitations:

  Arguments:
    fd     [in] - the file descriptor of main file.
    pSize [out] - the added size after invoking this function.

  Returns: the size this box takes

  See also: vid3GPSTTSBoxAdd

 **************************************************************************/
static UINT16
vidMP4STSSBoxAdd(
	MP4MuxInfo_t *gpPackBitsInfo,
	FILE  *fd,
	UINT32 *pSize
)
{
	UINT8 *pbuf;
	UINT8 *pbufStart;
	UINT32 size;
	UINT32 indexItem;
	UINT32 indexFD;
	UINT32 sizeAdd;
	UINT16 err;

	/* size : 	4(size of size field) +
				4(size of name field) +
				4(size of version_flag field) +
				4(size of entry count field) +
				(entry count) * 4 */
	size = VID_STSS_HEADER_SIZE + gpPackBitsInfo->gVidIFrmNo * 4;
	indexItem = IDX_VID_STSS;
	indexFD = gpPackBitsInfo->gbIdxFd[indexItem];
	if ( indexFD == 0 ) 
	{
		pbuf = gpPackBitsInfo->gpMP4IdxBufStart[indexItem];
	}
	else 
	{
		pbuf = gpPackBitsInfo->gpMP4HeaderStart;
	}
	pbufStart = pbuf;
	sizeAdd = vidMP4FullBoxAdd(pbuf, size, "stss", 0x00, 0x00);
	pbuf += sizeAdd;
	/* entry count */
	LWRITE32_AND_MOVE( pbuf, gpPackBitsInfo->gVidIFrmNo );
	if ( indexFD == 0 ) 
	{
		pbuf = gpPackBitsInfo->gpMP4IdxBufStart[indexItem];
		err = vidMP4PacketAdd(gpPackBitsInfo, fd, pbuf, size);
		if (err != SUCCESS) 
		{
			return err;
		}
	}
	else 
	{
		err = vidMP4IdxInfoUpdate(gpPackBitsInfo, pbufStart, pbuf - pbufStart, indexItem, &sizeAdd);
		if (err != SUCCESS)
		{
			return err;
		}
		size += sizeAdd;
	}
	*pSize = size;

	return SUCCESS;
}


/**************************************************************************

  Function Name: vidMP4HeaderSizeUpdate

  Purposes: Update the really size of the box header.

  Limitations:

  Arguments:
    fd     [in] - the record file descriptor
    offset [in] - the file offset of the header to be updated.
    size   [in] - the actually size of the header

  Returns: SUCCESS or error code

  See also: vidMP4HeaderUpdate

 **************************************************************************/
static UINT16
vidMP4HeaderSizeUpdate(
 	MP4MuxInfo_t *gpPackBitsInfo,
	FILE  *fd,
 	UINT32 offset,
 	UINT32 size
)
{
	UINT8 *pbuf, *ptr;
	SINT32 err;
	UINT32 offset1, offset2;
	UINT32 bufferSize;
	SINT32 sizeRead;
	SINT32 sizeWrite;

	pbuf = gpPackBitsInfo->gpMP4HeaderStart;
	ptr = pbuf;

	bufferSize = HEADER_BUF_SIZE;
	offset1 = (offset / 512) * 512;
	offset2 = offset % 512;

	err = fseek(fd, offset1, SEEK_SET);
	if ( err == -1 ) {
	    return FAIL;
	}
	err = ftell(fd);
	if ((UINT32)err != offset1 )
	{
		return FAIL;
	}	
	sizeRead = fread(pbuf, 1, bufferSize, fd);
	if (sizeRead <= 0)
	{
		return FAIL;
	}

	ptr = pbuf + offset2;
	LWRITE32_AND_MOVE(ptr, size); 

	err = fseek(fd, offset1, SEEK_SET);
	if ( err == -1 ) {
	    return FAIL;
	}
	err = ftell(fd);
	if ((UINT32)err != offset1 )
	{
		return FAIL;
	}	
	
	sizeWrite = fwrite(pbuf, 1, sizeRead, fd);
	if (sizeWrite <= 0) 
	{
		return FAIL;
	}

	return SUCCESS;
}


/**************************************************************************

  Function Name:

  Purposes:

  Limitations:

  Arguments:

  Returns:

  See also:

 **************************************************************************/
static UINT16
vidMP4HeaderUpdate(
	MP4MuxInfo_t *gpPackBitsInfo,
	FILE   *fd,
	UINT32 *pFileSize
)
{
	UINT8* pBuf;
	UINT32 i;
	UINT32 trakID;
	UINT32 ftypBoxSize, mvhdBoxSize, tkhdBoxSize, mdhdBoxSize;
	UINT32 hdlrBoxSize, v_s_mhdBoxSize, drefBoxSize;
	UINT32 moovHdrSize, trakHdrSize, mdiaHdrSize, minfHdrSize, dinfHdrSize, hdlrBoxSize2;
	UINT32 mdatSize = 0, moovSize = 0, trakSize, mdiaSize, minfSize, stblSize;
	UINT32 offset = 0;
	UINT32 audTrakOffset = 0;
	UINT32 added = 0;
	SINT32 err;
	UINT32 sizeAdd;

	pBuf = gpPackBitsInfo->gpMP4HeaderStart;
	memset(gpPackBitsInfo->gpMP4HeaderStart,0,HEADER_BUF_SIZE);
	sizeAdd = vidMP4BoxAdd(pBuf, 0x00, "moov"); 		/* Add moov container box */
	pBuf += sizeAdd;
	moovHdrSize = sizeAdd;

	sizeAdd = vidMP4MVHDBoxAdd(gpPackBitsInfo, pBuf);					/* Add mvhd box */
	pBuf += sizeAdd;
	mvhdBoxSize = sizeAdd;

	mdatSize = gpPackBitsInfo->gAudRecSize + gpPackBitsInfo->gVidRecSize + BOX_HEADER_SIZE;
	diag_printf("mdat size: 0x%x\n", mdatSize);
	for ( i = 0; i < 2; i++ )
	{
		if ( i == 0 )
		{
			trakID = VIDEO_TRAK;
		}
		else
		{
			if (gpPackBitsInfo->audType == AUDIO_TYPE_NONE)
			{
				continue;
			}
			trakID = AUDIO_TRAK;
		}
		sizeAdd = vidMP4BoxAdd(pBuf, 0x00, "trak");		/* Add trak container box */
		pBuf += sizeAdd;
		trakHdrSize = sizeAdd;

		sizeAdd = vidMP4TKHDBoxAdd(gpPackBitsInfo, pBuf, trakID);		/* Add tkhd box */
		pBuf += sizeAdd;
		tkhdBoxSize = sizeAdd;

		sizeAdd = vidMP4BoxAdd(pBuf, 0x00, "mdia");		/* Add mdia container box */
		pBuf += sizeAdd;
		mdiaHdrSize = sizeAdd;

		sizeAdd = vidMP4MDHDBoxAdd(gpPackBitsInfo, pBuf, trakID);		/* Add mdhd box */
		pBuf += sizeAdd;
		mdhdBoxSize = sizeAdd;

		sizeAdd = vidMP4HDLRBoxAdd(gpPackBitsInfo, pBuf, trakID);		/* Add hdlr box */
		pBuf += sizeAdd;
		hdlrBoxSize = sizeAdd;

		sizeAdd = vidMP4BoxAdd(pBuf, 0x00, "minf");		/* Add minf container box */
		pBuf += sizeAdd;
		minfHdrSize = sizeAdd;

		if ( trakID == VIDEO_TRAK ) 
		{
			sizeAdd = vidMP4VMHDBoxAdd(pBuf);			/* Add vmhd box */
		}
		else
		{
			sizeAdd = vidMP4SMHDBoxAdd(pBuf);			/* Add smhd box */
		}
		pBuf += sizeAdd;
		v_s_mhdBoxSize = sizeAdd;
		
		hdlrBoxSize2 = 0;
		if (gpPackBitsInfo->mode == MODE_MOV)
		{
			sizeAdd = vidMP4HDLRBoxAdd(gpPackBitsInfo, pBuf, 0);
			pBuf += sizeAdd;
			hdlrBoxSize2 = sizeAdd;
		}

		sizeAdd = vidMP4BoxAdd(pBuf, 0x24, "dinf");		/* Add dinf container box */
		pBuf += sizeAdd;
		dinfHdrSize = sizeAdd;

		sizeAdd = vidMP4DREFBoxAdd(pBuf);				/* Add dref box */
		pBuf += sizeAdd;
		drefBoxSize = sizeAdd;

		stblSize = 0;
		sizeAdd = vidMP4BoxAdd(pBuf, 0x00, "stbl");		/* Add stbl container box */
		pBuf += sizeAdd;
		stblSize += sizeAdd;

		sizeAdd = vidMP4STSDBoxAdd(gpPackBitsInfo, pBuf, trakID);		/* Add stsd box */
		pBuf += sizeAdd;
		stblSize += sizeAdd;

		sizeAdd = vidMP4STSCBoxAdd(pBuf, trakID);		/* Add stsc box */
		pBuf += sizeAdd;
		stblSize += sizeAdd;

		/* To avoid buffer overflow, we should do flush data here */
		err = vidMP4PacketAdd(gpPackBitsInfo, fd, gpPackBitsInfo->gpMP4HeaderStart, pBuf - gpPackBitsInfo->gpMP4HeaderStart);
		memset(gpPackBitsInfo->gpMP4HeaderStart,0,pBuf - gpPackBitsInfo->gpMP4HeaderStart);
		if (err != SUCCESS)
		{
			return FAIL;
		}
		err = vidMP4PacketFlush(gpPackBitsInfo, fd, &sizeAdd);
		if (err != SUCCESS) 
		{
			return FAIL;
		}

		/* for next audio usage */
		pBuf = gpPackBitsInfo->gpMP4HeaderStart;

		/* Time of each sample  */
		err = vidMP4STTSBoxAdd(gpPackBitsInfo, fd, trakID, &sizeAdd); /* Add stts box */
		if (err != SUCCESS) 
		{
			return FAIL;
		}
		stblSize += sizeAdd;
		err = vidMP4PacketFlush(gpPackBitsInfo, fd, &sizeAdd);
		if (err != SUCCESS)
		{
			return FAIL;
		}

		/* Size of each sample  */
		err = vidMP4STSZBoxAdd(gpPackBitsInfo, fd, trakID, &sizeAdd); /* Add stsz Box, same as above */
		if (err != SUCCESS)
		{
			return FAIL;
		}
		stblSize += sizeAdd;
		err = vidMP4PacketFlush(gpPackBitsInfo, fd, &sizeAdd);
		if (err != SUCCESS) 
		{
			return FAIL;
		}

		/* Offset of each sample */
		err = vidMP4STCOBoxAdd(gpPackBitsInfo, fd, trakID, &sizeAdd); /* Add stco Box, same as above */
		if (err != SUCCESS) 
		{
			return FAIL;
		}
		stblSize += sizeAdd;
		err = vidMP4PacketFlush(gpPackBitsInfo, fd, &sizeAdd);
		if (err != SUCCESS) 
		{
			return FAIL;
		}

		if ( trakID == VIDEO_TRAK )
		{
			err = vidMP4STSSBoxAdd(gpPackBitsInfo, fd, &sizeAdd); /* Add stss Box */
			if (err != SUCCESS)
			{
				return FAIL;
			}
			stblSize += sizeAdd;
			err = vidMP4PacketFlush(gpPackBitsInfo, fd, &sizeAdd);
			if (err != SUCCESS) 
			{
				return FAIL;
			}
		}

		/* collect the size of "moov", "trak", "mdia", "minf" and "stbl" */
		ftypBoxSize = FTYP_BOX_SIZE;
		minfSize = stblSize + dinfHdrSize + drefBoxSize + v_s_mhdBoxSize + minfHdrSize + hdlrBoxSize2;
		mdiaSize = minfSize + mdhdBoxSize + hdlrBoxSize + mdiaHdrSize;
		trakSize = mdiaSize + tkhdBoxSize + trakHdrSize;
		if ( added == 0 ) 
		{
			moovSize += moovHdrSize + mvhdBoxSize;
			added = 1;
		}
		moovSize += trakSize;

		if ( trakID == VIDEO_TRAK )
		{
			offset = FTYP_BOX_SIZE + mdatSize + moovHdrSize + mvhdBoxSize;
		}
		else 
		{
			offset = audTrakOffset;
		}
		diag_printf("trak offset: 0x%x, size: 0x%x\n", offset, trakSize);
		err = vidMP4HeaderSizeUpdate(gpPackBitsInfo, fd, offset, trakSize);
		if ( err != SUCCESS ) 
		{
			return FAIL;
		}
		offset += trakHdrSize + tkhdBoxSize;
		diag_printf("mdia offset: 0x%x, size: 0x%x\n", offset, mdiaSize);
		err = vidMP4HeaderSizeUpdate(gpPackBitsInfo, fd, offset, mdiaSize);
		if ( err != SUCCESS )
		{
			return FAIL;
		}
		offset += mdiaHdrSize + mdhdBoxSize + hdlrBoxSize;
		diag_printf("minf offset: 0x%x, size: 0x%x\n", offset, minfSize);
		err = vidMP4HeaderSizeUpdate(gpPackBitsInfo, fd, offset, minfSize);
		if ( err != SUCCESS ) 
		{
			return FAIL;
		}
		offset += hdlrBoxSize2 + minfHdrSize + v_s_mhdBoxSize + dinfHdrSize + drefBoxSize;
		diag_printf("stbl offset: 0x%x, size: 0x%x\n", offset, stblSize);
		err = vidMP4HeaderSizeUpdate(gpPackBitsInfo, fd, offset, stblSize);
		if ( err != SUCCESS )
		{
			return FAIL;
		}
		audTrakOffset = offset + stblSize;
		/* Set the file pointer to the end of the file */
		diag_printf("audTrakOffset: 0x%x\n", audTrakOffset);
		err = fseek(fd, audTrakOffset, SEEK_SET);
		if ( err == -1 ) {
		    return FAIL;
		}
		err = ftell(fd);
		if ((UINT32)err != audTrakOffset ) 
		{
			return FAIL;
		}
	}
	offset = FTYP_BOX_SIZE;
	diag_printf("mdat offset: 0x%x, size: 0x%x\n", offset, mdatSize);
	err = vidMP4HeaderSizeUpdate(gpPackBitsInfo, fd, offset, mdatSize);
	if ( err != SUCCESS ) 
	{
		return FAIL;
	}
	offset += mdatSize;
	diag_printf("moov offset: 0x%x, size: 0x%x\n", offset, moovSize);
	err = vidMP4HeaderSizeUpdate(gpPackBitsInfo, fd, offset, moovSize);
	if ( err != SUCCESS )
	{
		return FAIL;
	}

	*pFileSize = mdatSize + moovSize + FTYP_BOX_SIZE;

	return SUCCESS;
}


/**************************************************************************

  Function Name: vidMP4PackerMemSizeGet

  Purposes: Return the 3GP video packer required memory size.

  Limitations:

  Arguments:

  Returns: The required memory size of 3GP video packer

  See also: vid3GPPackerInit

 **************************************************************************/
static UINT32
vidMP4PackerMemSizeGet(
	void
)
{

	UINT32 size = 0;

	size  = (sizeof(MP4MuxInfo_t) + 0x400)&0xFFFFFC00;
	size += FREE_BOX_BUF_SIZE + VID_PACKER_PKT_SIZE + ATOM_BUF_SIZE + HEADER_BUF_SIZE;
	
	return size;
};


/**************************************************************************

  Function Name: vidMP4PackerInit

  Purposes: Initialize the 3GP video packer.

  Limitations:

  Arguments:
        addr [in] - The free address of memory for 3GP video packer to
                    manage its internal data structure.
      stream [in] - The file descriptor of the record file.
    bitsInfo [in] - The bit stream information about the record file

  Returns: SUCCESS or error code

  See also: vid3GPPackerMemSizeGet

 **************************************************************************/
static UINT32
vidMP4PackerInit(
	MP4MuxInfo_t *gpPackBitsInfo,
	UINT32 addr,
	HANDLE stream	
)
{
	UINT8* ptr;
	UINT32 i;
	UINT32 size[4];
	UINT32 header[4];
	

	gpPackBitsInfo->gPackFd = (FILE*)stream;	
	gpPackBitsInfo->gbReserved = 0;

	ptr = (UINT8*)addr;

	gpPackBitsInfo->gFreeBuf = ptr;
	ptr += FREE_BOX_BUF_SIZE;
	/* Packer buffer initial */
	gpPackBitsInfo->gpMP4PktBufStart = ptr;
	gpPackBitsInfo->gpMP4PktBufCurr = ptr;
	gpPackBitsInfo->gpMP4PktBufEnd = ptr + VID_PACKER_PKT_SIZE;
	ptr += VID_PACKER_PKT_SIZE;

	/* Video STTS/STSZ/STCO/STSS atom buffer */
	size[IDX_VID_STTS] = VID_STTS_BUF_SIZE;
	size[IDX_VID_STSZ] = VID_STSZ_BUF_SIZE;
	size[IDX_VID_STCO] = VID_STCO_BUF_SIZE;
	size[IDX_VID_STSS] = VID_STSS_BUF_SIZE;
	header[IDX_VID_STTS] = VID_STTS_HEADER_SIZE;
	header[IDX_VID_STSZ] = VID_STSZ_HEADER_SIZE;
	header[IDX_VID_STCO] = VID_STCO_HEADER_SIZE;
	header[IDX_VID_STSS] = VID_STSS_HEADER_SIZE;
	for (i = IDX_VID_STTS; i <= IDX_VID_STSS; i++)
	{
		gpPackBitsInfo->gpMP4IdxBufStart[i] = ptr;
		gpPackBitsInfo->gpMP4IdxBufCurr[i] = ptr + header[i];
		gpPackBitsInfo->gpMP4IdxBufEnd[i] = ptr + size[i];
		ptr += size[i];
	}
	/* Audio STSZ/STCO atom buffer */
	gpPackBitsInfo->gpMP4IdxBufStart[IDX_AUD_STSZ] = ptr;
	gpPackBitsInfo->gpMP4IdxBufCurr[IDX_AUD_STSZ] = ptr + AUD_STSZ_HEADER_SIZE;
	gpPackBitsInfo->gpMP4IdxBufEnd[IDX_AUD_STSZ] = ptr + AUD_STSZ_BUF_SIZE;
	ptr += AUD_STSZ_BUF_SIZE;

	gpPackBitsInfo->gpMP4IdxBufStart[IDX_AUD_STCO] = ptr;
	gpPackBitsInfo->gpMP4IdxBufCurr[IDX_AUD_STCO] = ptr + AUD_STCO_HEADER_SIZE;
	gpPackBitsInfo->gpMP4IdxBufEnd[IDX_AUD_STCO] = ptr + AUD_STCO_BUF_SIZE;
	ptr += AUD_STCO_BUF_SIZE;

	gpPackBitsInfo->gpMP4HeaderStart = ptr;

	gpPackBitsInfo->gEstSize = vidMP4HeaderInit(gpPackBitsInfo) + HEADER_BUF_SIZE;

	/* initial fw variables */
	gpPackBitsInfo->gMP4CurOff = FIRST_CHUNK_OFFSET;
	gpPackBitsInfo->gPrevSampleTime = 0;
	gpPackBitsInfo->gAudRecSize = 0;
	gpPackBitsInfo->gVidRecSize = 0;
	gpPackBitsInfo->gSTTSEntryCnt = 0;
	gpPackBitsInfo->gVidFrmNo = 0;
	gpPackBitsInfo->gAudFrmNo = 0;
	gpPackBitsInfo->gVidIFrmNo = 0;
	for ( i = 0; i < IDX_NUM; i++ )
	{
		gpPackBitsInfo->gbIdxFd[i] = 0;
	}

	gpPackBitsInfo->gPackVidDuration = 0;
	gpPackBitsInfo->gPackAudDuration = 0;
	gpPackBitsInfo->gPackAudDurationT = 0;

	diag_printf("Start/End/Curr: %x/%x/%x\n", (UINT32)gpPackBitsInfo->gpMP4PktBufStart, (UINT32)gpPackBitsInfo->gpMP4PktBufEnd, (UINT32)gpPackBitsInfo->gpMP4PktBufCurr);

	return SUCCESS;
}


/**************************************************************************

  Function Name: vidMP4VideoPack

  Purposes: Pack the record video frame into the file.

  Limitations:

  Arguments:
    gpPackBitsInfo [in] - MP4 pack bits info
	pBuf [in] - Pointer to the video frame data to be packed
    size [in] - The size of the video frame
    time [in] - The frame time (duration of this frame) of the video frame

  Returns: SUCCESS or error code.

  See also: vid3GPAudioPack

 **************************************************************************/
static UINT32
vidMP4VideoPack(
	MP4MuxInfo_t *gpPackBitsInfo,
	UINT8* pBuf,
	UINT32 size,
	UINT32 time,
	UINT8  frmType
)
{
	UINT16 err = 0;
	UINT8 *pHBuf;
	UINT32 uiSize, uiHSize;
	UINT32 uiVolStart;

	if ((gpPackBitsInfo->gEstSize + size) >= gpPackBitsInfo->maxFileSize) 
	{
        if(gpPackBitsInfo->maxFileSize < MP4_MAX_FILE_SIZE) {
            return MUX_MEM_FULL;        /*memory full*/
        }
        else {
            return MUX_FILE_SIZE_REACH; /*single file reach max size*/
        }
	}

	uiSize = size;
	pHBuf = pBuf;
	if (gpPackBitsInfo->vidExtraLen == 0 && gpPackBitsInfo->pVidExtraData == NULL) {
		if(gpPackBitsInfo->vidType == VIDEO_TYPE_MPEG4_SP) {		
			do {
				uiVolStart = LREAD32(pHBuf);
				if (uiVolStart == 0x000001B6 || uiVolStart == 0x000001B3)
				{
					break;
				} 
				else
				{
					pHBuf++;
				}
			} while (1);
			uiHSize = pHBuf - pBuf;
			uiSize -= uiHSize;
			gpPackBitsInfo->vidExtraLen = uiHSize;
			gpPackBitsInfo->pVidExtraData = osMalloc(uiHSize);
			if ( gpPackBitsInfo->pVidExtraData == NULL ) {
			    diag_printf("Error:VidExtraData malloc failed!!\n");
			}
			else {
			    memcpy(gpPackBitsInfo->pVidExtraData, pBuf, uiHSize);
			}			
		}
		else if(gpPackBitsInfo->vidType == VIDEO_TYPE_H264_BP) {   
		int i=0;
    		do {	
    			uiVolStart = LREAD32(pHBuf);
    			if (uiVolStart == (0x00000100|NALU_IDR) ||uiVolStart == (0x00000100|NALU_SLC)||(uiVolStart==0x00000001)) {
					if(i==0)
					{
						pHBuf++;
						diag_printf("I have find SPS\n");
					}	
					else if(i==1)
					{
						pHBuf++;
						diag_printf("I have get PPS\n");
					}	
					else if(i==2)
					{
						diag_printf("I get the frist frame\n");
						break;
					}	
					i++;
					} 
					else {
						pHBuf++;
					}
    		} while (1);
    		uiHSize =pHBuf - pBuf ;
    		uiSize -= uiHSize;
    		gpPackBitsInfo->vidExtraLen = uiHSize;
		diag_printf("extra size %d\n",gpPackBitsInfo->vidExtraLen);
		gpPackBitsInfo->pVidExtraData = osMalloc(uiHSize);
		if ( gpPackBitsInfo->pVidExtraData == NULL ) {
		    diag_printf("Error:VidExtraData malloc failed!!\n");
		}
		else {
		    memcpy(gpPackBitsInfo->pVidExtraData, pBuf, uiHSize);
		}
        }
	
	}

	if(gpPackBitsInfo->vidType == VIDEO_TYPE_H264_BP) {
		UINT8* pTmp = pHBuf;
   		do {
			uiVolStart = LREAD32(pHBuf);
            if( uiVolStart == (0x00000100|NALU_IDR) ||  
				uiVolStart == (0x00000100|NALU_SLC)||(uiVolStart==0x00000001)) {
     
                //lwrite(pHBuf, uiSize - 4, 4);
                UINT32 pktSize;
                lwrite(&pktSize, uiSize - 4, 4);
                if (pHBuf > pTmp)
                	err = vidMP4PacketAdd(gpPackBitsInfo, gpPackBitsInfo->gPackFd, pTmp, pHBuf - pTmp);
                err += vidMP4PacketAdd(gpPackBitsInfo, gpPackBitsInfo->gPackFd, (UINT8*)&pktSize, 4);
                err += vidMP4PacketAdd(gpPackBitsInfo, gpPackBitsInfo->gPackFd, pHBuf+4, uiSize-4-(pHBuf - pTmp));
				break;
			} 
			else {
				pHBuf++;
			}
		} while (1);    
    }
	else
		err = vidMP4PacketAdd(gpPackBitsInfo, gpPackBitsInfo->gPackFd, pHBuf, uiSize);
	if (err != SUCCESS)
	{
		return err;
	}
	gpPackBitsInfo->gEstSize += uiSize;

	err = vidMP4VidInfoProc(gpPackBitsInfo, time, uiSize, gpPackBitsInfo->gMP4CurOff, frmType);
	if (err != SUCCESS) 
	{
		diag_printf("[vidMP4VidInfoProc] err = %d\n",err);
		return err;
	}
	
	gpPackBitsInfo->gMP4CurOff += uiSize;
	gpPackBitsInfo->gVidFrmNo++;
	gpPackBitsInfo->gVidRecSize += uiSize;
	gpPackBitsInfo->gPackVidDuration += time;	

	return err;
}


/**************************************************************************

  Function Name: vidMP4AudioPack

  Purposes: Pack the record audio frame into the file.

  Limitations:

  Arguments:
    pBuf [in] - Pointer to the audio frame data to be packed
    size [in] - The size of the audio frame
    time [in] - The frame time (duration of this frame) of the audio frame

  Returns: SUCCESS or error code.

  See also: vid3GPVideoPack

 **************************************************************************/
static UINT32
vidMP4AudioPack(
	MP4MuxInfo_t *gpPackBitsInfo,
	UINT8* pBuf,
	UINT32 size,
	UINT32 time
)
{
	UINT16 err;

	if ((gpPackBitsInfo->gEstSize + size) >= gpPackBitsInfo->maxFileSize) 
	{
		if(gpPackBitsInfo->maxFileSize < MP4_MAX_FILE_SIZE) {
            return MUX_MEM_FULL;        /*memory full*/
        }
        else {
            return MUX_FILE_SIZE_REACH; /*single file reach max size*/
        }
	}

	err = vidMP4PacketAdd(gpPackBitsInfo, gpPackBitsInfo->gPackFd, pBuf, size);
	if (err != SUCCESS) 
	{
		return err;
	}
	gpPackBitsInfo->gEstSize += size;

	err = vidMP4AudInfoProc(gpPackBitsInfo, 0, size, gpPackBitsInfo->gMP4CurOff);
	if (err != SUCCESS) 
	{
		diag_printf("[vidMP4AudInfoProc] err = %d\n",err);
		return err;
	}
	
	gpPackBitsInfo->gMP4CurOff += size;
	gpPackBitsInfo->gAudFrmNo++;
	gpPackBitsInfo->gAudRecSize += size;
	gpPackBitsInfo->gPackAudDuration += time;

	return err;
}

/**************************************************************************

  Function Name: vidMP4SizePacked

  Purposes: Return the total size packed to the file by the 3GP packer.

  Limitations:

  Arguments:

  Returns: The size packed.

  See also:

 **************************************************************************/
UINT32
vidMP4SizePacked(
	MP4MuxInfo_t *gpPackBitsInfo
)
{
	return gpPackBitsInfo->gEstSize;
}


/**************************************************************************

  Function Name: vidMP4PackerFinalize

  Purposes: Notice the packer to close the pack operation since the
            application stops the record operation. All things about record
            such as update header should be done since the application will
            not use the packer for further actions.

  Limitations:

  Arguments: NONE

  Returns: SUCCESS or error code

  See also:

 **************************************************************************/
static UINT32
vidMP4PackerFinalize(
	MP4MuxInfo_t *gpPackBitsInfo
)
{

	UINT32 ret;
	UINT32 fileSize;

	if (gpPackBitsInfo->audType == AUDIO_TYPE_AMR_NB)
	{
		gpPackBitsInfo->gPackAudDurationT = gpPackBitsInfo->gAudFrmNo * 20;
	}
	if (gpPackBitsInfo->audType == AUDIO_TYPE_AAC) 
	{
		gpPackBitsInfo->gPackAudDurationT = gpPackBitsInfo->gAudFrmNo * AAC_INPUT_FRAME_LEN;
	}
	if (gpPackBitsInfo->audType == AUD_FORMAT_PCM) 
	{
		gpPackBitsInfo->gPackAudDurationT = gpPackBitsInfo->gAudRecSize/2;//gpPackBitsInfo->gAudFrmNo * 360;
	}

	ret = vidMP4HeaderUpdate(gpPackBitsInfo, gpPackBitsInfo->gPackFd, &fileSize);
        
	return ret;
}


/**************************************************************************

  Function Name: vidMP4PackerVerGet

  Purposes: Get the version information of this parser.

  Limitations:

  Arguments:
    major  [out]
      - The major number of version. Modify it if there is a big jump.
    minor  [out]
      - The minor number of version. Even for formal release and odd for
        experiment.
    serial [out]
      - The general serial number.

  Returns: None

  See also:

 **************************************************************************/
void
vidMP4PackerVerGet(
	UINT32* major,
	UINT32* minor,
	UINT32* serial
)
{
	*major = PACKER_MP4_MAJOR_VER;
	*minor = PACKER_MP4_MINOR_VER;
	*serial = PACKER_MP4_SERIAL_VER;
}

/**************************************************************************

  Function Name: vidVoVolDataAdd

  Purposes: Add the vovol header.

  Limitations:

  Arguments:
    pbuf  [out]
      - pointer to free buffer to be written.
    pInfo [in]
      - bitstream information that the vovol header should be added.

  Returns: the size of vovol header takes.

  See also:

 **************************************************************************/
UINT32 vidVoVolDataAdd(
	UINT8* pbuf,
	MP4MuxInfo_t* pInfo
)
{
	UINT32 len[11];
	UINT32 con[11];
	UINT32 buffer1 = 0;
	UINT32 tmp1, tmp2, tmp3;
	UINT32 packnum = 0;
	UINT32 i, j;

	/* 0x00000100 */
	/* VO_START_CODE(length:27,content:8) and vo_id(length:5,content:0) */
	LWRITE32_AND_MOVE( pbuf, 0x00000100 );
	/* 0x00000120 */
	/* VOL_START_CODE(length:28,content:18) and vol_id(length:4,content:0) */
	LWRITE32_AND_MOVE( pbuf, 0x00000120 );
	/* 0x00C488 */
	/* random_accessible_vol(length:1,content:0) */
	/* video_object_type_indication(length:8,content:1==>video) */
	/* is_object_layer_identifier(length:1,content:1) */
	/* visual_object_layer_ver_id(length:4,content:1) */
	/* visual_object_layer_priority(length:3,content:1) */
	/* aspect_ratio_info(length:4,content:1) */
	/* vol_control_parameter(length:1,content:0) */
	/* vol_shape(length:2,content:0==>rectangular) */
	LWRITE24_AND_MOVE( pbuf, 0x0000C488 );
	/* marker(length:1,content:1) */
	len[0] = 1;
	con[0] = 1;
	/* time increment resolution(length:16,content:30000) */
	len[1] = 16;
	con[1] = 30000;
	/* marker(length:1,content:1) */
	len[2] = 1;
	con[2] = 1;
	/* fixed_vop_rate(length:1,content:1) */
	len[3] = 1;
	con[3] = 1;
	/* (time increment resolution)/(frame rate), assume the frame rate is 15fps(length:15,content:2000) */
	len[4] = 15;
	con[4] = 1000 * (30 / pInfo->frmRate) ;
	/* marker(length:1,content:1) */
	len[5] = 1;
	con[5] = 1;

	/* width(length:13,content:MP4_HSize) */
	len[6] = 13;
	con[6] = pInfo->width;
	/* marker(length:1,content:1) */
	len[7] = 1;
	con[7] = 1;
	/* height(length:13,content:MP4_VSize) */
	len[8] = 13;
	con[8] = pInfo->height;
	/* marker(length:1,content:1) */
	len[9] = 1;
	con[9] = 1;
	/* interlaced(length:1,content:0) */
	len[10] = 1;
	con[10] = 0;
	/* pack con[0]~con[10] */
	for ( i = 0; i < 11; i++ )
	{
		tmp1 = con[i] << (32 - len[i]);
		tmp2 = 0x80000000;
		for ( j = 0; j < len[i]; j++ )
		{
			buffer1 = buffer1 << 1;
			tmp3 = tmp1 & tmp2;
			tmp3 = tmp3 >> (31-j);
			buffer1 = buffer1 | tmp3;
			tmp2 = tmp2 >> 1;
			packnum++;
			if ( packnum == 32 )
			{
				packnum = 0;
				LWRITE32_AND_MOVE( pbuf, buffer1 );
				buffer1 = 0;
			}
		}
	}
	/* OBMC_disabled(length:1,content:1) */
	/* vol_sprite_usage(length:1,content:0) */
	/* not_8_bit(length:1,content:0) */
	/* vol_quant_type(length:1,content:0) */
	/* complexity_estimation_disable(length:1,content:1) */
	/* resync_marker_disable(length:1,content:1) */
	/* data_partitioning_enabled(length:1,content:0) */
	/* scalability(length:1,content:0) */
	*pbuf++ = 0x8C;

	return 20;

}

/**
*  initial info before muxing
*  @stream: handle of stream 
*  @return the address of allocated memory
*/
SINT32 
muxMP4Open(
    HANDLE stream
)
{
	SINT32 iRet = FAIL;
	MP4MuxInfo_t *gpPackBitsInfo;
	UINT32 pTBuf;

	gpPackBitsInfo = NULL;
	pTBuf = (UINT32)gpChunkMemAlloc(vidMP4PackerMemSizeGet());
	if ( pTBuf == 0 ) {
	    return 0;
	}

	gpPackBitsInfo = (MP4MuxInfo_t *)pTBuf;
	memset(gpPackBitsInfo, 0, sizeof(MP4MuxInfo_t));
	gpPackBitsInfo->vidType = VIDEO_TYPE_H264_BP;
	gpPackBitsInfo->mode = MODE_MP4;
	pTBuf += (sizeof(MP4MuxInfo_t) + 0x400)&0xFFFFFC00;

	iRet = (SINT32)vidMP4PackerInit(gpPackBitsInfo, pTBuf, stream);

	return (SINT32)(gpPackBitsInfo);
}

SINT32 
muxMOVOpen(
    HANDLE stream
)
{
	SINT32 iRet = FAIL;
	MP4MuxInfo_t *gpPackBitsInfo;
	UINT32 pTBuf;

	gpPackBitsInfo = NULL;
	pTBuf = (UINT32)gpChunkMemAlloc(vidMP4PackerMemSizeGet());
	if ( pTBuf == 0 ) {
	    return 0;
	}

	gpPackBitsInfo = (MP4MuxInfo_t *)pTBuf;
	memset(gpPackBitsInfo, 0, sizeof(MP4MuxInfo_t));
	gpPackBitsInfo->vidType = VIDEO_TYPE_H264_BP;
	gpPackBitsInfo->mode = MODE_MOV;
	pTBuf += (sizeof(MP4MuxInfo_t) + 0x400)&0xFFFFFC00;

	iRet = (SINT32)vidMP4PackerInit(gpPackBitsInfo, pTBuf, stream);

	return (SINT32)(gpPackBitsInfo);
}

SINT32 
muxMP4Close(
    SINT32 hd
)
{    
    SINT32 iRet = FAIL;
	MP4MuxInfo_t *gpPackBitsInfo = (MP4MuxInfo_t *) hd;

	iRet = vidMP4PackerFinalize(gpPackBitsInfo);
	if ( gpPackBitsInfo != NULL) {
	    gpChunkMemFree(gpPackBitsInfo);
		gpPackBitsInfo = NULL;
	}	
    
    return iRet;
}

/**
*  pack video and audio frame data
*  @hd: the address of memory begin
*  @pkt: info of packeted data
*  @type: sign of video or audio
*  @return FAIL or SUCCESS
*/
SINT32 
muxMP4Pack(
    SINT32 hd, 
    gpMuxPkt_t *pkt, 
    gpEsType_t type
)
{   
    UINT32 uiRet = SUCCESS;
	MP4MuxInfo_t *gpPackBitsInfo = (MP4MuxInfo_t *) hd;

	if ( type == GP_ES_TYPE_VIDEO ) {
	    uiRet = vidMP4VideoPack(gpPackBitsInfo, pkt->data, pkt->size, pkt->pts, pkt->keyFrame);
	}
	else  if ( type == GP_ES_TYPE_AUDIO) {
	    uiRet = vidMP4AudioPack(gpPackBitsInfo, pkt->data, pkt->size, pkt->pts);
	}
    
    return uiRet ;
}
/**
*  initial param of record
*  @hd: the address of memory begin
*  @param: param
*  @value: the value of param
*  @return FAIL or SUCCESS
*/
SINT32 
muxMP4Set( 
    SINT32 hd, 
    UINT32 param, 
    UINT32 value
)
{
	UINT8 *pNameBuf;
	MP4MuxInfo_t *gpPackBitsInfo = (MP4MuxInfo_t *) hd;
	
	switch ( param ) {
	case MUX_MAX_SIZE:
	    gpPackBitsInfo->maxFileSize = value;
	    break;
	case MUX_AUD_TYPE:
	    gpPackBitsInfo->audType = value;
	    break;
	case MUX_VID_TYPE:
	    gpPackBitsInfo->vidType = value;
	    break;
	case MUX_WIDTH:
	    gpPackBitsInfo->width   = value;
	    break;
	case MUX_HEIGHT:
	    gpPackBitsInfo->height  = value;
	    break;
	case MUX_FRMRATE:
	    gpPackBitsInfo->frmRate = value;
	    break;
	case MUX_DPEN:
	    gpPackBitsInfo->dpEn    = value;
	    break;
	case MUX_AUDSR:
	    gpPackBitsInfo->audSR   = value;
	    break;
	case MUX_AUD_CHANNEL:
	    gpPackBitsInfo->audChannel = value;
	    break;
	case MUX_AUD_BIT:
	    gpPackBitsInfo->audBits = value;
	    break;
	case MUX_PATH_TEMP_FILE:
		pNameBuf = (UINT8 *)value;
		strcpy((char *)gpPackBitsInfo->tempFile, (char *)pNameBuf);
	    break;
	case MUX_CLUSTER_SIZE:
		gpPackBitsInfo->clusterSize = value;
		break;
	case MUX_PARAM_END:
	    
	    break;
	default:
	    
	    break;
	}
	return SUCCESS;   
}
/**
*  get param info during muxing
*  @hd: the address of memory begin
*  @param: param
*  @value: the address of value
*  @return FAIL or SUCCESS
*/
SINT32 
muxMP4Get(
    SINT32 hd, 
    UINT32 param, 
    UINT32 *value
)
{
	MP4MuxInfo_t *gpPackBitsInfo = (MP4MuxInfo_t *) hd;

	switch ( param ) {
	case MUX_MAX_SIZE:
	    *value = gpPackBitsInfo->maxFileSize;
	    break;
	case MUX_AUD_TYPE:
	    *value = gpPackBitsInfo->audType;
	    break;
	case MUX_VID_TYPE:
	    *value = gpPackBitsInfo->vidType;
	    break;
	case MUX_WIDTH:
	    *value = gpPackBitsInfo->width;
	    break;
	case MUX_HEIGHT:
	    *value = gpPackBitsInfo->height;
	    break;
	case MUX_FRMRATE:
	    *value = gpPackBitsInfo->frmRate;
	    break;
	case MUX_DPEN:
	    *value = gpPackBitsInfo->dpEn;
	    break;
	case MUX_AUDSR:
	    *value = gpPackBitsInfo->audSR;
	    break;
	case MUX_AUD_CHANNEL:
	    *value = gpPackBitsInfo->audChannel;
	    break;
	case MUX_AUD_BIT:
	    *value = gpPackBitsInfo->audBits;
	    break;	
	case MUX_CLUSTER_SIZE:
		*value = gpPackBitsInfo->clusterSize;
		break;
	case MUX_PARAM_END:
	    
	    break;
	default:
	    
	    break;
	}
	
	return SUCCESS;
}

