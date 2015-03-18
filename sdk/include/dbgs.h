/*
**********************************************************************************************************************
*
* Moudle  : debug define module
* File    : dbgs.h
*
* By      : Steven
* Version : v1.0
* Date    : 2008-10-21 9:06:55
**********************************************************************************************************************
*/

#ifndef __DBGS_H__
#define __DBGS_H__
//30ºÚ 31ºì 32ÂÌ 33»Æ 34À¶ 35×Ï 36ÉîÂÌ 37°×
#define DBG_COLOR_BLEAK	        "\033[0m\033[30m" 
#define DBG_COLOR_RED			"\033[0m\033[31m"
#define DBG_COLOR_GREEN	        "\033[0m\033[32m"
#define DBG_COLOR_YELLOW		"\033[0m\033[33m"
#define DBG_COLOR_BLUE	    	"\033[0m\033[34m"
#define DBG_COLOR_PURPLE		"\033[0m\033[35m"
#define DBG_COLOR_DARK_G		"\033[0m\033[36m"
#define DBG_COLOR_WHITE	        "\033[0m\033[37m"
#define DBG_COLOR_END			"\033[0m"


#ifndef __DBGNAME__
#error __DBGNAME__ not defined!
#define __DBGNAME__ printf
#define __DBGLVL__       0
#endif
#ifndef __DBGLVL__
#define __DBGLVL__       0
#endif
#ifndef __MODULE__
#define __MODULE__ ""
#endif

#define EPDK_DEBUG_LEVEL_NOCHECK       0            /* No run time checks are performed                             */
#define EPDK_DEBUG_LEVEL_CHECK_ALL     1            /* Parameter checks and consistency checks are performed        */
#define EPDK_DEBUG_LEVEL_LOG_ERRORS    2            /* Errors are recorded                                          */
#define EPDK_DEBUG_LEVEL_LOG_WARNINGS  3            /* Errors & Warnings are recorded                               */
#define EPDK_DEBUG_LEVEL_LOG_ALL       4            /* Errors, Warnings and Messages are recorded.                  */

#define EPDK_DEBUG_LEVEL EPDK_DEBUG_LEVEL_LOG_ALL
#define __WUXUEFU__ADD__  //by wuxuefu add 
#ifndef EPDK_DEBUG_LEVEL
    #error EPDK_DEBUG_LEVEL not defined!
#endif

#if  (EPDK_DEBUG_LEVEL      == EPDK_DEBUG_LEVEL_NOCHECK)

#elif(EPDK_DEBUG_LEVEL    == EPDK_DEBUG_LEVEL_CHECK_ALL)
    #if(__DBGLVL__ == 0)

    #elif(__DBGLVL__ >= 1)
        #define __ASSERT
    #endif
#elif(EPDK_DEBUG_LEVEL    == EPDK_DEBUG_LEVEL_LOG_ERRORS)
    #if(__DBGLVL__ == 0)

    #elif(__DBGLVL__ == 1)
        #define __ASSERT
    #elif(__DBGLVL__ >= 2)
        #define __ERR
        #define __ASSERT
    #endif

#elif(EPDK_DEBUG_LEVEL    == EPDK_DEBUG_LEVEL_LOG_WARNINGS)
    #if(__DBGLVL__ == 0)

    #elif(__DBGLVL__ == 1)
        #define __ASSERT
    #elif(__DBGLVL__ == 2)
        #define __ERR
        #define __ASSERT
    #elif(__DBGLVL__ >= 3)
        #define __ERR
        #define __ASSERT
        #define __WRN
    #endif

#elif(EPDK_DEBUG_LEVEL    == EPDK_DEBUG_LEVEL_LOG_ALL)
    #if(__DBGLVL__ == 0)

    #elif(__DBGLVL__ == 1)
        #define __ERR
        #define __ASSERT
    #elif(__DBGLVL__ == 2)
        #define __ERR
        #define __WRN
        #define __ASSERT
    #elif(__DBGLVL__ == 3)
        #define __ERR
        #define __ASSERT
        #define __WRN
        #define __MSG
    #elif(__DBGLVL__ == 4)
        #define __ERR
        #define __ASSERT
        #define __WRN
        #define __MSG
        #define __HERE
        #define __INF
        #define __LOG
    #endif
#else
    #error EPDK_DEBUG_LEVEL is error, please config DEBUGLEVEL to 0/1/2/3/4!
#endif


#ifdef __ASSERT
    #define __ast(condition)    (if(!condition)                                                     \
                                 __DBGNAME__("AST:L%d(%s): condition err!", __LINE__, __MODULE__)   )
#else
    #define __ast(condition)	do{}while(0)
#endif

#if defined(__MSG)
	#define __msg(fmtstr, args...) do{__DBGNAME__("MSG:L%d(%s):" fmtstr DBG_COLOR_END, \
										__LINE__, __MODULE__,  ##args);}while(0)
#else
    #define __msg(...)			     do{}while(0)
#endif

#if defined(__ERR)
	#define __err(fmtstr, args...) do{__DBGNAME__(DBG_COLOR_RED "ERR:L%d(%s):" fmtstr DBG_COLOR_END, \
										__LINE__, __MODULE__,  ##args);}while(0)

#else
    #define __err(...)    		    do{}while(0)
#endif

#if defined(__WRN)
	#define __wrn(fmtstr, args...) do{__DBGNAME__(DBG_COLOR_YELLOW"WRN:L%d(%s):" fmtstr DBG_COLOR_END, \
										__LINE__, __MODULE__, ##args);}while(0)
#else
    #define __wrn(...)				do{}while(0)
#endif

#if defined(__HERE)
    #define __here__            __DBGNAME__(DBG_COLOR_DARK_G"@L%d(%s)[%s()]%s\n", __LINE__, __MODULE__,__func__,DBG_COLOR_END);
    #define __wait__            __DBGNAME__("@L%d(%s)(press any key:\n", __LINE__, __MODULE__)
#else
    #define __here__			do{}while(0)
    #define __wait__			do{}while(0)
#endif


#if defined(__INF)
	#define __inf(fmtstr, args...) do{__DBGNAME__(DBG_COLOR_PURPLE"INF:L%d(%s):" fmtstr DBG_COLOR_END, \
										__LINE__, __MODULE__, ##args);}while(0)
#else
    #define __inf(...)				 do{}while(0)
#endif
	#define __log(fmtstr, args...) do{__DBGNAME__(DBG_COLOR_BLUE"LOG:L%d(%s):" fmtstr DBG_COLOR_END, \

#endif  /* _DBGS_H_ */

