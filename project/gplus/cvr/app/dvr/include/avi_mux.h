/***************************************************************************
 * Name: gp_mux.h
 *
 * Purpose:
 *
 * Developer:
 *     liuliang, 2010-12-27
 *
 * Copyright (c) 2010-2011 by Sunplus mMobile Inc.
 ***************************************************************************/

#ifndef _GP_AVI_MUX_H_
#define _GP_AVI_MUX_H_

/***************************************************************************
 * Header Files
 ***************************************************************************/

#include "typedef.h"
#include "gp_avcodec.h"
#include "gp_mux.h"

#ifdef cplusplus
extern "C" {
#endif

/***************************************************************************
 * Constants
 ***************************************************************************/
typedef enum param_e {
    AVI_MUX_PARAM_END = MUX_PARAM_END
}param_t;

#define AVI_RIFF         FOUR_CC('R','I','F','F')
#define AVI_LIST         FOUR_CC('L','I','S','T')
#define AVI_JUNK         FOUR_CC('J','U','N','K')
#define AVI_AVI          FOUR_CC('A','V','I',' ')
#define AVI_AVIX         FOUR_CC('A','V','I','X')
#define AVI_WAVE         FOUR_CC('W','A','V','E')
#define AVI_INFO         FOUR_CC('I','N','F','O')

#define AVI_avih         FOUR_CC('a','v','i','h')
#define AVI_hdrl         FOUR_CC('h','d','r','l')
#define AVI_movi         FOUR_CC('m','o','v','i')
#define AVI_idx1         FOUR_CC('i','d','x','1')

#define AVI_strl         FOUR_CC('s','t','r','l')
#define AVI_strh         FOUR_CC('s','t','r','h')
#define AVI_strf         FOUR_CC('s','t','r','f')
#define AVI_strd         FOUR_CC('s','t','r','d')
#define AVI_strn         FOUR_CC('s','t','r','n')
#define AVI_indx         FOUR_CC('i','n','d','x')

#define AVI_rec          FOUR_CC('r','e','c',' ')
#define AVI_auds         FOUR_CC('a','u','d','s')
#define AVI_vids         FOUR_CC('v','i','d','s')
#define AVI_txts         FOUR_CC('t','x','t','s')
#define AVI_mids         FOUR_CC('m','i','d','s')

#define AVI_IARL         FOUR_CC('I','A','R','L')
#define AVI_IART         FOUR_CC('I','A','R','T')
#define AVI_ICMS         FOUR_CC('I','C','M','S')
#define AVI_ICMT         FOUR_CC('I','C','M','T')
#define AVI_ICOP         FOUR_CC('I','C','O','P')
#define AVI_ICRD         FOUR_CC('I','C','R','D')
#define AVI_ICRP         FOUR_CC('I','C','R','P')
#define AVI_IDIM         FOUR_CC('I','D','I','M')
#define AVI_IDPI         FOUR_CC('I','D','P','I')
#define AVI_IENG         FOUR_CC('I','E','N','G')
#define AVI_IGNR         FOUR_CC('I','G','N','R')
#define AVI_IKEY         FOUR_CC('I','K','E','Y')
#define AVI_ILGT         FOUR_CC('I','L','G','T')
#define AVI_IMED         FOUR_CC('I','M','E','D')
#define AVI_INAM         FOUR_CC('I','N','A','M')
#define AVI_IPLT         FOUR_CC('I','P','L','T')
#define AVI_IPRD         FOUR_CC('I','P','R','D')
#define AVI_ISBJ         FOUR_CC('I','S','B','J')
#define AVI_ISFT         FOUR_CC('I','S','F','T')
#define AVI_ISHP         FOUR_CC('I','S','H','P')
#define AVI_ISRC         FOUR_CC('I','S','R','C')
#define AVI_ISRF         FOUR_CC('I','S','R','F')
#define AVI_ITCH         FOUR_CC('I','T','C','H')
#define AVI_ISMP         FOUR_CC('I','S','M','P')
#define AVI_IDIT         FOUR_CC('I','D','I','T')

#define AVIF_HASINDEX                   ((UINT32)0x00000010)
#define AVIF_MUSTUSEINDEX               ((UINT32)0x00000020)
#define AVIF_ISINTERLEAVED              ((UINT32)0x00000100)
#define AVIF_WASCAPTUREFILE             ((UINT32)0x00010000)
#define AVIF_COPYRIGHTED                ((UINT32)0x00020000)
#define AVIF_KNOWN_FLAGS                ((UINT32)0x00030130)

#define AVI_MOVI_SIZE       256*1024    /* movi buffer */
#define AVI_MUX_WRITE_SIZE  32*1024
#define AVI_INDEX_SIZE      16*1024     /*index buffer*/
#define STR01_WB            0x30317762  /* "01wb" */
#define STR00_WB            0x30307762  /*" 00wb "*/
#define STR00_DC            0x30306463  /* "00dc" */
#define STR00_DB            0x30306462  /* "00db" */
#define AVI_VID_I_FRM  0 
#define AVI_VID_P_FRM  1 
#define AVIIF_KEYFRAME                  ((UINT32)0x00000010)
#define AVI_INDEX_ENTRY_SIZE (16)

/***************************************************************************
 * Macros
 ***************************************************************************/
#define REVERSE_BUF32(x)    (((UINT32)(x) << 24) | (((UINT32)(x) & 0x0000ff00) << 8) | (((UINT32)(x) & 0x00ff0000) >> 8) | ((UINT32)(x) >> 24))
#define PRODUCT_PRETAG         FOUR_CC('S','P','M','P')
/***************************************************************************
 * Data Types
 ***************************************************************************/
 /**@brief hd structure*/
 typedef struct memAddr_s {
    UINT32 baseAddr;          /*!>init position*/
    UINT32 aviPackerMemAddr;  /*!>the position of packer*/
    UINT32 aviParamMemAddr;   /*!>the position of param*/
    UINT32 aviMoviMemAddr;    /*!>the position of movi*/
    UINT32 aviIndexMemAddr;   /*!>the position of index*/
} memAddr_t;

/**@brief the info of Avi Muxing*/
typedef struct AviPacker_s {
    void  *pstMovi;            /*!>avi stream */
    void  *pstIndex;           /*!>index stream*/
    UINT32 uiNowPacktedSize;
    UINT32 uiNowMoviOffset;    /*!>record MoviOffset*/
    UINT32 uiVidMsTime;
    UINT32 uiAudMsTime;
    UINT32 uiMoviSize;         /*!>movi used size*/
    UINT32 uiIndexSize;        /*!>index used size*/
    UINT32 uiTotalFrame;
    UINT32 uiNowMaxFrameSize;    
    UINT8  *pTempName;
}AviPacker_t;

/**@brief list of avi*/
typedef struct list_s {
    UINT32 chunkId;
    UINT32 chunkSize;
    UINT32 chunkData;
} list_t;

/**@brief chunk of avi*/
typedef struct chunk_t{
    UINT32 chunkId;
    UINT32 chunkSize;
}chunk_t;

typedef struct avi_stream_header_s {
    UINT32 fccType;                /*!>vids, auds, txts*/
    UINT32 fccHandler;             /*!>fourcc of codec to be used*/ 
    UINT32 dwFlags;     
    UINT32 dwReserved1;          
    UINT32 dwInitialFrames;        /*!>number of the first block of the stream*/                       
    UINT32 dwScale;                /*!>dwrate / dwscale = samples / second(audio)    or frames / second(video)*/       
    UINT32 dwRate;              
    UINT32 dwStart;                /*!>start time of stream*/         
    UINT32 dwLength;               /*!>size of stream in units*/            
    UINT32 dwSuggestedBufferSize;  /*size of buffer necessary*/
    UINT32 dwQuality;              /*!>the quality of the stream*/       
    UINT32 dwSampleSize;           /*!>number of bytes of one stream atom*/  
    UINT16 wRcFrameX0;           
    UINT16 wRcFrameY0;           
    UINT16 wRcFrameX1;       
    UINT16 wRcFrameY1;            
} avi_stream_header_t;
/**@brief header of avi*/
typedef struct main_avi_header_s {
    UINT32 dwMicroSecPerFrame;      /*!>the duration of one video frame in microseconds*/
    UINT32 dwMaxBytesPerSec;        /*!>highest occuring data rate within the file*/
    UINT32 dwReserved1;      
    UINT32 dwFlags;                 
    UINT32 dwTotalFrames;           /*!>frames in file*/
    UINT32 dwInitialFrames;
    UINT32 dwStreams;               /*!>number of streams in the file*/
    UINT32 dwSuggestedBufferSize;   /*!>size of buffer required to hold chunks of the file*/
    UINT32 dwWidth;                 /*!>width of video stream*/
    UINT32 dwHeight;                /*!>height of video stream*/
    UINT32 dwScale;
    UINT32 dwRate;
    UINT32 dwStart;
    UINT32 dwLength;
} main_avi_header_t;

typedef struct bitmapinfoheader_s {
    UINT32 biSize;      
    UINT32 biWidth;    
    UINT32 biHeight;    
    UINT16 biPlanes;     
    UINT16 biBitCount;    
    UINT32 biCompression;    
    UINT32 biSizeImage;     
    UINT32 biXPelsPerMeter;  
    UINT32 biYPelsPerMeter;   
    UINT32 biClrUsed;        
    UINT32 biClrImportant;  
} bitmapinfoheader_t;

typedef struct wav_format_s {
    UINT16 wFormatTag;  
    UINT16 nChannels;        
    UINT32 nSamplesPerSec;     
    UINT32 nAvgBytesPerSec;
    UINT16 nBlockAlign;      
    UINT16 wBitsPerSample;    
    UINT16 cbSize;          
    UINT16 wSamplesPerBlock;
} wav_format_t;

/**@brief str_list of avi*/
typedef struct str_list1_s{
    list_t stStrlList1;/*LIST,size,strl*/
    chunk_t stStrhChunk1;
    avi_stream_header_t stStrHeader;
    chunk_t stStrfChunk1;
    bitmapinfoheader_t stVidFormat;
}str_list1_t;

typedef struct str_stuffed1_s{
    chunk_t stChunk;
    UINT8 uiStuffed[sizeof(str_list1_t)-8];
}str_stuffed1_t;

/**@brief str_list2 of avi*/
typedef struct str_list2_s{
    list_t stStrlList2;      /*LIST,size,strl*/
    chunk_t stStrhChunk2;
    avi_stream_header_t stStrHeader;
    chunk_t stStrfChunk2;
    wav_format_t stAudFormat;
}str_list2_t;

typedef struct str_stuffed2_s{
    chunk_t stChunk;
    UINT8 uiStuffed[sizeof(str_list2_t)-8];
}str_stuffed2_t;

/**@brief header of avi*/
typedef struct avi_header_s{
    list_t stRiffList;     /*!>RIFF,size,AVI */
    list_t stHdrlList;     /*!>LIST,size,hdrl*/
    chunk_t stHdrChunk;
    main_avi_header_t stMainHdr;
    union {
        str_stuffed1_t stStuffed1;
        str_list1_t stStrList1;
    }unStrList1 ;
    union {
        str_stuffed2_t stStuffed2;
        str_list2_t stStrList2;
    }unStrList2;
    list_t stMovList;
}avi_header_t;

/**@brief movi entry of avi*/
typedef struct avi_movi_entry_s{
    UINT32 ckid;
    UINT32 dwSize;
}avi_movi_entry_t;

typedef struct avi_index_entry_s {
    UINT32 ckid;                      /*!>specifies a four-character*/
    UINT32 dwFlags;             /*keyframe or other*/
    UINT32 dwChunkOffset;  /*!>the position of the header of corresponding chunk*/
    UINT32 dwChunkLength;  /*!>the bytes of the corresponding chunk*/
} avi_index_entry_t;


/***************************************************************************
 * Global Data
 ***************************************************************************/

/***************************************************************************
 * Function Declarations
 ***************************************************************************/
SINT32 muxAviOpen(HANDLE out);
SINT32 muxAviClose(SINT32 hd);
SINT32 muxAviPack(SINT32 hd, gpMuxPkt_t *pkt, gpEsType_t type);
SINT32 muxAviSet(SINT32 hd, UINT32 param, UINT32 value);
SINT32 muxAviGet(SINT32 hd, UINT32 param, UINT32 *value);

extern void arm_block_copy_8bytes(UINT32 start_srcaddr, UINT32 end_srcaddr, UINT32 start_desaddr);
/***************************************************************************
 * Inline Function Definitions
 ***************************************************************************/

#ifdef cplusplus
}
#endif

#endif   /* _GP_AVI_MUX_H_  */

/***************************************************************************
 * The gp_avi_mux.h file end
 ***************************************************************************/

