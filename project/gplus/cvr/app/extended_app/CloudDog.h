#ifndef CARDV_CLOUDDOG_H
#define CARDV_CLOUDDOG_H

#define ENUM_DUMMY4WORD(name)   E_##name = 0x10000000

#ifndef NULL
#define NULL (void *)0
#endif

#if defined(__arm )

#define _ALIGNED(x) __align(x)
#define _PACKED_BEGIN __packed
#define _PACKED_END
#define _INLINE static __inline
#define _ASM_NOP __asm{nop;}
#define _SECTION(sec)
#define _CALLBACK()
#define _CALLBACK_SECTION(sec)
#define _MIPS_TODO 1
#define _DRV650_TODO 0
#define _STOPTRACE
#elif defined( __WIN32__)
#define _PACKED_BEGIN
#define _PACKED_END
#elif defined( __GNUC__)

#define _ALIGNED(x) __attribute__((aligned(x)))
#define _PACKED_BEGIN
#define _PACKED_END __attribute__ ((packed))
#define _INLINE static inline
#define _ASM_NOP __asm__("nop");
#define _SECTION(sec)          __attribute__ ((section (sec)))
#define _CALLBACK()            __attribute__ ((weak))
#define _CALLBACK_SECTION(sec) __attribute__ ((weak, section(sec)))
#define _MIPS_TODO 0
#define _DRV650_TODO 0
#define _STOPTRACE __attribute__((__no_instrument_function__))
#endif

#define __SYSTEM_32_BIT__ 1
#define __SYSTEM_64_BIT__ 2
#define __SYSTEM_USER_BIT__ 3
#define __SYSTEM_DATA_TYPEDEF__ __SYSTEM_32_BIT__

#if (__SYSTEM_DATA_TYPEDEF__ == __SYSTEM_32_BIT__)
typedef unsigned long long  UINT64;     ///< Unsigned 64 bits data type
typedef signed long long    INT64;      ///< Signed 64 bits data type
typedef unsigned int       UINT32;     ///< Unsigned 32 bits data type
typedef signed int         INT32;      ///< Signed 32 bits data type
typedef unsigned short      UINT16;     ///< Unsigned 16 bits data type
typedef signed short        INT16;      ///< Signed 16 bits data type
typedef unsigned char       UINT8;      ///< Unsigned 8 bits data type
typedef signed char         INT8;       ///< Signed 8 bits data type
typedef float               FLOAT;      ///< Floating point integer
typedef unsigned int        UBITFIELD;  ///< Unsigned bit field
typedef signed int          BITFIELD;   ///< Signed bit field
typedef UINT32              BOOL;
#elif(__SYSTEM_DATA_TYPEDEF__ == __SYSTEM_64_BIT__)
typedef unsigned  long  UINT64;     ///< Unsigned 64 bits data type
typedef signed  long    INT64;      ///< Signed 64 bits data type
typedef unsigned int       UINT32;     ///< Unsigned 32 bits data type
typedef signed int         INT32;      ///< Signed 32 bits data type
typedef unsigned short      UINT16;     ///< Unsigned 16 bits data type
typedef signed short        INT16;      ///< Signed 16 bits data type
typedef unsigned char       UINT8;      ///< Unsigned 8 bits data type
typedef signed char         INT8;       ///< Signed 8 bits data type
typedef float               FLOAT;      ///< Floating point integer
typedef unsigned int        UBITFIELD;  ///< Unsigned bit field
typedef signed int          BITFIELD;   ///< Signed bit field
typedef UINT32              BOOL;
#else
typedef UINT32              BOOL;
#endif
typedef enum
{
    TYPE_SECURITY_BOOT=0x20,
    TYPE_GPS_INFO = 0x10,
    TYPE_FIXED_ALARM = 0x11,
    TYPE_RADAR_ALARM = 0x12,
    TYPE_MENU_INFO = 0x13,
    TYPE_WEATHER = 0x18,
    TYPE_COMPLAIN = 0x19,
    ENUM_DUMMY4WORD(DATA_TYPES),
}DATA_TYPES;
typedef enum
{
    ALARM_FIXED_SPEED  = 12,//固定测速
    ALARM_INTERVAL_VELOCITY = 20,//区间测速
    ALARM_INTERVAL_VELOCITY_STARTING=21,//区间测速起点
    ALARM_INTERVAL_VELOCITY_END = 22,//区间测速终点
    ALARM_FLOW_VELOCITY = 24,//流动测速
    ALARM_RED_CAMERA = 2,//闯红灯拍照
    ALARM_RED_NO_CAMERA = 3,//闯红灯无拍照
    ALARM_ACCIDENT_MULTIPLE = 93,//事故多发路段
    ALARM_SCHOOL_SECTIONS = 90,//学校路段
    ALARM_SNOW_ROAD_SECTIONS = 91,//冰雪路段
    ALARM_MOUNTAIN_ROAD_SECTIONS = 92,//山地路段
    ALARM_ANXIOUS_TURN = 94,//急转弯
    ALARM_CONTINUOUS_CURVE = 95,//连续弯道
    ALARM_ANXIOUS_DOWNHILL = 96,//急下坡
    ALARM_ROCKFALL = 97,//落石
    ALARM_FOGGY = 98,//多雾
    ALARM_MILITARY_CONTROL = 99,//军事管制
    ALARM_FEE_STATION = 108,//收费站
    ALARM_GAS_STATION = 101,//加油站
    ALARM_SERVICE_AREA = 102,//服务区
    ALARM_TUNNEL = 110,//隧道
    ALARM_DAY_ROAD_LIGHTS = 103,//全天候开头灯路段
    ALARM_ELECTRONIC_MONITORING = 81,//电子监控
    ALARM_CAR_TRAFFIC_MONITORING = 82,//汽车流量监控
    ALARM_DRIVING_BEHAVIOR = 83,//驾驶行为
    ALARM_VIOLATION = 84,//压实线,违规变道
    ALARM_BUS_LANES = 85,//公交车专用车道
    ALARM_OCCUPANCY_SHUNT = 86,//占用分流道
    ALARM_UNIDIRECTIONAL_RETROGRADE = 87,//单向逆行
    ALARM_PROHIBITED_LINKS = 88,//禁行路段
    ALARM_ILLEGAL_PARKING = 89,//违章停车
    ALARM_TEMPORARY_PARKING_BAN = 70,//禁止临时停车
    ALARM_PROHIBITING_OFF_HEAD = 71,//禁止掉头
    ALARM_PROHIBITED_LEFT_TURN = 72,//禁止左转
    ALARM_PROHIBITED_RIGHT_TURN = 73,//禁止右转
    ENUM_DUMMY4WORD(ALARM_TYPE_TABLE),
}ALARM_TYPE_TABLE;

