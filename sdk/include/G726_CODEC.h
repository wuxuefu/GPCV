/**************************************************************************/
//  ITU G.726 ADPCM CODEC
//  v1000 (140108)
/**************************************************************************/

#define Buf_Size 360 //(inWords)

//----- Format of Encode Input Data / Decode Ouptut Data -----//
#define	AUDIO_FORMAT_ULAW		(1)		/* ISDN u-law */
#define	AUDIO_FORMAT_ALAW		(2)		/* ISDN A-law */
#define	AUDIO_FORMAT_LINEAR		(3)		/* PCM 2's-complement (0-center) */

//----- BitRate Type of G.726 -----//
#define G726_16kbps	 2			//for G.726 2 bits
#define G726_24kbps	 3			//for G.726 3 bits	
#define G726_32kbps	 4			//for G.726 4 bits
#define G726_40kbps	 5			//for G.726 5 bits

//========================================================
// Function Name : G726_GetVersion
// Syntax : char *G726_GetVersion(void)
// Purpose : get Version of Lib
// Parameters : none
// Return : Version of Lib
//========================================================
extern char *G726_GetVersion(void);

//========================================================
// Function Name : G726_Get_WorkingMem_Size
// Syntax : int G726_Get_WorkingMem_Size(void);
// Purpose : get Working Memory Size
// Parameters : none
// Return : size of working memory(Bytes)
//========================================================
int G726_Get_WorkingMem_Size(void);

//========================================================
// Function Name : G726_Enc_Init
// Syntax : int G726_Enc_Init(void *pWorkMem, int CodecBitsType, int InputDataType);
// Purpose : G726 Enc init
// Parameters : void *pWorkMem: allocated working memory pointer
//				int CodecBitsType: "2" : Encode G.726 16kbps @ 2bits format
//								   "3" : Encode G.726 24kbps @ 3bits format
//								   "4" : Encode G.726 32kbps @ 4bits format
//								   "5" : Encode G.726 40kbps @ 5bits format
//				int InputDataType: "1" : u-law fromat
//								   "2" : A-law fromat
//								   "3" : Linear PCM
// Return : Length of InputData per Encode process (Bytes)
//========================================================
extern int G726_Enc_Init(void *pWorkMem, int CodecBitsType, int InputDataType);

//========================================================
// Function Name : G726_Dec_Init
// Syntax : int G726_Dec_Init(void *pWorkMem, int CodecBitsType, int InputDataType);
// Purpose : G726 Dec init
// Parameters : void *pWorkMem: allocated working memory pointer
//				int CodecBitsType: "2" : Decode G.726 16kbps @ 2bits format
//								   "3" : Decode G.726 24kbps @ 3bits format
//								   "4" : Decode G.726 32kbps @ 4bits format
//								   "5" : Decode G.726 40kbps @ 5bits format
//				int OutputDataType: "1" : u-law fromat
//								    "2" : A-law fromat
//								    "3" : Linear PCM
// Return : Length of InputData per Decode process (Bytes)
//========================================================
extern int G726_Dec_Init(void *pWorkMem, int CodecBitsType, int InputDataType);

//========================================================
// Function Name : G726_Enc_Run
// Syntax : int G726_Enc_Run(int Buf, int EncBitsType, int InputDataType, void *WorkMem);
// Purpose : G726 encdoe process
// Parameters : const short *InBuf : Input Buffer
//				unsigned short *OutBuf : Output Buffer
//				void *WorkMem: allocated working memory pointer
// Return : Length of Encode Result (Bytes)
//========================================================
extern int G726_Enc_Run(short *InBuf, unsigned short *OutBuf, void *pWorkMem);

//========================================================
// Function Name : G726_Dec_Run
// Syntax : int G726_Dec_Run(int Buf, int EncBitsType, int InputDataType, void *WorkMem);
// Purpose : G726 decdoe process
// Parameters : const short *InBuf : Input Buffer
//				unsigned short *OutBuf : Output Buffer
//				void *WorkMem: allocated working memory pointer
// Return : Length of Decode Result (Bytes)
//========================================================
extern int G726_Dec_Run(unsigned char *InBuf, short *OutBuf, void *pWorkMem);
