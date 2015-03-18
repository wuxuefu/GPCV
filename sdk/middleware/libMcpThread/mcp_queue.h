/**************************************************************************
 *                                                                        *
 *         Copyright (c) 2011 by Generalplus Inc.                         *
 *                                                                        *
 *  This software is copyrighted by and is the property of Generalplus    *
 *  Inc. All rights are reserved by Generalplus Inc.                      *
 *  This software may only be used in accordance with the                 *
 *  corresponding license agreement. Any unauthorized use, duplication,   *
 *  distribution, or disclosure of this software is expressly forbidden.  *
 *                                                                        *
 *  This Copyright notice MUST not be removed or modified without prior   *
 *  written consent of Generalplus Technology Co., Ltd.                   *
 *                                                                        *
 *  Generalplus Inc. reserves the right to modify this software           *
 *  without notice.                                                       *
 *                                                                        *
 *  Generalplus Inc.                                                      *
 *  No.19, Industry E. Rd. IV, Hsinchu Science Park                       *
 *  Hsinchu City 30078, Taiwan, R.O.C.                                    *
 *                                                                        *
 **************************************************************************/
#ifndef __MCP_QUEUE_H__
#define __MCP_QUEUE_H__

#include <mach/typedef.h>

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/
#define C_ACK_SUCCESS			0x00000001
#define C_ACK_FAIL				0x80000000

#define OS_NO_ERR				0
#define OS_ERR_POST_NULL_PTR    3
#define OS_ERR_PDATA_NULL       9	
#define OS_TIMEOUT              10

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
#define OSMboxPost				OSQPost
#define OSMboxPend				OSQPend

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/ 
UINT8 OSQPost(mcp_mbox_t *mbox, void *msg);
void *OSQPend(mcp_mbox_t *mbox, UINT16 timeout, UINT8 *err);
SINT32 SendMessage(mcp_mbox_t *msg_mbox, void *msg, mcp_mbox_t *ack_mbox, UINT16 timeout);


#endif //__MCP_QUEUE_H__