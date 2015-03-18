#ifndef __CSI__MD__H__
#define __CSI__MD__H__

typedef int md_triger_f(void *arg);

typedef struct csi_md_area_s 
{
	int enable;
	int x;
	int y;
	int width;
	int height;
	int detect[5];
	int count[5];
	unsigned int detect_sum;
	unsigned int sum;
	md_triger_f *md_triger;
	void *arg;
}csi_md_area_t;

#define AREA_NUM 5

extern struct csi_md_area_s g_md_area[AREA_NUM]; 
extern int g_md_enable;
/**********************************
��ʼ��motion detect
***********************************/
int gd_md_open();
/***********************************
�ر�motion detect
***********************************/
int gd_md_close();
/*********************************
��ȡ��ⷶΧ
*********************************/
int gd_md_get_resolution(int *width,int *height);
/***********************************************
ע����ⷶΧ
x,y widht,height Ϊ��ⷶΧ
md_triger_f callback����.���Ե��ƶ�����. ע��: �ú�����Ҫ��������,����˯��
arg : callback�����Ĳ���
************************************************/
int gp_md_register_area(int x,int y,int width,int height,md_triger_f f,void *arg);
/*********************
ȡ����� ,iΪgp_md_register_area�ķ���ֵ
************************/
int gd_md_unregister_area(int i);
#endif
