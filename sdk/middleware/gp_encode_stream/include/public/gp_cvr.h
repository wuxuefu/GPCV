#ifndef __GP_CVR_H__
#define __GP_CVR_H__

//Lane Departure Warning Function
typedef struct LDW_param_s
{
	UINT32 mode; //0: Sedan Car 小轎車 / 1: Commercial Verhicle 商務休旅車 / 2: Truck 卡車
	UINT32 sensitivity;	//0: low 1: high 2: unlimited
	UINT32 region;		//for different road 0: China 1: Taiwan
	UINT32 turn_on_speed; //LDW turn on speed. 0: very low 1: low speed 2: high speed
	UINT32 sfcw;		//FCW: forward collision warning, 0: disable 1: enable
	UINT32 StopAndGo;	//Stop And Go, 0: disable 1: enable
	int (*enable_f)(int);	//notify function for LDW enable, when LDW is turn on, this function will be trigger.
	int (*alarm_f)(int);	//notify function for LDW warning alarm, when LDW alram is on, this function will be trigger
	int (*getgValue)(void);//LDW get gsensor value
	int (*FCW_enable)(int);
    int (*GOALARM_enable)(int);
	int (*FCW_dispFlag)(int);
    int (*StopAndGOdispFlag)(int);	
} LDW_param_t;

typedef struct LDW_DISPLINE_S
{
	int LT_alarmP_X;//red 
	int LT_alarmP_Y;
	int LB_alarmP_X;
	int LB_alarmP_Y;

	int RT_alarmP_X;
	int RT_alarmP_Y;
	int RB_alarmP_X;
	int RB_alarmP_Y;

	int LTP_X;//green
	int LTP_Y;
	int LBP_X;
	int LBP_Y;

	int RTP_X;
	int RTP_Y;
	int RBP_X;
	int RBP_Y;

	int LLcheckFlg;
	int RLcheckFlg;
	int LLAlarmFlg;
	int RLAlarmFlg;

} LDW_DISPLINE_t;


void gp_CVR_LDW_Set(LDW_param_t *param);
void gp_CVR_LDW_GetLine(LDW_DISPLINE_t * para);

void gp_CVR_LDW_Start();
void gp_CVR_LDW_Stop();

#endif
