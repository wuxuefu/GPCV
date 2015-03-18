#include "CloudDog.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <strings.h>
#define __MODULE__          "CloudDog_app"
#define __DBGLVL__          1 // 0=OFF, 1=ERROR, 2=WAENING 3=MSG 4=ALL
#define __DBGNAME__         printf
#include "dbgs.h"
#define DBG_MSG  __inf

#define DEUG_OUT 1
#define DEBU_OUT_HEX 0
#define CHECK_FAILED_CLEARED_DATA 0//检验失败后是否清除上次数据
#define  __PUT_CHAR(c) putchar(c)

static char IMIE_ID[16]={'0'};
static char phoneID[12]={'0'};
static BOOL isCheckErr=0;
static BITMAPFILEHEADER bmpFileHead;//14byte文件头
static BITMAPINFOHEADER infoHead; //40byte信息头

char *getCurrentIMIE_ID(void) {  return IMIE_ID; }
char *getCurrentPhone_ID(void){  return phoneID; }
BOOL isCheckCurrSumError(void) {return isCheckErr;}
static void *transformSecurityBoot(PSECURITY_BOOT data);
static void *transformGpsInfo(PGPS_INFO data);
static void *transformFixedAlarm(PFIXED_ALARM  data);
static void *transformRadarAlarm(PRADAR_ALARM data);
static void *transformMenuInfo(PMENU_INFO data);
static void *transformWeather(PWEATHER data);
static void *transformComplain(PCOMPLAIN data);
BOOL  putDataStruct(void *m_struct,DATA_TYPES type)
{
    PEND_PACKET endPackData;
    UINT32 offsetSize=0,structLength=0,i=0;
    UINT8  CheckSum = 0;
    char *checksemChar = (char *)m_struct;
    switch(type)
    {
        case TYPE_SECURITY_BOOT:
            offsetSize   = (offsetof(SECURITY_BOOT,endData));
            structLength = sizeof(SECURITY_BOOT);
        break;
        case TYPE_GPS_INFO:
            offsetSize   = (offsetof(GPS_INFO,endData));
            structLength = sizeof(GPS_INFO);
        break;
        case TYPE_FIXED_ALARM:
            offsetSize   = (offsetof(FIXED_ALARM,endData));
            structLength = sizeof(FIXED_ALARM);
        break;
        case TYPE_RADAR_ALARM:
            offsetSize   = (offsetof(RADAR_ALARM,endData));
            structLength = sizeof(RADAR_ALARM);
        break;
        case  TYPE_MENU_INFO:
            offsetSize   = (offsetof(MENU_INFO,endData));
            structLength = sizeof(MENU_INFO);
        break;
        case TYPE_WEATHER:
            offsetSize   = (offsetof(WEATHER,endData));
            structLength = sizeof(WEATHER);
        break;
        case TYPE_COMPLAIN:
            offsetSize   = (offsetof(COMPLAIN,endData));
            structLength = sizeof(COMPLAIN);
        break;
    default:
        return 0;
    }
    //计算校验和并填充结束数据包
    endPackData  = (PEND_PACKET)(m_struct+offsetSize);
    for(i=0;i<structLength-sizeof(END_PACKET);i++)
    {
        CheckSum += *checksemChar;
        checksemChar++;
    }
    endPackData->CheckSum = CheckSum;
    endPackData->endString[0] = 0x0D;
    endPackData->endString[1] = 0x0A;
   //发送结构体
    for(i=0;i<structLength;i++)
    {
        __PUT_CHAR(*((char *)m_struct++));
    }
    __PUT_CHAR('\n');
    return 1;
}
void * getDataStruct(char *string,DATA_TYPES *type)
{
    PSTART_PACKET start;
    void *result = NULL;
    start = (PSTART_PACKET)string;
    *type = start->type;
    switch(*type)
    {
        case TYPE_SECURITY_BOOT:
            result = transformSecurityBoot((PSECURITY_BOOT)string);
        break;
        case TYPE_GPS_INFO:
            result = transformGpsInfo((PGPS_INFO)string);
        break;
        case TYPE_FIXED_ALARM:
            result = transformFixedAlarm((PFIXED_ALARM)string);
        break;
        case TYPE_RADAR_ALARM:
            result = transformRadarAlarm((PRADAR_ALARM)string);
        break;
        case  TYPE_MENU_INFO:
            result = transformMenuInfo((PMENU_INFO)string);
        break;
        case TYPE_WEATHER:
            result = transformWeather((PWEATHER)string);
        break;
        case TYPE_COMPLAIN:
            result = transformComplain((PCOMPLAIN)string);
        break;
    default:
        return NULL;
    }
      return result;
}
static void *transformSecurityBoot(PSECURITY_BOOT data)
{
    //PSECURITY_BOOT security = data;
    static SECURITY_BOOT result;
    {
        UINT8  CheckSum = 0;
        int i=0;
        char *checksemChar = (char *)&result;
        for(i=0;i<sizeof(result)-sizeof(END_PACKET);i++)
        {
            CheckSum += *checksemChar;
            checksemChar++;
        }
        if(CheckSum == result.endData.CheckSum)
        {
            isCheckErr = 0;
            memset(&result,0,sizeof(result));
            result =  *data;
            result.endData.endString[0] = 0x0D;
            result.endData.endString[1] = 0x0A;
            result.status = endianSwap16(result.status);
        }
        else
        {
            isCheckErr = 1;
            #if CHECK_FAILED_CLEARED_DATA
            memset(&result,0,sizeof(result));
            #endif
        }
    }
#if DEUG_OUT
    #if DEBU_OUT_HEX
    DBG_MSG("PSECURITY_BOOT::startString:0x%X 0x%X 0x%X length:0x%X type:0x%X status:0x%X CheckSum:0x%X endString:0x%X 0x%X\n",\
           result.startPack.startString[0],result.startPack.startString[1],result.startPack.startString[2],\
           result.startPack.length,result.startPack.type,result.status,result.endData.CheckSum,\
           result.endData.endString[0],result.endData.endString[1]);
    #else
    DBG_MSG("PSECURITY_BOOT::startString:%d %d %d length:%d type:%d status:%d CheckSum:%d endString:%d %d\n",\
             result.startPack.startString[0],result.startPack.startString[1],result.startPack.startString[2],\
             result.startPack.length,result.startPack.type,result.status,result.endData.CheckSum,\
             result.endData.endString[0],result.endData.endString[1]);
    #endif
#endif
    return (void *)&result;
}
static void *transformGpsInfo(PGPS_INFO data)
{
    static GPS_INFO result;
    {
        UINT8  CheckSum = 0;
        int i=0;
        char *checksemChar = (char *)&result;
        for(i=0;i<sizeof(result)-sizeof(END_PACKET);i++)
        {
            CheckSum += *checksemChar;
            checksemChar++;
        }
        if(CheckSum == result.endData.CheckSum)
        {
            isCheckErr = 0;
            memset(&result,0,sizeof(result));
            result =  *data;
            result.endData.endString[0] = 0x0D;
            result.endData.endString[1] = 0x0A;
        }
        else
        {
            isCheckErr = 1;
            #if CHECK_FAILED_CLEARED_DATA
            memset(&result,0,sizeof(result));
            #endif
        }
    }
#if DEUG_OUT
    #if DEBU_OUT_HEX
    DBG_MSG("GPS_INFO::年:%d 月:%d 日:%d 时:%d 分:%d 秒:%d length:0x%X Type:0x%X gpsStatus:0x%X latitude:0x%X latitudeHalf:0x%X longitude:0x%X longitudeHalf:0x%X speed:0x%X directionAngle:0x%X CheckSum:0x%X",\
           result.year,result.month,result.day,result.hour,result.minute,result.second,\
           result.startPack.length,result.startPack.type,result.gpsStatus,result.latitude,result.latitudeHalf,\
           result.longitude,result.longitudeHalf,result.speed,result.directionAngle,result.endData.CheckSum);
    #else
    DBG_MSG("GPS_INFO::年:%d 月:%d 日:%d 时:%d 分:%d 秒:%d length:%d Type:%d gpsStatus:%d latitude:%d latitudeHalf:%d longitude:%d longitudeHalf:%d speed:%d directionAngle:%d CheckSum:%d",\
             result.year,result.month,result.day,result.hour,result.minute,result.second,\
             result.startPack.length,result.startPack.type,result.gpsStatus,result.latitude,result.latitudeHalf,\
             result.longitude,result.longitudeHalf,result.speed,result.directionAngle,result.endData.CheckSum);
    #endif
#endif
    return (void *)&result;
}
static void *transformFixedAlarm(PFIXED_ALARM data)
{
    static FIXED_ALARM result;
    {
        UINT8  CheckSum = 0;
        int i=0;
        char *checksemChar = (char *)&result;
        for(i=0;i<sizeof(result)-sizeof(END_PACKET);i++)
        {
            CheckSum += *checksemChar;
            checksemChar++;
        }
        if(CheckSum == result.endData.CheckSum)
        {
            isCheckErr = 0;
            memset(&result,0,sizeof(result));
            result =  *data;
            result.endData.endString[0] = 0x0D;
            result.endData.endString[1] = 0x0A;
        }
        else
        {
            isCheckErr = 1;
            #if CHECK_FAILED_CLEARED_DATA
            memset(&result,0,sizeof(result));
            #endif
        }
    }
#if DEUG_OUT
    #if DEBU_OUT_HEX
    DBG_MSG("startString:0x%X 0x%X 0x%X length:0x%X type:0x%X warningType:0x%X distance:0x%X speed:0x%X currSpeed:0x%X direction:0x%X gpsAvailableNumber:0x%X msg:%s\n",\
           result.startPack.startString[0],result.startPack.startString[1],result.startPack.startString[2],result.startPack.length,result.startPack.type,\
           result.warningType,result.distance,result.speed,result.currSpeed,result.direction,result.gpsAvailableNumber,result.msg);
    #else
    DBG_MSG("startString:%d %d %d length:%d type:%d warningType:%d distance:%d speed:%d currSpeed:%d direction:%d gpsAvailableNumber:%d msg:%s\n",\
            result.startPack.startString[0],result.startPack.startString[1],result.startPack.startString[2],result.startPack.length,result.startPack.type,\
            result.warningType,result.distance,result.speed,result.currSpeed,result.direction,result.gpsAvailableNumber,result.msg);
    #endif
#endif
    return (void *)&result;
}
static void *transformRadarAlarm(PRADAR_ALARM data)
{
    static RADAR_ALARM result;
    {
        UINT8  CheckSum = 0;
        int i=0;
        char *checksemChar = (char *)&result;
        for(i=0;i<sizeof(result)-sizeof(END_PACKET);i++)
        {
            CheckSum += *checksemChar;
            checksemChar++;
        }
        if(CheckSum == result.endData.CheckSum)
        {
            isCheckErr = 0;
            memset(&result,0,sizeof(result));
            result =  *data;
            result.endData.endString[0] = 0x0D;
            result.endData.endString[1] = 0x0A;
        }
        else
        {
            isCheckErr = 1;
            #if CHECK_FAILED_CLEARED_DATA
            memset(&result,0,sizeof(result));
            #endif
        }
    }
#if DEUG_OUT
    #if DEBU_OUT_HEX
    DBG_MSG("startString:0x%X 0x%X 0x%X length:0x%X type:0x%X channel:0x%X intensity:0x%X frequency:0x%X currSpeed:0x%X currDirection:0x%X gpsAvailableNumber:0x%X CheckSum:0x%X\n",\
            result.startPack.startString[0],result.startPack.startString[1],result.startPack.startString[2],result.startPack.length,result.startPack.type,\
            result.channel,result.intensity,result.frequency,result.currSpeed,result.currDirection,result.gpsAvailableNumber,result.endData.CheckSum);
    #else
    DBG_MSG("startString:%d %d %d length:%d type:%d channel:%d intensity:%d frequency:%d currSpeed:%d currDirection:%d gpsAvailableNumber:%d CheckSum:%d\n",\
              result.startPack.startString[0],result.startPack.startString[1],result.startPack.startString[2],result.startPack.length,result.startPack.type,\
              result.channel,result.intensity,result.frequency,result.currSpeed,result.currDirection,result.gpsAvailableNumber,result.endData.CheckSum);
    #endif
#endif
    return (void *)&result;
}
static void *transformMenuInfo(PMENU_INFO data)
{
    static MENU_INFO result;
    int i=0,count=0;
    char *temp;
    {
        UINT8  CheckSum = 0;
        int i=0;
        char *checksemChar = (char *)&result;
        for(i=0;i<sizeof(result)-sizeof(END_PACKET);i++)
        {
            CheckSum += *checksemChar;
            checksemChar++;
        }
        if(CheckSum == result.endData.CheckSum)
        {
            isCheckErr = 0;
            memset(&result,0,sizeof(result));
            result =  *data;
            result.endData.endString[0] = 0x0D;
            result.endData.endString[1] = 0x0A;
        }
        else
        {
            isCheckErr = 1;
            #if CHECK_FAILED_CLEARED_DATA
            memset(&result,0,sizeof(result));
            #endif
        }
    }
    //53 96 75 30 30 35 75 08(设备 IMEI 号:356957030353578(15 位))
    temp = result.IMEI_ID;
    count = sizeof(result.IMEI_ID)/sizeof(result.IMEI_ID[0])*2;
    for(i=0;i<count;i+=2)
    {
        IMIE_ID[i]   = (*temp&0xf)+0x30;
        IMIE_ID[i+1] = ((*temp&0xf0)>> 4)+0x30;
        temp++;
    }
    IMIE_ID[count-1] = '\n';
    printf("count:%d IMIE_ID:%s\n",count,IMIE_ID);
    //本机号码:31 14 72 36 89 04(设备本机号码:13412763984)
    temp = result.id;
    count = sizeof(result.id)/sizeof(result.id[0])*2;
    for(i=0;i<count;i+=2)
    {
        phoneID[i]   = (*temp&0xf)+0x30;
        phoneID[i+1] = ((*temp&0xf0)>> 4)+0x30;
        temp++;
    }
    phoneID[count-1] = '\n';
    printf("count:%d phoneID:%s\n",count,phoneID);
#if DEUG_OUT
    #if DEBU_OUT_HEX
    DBG_MSG("startString:0x%X 0x%X 0x%X length:0x%X type:0x%X muteSetting:0x%X volume:0x%X silentSpeed:0x%X speedSetting:0x%X  modeSetting:0x%X speedCompensation:0x%X \
            trafficStatu:0x%X version:0x%X  IMEI_ID:%s ID:%s   defaultSetting:0x%X  CheckSum:0x%X \n",\
            result.startPack.startString[0],result.startPack.startString[1],result.startPack.startString[2],result.startPack.length,result.startPack.type,\
            result.muteSetting,result.volume,result.silentSpeed,result.speedSetting,result.modeSetting,result.speedCompensation,result.trafficStatu,\
            result.version,result.IMEI_ID,result.id,result.defaultSetting,result.endData.CheckSum);
    #else
    DBG_MSG("startString:%d %d %d length:%d type:%d muteSetting:%d volume:%d silentSpeed:%d speedSetting:%d  modeSetting:%d speedCompensation:%d \
             trafficStatu:%d version:%d  IMEI_ID:%d  ID:%s   defaultSetting:%d  CheckSum:%d \n",\
             result.startPack.startString[0],result.startPack.startString[1],result.startPack.startString[2],result.startPack.length,result.startPack.type,\
             result.muteSetting,result.volume,result.silentSpeed,result.speedSetting,result.modeSetting,result.speedCompensation,result.trafficStatu,\
             result.version,result.IMEI_ID,result.id,result.defaultSetting,result.endData.CheckSum);
    #endif
#endif
    return (void *)&result;
}
static void *transformWeather(PWEATHER data)
{
    static WEATHER result;
    {
        UINT8  CheckSum = 0;
        int i=0;
        char *checksemChar = (char *)&result;
        for(i=0;i<sizeof(result)-sizeof(END_PACKET);i++)
        {
            CheckSum += *checksemChar;
            checksemChar++;
        }
        if(CheckSum == result.endData.CheckSum)
        {
            isCheckErr = 0;
            memset(&result,0,sizeof(result));
            result =  *data;
            result.endData.endString[0] = 0x0D;
            result.endData.endString[1] = 0x0A;
        }
        else
        {
            isCheckErr = 1;
            #if CHECK_FAILED_CLEARED_DATA
            memset(&result,0,sizeof(result));
            #endif
        }
    }
#if DEUG_OUT
    #if DEBU_OUT_HEX
    DBG_MSG("startString:0x%X 0x%X 0x%X length:0x%X type:0x%X  typeMsg:0x%X MSG:0x%s CheckSum:0x%X\n",\
            result.startPack.startString[0],result.startPack.startString[1],result.startPack.startString[2],result.startPack.length,result.startPack.type,\
            result.type,result.msg,result.endData.CheckSum);
    #else
    DBG_MSG("startString:%d %d %d length:%d type:%d  typeMsg:%d MSG:0x%s CheckSum:%d\n",\
            result.startPack.startString[0],result.startPack.startString[1],result.startPack.startString[2],result.startPack.length,result.startPack.type,\
            result.type,result.msg,result.endData.CheckSum);
    #endif
#endif
    return (void *)&result;
}
static void *transformComplain(PCOMPLAIN data)
{
    static COMPLAIN result;
    {
        UINT8  CheckSum = 0;
        int i=0;
        char *checksemChar = (char *)&result;
        for(i=0;i<sizeof(result)-sizeof(END_PACKET);i++)
        {
            CheckSum += *checksemChar;
            checksemChar++;
        }
        if(CheckSum == result.endData.CheckSum)
        {
            isCheckErr = 0;
            memset(&result,0,sizeof(result));
            result =  *data;
            result.endData.endString[0] = 0x0D;
            result.endData.endString[1] = 0x0A;
        }
        else
        {
            isCheckErr = 1;
            #if CHECK_FAILED_CLEARED_DATA
            memset(&result,0,sizeof(result));
            #endif
        }
    }
#if DEUG_OUT
    #if DEBU_OUT_HEX
    DBG_MSG("startString:0x%X 0x%X 0x%X length:0x%X type:0x%X satus:0x%X CheckSum:0x%X\n",\
            result.startPack.startString[0],result.startPack.startString[1],result.startPack.startString[2],result.startPack.length,result.startPack.type,\
            result.satus,result.endData.CheckSum);
    #else
    DBG_MSG("startString:%d %d %d length:%d type:%d  satus:%d  CheckSum:%d\n",\
            result.startPack.startString[0],result.startPack.startString[1],result.startPack.startString[2],result.startPack.length,result.startPack.type,\
            result.satus,result.endData.CheckSum);
    #endif
#endif
    return (void *)&result;
}


