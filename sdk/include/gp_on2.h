#include <mach/typedef.h>
#include <mach/gp_chunkmem.h> 

#ifdef __cplusplus
extern "C" {
#endif

void gp_on2_invalid_cache (unsigned int start,unsigned int num);
void gp_on2_flush_cache (unsigned int start,unsigned int num);
UINT32 gp_on2_readmemory(UINT32 busAddress);
chunk_block_t gp_on2_chunkmem_alloc(UINT32 size);
void gp_on2_chunkmem_free(void* pMem);
UINT32 gp_on2_chunkmem_va2pa(void* pMem);

SINT32 gpOn2Jpeg_Load(UINT32 format);
SINT32 gpOn2Jpeg_InstanceSize(void);
SINT32 gpOn2Jpeg_Init(void* arg);
SINT32 gpOn2Jpeg_Exec(void *arg);
SINT32 gpOn2Jpeg_Uninit(void *arg);
void gpOn2Jpeg_Unload();

SINT32 gpOn2Mpeg4_Load(UINT32 format);
SINT32 gpOn2Mpeg4_InstanceSize(void);
SINT32 gpOn2Mpeg4_Init(void* arg);
SINT32 gpOn2Mpeg4_Exec(void *arg);
SINT32 gpOn2Mpeg4_Uninit(void *arg);
void gpOn2Mpeg4_Unload();
/*
SINT32 hantroMp4DecLoad(UINT32 format);
SINT32 hantroMp4DecInstanceSize(void);
SINT32 hantroMp4DecInit(void* arg);
SINT32 hantroMp4DecDecodeExec(void *arg);
void hantroMp4DecFree(GPHD hd);
SINT32 hantroMp4DecUninit(void *arg);
void gpOn2Mpeg4_Unload();
*/
SINT32 gpOn2H264high_Load(UINT32 format);
SINT32 gpOn2H264high_InstanceSize(void);
SINT32 gpOn2H264high_Init(void* arg);
SINT32 gpOn2H264high_Exec(void *arg);
SINT32 gpOn2H264high_Uninit(void *arg);
/*
SINT32 gpOn2vp8_Load(UINT32 format);
SINT32 gpOn2vp8_InstanceSize(void);
SINT32 gpOn2vp8_Init(void* arg);
SINT32 gpOn2vp8_Exec(void *arg);
SINT32 gpOn2vp8_Uninit(void *arg);
*/
SINT32 gpOn2Jpeg_Load(UINT32 format);
SINT32 gpOn2Jpeg_InstanceSize(void);
SINT32 gpOn2Jpeg_Init(void* arg);
SINT32 gpOn2Jpeg_Exec(void *arg);
SINT32 gpOn2Jpeg_Uninit(void *arg);

SINT32 gpOn2vp8_Load(UINT32 format);
SINT32 gpOn2vp8_InstanceSize(void);
SINT32 gpOn2vp8_Init(void* arg);
SINT32 gpOn2vp8_Exec(void *arg);
SINT32 gpOn2vp8_Uninit(void *arg);

SINT32 gpOn2JpegEncode_Load(UINT32 format);
SINT32 gpOn2JpegEncode_InstanceSize(void);
SINT32 gpOn2JpegEncode_Init(void* arg);
SINT32 gpOn2JpegEncode_Exec(void *arg);
SINT32 gpOn2JpegEncode_Uninit(void *arg);
void gpOn2JpegEncode_Unload();

SINT32 gpOn2H264Encode_Load(UINT32 format);
SINT32 gpOn2H264Encode_InstanceSize(void);
SINT32 gpOn2H264Encode_Init(void* arg);
SINT32 gpOn2H264Encode_Exec(void *arg);
SINT32 gpOn2H264Encode_Uninit(void *arg);
void gpOn2H264Encode_Unload();

SINT32 gpOn2Codec_Load(UINT32 format);
SINT32 gpOn2Codec_InstanceSize(void);
SINT32 gpOn2Codec_Init(void* arg);
SINT32 gpOn2Codec_Exec(void *arg);
SINT32 gpOn2Codec_Uninit(void *arg);
void gpOn2Codec_Unload();

#ifdef __cplusplus
}
#endif
