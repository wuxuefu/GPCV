/***************************************************************************
 * Name: gp_avi_mux.c
 *
 * Purpose:
 *
 * Developer:
 *     liuliang, 2010-12-27
 *
 * Copyright (c) 2010-2011 by Sunplus mMobile Inc.
 ***************************************************************************/
     
/***************************************************************************
 * Header Files
 ***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "dv_used.h"
#include "gp_avcodec.h"
#include "avi_mux.h"
//#include "chunkmem_service.h"
#include <chunkmem.h>

#define AVI_PAD_SIZE   2
#define MP3_AVG_BPS_   8

#ifdef USE_SYSTEM_FS
#define fread(buf, size, count, fd)  read((SINT32)(fd), buf, (size)*(count))
#define fwrite(buf, size, count, fd) write((SINT32)(fd), buf, (size)*(count))
#define fseek(fd, off, pos)          lseek((SINT32)(fd), off, pos)
#define ftell(fd)                    lseek((SINT32)(fd), 0, SEEK_CUR)
#define fclose(fd)                   close((SINT32)(fd))    
#endif

/***************************************************************************
 * Static Variables
 ***************************************************************************/

/***************************************************************************
 * Import Functions
 ***************************************************************************/

/***************************************************************************
 * Import Variables
 ***************************************************************************/

/***************************************************************************
 * Export Functions
 ***************************************************************************/

/***************************************************************************
 * Export Variables
 ***************************************************************************/

/***************************************************************************
 * Global Definitions
 ***************************************************************************/

struct gpMux_s AVI_packer={
	.init= NULL,
	.uninit =NULL,
	.open=muxAviOpen,
	.close = muxAviClose,
	.pack = muxAviPack,
	.set = muxAviSet,
	.get =muxAviGet,
};

/**
*  Add video and audio frame data into stream
*  @pkt: info of packeted data
*  @hd: the address of memory begin
*  @uiFlag: sign of video or audio
*  @return FAIL or SUCCESS
*/
static UINT32 
aviAddMovi(
    gpMuxPkt_t *pkt,
    SINT32 hd,
    UINT32 uiFlag
)
{
    memAddr_t *pMemAddr;
    AviPacker_t *pAviPacker;
    avi_movi_entry_t stMovi;
    UINT8 *pMovi, *pTempBuf;    
    UINT32 RetErr = SUCCESS;    
    UINT32 uiPadSize = 0;
    
    pMemAddr = (memAddr_t *)(hd);
    /*in linux server is a 64 bit address*/
    pAviPacker = (AviPacker_t *)(pMemAddr->aviPackerMemAddr);
    pMovi = (UINT8 *)(pMemAddr->aviMoviMemAddr);    
    
    uiPadSize = pkt->size & 0x01;    
    stMovi.ckid = REVERSE_BUF32(uiFlag);
    stMovi.dwSize = pkt->size;    
    
    memcpy(pMovi + pAviPacker->uiMoviSize, (UINT8 *)&stMovi, sizeof(avi_movi_entry_t));
    pAviPacker->uiMoviSize += sizeof(avi_movi_entry_t);
    if(pkt->size > 0) {        
        memcpy(pMovi + pAviPacker->uiMoviSize, pkt->data, pkt->size);    
        pAviPacker->uiMoviSize += pkt->size;
        if( uiPadSize > 0) {            
            memset(pMovi + pAviPacker->uiMoviSize, 0, uiPadSize);
            pAviPacker->uiMoviSize += uiPadSize;
        }
    }
    
    if ( pAviPacker->uiMoviSize > AVI_MUX_WRITE_SIZE ) {
        for ( pTempBuf = pMovi; 
              pAviPacker->uiMoviSize > AVI_MUX_WRITE_SIZE; 
              pAviPacker->uiMoviSize -= AVI_MUX_WRITE_SIZE ) {
    
            RetErr = fwrite((char *)pTempBuf, 1, AVI_MUX_WRITE_SIZE, (FILE *)pAviPacker->pstMovi);
            if(RetErr != AVI_MUX_WRITE_SIZE) {   
                return FAIL;
            }
            pTempBuf += AVI_MUX_WRITE_SIZE;
        }
        memcpy(pMovi, pTempBuf, pAviPacker->uiMoviSize);
    }    
    
    pAviPacker->uiNowPacktedSize += ( pkt->size
                                    + sizeof(avi_movi_entry_t)
                                    + uiPadSize);    
    
    return SUCCESS;
}

