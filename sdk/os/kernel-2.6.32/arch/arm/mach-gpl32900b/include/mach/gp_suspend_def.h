#ifndef _GP_SUSPEND_DEF_H_
#define  _GP_SUSPEND_DEF_H_
typedef enum{
    GP_SUSPEND_TYPE_MEM,
	GP_SUSPEND_TYPE_SOC_PWR_DOWN_MEM_EMU,
    GP_SUSPEND_TYPE_SOC_PWR_DOWN_I2C,
    GP_SUSPEND_TYPE_SOC_PWR_REG_CMD
}GP_SUSPEND_TYPE;


typedef struct{
	GP_SUSPEND_TYPE mType;
	int mCommandArray[16];
	int mNumCommand;
	int mCommandDelay;
	int mI2C_slave_Addr;
	int mI2C_clock;
} gp_suspend_helper;

extern GP_SUSPEND_TYPE soc_pwr_get_info_suspend_type(void);
#endif
