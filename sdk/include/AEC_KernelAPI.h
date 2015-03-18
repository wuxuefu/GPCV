//========================================================
// Adaptive Echo Cancellation (AEC)
//
//========================================================

//========================================================
// Function Name : AEC_GetVersion
// Syntax : char *AEC_GetVersion(void)
// Purpose : get Version of Lib
// Parameters : none
// Return : Version of Lib
//========================================================
char *AEC_GetVersion(void);

//========================================================
// Function Name : AEC_API_Init
// Syntax : void AEC_API_Init(void)
// Purpose : AEC init process
// Parameters : none
// Return : none
//========================================================
void AEC_API_Init(void);

//========================================================
// Function Name : AEC_API_ServiceLoop
// Syntax : void AEC_API_ServiceLoop(short *R_Far, short *R_Mic, short *R_Out)
// Purpose : AEC main process API
// Parameters : short *R_Far : Far-End PCM data
//				short *R_Mic : Near-End PCM data from ADC
//				short *R_Out : Echo Canceled PCM data to Far-End
// Return : none
//========================================================
void AEC_API_ServiceLoop(short *R_Far, short *R_Mic, short *R_Out);
