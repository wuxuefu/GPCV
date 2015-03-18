/**************************************************************************
 *                                                                        *
 *         Copyright (c) 2014 by Generalplus Inc.                         *
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
#ifndef __MCP_THREAD_H__
#define __MCP_THREAD_H__

#include <mach/typedef.h>

#ifdef USE_ROS_API
#include "os/ros_api.h"
#else
#include <pthread.h>
#endif

/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/
#define MCP_FLAG_WAITMODE_AND      (0)
#define MCP_FLAG_WAITMODE_OR       (2)
#define MCP_FLAG_WAITMODE_CLR      (1)

#ifdef USE_ROS_API
#define MCP_THREAD_PRIO_MIN			31
#define MCP_THREAD_PRIO_MAX			1
#else
#define MCP_THREAD_PRIO_MIN			1
#define MCP_THREAD_PRIO_MAX			31
#endif

#define MCP_MBOX_MAX_CNT			20

/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
#ifdef USE_ROS_API
#define mcp_mutex_init(mutex)      ros_mutex_create(mutex, "mcp_mutex", 1)
#define mcp_mutex_free(mutex)      ros_mutex_destroy(mutex)
#define mcp_mutex_lock(mutex)      ros_mutex_lock(mutex)
#define mcp_mutex_unlock(mutex)    ros_mutex_unlock(mutex)
#define mcp_cond_init(cond, mutex) ros_cond_create(cond, "mcp_cond", mutex)
#define mcp_cond_free(cond)        ros_cond_destroy(cond)
#define mcp_cond_broadcast(cond)   ros_cond_broadcast(cond)
#define mcp_cond_wait(cond, mutex) ros_cond_wait(cond)

#else
#if 0
#define mcp_mutex_init(mutex)      pthread_mutex_init(mutex, NULL)
#define mcp_mutex_free(mutex)      pthread_mutex_destroy(mutex)
#define mcp_mutex_lock(mutex)      pthread_mutex_lock(mutex)
#define mcp_mutex_unlock(mutex)    pthread_mutex_unlock(mutex)
#endif

#define mcp_cond_init(cond, mutex) pthread_cond_init(cond, NULL)
#define mcp_cond_free(cond)        pthread_cond_destroy(cond)
#define mcp_cond_broadcast(cond)   pthread_cond_broadcast(cond)
#define mcp_cond_signal(cond)	   pthread_cond_signal(cond)
#define mcp_cond_wait(cond, mutex) pthread_cond_wait(cond, mutex)
#endif

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/ 
#ifdef USE_ROS_API
typedef ros_mutex   mcp_mutex_t;
typedef ros_cond    mcp_cond_t;
typedef ros_thread* mcp_thread_t;
typedef ros_flag    mcp_flag_t;

#else
typedef pthread_mutex_t mcp_mutex_t;
typedef pthread_cond_t  mcp_cond_t;
typedef pthread_t       mcp_thread_t;

typedef struct mcp_flag_s 
{
    UINT32 m_value; 
    mcp_mutex_t *m_lock;
    mcp_cond_t  *m_cond;
} mcp_flag_t;
#endif
 
typedef struct mcp_link_s
{
	UINT32 data;
	struct mcp_link_s *next;
} mcp_link_t;

typedef struct mcp_mbox_s
{
    mcp_link_t m_box[MCP_MBOX_MAX_CNT];
    mcp_link_t *m_head;
    mcp_link_t *m_tail;
    mcp_mutex_t *m_lock;
    mcp_cond_t  *m_cond;
	UINT32 m_count;
} mcp_mbox_t; 
 
/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/
mcp_thread_t mcp_thread_create(void *(*fn)(void *), void *data, SINT32 priority);
void mcp_thread_kill(mcp_thread_t thread);
mcp_thread_t mcp_thread_self(void);
void mcp_thread_join(mcp_thread_t thread);
void mcp_sleep(UINT32 ms);

#if 1 
UINT32 mcp_mutex_init(mcp_mutex_t *mutex);
UINT32 mcp_mutex_free(mcp_mutex_t *mutex);
UINT32 mcp_mutex_lock(mcp_mutex_t *mutex);
UINT32 mcp_mutex_trylock(mcp_mutex_t *mutex);
UINT32 mcp_mutex_unlock(mcp_mutex_t *mutex); 
#endif
 
UINT32 mcp_cond_timedwait(mcp_cond_t *cond, mcp_mutex_t *mutex, UINT32 ms);
UINT32 mcp_flag_init(mcp_flag_t *flag);
void mcp_flag_free(mcp_flag_t *flag);
UINT32 mcp_flag_peek(mcp_flag_t *flag);
void mcp_flag_setbits(mcp_flag_t *flag, UINT32 value);
void mcp_flag_maskbits(mcp_flag_t *flag, UINT32 value);
UINT32 mcp_flag_wait(mcp_flag_t *flag, UINT32 pattern, UINT32 wait_mode);
UINT32 mcp_flag_loop_wait(mcp_flag_t *flag, UINT32 pattern, UINT32 wait_mode);
UINT32 mcp_flag_timed_wait(mcp_flag_t *flag, UINT32 pattern, UINT32 wait_mode, UINT32 ms);

UINT32 mcp_mbox_init(mcp_mbox_t *mbox);
void mcp_mbox_free(mcp_mbox_t *mbox);
UINT32 mcp_mbox_peek(mcp_mbox_t *mbox);
UINT32 mcp_mbox_put(mcp_mbox_t *mbox, UINT32 data);
UINT32 mcp_mbox_timed_put(mcp_mbox_t *mbox, UINT32 data, UINT32 ms);
UINT32 mcp_mbox_get(mcp_mbox_t *mbox);
UINT32 mcp_mbox_timed_get(mcp_mbox_t *mbox, UINT32 ms);

#endif /* __MCP_THREAD_H__ */