#ifndef offsetof
#define offsetof(type, field) ((UINT32)&(((type *)0)->field))
#endif
/*----大小端转换-----------------------------------------------------------------*/
#define endianSwap16(x) (((x&0xff)<< 8)|((x&0xff00)>>8))
#define endianSwap8(x)  (((x&0xf)<< 4)|((x&0xf0)>>4))
/*---------------------起始/结束字符--------------------------------------*/
#if defined( __WIN32__)
#pragma pack(1)
#endif



#if defined( __GNUC__)
typedef   struct
#else
typedef _PACKED_BEGIN  struct
#endif
{
    UINT8 startString[3]; //0x41 0x54 0x55
	UINT16 length;
	UINT8  type;
}_PACKED_END START_PACKET,*PSTART_PACKET;



#if defined( __GNUC__)
typedef   struct
#else
typedef _PACKED_BEGIN  struct
#endif
{
    UINT8  CheckSum;
    UINT8  endString[2];//0x0D 0x0A(0D 0A 不算长度,不算校验码)
}_PACKED_END END_PACKET,*PEND_PACKET;

/*----启动安防开机协议（电子狗<=行车记录仪）（type:0x20）---------------------------------*/
 #if defined( __GNUC__)
typedef   struct
#else
typedef _PACKED_BEGIN  struct
#endif
{
    START_PACKET startPack;
    UINT16 status;//0 撤销安防    1 启动安防
    END_PACKET endData;
}_PACKED_END SECURITY_BOOT,*PSECURITY_BOOT; //65 84 85 8 0 32 0 1 19

/*----GPS 信息（电子狗→行车记录仪）（type:0x10）-------------------------------*/
#if defined( __GNUC__)
typedef   struct
#else
typedef _PACKED_BEGIN  struct
#endif
{
    START_PACKET startPack;
    UINT8  gpsStatus;//卫星状态 ‘A’or‘V’
    UINT32 latitude;//纬度*100000
    UINT8  latitudeHalf;//‘N’or‘S’
    UINT32 longitude;//经度*100000
    UINT8  longitudeHalf;// ‘W’or‘E’
    UINT8  speed;//实时速度
    UINT16 directionAngle;//方向角度2 实际行驶方向(0--360)
    UINT8  gpsNumber;//可用卫星数1
    UINT16 height;//海拔高度
    UINT16 year;
    UINT8  month;
    UINT8  day;
    UINT8  hour;
    UINT8  minute;
    UINT8  second;
    UINT8  gsmSignal; //GSM 信号强度 计算公式:level=Signal*(5-1)/31 + 1;  If(level>4)  Level = 4;
    UINT8  Astatus;//A 状态 保留
    UINT8  warning;//报警状态 1 0 表示无报警，1 表示报警中
    END_PACKET endData;
}_PACKED_END GPS_INFO,*PGPS_INFO;

