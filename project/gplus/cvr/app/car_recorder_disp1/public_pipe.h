#ifndef _PUBLIC_PIPE_H_
#define _PUBLIC_PIPE_H_

/***************************************************
   Data struct
   *****************************************************/
#ifndef __SP_INT_TYPES__
#define __SP_INT_TYPES__
/** @brief UINT64 is a 64 bits unsigned integer */
typedef unsigned long long UINT64;
/** @brief SINT64 is a 64 bits signed integer */
typedef signed long long   SINT64;
/** @brief UINT32 is a 32 bits unsigned integer */
typedef unsigned int       UINT32;
/** @brief SINT32 is a 32 bits signed integer */
typedef signed int         SINT32;
/** @brief UINT16 is a 16 bits unsigned integer */
typedef unsigned short     UINT16;
/** @brief SINT16 is a 16 bits signed integer */
typedef signed short       SINT16;
/** @brief UINT8 is a 8 bits unsigned integer */
typedef unsigned char      UINT8;
/** @brief SINT8 is a 8 bits signed integer */
typedef signed char        SINT8;
#endif

#define success   0
#define fail     -1

enum responseResult{
	RESPONSE_WAIT=-1,
	RESPONSE_OK = 0,
	RESPONSE_FAIL=1,
	RESPONSE_TIMEOUT=2
};

typedef struct publicResponse_s {
	UINT32 Cmd;					/*the cmd need response*/
	unsigned long beginTime;	/*cmd begin time*/
	//unsigned long timeOut;		/*cmd time out,0:not,time:time out*/
	volatile enum responseResult result;				/*cmd ok or fail*/
	struct publicResponse_s *next;
} publicResponse_t;

typedef struct publicPlayerCmdPacket_s {

	UINT32 infoID;						/*ID of Message, Setup or Command*/
	UINT32 dataSize;					/*Size of variable lenth data*/
	UINT8  data[0];						/*Variable length data*/

} publicPlayerCmdPacket_t;
/****************************************************
function
*****************************************************/
/*
brief init the public pipe
callback:  callback function to parse the ack msg
return    success: 0
            fail       -1
*/
extern int public_pipeinit( void (*callback)(publicPlayerCmdPacket_t *CmdPacket,UINT8 *pData,pthread_mutex_t *mutex_resp ));
/*
brief delet pipe
*/
extern void pipeDelete( void );
/*
brief  find the response from cmd
*/
extern publicResponse_t *public_isInlist(UINT32 cmd);
/*
breif sent message to pipe
pCmdPacket : message packet to sent
waitresp :  1 wait ack
                0 do not wait
*/
extern int sendPacket(publicPlayerCmdPacket_t *pCmdPacket,int waitresp);
#endif