/**
*  Add avi index entry into stream
*  @pkt: info of packeted data
*  @hd: the address of memory begin
*  @uiFlag: keyframe or not
*  @return FAIL or SUCCESS 
*/
static UINT32 
aviAddIndex(
    gpMuxPkt_t *pkt,
    SINT32 hd,
    UINT32 uiFlag
)
{
    avi_index_entry_t stIndex;
    memAddr_t *pMemAddr;
    AviPacker_t *pAviPacker;
    UINT8 *pIndex;
    UINT32 RetErr,uiPadSize = 0 ;
    
    pMemAddr = (memAddr_t *)(hd);
    pAviPacker = (AviPacker_t *)(pMemAddr->aviPackerMemAddr);
    pIndex = (UINT8 *)(pMemAddr->aviIndexMemAddr);
    uiPadSize = pkt->size & 0x01;    
    
    stIndex.ckid = REVERSE_BUF32( uiFlag );
    if( ( uiFlag == STR00_DC )&&( pkt->keyFrame == AVI_VID_I_FRM  ) ) {
        stIndex.dwFlags = AVIIF_KEYFRAME;
    }else {
        stIndex.dwFlags = 0;//AVIIF_KEYFRAME;
    }
    stIndex.dwChunkLength =  pkt->size;
    stIndex.dwChunkOffset = pAviPacker->uiNowMoviOffset;
    if ((pAviPacker->uiIndexSize + sizeof(stIndex)) > AVI_INDEX_SIZE) {

        RetErr = fwrite((char *)pIndex, 1, pAviPacker->uiIndexSize, (FILE *)pAviPacker->pstIndex);
        if(RetErr != pAviPacker->uiIndexSize) {   
            return FAIL;
        }

        pAviPacker->uiIndexSize = 0;
    }
    
    memcpy(pIndex + pAviPacker->uiIndexSize, (UINT8*)&stIndex, sizeof(stIndex));
    pAviPacker->uiIndexSize += sizeof(stIndex);
    pAviPacker->uiNowPacktedSize += AVI_INDEX_ENTRY_SIZE;
    pAviPacker->uiNowMoviOffset += (pkt->size
                                  + sizeof(avi_movi_entry_t)
                                  + uiPadSize);
    
    return SUCCESS;
}

