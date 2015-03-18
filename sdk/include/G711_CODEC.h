/**************************************************************************/
//  ITU G.711 A-law/u-law CODEC
//  v1001 (140318)
/**************************************************************************/


//========================================================
// Function Name : G711_Enc_Init
// Syntax : void G711_Enc_Init(short Set_FrameSize)
// Purpose : set FrameSize to G711 encoder
// Parameters : FrameSize(in words) (no size limit)
// Return : none
//========================================================
void G711_Enc_Init(short Set_FrameSize);

//========================================================
// Function Name : G711_Dec_Init
// Syntax : void G711_Dec_Init(short Set_FrameSize)
// Purpose : set FrameSize to G711 decoder
// Parameters : FrameSize(in bytes) (no size limit)
// Return : none
//========================================================
void G711_Dec_Init(short Set_FrameSize);

//========================================================
// Function Name : G711_GetVersion
// Syntax : char *G711_GetVersion(void)
// Purpose : get Version of Lib
// Parameters : none
// Return : Version of Lib
//========================================================
extern char *G711_GetVersion(void);

//========================================================
// Function Name : G711_Dec_Run
// Syntax : int G711_Dec_Run( short *InBuf, unsigned char *OutBuf, int EncType);
// Purpose : G711 decdoe process
// Parameters : short *InBuf : Input Buffer
//				unsigned char *OutBuf : Output Buffer
//				int EncType: "1" : Linear to ulaw
//							 "2" : Linear to Alaw
// Return : Length of Encode Result (Bytes) ??
//========================================================
short G711_Enc_Run( short *InBuf, unsigned char *OutBuf, int EncType);

//========================================================
// Function Name : G711_Dec_Run
// Syntax : int G711_Dec_Run( unsigned char *InBuf, short *OutBuf, int DecType);
// Purpose : G711 decdoe process
// Parameters : unsigned char *InBuf : Input Buffer
//				short *OutBuf : Output Buffer
//				int DecType: "1" : ulaw to Linear
//							 "2" : Alaw to Linear
// Return : Length of Decode Result (Bytes)
//========================================================
short G711_Dec_Run( unsigned char *InBuf, short *OutBuf, int DecType);
