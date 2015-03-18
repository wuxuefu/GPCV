#ifndef __GP__CSI__ATTR__H__
#define __GP__CSI__ATTR__H__

#include "mach/typedef.h"

#define GP_CSI_FREQUENCY_50HZ	0
#define GP_CSI_FREQUENCY_60HZ	1

typedef struct gpIPC_VDEV_ATTR_S {
    UINT32 grayMode;    /*gray mode: 0: disable, 1: enable*/
	UINT32 filp;
    UINT32 Mirror;      /*Mirror: 0: disable, 1: enable*/
    UINT32 LumaVal;     /*luminance : [0~100], default: 50*/
    UINT32 ContrVal;    /*contrast  : [0~100], default: 50*/
    UINT32 HueVal;      /*hue       : [0~100], default: 50*/
    UINT32 SatuVal;     /*satuature : [0~100], default: 50*/

    UINT32 EV;			//Exposure Value 0:+2, 1:+5/3, 2:+4/3, 3:+1.0, 4:+2/3, 5:+1/3, 6:+0.0, 7:-1/3, 8:-2/3, 9:-1.0, 10:-4/3, 11:-5/3, 12:-2.0 
    UINT32 Sharpness;	//0:Soft 1: Normal 2: Strong
    UINT32 AwbMode;		//AWB_AUTO_DC, AWB_DAYLIGHT, AWB_CLOULDY, AWB_LAMP, AWB_FLUORESCENT, please see mach/gp_cdsp.h
    UINT32 SpecialEffect;	//MODE_NORMAL, MODE_BLACKWHITE, MODE_SEPIA, please see mach/gp_cdsp.h
    UINT32 Iso;				//ISO_AUTO, 100, 200, 400, 800
    UINT32 Frequency;		//GP_CSI_FREQUENCY_50HZ, GP_CSI_FREQUENCY_60HZ
}IPC_VDev_Attr_s;
/****************************************************************
 * description: 打开Sensor设备函数。该函数需要与gp_IPC_VDev_Close配合使用。
 * 打开设备后，处于disable状态，可以调用其他函数进行设置。通过调用
 * gp_IPC_VDev_Enable函数使得设备进入enable状态。打开成功返回设备的
 * handle。
 * param: dev_attr [in]: IPC_VDev_Attr_s *, 需要设置的参数。
 * return: NULL:打开失败，其他值: 设备的handle。
 * **************************************************************/
void *gp_IPC_VDev_Open(IPC_VDev_Attr_s *dev_attr);

/*****************************************************************
 * description: 关闭设备函数。
 * param: CHandle [in]: 设备handle.
 * return: GP_SUCCESS: 成功，other value: 错误码。
 * **************************************************************/
SINT32 gp_IPC_VDev_Close(void *DHandle);

/****************************************************************
 * description: 获得设备设置参数。
 * param: CHandle [in]: 设备handle.
 * param: dev_attr [out]: IPC_VDev_Attr_s *, 保存获得的参数。
 * return: NULL:打开失败，其他值: 设备的handle。
 * **************************************************************/
SINT32 gp_IPC_VDev_Get_Attr(void *handle, IPC_VDev_Attr_s *dev_attr);

/*****************************************************************
 * description: enable 设备函数。open设备后需要调用该函数，设备才
 * 可以使用。
 * param: DHandle [in]: 设备handle.
 * return: GP_SUCCESS: 成功，other value: 错误码。delet
 * **************************************************************/
SINT32 gp_IPC_VDev_Enable(void *DHandle);

/*****************************************************************
 * description: disable 设备函数。
 * param: DHandle [in]: 设备handle.
 * return: GP_SUCCESS: 成功，other value: 错误码。delet
 * **************************************************************/
SINT32 gp_IPC_VDev_Disable(void *DHandle);

/*****************************************************************
 * description: enable 灰度模式。
 * param: DHandle [in]: 设备handle.
 * return: GP_SUCCESS: 成功，other value: 错误码。
 * **************************************************************/
SINT32 gp_IPC_VDev_Enable_GrayMode(void *DHandle);

/*****************************************************************
 * description: disable 灰度模式。
 * param: DHandle [in]: 设备handle.
 * return: GP_SUCCESS: 成功，other value: 错误码。
 * **************************************************************/
SINT32 gp_IPC_VDev_Disable_GrayMode(void *DHandle);

/*****************************************************************
 * description: 相对于当前Image做镜像。
 * param: DHandle [in]: 设备handle.
 * return: GP_SUCCESS: 成功，other value: 错误码。
 * **************************************************************/
SINT32 gp_IPC_VDev_MirrorImage(void *DHandle, int mirror);

/*****************************************************************
 * description: 相对于当前Image做Filp。
 * param: DHandle [in]: 设备handle.
 * return: GP_SUCCESS: 成功，other value: 错误码。
 * **************************************************************/
SINT32 gp_IPC_VDev_FlipImage(void *DHandle, int flip);

/*****************************************************************
 * description: 设置亮度。
 * param: DHandle [in]: 设备handle.
 * param: LumaVal [in]: UINT32 ,取值0~100。
 * return: GP_SUCCESS: 成功，other value: 错误码。
 * **************************************************************/
SINT32 gp_IPC_VDev_Set_Luminance(void *DHandle, UINT32 LumaVal);

/*****************************************************************
 * description: 设置对比度。
 * param: DHandle [in]: 设备handle.
 * param: ContrVal [in]: UINT32 ,取值0~100。
 * return: GP_SUCCESS: 成功，other value: 错误码。
 * **************************************************************/
SINT32 gp_IPC_VDev_Set_Contrast(void *DHandle, UINT32 ContrVal);

/*****************************************************************
 * description: 设置色彩度。
 * param: DHandle [in]: 设备handle.
 * param: HueVal [in]: UINT32 ,取值0~100。
 * return: GP_SUCCESS: 成功，other value: 错误码。
 * **************************************************************/
SINT32 gp_IPC_VDev_Set_Hue(void *DHandle, UINT32 HueVal);

/*****************************************************************
 * description: 设置饱和度。
 * param: DHandle [in]: 设备handle.
 * param: SatuVal [in]: UINT32 ,取值0~100。
 * return: GP_SUCCESS: 成功，other value: 错误码。
 * **************************************************************/
SINT32 gp_IPC_VDev_Set_Satuature(void *DHandle, UINT32 SatuVal);

/*****************************************************************
 * description: Set EV/Sharpness/AWB/Special Effect/ISO
 * param: DHandle [in]: 设备handle.
 * param: Val [in]: UINT32 see define in IPC_VDev_Attr_s
 * return: GP_SUCCESS: 成功，other value: 错误码。
 * **************************************************************/
SINT32 gp_IPC_VDev_Set_Exposure(void *DHandle, UINT32 Val);
SINT32 gp_IPC_VDev_Set_Sharpness(void *DHandle, UINT32 Val);
SINT32 gp_IPC_VDev_Set_AWB(void *DHandle, UINT32 Val);
SINT32 gp_IPC_VDev_Set_Effect(void *DHandle, UINT32 Val);
SINT32 gp_IPC_VDev_Set_ISO(void *DHandle, UINT32 Val);
SINT32 gp_IPC_VDev_Set_Frequency(void* handle, UINT32 Val);

/*****************************************************************
 * description: Reset all to default
 * param: DHandle [in]: 设备handle.
 * return: GP_SUCCESS: 成功，other value: 错误码。
 * **************************************************************/
SINT32 gp_IPC_VDev_Reset_Attr(void *handle);

#endif