/**
*  Add avi header info into stream
*  @pHdr: header info 
*  @pstPacker: used info during packeting data
*  @pstInfo: common info during packeting data
*  @return SUCCESS
*/
static UINT32 
AviAddHeader(
    avi_header_t *pHdr,
    AviPacker_t *pstPacker,
    UINT32 *pstInfo
)
{
    UINT32 uiTimeSec;
    UINT32 uiTimeMs;
    UINT64 tmpDwRate;
    /*RIFF Chunk*/
    pHdr->stRiffList.chunkId = ( AVI_RIFF );
    pHdr->stRiffList.chunkSize = pstPacker->uiNowPacktedSize - 8;
    pHdr->stRiffList.chunkData = ( AVI_AVI );
    /*header list*/
    pHdr->stHdrlList.chunkId = (AVI_LIST);
    /*RIFF Chunk,Header list,movi list(12 8 12)*/
    pHdr->stHdrlList.chunkSize = sizeof(avi_header_t) - 32 ;
    pHdr->stHdrlList.chunkData = ( AVI_hdrl );
    /*avi main header chunk*/
    pHdr->stHdrChunk.chunkId = (AVI_avih);
    pHdr->stHdrChunk.chunkSize = sizeof(main_avi_header_t);
    /*avi main header data*/
    pHdr->stMainHdr.dwFlags = AVIF_HASINDEX|AVIF_WASCAPTUREFILE;
    pHdr->stMainHdr.dwInitialFrames = 0;
    pHdr->stMainHdr.dwStart = 0;
    pHdr->stMainHdr.dwScale = 0;
    pHdr->stMainHdr.dwStreams = 0;    

    if( pstInfo[MUX_VID_TYPE] != VID_UNKNOWN) {
        pHdr->stMainHdr.dwHeight = pstInfo[MUX_HEIGHT];
        pHdr->stMainHdr.dwWidth = pstInfo[MUX_WIDTH];
        /*interval between two video frame*/
        //pHdr->stMainHdr.dwMicroSecPerFrame = 1000000/pstInfo->uiFrmRate;
        pHdr->stMainHdr.dwMicroSecPerFrame = /*(float)*/pstPacker->uiVidMsTime
                                                  / pstPacker->uiTotalFrame;
        
        /*we just compution video rate*/        
        pHdr->stMainHdr.dwMaxBytesPerSec = pstPacker->uiNowMaxFrameSize
                                         * pstInfo[MUX_FRMRATE];
        /*the buffer can contain the max block*/        
        pHdr->stMainHdr.dwSuggestedBufferSize = pstPacker->uiNowMaxFrameSize;
        pHdr->stMainHdr.dwTotalFrames = pstPacker->uiTotalFrame;/* video frame*/
        pHdr->stMainHdr.dwStreams ++;
    }
    if(pstInfo[MUX_AUD_TYPE] != AUD_FORMAT_UNKNOWN ) {
        pHdr->stMainHdr.dwStreams ++;
    }    

    /*avi video list*/
    if(pstInfo[MUX_VID_TYPE] != VID_UNKNOWN) {    
        str_list1_t *plist = &(pHdr->unStrList1.stStrList1);
        /*string list chunk*/
        plist->stStrlList1.chunkId = (AVI_LIST);
        plist->stStrlList1.chunkSize = sizeof(str_list1_t) - 8;
        plist->stStrlList1.chunkData = (AVI_strl);
        plist->stStrhChunk1.chunkId = ( AVI_strh);
        plist->stStrhChunk1.chunkSize =sizeof(avi_stream_header_t);
        plist->stStrHeader.dwFlags = 0;
        plist->stStrHeader.dwInitialFrames = 0;
        plist->stStrHeader.dwStart = 0;
        plist->stStrHeader.dwScale = 1000;        
        tmpDwRate =(UINT64)((UINT64)(pstPacker->uiTotalFrame
                                   * plist->stStrHeader.dwScale)
                                   * 1000)  
                           / /*(double)*/ (pstPacker->uiVidMsTime);        
        plist->stStrHeader.dwRate =  (UINT32)tmpDwRate;
        plist->stStrHeader.dwLength = pstPacker->uiTotalFrame;
        plist->stStrHeader.dwQuality = 10000;
        plist->stStrHeader.dwSuggestedBufferSize = pstPacker->uiNowMaxFrameSize;
        plist->stStrHeader.fccType = (AVI_vids);
        plist->stStrHeader.fccHandler = (PRODUCT_PRETAG);
        plist->stStrHeader.dwSampleSize = 0;
        plist->stStrHeader.wRcFrameX0 = 0;    // Left
        plist->stStrHeader.wRcFrameY0 = 0;    // Top
        plist->stStrHeader.wRcFrameX1 = (UINT16)pstInfo[MUX_WIDTH];     // Right
        plist->stStrHeader.wRcFrameY1 = (UINT16)pstInfo[MUX_HEIGHT];  // bottom    
        /*str format chunk*/
        plist->stStrfChunk1.chunkId = (AVI_strf);
        plist->stStrfChunk1.chunkSize = sizeof(bitmapinfoheader_t);
        /*the size of the bmih*/
        plist->stVidFormat.biSize = sizeof(bitmapinfoheader_t);
        /* how much bits being used to represent a pixel*/
        plist->stVidFormat.biBitCount = 16;
        plist->stVidFormat.biClrImportant = 0;
        plist->stVidFormat.biClrUsed = 0;
        plist->stVidFormat.biCompression = VID_H264;//pstInfo[MUX_VID_TYPE];
        plist->stVidFormat.biHeight = pstInfo[MUX_HEIGHT];
        plist->stVidFormat.biWidth = pstInfo[MUX_WIDTH];
        /* this value must been setting to 1*/
        plist->stVidFormat.biPlanes = 0x01;
        /*the image size in bytes*/
        plist->stVidFormat.biSizeImage = pstInfo[MUX_HEIGHT]*pstInfo[MUX_WIDTH]*3/2;
        plist->stVidFormat.biXPelsPerMeter = 0;
        plist->stVidFormat.biYPelsPerMeter = 0;
    }
    else {    
        str_stuffed1_t *pChunk = &(pHdr->unStrList1.stStuffed1);
        pChunk->stChunk.chunkId = (AVI_JUNK);
        pChunk->stChunk.chunkSize = sizeof(str_stuffed1_t) - 8;
    }

    if(pstInfo[MUX_AUD_TYPE] != AUD_FORMAT_UNKNOWN ) {
        UINT32 uiBlock;        
        str_list2_t * plist2 = &(pHdr->unStrList2.stStrList2);
        plist2->stStrlList2.chunkId = ( AVI_LIST);
        plist2->stStrlList2 .chunkSize =sizeof(str_list2_t) -8;
        plist2->stStrlList2.chunkData = ( AVI_strl);
        plist2->stStrhChunk2.chunkId = ( AVI_strh);
        plist2->stStrhChunk2.chunkSize = sizeof(avi_stream_header_t);
        /*common audio information*/
        plist2->stStrHeader.dwFlags = 0;
        plist2->stStrHeader.dwInitialFrames = 0;
        plist2->stStrHeader.dwStart = 0;
        plist2->stStrHeader.dwQuality = 0xFFFFFFFF;
        plist2->stStrHeader.dwSuggestedBufferSize = 8000;
        plist2->stStrHeader.fccType = (AVI_auds);
        plist2->stStrHeader.fccHandler = 0;
        
        plist2->stStrfChunk2.chunkId = (AVI_strf);
        plist2->stStrfChunk2.chunkSize = sizeof(wav_format_t);
        plist2->stAudFormat.nChannels = pstInfo[MUX_AUD_CHANNEL];
        plist2->stAudFormat.nSamplesPerSec = pstInfo[MUX_AUDSR];
        plist2->stAudFormat.wBitsPerSample = pstInfo[MUX_AUD_BIT];
        switch( pstInfo[MUX_AUD_TYPE]) {
        case AUD_FORMAT_PCM :/*PCM*/                
            uiBlock = (UINT16)((pstInfo[MUX_AUD_BIT]>> 3)
                              * pstInfo[MUX_AUD_CHANNEL]);
            plist2->stAudFormat.nBlockAlign = (UINT16)uiBlock;
            plist2->stStrHeader.dwSampleSize = (UINT16)uiBlock;
            plist2->stAudFormat.wFormatTag = AUD_FORMAT_PCM;
            plist2->stAudFormat.wSamplesPerBlock = 1;/*PCM has no this field*/
            plist2->stAudFormat.cbSize = 0x00;/*no extended data*/
            plist2->stAudFormat.nAvgBytesPerSec = pstInfo[MUX_AUDSR] * uiBlock;
            break;
        case AUD_FORMAT_ALAW:
        case AUD_FORMAT_MULAW:
        	uiBlock = (UINT16)((pstInfo[MUX_AUD_BIT]>> 3)
                              * pstInfo[MUX_AUD_CHANNEL]);
            plist2->stAudFormat.nBlockAlign = (UINT16)uiBlock;
            plist2->stStrHeader.dwSampleSize = (UINT16)uiBlock;
            plist2->stAudFormat.wFormatTag = pstInfo[MUX_AUD_TYPE];
            plist2->stAudFormat.wSamplesPerBlock = 1;/*PCM has no this field*/
            plist2->stAudFormat.cbSize = 0x00;/*no extended data*/
            plist2->stAudFormat.nAvgBytesPerSec = pstInfo[MUX_AUDSR] * uiBlock;
            break;
        case AUD_FORMAT_G726:
            plist2->stAudFormat.nBlockAlign = 1;
            plist2->stStrHeader.dwSampleSize = 1;
            plist2->stAudFormat.wFormatTag = pstInfo[MUX_AUD_TYPE];
            plist2->stAudFormat.wSamplesPerBlock = 1;/*PCM has no this field*/
            plist2->stAudFormat.cbSize = 0x00;/*no extended data*/
            plist2->stAudFormat.nAvgBytesPerSec = (pstInfo[MUX_AUDSR] * pstInfo[MUX_AUD_BIT]) >> 3;
            break;
        case AUD_FORMAT_IMA_ADPCM:/*AD_PCM*/
            uiBlock = pstInfo[MUX_AUDSR] /11000;
            if (0 == uiBlock) {
                uiBlock = 1;  // for sample rate 8000
            }
            uiBlock = uiBlock * pstInfo[MUX_AUD_CHANNEL] * 256;
            plist2->stAudFormat.nBlockAlign = (UINT16)uiBlock;
            //plist2->stStrHeader.dwSampleSize = 0x0200 /* pstInfo->uiAudChannel*/;
            plist2->stStrHeader.dwSampleSize = uiBlock;            
            plist2->stAudFormat.wFormatTag = AUD_FORMAT_IMA_ADPCM;
            plist2->stAudFormat.cbSize = 0x02;
            plist2->stAudFormat.wSamplesPerBlock = 
                                    (UINT16)((pstInfo[MUX_AUD_CHANNEL] 
                                           + (plist2->stAudFormat.nBlockAlign
                                            - 4 * pstInfo[MUX_AUD_CHANNEL]) * 2)
                                            / pstInfo[MUX_AUD_CHANNEL]);
            plist2->stAudFormat.nAvgBytesPerSec = pstInfo[MUX_AUDSR]
                                                * plist2->stAudFormat.nBlockAlign
                                                / plist2->stAudFormat.wSamplesPerBlock;
            plist2->stAudFormat.wBitsPerSample = 0x04;
            break;
        case AUD_FORMAT_MPEGLAYER3:
            uiBlock = (UINT16)((pstInfo[MUX_AUD_BIT]>> 3) * pstInfo[MUX_AUD_CHANNEL]);
            plist2->stAudFormat.nBlockAlign = (UINT16)uiBlock;
            plist2->stStrHeader.dwSampleSize = (UINT16)uiBlock;
            plist2->stAudFormat.wFormatTag = AUD_FORMAT_MPEGLAYER3;
            plist2->stAudFormat.nAvgBytesPerSec = 
                (pstInfo[MUX_AUD_BIT]/MP3_AVG_BPS_) * 1000;/*32kbps*/
            break;
        default : 
            break;/*other we not support*/
        }
        plist2->stStrHeader.dwScale = plist2->stAudFormat.nBlockAlign;
        plist2->stStrHeader.dwRate = plist2->stAudFormat.nAvgBytesPerSec;        
        uiTimeSec = pstPacker->uiVidMsTime/*pstPacker->uiAudMsTime*/ /1000;
        uiTimeMs = pstPacker->uiVidMsTime/*pstPacker->uiAudMsTime*/ %1000;
        plist2->stStrHeader.dwLength = uiTimeSec
                                     * plist2->stStrHeader.dwRate
                                     / /*(float)*/plist2->stStrHeader.dwScale;
        plist2->stStrHeader.dwLength += (uiTimeMs
                                       * plist2->stStrHeader.dwRate
                                       / /*(float)*/plist2->stStrHeader.dwScale
                                       / 1000);
    }
    else {
        str_stuffed2_t *pChunk2 = &(pHdr->unStrList2.stStuffed2);
        pChunk2->stChunk.chunkId = (AVI_JUNK);
        pChunk2->stChunk.chunkSize = sizeof(str_stuffed2_t) - 8;
    }

    pHdr->stMovList.chunkId = ( AVI_LIST);
    pHdr->stMovList.chunkSize = pstPacker->uiNowMoviOffset ;
    pHdr->stMovList.chunkData = ( AVI_movi);
    return SUCCESS;
}

