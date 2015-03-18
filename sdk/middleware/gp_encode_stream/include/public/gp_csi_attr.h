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
 * description: ��Sensor�豸�������ú�����Ҫ��gp_IPC_VDev_Close���ʹ�á�
 * ���豸�󣬴���disable״̬�����Ե������������������á�ͨ������
 * gp_IPC_VDev_Enable����ʹ���豸����enable״̬���򿪳ɹ������豸��
 * handle��
 * param: dev_attr [in]: IPC_VDev_Attr_s *, ��Ҫ���õĲ�����
 * return: NULL:��ʧ�ܣ�����ֵ: �豸��handle��
 * **************************************************************/
void *gp_IPC_VDev_Open(IPC_VDev_Attr_s *dev_attr);

/*****************************************************************
 * description: �ر��豸������
 * param: CHandle [in]: �豸handle.
 * return: GP_SUCCESS: �ɹ���other value: �����롣
 * **************************************************************/
SINT32 gp_IPC_VDev_Close(void *DHandle);

/****************************************************************
 * description: ����豸���ò�����
 * param: CHandle [in]: �豸handle.
 * param: dev_attr [out]: IPC_VDev_Attr_s *, �����õĲ�����
 * return: NULL:��ʧ�ܣ�����ֵ: �豸��handle��
 * **************************************************************/
SINT32 gp_IPC_VDev_Get_Attr(void *handle, IPC_VDev_Attr_s *dev_attr);

/*****************************************************************
 * description: enable �豸������open�豸����Ҫ���øú������豸��
 * ����ʹ�á�
 * param: DHandle [in]: �豸handle.
 * return: GP_SUCCESS: �ɹ���other value: �����롣delet
 * **************************************************************/
SINT32 gp_IPC_VDev_Enable(void *DHandle);

/*****************************************************************
 * description: disable �豸������
 * param: DHandle [in]: �豸handle.
 * return: GP_SUCCESS: �ɹ���other value: �����롣delet
 * **************************************************************/
SINT32 gp_IPC_VDev_Disable(void *DHandle);

/*****************************************************************
 * description: enable �Ҷ�ģʽ��
 * param: DHandle [in]: �豸handle.
 * return: GP_SUCCESS: �ɹ���other value: �����롣
 * **************************************************************/
SINT32 gp_IPC_VDev_Enable_GrayMode(void *DHandle);

/*****************************************************************
 * description: disable �Ҷ�ģʽ��
 * param: DHandle [in]: �豸handle.
 * return: GP_SUCCESS: �ɹ���other value: �����롣
 * **************************************************************/
SINT32 gp_IPC_VDev_Disable_GrayMode(void *DHandle);

/*****************************************************************
 * description: ����ڵ�ǰImage������
 * param: DHandle [in]: �豸handle.
 * return: GP_SUCCESS: �ɹ���other value: �����롣
 * **************************************************************/
SINT32 gp_IPC_VDev_MirrorImage(void *DHandle, int mirror);

/*****************************************************************
 * description: ����ڵ�ǰImage��Filp��
 * param: DHandle [in]: �豸handle.
 * return: GP_SUCCESS: �ɹ���other value: �����롣
 * **************************************************************/
SINT32 gp_IPC_VDev_FlipImage(void *DHandle, int flip);

/*****************************************************************
 * description: �������ȡ�
 * param: DHandle [in]: �豸handle.
 * param: LumaVal [in]: UINT32 ,ȡֵ0~100��
 * return: GP_SUCCESS: �ɹ���other value: �����롣
 * **************************************************************/
SINT32 gp_IPC_VDev_Set_Luminance(void *DHandle, UINT32 LumaVal);

/*****************************************************************
 * description: ���öԱȶȡ�
 * param: DHandle [in]: �豸handle.
 * param: ContrVal [in]: UINT32 ,ȡֵ0~100��
 * return: GP_SUCCESS: �ɹ���other value: �����롣
 * **************************************************************/
SINT32 gp_IPC_VDev_Set_Contrast(void *DHandle, UINT32 ContrVal);

/*****************************************************************
 * description: ����ɫ�ʶȡ�
 * param: DHandle [in]: �豸handle.
 * param: HueVal [in]: UINT32 ,ȡֵ0~100��
 * return: GP_SUCCESS: �ɹ���other value: �����롣
 * **************************************************************/
SINT32 gp_IPC_VDev_Set_Hue(void *DHandle, UINT32 HueVal);

/*****************************************************************
 * description: ���ñ��Ͷȡ�
 * param: DHandle [in]: �豸handle.
 * param: SatuVal [in]: UINT32 ,ȡֵ0~100��
 * return: GP_SUCCESS: �ɹ���other value: �����롣
 * **************************************************************/
SINT32 gp_IPC_VDev_Set_Satuature(void *DHandle, UINT32 SatuVal);

/*****************************************************************
 * description: Set EV/Sharpness/AWB/Special Effect/ISO
 * param: DHandle [in]: �豸handle.
 * param: Val [in]: UINT32 see define in IPC_VDev_Attr_s
 * return: GP_SUCCESS: �ɹ���other value: �����롣
 * **************************************************************/
SINT32 gp_IPC_VDev_Set_Exposure(void *DHandle, UINT32 Val);
SINT32 gp_IPC_VDev_Set_Sharpness(void *DHandle, UINT32 Val);
SINT32 gp_IPC_VDev_Set_AWB(void *DHandle, UINT32 Val);
SINT32 gp_IPC_VDev_Set_Effect(void *DHandle, UINT32 Val);
SINT32 gp_IPC_VDev_Set_ISO(void *DHandle, UINT32 Val);
SINT32 gp_IPC_VDev_Set_Frequency(void* handle, UINT32 Val);

/*****************************************************************
 * description: Reset all to default
 * param: DHandle [in]: �豸handle.
 * return: GP_SUCCESS: �ɹ���other value: �����롣
 * **************************************************************/
SINT32 gp_IPC_VDev_Reset_Attr(void *handle);

#endif