/*----固定报警信息（电子狗→行车记录仪）（type:0x11）-------------------------------*/
#if defined( __GNUC__)
typedef   struct
#else
typedef _PACKED_BEGIN  struct
#endif
{
    START_PACKET startPack;
    UINT8  warningType;// 请见类型说明文档
    UINT16 distance;//与电子眼间的距离
    UINT8  speed;// ?? 限速值
    UINT8  currSpeed;//当前速度
    UINT16 direction ;// 方向角度
    UINT8  gpsAvailableNumber;//  可用卫星数
    UINT8  msg[100];// 报警内容 例如：有固定测速拍照,限速80公里 or 前方300米有闯红灯
    END_PACKET endData;
}_PACKED_END FIXED_ALARM,*PFIXED_ALARM;

/*----雷达报警信息（电子狗→行车记录仪）（type:0x12）-------------------------------*/
#if defined( __GNUC__)
typedef   struct
#else
typedef _PACKED_BEGIN  struct
#endif
{
    START_PACKET startPack;
    UINT8 channel;//雷达频段 0:NULL,0x40:Ka,0x20:K,0x10:X,0x80:La
    UINT8  intensity;// 0,1,2,3
    UINT16 frequency;//频率(GHz)*100
    UINT8  currSpeed;//当前速度
    UINT16 currDirection; //当前方向
    UINT8  gpsAvailableNumber;//  可用卫星数
    END_PACKET endData;
}_PACKED_END RADAR_ALARM,*PRADAR_ALARM;

/*----功能菜单信息获取（电子狗<=>行车记录仪）（type:0x13）-------------------------------*/
#if defined( __GNUC__)
typedef   struct
#else
typedef _PACKED_BEGIN  struct
#endif
{
    START_PACKET startPack;
    UINT8  muteSetting;//静音设置 0:静音关闭,为 1 静音开启,云狗静音
    UINT8  volume; //音量设置  1,2,3,4,5,6
    UINT8  silentSpeed;//雷达静音速度  0,10,20,30,40,50,60
    UINT8  speedSetting;//超速设置  80,90,100,110,120,130(关闭)
    UINT8  modeSetting;//模式设置  0,1,2
    UINT8  speedCompensation;//速度补偿  -10~10
    UINT8  trafficStatu;//路况播报设置  此协议暂无用，不做菜单
    UINT16 version;//版本号  182
    UINT8  IMEI_ID[8];//IMEI 号
    UINT8  id[6];//本机号码
    UINT8  defaultSetting;//恢复出厂设置  0
    END_PACKET endData;
}_PACKED_END MENU_INFO,*PMENU_INFO;

/*----天气预报（电子狗=>行车记录仪）（type:0x18）-------------------------------*/
#if defined( __GNUC__)
typedef   struct
#else
typedef _PACKED_BEGIN  struct
#endif
{
    START_PACKET startPack;
    UINT8 type;//保留
    UINT8 msg[40];/*文字描述
    (F1 6D 33 57 02 5E 20 00 32 00 36 00 2D 00 33 00 32 00 44 64 0F 6C A6 5E 20 00
    35 96 E8 96 20 00(深圳市 26-32 摄氏度 阵雨))
    */
    END_PACKET endData;
}_PACKED_END WEATHER,*PWEATHER;

/*----投诉键协议(电子狗<=行车记录仪) ( type:0x19)-------------------------------*/
#if defined( __GNUC__)
typedef   struct
#else
typedef _PACKED_BEGIN  struct
#endif
{
    START_PACKET startPack;
    UINT8 satus;//// 申请投诉
    END_PACKET endData;
}_PACKED_END COMPLAIN,*PCOMPLAIN;

#if defined( __WIN32__)
#pragma pack()
#endif
///////////////////////////////////////////////////////////////////////////////////
/*
   函数名： getDataStruct()
   参 数 ：string==>串口获取到的字符串（数据包）
          type====>当前数据包类型
   返回值:返回当前数据包对应的结构体
   示例：
       string: 0x41,0x54,0x55,0x08,0x00,0x20,0x00,0x01,0x13,'\n'
       type:   <== 0x20
       return: <== SECURITY_BOOT (typedef struct SECURITY_BOOT)
*/
extern void *getDataStruct(char *string,DATA_TYPES *type);
extern BOOL  putDataStruct(void *m_struct,DATA_TYPES type);
extern BOOL  isCheckCurrSumError(void);
extern char *getCurrentIMIE_ID(void);
extern char *getCurrentPhone_ID(void);
#endif