/**
*  initial info before muxing
*  @out: handle of stream 
*  @return the address of allocated memory
*/
SINT32 
muxAviOpen(
    HANDLE out
) 
{
    memAddr_t *pMemAddr;
    AviPacker_t *pAviPacker;
    UINT8 *pMovi;
    UINT8 *pIndex;
    UINT32 memAddr;
    UINT32 uiMoviOffset;
    chunk_t stChunk;

    memAddr = (UINT32)gpChunkMemAlloc(sizeof(memAddr_t)
                          + sizeof(AviPacker_t)
                          + 4 * AVI_MUX_PARAM_END
                          + AVI_MOVI_SIZE
                          + AVI_INDEX_SIZE);
    if (memAddr == 0) {
        diag_printf("[muxAviOpen]malloc memory fail\n");
        return 0;
    }
    memset((UINT8 *)(memAddr), 0, sizeof(memAddr_t)
                                + sizeof(AviPacker_t)
                                + 4 * AVI_MUX_PARAM_END);
    pMemAddr = (memAddr_t *)(memAddr);
    pMemAddr->baseAddr = memAddr;
    pMemAddr->aviPackerMemAddr = memAddr + sizeof(memAddr_t);
    pMemAddr->aviParamMemAddr  = memAddr
                               + sizeof(memAddr_t)
                               + sizeof(AviPacker_t);
    pMemAddr->aviMoviMemAddr   = memAddr
                               + sizeof(memAddr_t)
                               + sizeof(AviPacker_t)
                               + 4 * AVI_MUX_PARAM_END;
    pMemAddr->aviIndexMemAddr  = memAddr
                               + sizeof(memAddr_t)
                               + sizeof(AviPacker_t)
                               + 4 * AVI_MUX_PARAM_END
                               + AVI_MOVI_SIZE;
    pAviPacker = (AviPacker_t *)(pMemAddr->aviPackerMemAddr);
    pAviPacker->pstMovi = (void *)out;    

    /*initial aviheader contain*/
    pMovi = (UINT8 *)(pMemAddr->aviMoviMemAddr);
    memset(pMovi, 0, AVI_MOVI_SIZE);
    uiMoviOffset = sizeof(avi_header_t);
    memset(pMovi, 0, uiMoviOffset);
    pAviPacker->uiMoviSize += uiMoviOffset;

    /*init idx*/
    pIndex = (UINT8 *)(pMemAddr->aviIndexMemAddr);
    memset(pIndex, 0, AVI_INDEX_SIZE);
    stChunk.chunkId = AVI_idx1;
    stChunk.chunkSize = 0;
    memcpy(pIndex, (UINT8*)&stChunk, sizeof(chunk_t));
    pAviPacker->uiIndexSize += sizeof(chunk_t);

    /*init info of pAviPacker*/
    pAviPacker->uiNowMoviOffset = 0x04;
    pAviPacker->uiNowPacktedSize = uiMoviOffset;
    pAviPacker->uiNowPacktedSize +=sizeof(chunk_t);
    
    return memAddr;
}

