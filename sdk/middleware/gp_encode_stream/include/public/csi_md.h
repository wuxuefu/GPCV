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
初始化motion detect
***********************************/
int gd_md_open();
/***********************************
关闭motion detect
***********************************/
int gd_md_close();
/*********************************
获取侦测范围
*********************************/
int gd_md_get_resolution(int *width,int *height);
/***********************************************
注册侦测范围
x,y widht,height 为侦测范围
md_triger_f callback函数.测试到移动触发. 注意: 该函数需要立即返回,不能睡眠
arg : callback函数的参数
************************************************/
int gp_md_register_area(int x,int y,int width,int height,md_triger_f f,void *arg);
/*********************
取消侦测 ,i为gp_md_register_area的返回值
************************/
int gd_md_unregister_area(int i);
#endif