/**
*  uninitial info after muxing
*  @hd: the adderss of memory begin
*  @return FAIL or SUCCESS
*/
SINT32 
muxAviClose(
    SINT32 hd
)
{
    memAddr_t *pMemAddr;
    AviPacker_t *pAviPacker;
    UINT32 *pParamArry;
    UINT8 *pMovi;
    UINT8 *pIndex;
    avi_header_t *pHdr;
    UINT32 uiIndexSize,size;
    SINT32 iRet = SUCCESS;
    
    pMemAddr   = (memAddr_t *)(hd);
    pAviPacker = (AviPacker_t *)(pMemAddr->aviPackerMemAddr);
    pMovi      = (UINT8 *)(pMemAddr->aviMoviMemAddr);
    pIndex     = (UINT8 *)(pMemAddr->aviIndexMemAddr);
    pParamArry = (UINT32 *)(pMemAddr->aviParamMemAddr);

    if(pAviPacker->pstIndex == NULL || pAviPacker->pstMovi == NULL) {
        iRet = FAIL;
        goto _AVI_CLOSE_END;
    }
    
    if (pAviPacker->uiMoviSize > 0) {
        iRet = fwrite((char *)pMovi, 1, pAviPacker->uiMoviSize, (FILE *)pAviPacker->pstMovi);
        if (iRet != pAviPacker->uiMoviSize) {
            iRet = FAIL;
            goto _AVI_CLOSE_END;
        }
    }
    
    if (pAviPacker->uiIndexSize > 0) {
        iRet = fwrite((char *)pIndex, 1, pAviPacker->uiIndexSize, (FILE *)pAviPacker->pstIndex);
        if (iRet != pAviPacker->uiIndexSize) {
            iRet = FAIL;
            goto _AVI_CLOSE_END;
        }
    }

    pHdr = (avi_header_t*)(pMemAddr->aviMoviMemAddr);
    AviAddHeader(pHdr, pAviPacker, pParamArry);

    /*update index file*/
    iRet = ftell((FILE *)pAviPacker->pstIndex);
    if (iRet < 0) {
        iRet = FAIL;
        goto _AVI_CLOSE_END;
    } 
    else {
        uiIndexSize = iRet - 8;
    }
    
    iRet = fseek((FILE *)pAviPacker->pstIndex, 4, SEEK_SET);
    if(iRet == -1) {    
        iRet = FAIL;
        goto _AVI_CLOSE_END;
    }
    
    iRet = fwrite((UINT8*)&uiIndexSize, 1, 4, (FILE *)pAviPacker->pstIndex);
    if(iRet != 4) {    
        iRet = FAIL;
        goto _AVI_CLOSE_END;
    }

    /*update movi file*/
    iRet = fseek((FILE *)pAviPacker->pstMovi, 0, SEEK_SET);
    if(iRet == -1) {    
        iRet = FAIL;
        goto _AVI_CLOSE_END;
    }
    
    iRet = fwrite((UINT8*)pHdr, 1, sizeof(avi_header_t), (FILE *)pAviPacker->pstMovi);
    if(iRet != sizeof(avi_header_t)) {    
        iRet = FAIL;
        goto _AVI_CLOSE_END;
    }

    /*let index file into movi stream*/
    iRet = fclose((FILE *)pAviPacker->pstIndex);
    if (iRet != 0) {
        iRet = FAIL;
        goto _AVI_CLOSE_END;
    }
    pAviPacker->pstIndex = NULL;
    #ifdef USE_SYSTEM_FS
    pAviPacker->pstIndex = (void *)open((char *)pAviPacker->pTempName, O_CREAT | O_RDWR);
    if(pAviPacker->pstIndex == (void *)(-1)) {
        iRet = FAIL;
        goto _AVI_CLOSE_END;
    }
    #else
    pAviPacker->pstIndex = (void *)fopen((char *)pAviPacker->pTempName, "rb");
    if (pAviPacker->pstIndex == NULL) {
        iRet = FAIL;
        goto _AVI_CLOSE_END;
    }
    #endif
        
    fseek((FILE *)pAviPacker->pstMovi, 0, SEEK_END);
    do {
        size = fread(pIndex, 1, AVI_INDEX_SIZE, (FILE *)pAviPacker->pstIndex);
        if (size <= 0) {
            break;
        }        

        iRet = fwrite(pIndex, 1, size, (FILE *)pAviPacker->pstMovi);
        if(iRet != size) {    
            iRet = FAIL;
            goto _AVI_CLOSE_END;
        }
    } while (size > 0);

    _AVI_CLOSE_END:
    /*uninit malloc and tempfile*/
    if(pAviPacker->pstIndex != NULL) {
        iRet = fclose((FILE *)pAviPacker->pstIndex);
        if (iRet != 0) {
            iRet = FAIL;
        }
        pAviPacker->pstIndex = NULL;
    }
    
    if ( pAviPacker->pTempName != NULL ) {
        iRet = remove((char *)pAviPacker->pTempName);
        if (iRet != 0) {
            iRet = FAIL;
        }
        osFree(pAviPacker->pTempName);
    }    

    if ( hd != 0 ) {
        gpChunkMemFree((void *)hd);
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
muxAviPack(
    SINT32 hd, 
    gpMuxPkt_t *pkt, 
    gpEsType_t type
)
{
    memAddr_t *pMemAddr;
    AviPacker_t *pAviPacker;
    UINT32 *pParamArry;
    UINT32 uiStrFlag;
    UINT32 uiRet = SUCCESS;

    pMemAddr = (memAddr_t *)(hd);
    pAviPacker = (AviPacker_t *)(pMemAddr->aviPackerMemAddr);
    pParamArry = (UINT32 *)(pMemAddr->aviParamMemAddr);
    
    if (pAviPacker->uiNowPacktedSize >= pParamArry[MUX_MAX_SIZE]) {
        diag_printf("\ncurrent size:%d limited size:%d \n", 
            pAviPacker->uiNowPacktedSize, pParamArry[MUX_MAX_SIZE]);

        if(pParamArry[MUX_MAX_SIZE] < AVI_MAX_FILE_SIZE ) {
            return MUX_MEM_FULL;        /*memory full*/
        }
        else {
            return MUX_FILE_SIZE_REACH; /*single file reach max size*/
        }
    }

 
    if (type == GP_ES_TYPE_VIDEO) {
        uiRet = aviAddMovi(pkt, hd, (UINT32)STR00_DC);
        if(uiRet != SUCCESS ) {    
            diag_printf("add movi error...\n");
            return FAIL;
        }
        uiRet = aviAddIndex(pkt, hd, (UINT32)STR00_DC);
        if(uiRet != SUCCESS ) {
            diag_printf("add index error...\n");
            return  FAIL;
        }
        pAviPacker->uiVidMsTime += pkt->pts;
        pAviPacker->uiTotalFrame++;
        if( pkt->size > pAviPacker->uiNowMaxFrameSize ) {
            pAviPacker->uiNowMaxFrameSize = pkt->size;
        }
    }
    else {
        if( pParamArry[MUX_VID_TYPE] == VID_UNKNOWN) {
            uiStrFlag = STR00_WB;
        }
        else {
            uiStrFlag = STR01_WB ;
        }
        pkt->keyFrame = AVI_VID_I_FRM;
        uiRet = aviAddMovi(pkt, hd, uiStrFlag);
        if(uiRet != SUCCESS ) {    
            return FAIL;
        }
        uiRet = aviAddIndex(pkt, hd, uiStrFlag);
        if(uiRet != SUCCESS ) {    
            return FAIL;
        }
        pAviPacker->uiAudMsTime += pkt->pts;
    }
    return SUCCESS ;
}
/**
*  initial param of record
*  @hd: the address of memory begin
*  @param: param
*  @value: the value of param
*  @return FAIL or SUCCESS
*/
SINT32 
muxAviSet( 
    SINT32 hd, 
    UINT32 param, 
    UINT32 value
)
{
    UINT32 *pParam;
    UINT8 *pNameBuf;
    UINT32 uiLenName;
    void        *tempStream;
    memAddr_t   *pMemAddr;
    AviPacker_t *pAviPacker;
    
    pParam =(UINT32 *) (((memAddr_t *)hd)->aviParamMemAddr);
    if (param > AVI_MUX_PARAM_END ||param < 0) {
        return FAIL;
    }
    else {
        pParam[param] = value;
        
        /*create index temp file*/
        if (param == MUX_PATH_TEMP_FILE) {            
            pMemAddr = (memAddr_t *)(hd);
            pAviPacker = (AviPacker_t *)(pMemAddr->aviPackerMemAddr);
            //pNameBuf = (UINT8 *)value;
            pNameBuf = osMalloc(strlen((char*)value) + 15);
			sprintf((char*)pNameBuf, "%sIDX000.TMP", (char*)value);
            #ifdef USE_SYSTEM_FS
            tempStream = (void *)open((char *)pNameBuf,   O_CREAT | O_RDWR);
            if (tempStream == (void *)(-1)) {
                diag_printf("cann`t Open temp file\n ");
                return FAIL;
            }
            #else
            tempStream = (void *)fopen((char *)pNameBuf, "wb");
            if (tempStream == NULL) {
                diag_printf("cann`t Open temp file\n ");
                return FAIL;
            }
            #endif
            
            pAviPacker->pstIndex = tempStream;
            uiLenName = (strlen((char *)pNameBuf) + 0x1F)&(~0x1F);
            pAviPacker->pTempName = (UINT8 *)osMalloc(uiLenName);
            if ( pAviPacker->pTempName == NULL ) {
                return FAIL;
            }            
            memset(pAviPacker->pTempName, 0, uiLenName);
            memcpy(pAviPacker->pTempName, pNameBuf, strlen((char *)pNameBuf));    
            /*diag_printf("avi temp file:%s \n", pAviPacker->pTempName);*/
        }
        return SUCCESS;
    }
}
/**
*  get param info during muxing
*  @hd: the address of memory begin
*  @param: param
*  @value: the address of value
*  @return FAIL or SUCCESS
*/
SINT32 
muxAviGet(
    SINT32 hd, 
    UINT32 param, 
    UINT32 *value
)
{
    UINT32 *pParam;
    
    pParam =(UINT32 *) (((memAddr_t *)hd)->aviParamMemAddr);
    if (param > AVI_MUX_PARAM_END ||param < 0) {
        return FAIL;
    }
    else {
        *value = pParam[param];
        return SUCCESS;
    }
}


