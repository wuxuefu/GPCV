#include "CloudDog.h"
#include "ap_state_config.h"
#include "font.h"
#include "disp.h"
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
extern gp_size_t lcd_size;
extern UINT8 HDMIxNum;
extern dv_set_t dv_set;
static MOVE_CAPTION *str_move_head=NULL;
static void callbak_func_weather_forecast(MOVE_CAPTION *arg);
static void callbak_func_data_time(MOVE_CAPTION *arg);
static void callbak_func_latitude(MOVE_CAPTION *arg);

MOVE_CAPTION move_string_init_tab[]={
    {0,0, NULL ,callbak_func_weather_forecast,{8,220,320,240,TRANSPARENT_COLOR,RGB888_TO_RGB565(0xffffff),0x618},NULL,NULL},
    {0,0, NULL ,callbak_func_data_time,       {8,220,320,240,TRANSPARENT_COLOR,RGB888_TO_RGB565(0xffffff),0x618},NULL,NULL},
    {0,0, NULL ,callbak_func_latitude,        {8,220,320,240,TRANSPARENT_COLOR,RGB888_TO_RGB565(0xffffff),0x618},NULL,NULL},
      };
static void callbak_func_weather_forecast(MOVE_CAPTION *arg)
{   //"深圳市 26-32 摄氏度 阵雨"
    static unsigned char weather[] = "深圳市 26-32 摄氏度 阵雨";
    //sprintf(arg->string_addr,"%s","北京市 26-32 摄氏度 阵雨");
    arg->string_addr  = weather;
}
static void callbak_func_data_time(MOVE_CAPTION *arg)
{ 
    //"2015年03月16号 星期一 00:00"
    const  unsigned char *week_string[]={"星期一","星期二","星期三","星期四","星期五","星期六","星期日"};
    static unsigned char data_time[]= "2015年03月16号 星期一 00:00";
    time_t t;
	struct tm tm;	
	time(&t);
	tm = *localtime(&t);
    sprintf(data_time,"%4d年%02d月%02d日 %s %02d:%02d",\
            (1900+tm.tm_year),tm.tm_mon,tm.tm_mday,week_string[tm.tm_wday],tm.tm_hour,tm.tm_min);
    arg->string_addr = data_time;
}
static void callbak_func_latitude(MOVE_CAPTION *arg)
{   
    static unsigned char latitude[] = "经度:1000.333 经度:1000.555";
    //"经度:1000.333 经度:1000.555"
    float jd,wd;
    //sprintf(arg->string_addr,"经度:%f 纬度:%f",jd,wd);
    arg->string_addr = latitude;
}
                 
/*********************************************************************************************
* gp_string_block_cpy + gp_string_block_cpy 实现显示自定义汉字
* move_string_insert_node + move_string_delete_node 向添加/删除移动字幕条目
*********************************************************************************************/
MOVE_CAPTION *move_string_insert_node(int l, MOVE_CAPTION *value, int *len)  
{ 
	int i; 
	MOVE_CAPTION *temp,*tail,*head = NULL;  
	if(NULL == str_move_head)
	{
	     str_move_head = (MOVE_CAPTION *)malloc(sizeof(MOVE_CAPTION)); 
	     str_move_head = value;
	     str_move_head->next = str_move_head->prev = str_move_head;
	     return ;
    }
	else head =str_move_head;
	temp = (MOVE_CAPTION *)malloc(sizeof(MOVE_CAPTION));  

	*temp = *value;
	
	temp->prev=NULL;    
	temp->next=NULL;   
	if(0 == l)           
	{ 
		head->next = temp;
 		temp->prev = head;
		//head=temp;   
	}    
	else            
	{  
		if(l == *len)      
		{
			tail = head->next;  
			for(i=1;i<l;i++)      
				tail = tail->next;  

			temp->prev = tail;  
			tail->next = temp;  
			
		}      
		else      
		{     
			tail = head;  
			for(i=1;i<l;i++)     
				tail = tail->next;   
			temp->prev = tail;   
			temp->next = tail->next;  
			(tail->next)->prev = temp; 
			tail->next = temp;     
		}   
	}
    (*len)++;    
	return(head);            
} 
MOVE_CAPTION *move_string_delete_node( int l, MOVE_CAPTION  **data, int *len) 
{ 
	
	int i;   
	MOVE_CAPTION *temp, *tail,*head;  
    if(NULL == str_move_head)
	{
	     return NULL;
	     printf("\033[31m%s() is NULL \n\033[0m",__func__);
    }
	else head = str_move_head;
	if( 1 == l)      
	{     
		tail = head;      
		head = head->next;       
		head->prev = NULL;  
	}   
	else  
	{      
		if(l == *len)   
		{                    
		    tail = head;         
			for(i=1;i<l;i++)         
			{            
				temp = tail;        
				tail = tail->next;  
			}  
			temp->next = NULL; 
		}      
		else     
		{           
		tail = head;        
			for(i=1;i<l;i++)     
			{             
				temp = tail;        
				tail = tail->next;         
			}         
			temp->next = tail->next;     
			(tail->next)->prev = temp; 
			//(tail->next)->prev = tail->prev;      
		}   
	}    
	
	(*len)--;                
	*data = tail;             
	free(tail);                  
	return(head);              
	
}  

static void gp_string_block_cpy(void *destbuf, int x_dest, int y_dest,int widthdest, int heightdest, void *srcbuf,  int widthsrc, int heightsrc,UINT16 trancolor,UINT16 background_color)
{
	int	i,j;
	short *src;
	short *dest;
	char *src_char;
	char* dest_char;
	char temp;
    dest = (short *)destbuf;
	src = (short *)srcbuf;
	dest = dest + (y_dest*widthdest + x_dest); 
    for(i=0;i<heightsrc*HDMIxNum;i++)
	{	
		src = (short *)srcbuf + (i/HDMIxNum)*widthsrc;
		//printf("i=%d,j=%d,dest =0x%x,src=0x%x\n",i,j,dest,src);
		for(j=0;j<widthsrc*HDMIxNum;j++)
		{
			if((*src&0xffff) != (trancolor&0xffff))
			{
				*dest++ = *src;
			}
			else if(background_color == trancolor)
			{
				*dest++ = background_color;
				*src;
			}
			else
			{
				*dest++ ;
				*src;
			}
			if(dv_set.display_mode== SP_DISP_OUTPUT_HDMI)
			{
				if((j%HDMIxNum==0)&&(j!=0))
					src++;
			}
			else
			{
				src++;
			}
		}
		dest = dest + widthdest - widthsrc*HDMIxNum;
	}
}
void gp_string_draw_unicode(UINT16*frame_buff,UINT8 *character,UNICODE_STRING_INFO *str_info)
{
    
    int x=0,y=0,offset,flag=0,str_count=0;
    int font_x=0,font_y=0;
    unsigned char *str_buffer;
    unsigned char *word = (unsigned char *)character;
    int strbufSize=0,fontSize=0,count=0,str2rgbSize=0;
    UINT16 *str2RgbBuf = NULL;
    void *destbuf = frame_buff;
    UINT32  font_width=0,font_height=0,font_width_new=0,font_height_new=0,fontDataOffset=0;

    UNICODE_STRING_INFO *strInfo= str_info;
    INT32   dev_width       = strInfo->dev_width        ? strInfo->dev_width        : 320;
	INT32   dev_height      = strInfo->dev_height       ? strInfo->dev_height       : 240;
    UINT16 tran_color       = strInfo->tran_color       ? strInfo->tran_color       : TRANSPARENT_COLOR;
    UINT16 string_color     = strInfo->string_color     ? strInfo->string_color     : 0xffff;
    UINT16 background_color = strInfo->background_color ? strInfo->background_color : TRANSPARENT_COLOR;

    if(strlen(word)<1)return ;
    //////////////////////////////////////////////
    FILE *fpch=NULL;
    fpch = fopen(FONT_PATH"font.bin", "rb");
    if(fpch==NULL){
		fprintf(stderr, "%s() open error\n",__func__);
		return ;
	}
    //////////////////////////////

    fseek(fpch, 0x05, SEEK_SET);
	fread(&font_width, 1, 1, fpch);
	fseek(fpch, 0x07, SEEK_SET);
	fread(&font_height, 1, 1, fpch);
	font_width_new = font_width;
	font_height_new = font_height;
    fontSize    = font_width*font_height/8;
    strbufSize  = fontSize+1;
    str2rgbSize = fontSize*sizeof(UINT16)*8;
    str2RgbBuf = (UINT16 *)malloc(str2rgbSize);
    memset(str2RgbBuf, '\0',str2rgbSize);
    str_buffer  = (unsigned char *)malloc(strbufSize);
    memset(str_buffer, '\0', strbufSize);
    //////////////////////////////////////////////////
	int next_str=0;
	unsigned char temp_str[3]={0};
	str_count = 0;
	font_x = strInfo->x*HDMIxNum;
    font_y =  strInfo->y*HDMIxNum;
	while(next_str < strlen(word))
	{
	    temp_str[0] = word[next_str];
	    temp_str[1] = word[next_str+1];
	    temp_str[3] = '\0';
        if (temp_str[0] < 0xa1)//ascii
        {
            next_str += 1;
            font_width_new = font_width/2;
            fseek(fpch, 0x08, SEEK_SET);
	        fread(&fontDataOffset, 1, 4, fpch);//read data offset
            offset = fontDataOffset+((UINT16)temp_str[0])*(fontSize/2);
            fseek(fpch, offset, SEEK_SET);
            fread(str_buffer, 1, fontSize/2, fpch);
            //printf("=====%c=====\n",temp_str[0]);
        }
        else
        {
            next_str += 2;
            font_width_new = font_width;
            fseek(fpch, 0x0c, SEEK_SET);
            fread(&fontDataOffset, 1, 4, fpch);//read data offset
            offset = fontDataOffset+(94*(UINT16)(temp_str[0]-0xa0-1)+(temp_str[1]-0xa0-1))*fontSize;
            fseek(fpch, offset, SEEK_SET);
            fread(str_buffer, 1, fontSize, fpch);
        }
        ///////////////////////////////////////////
         count = 0;
         for(y=0; y<font_height_new; y++)
         {
            for(x=0; x<font_width_new; x++)
            {
                flag = str_buffer[y*(font_width_new/8)+(x/8)]&(0x80>>(x%8));
                if(0 != flag)  *(str2RgbBuf+count) = string_color;
                else           *(str2RgbBuf+count) = tran_color;
                count++;
            }
        }
        //printf("str_count:%d next_str:%d strSize:%d str:%s font_width:%d font_height:%d x:%d y:%d\n",\
                str_count,next_str,strlen(word),temp_str,font_width_new,font_height_new,\
                font_x,font_y);
        gp_string_block_cpy(destbuf,font_x,font_y,strInfo->dev_width, strInfo->dev_height,str2RgbBuf,font_width_new,font_height_new,tran_color,background_color);
        str_count++;
        font_x += font_width_new;

    }
    fclose(fpch);
    free(str_buffer);
    free(str2RgbBuf);

}
void move_string_init(void)
{
    static int isInitOk = 0;
    int l=0,i=0;
    int arrlen = sizeof(move_string_init_tab)/sizeof(move_string_init_tab[0]);
    if(0 != isInitOk)
        return ;
    for(i=0; i<arrlen; i++)
    {
        move_string_init_tab[i].callbak_func(&move_string_init_tab[i]);
        printf("ID:%d string:%s arrlen:%d\n",i,move_string_init_tab[i].string_addr,arrlen);
        str_move_head = move_string_insert_node(l+1,&move_string_init_tab[i], &l);
    }
    MOVE_CAPTION *head=NULL,*currNod=NULL; 
    head = str_move_head;
    i=0;
    currNod = head;
    while(i<arrlen)
    {
        currNod = currNod->next;
        i++;   
        printf("ID:%d currNod:0x%X currNod->Next:0x%X currNod->Prev:0x%X\n",i,\
            currNod,currNod->next,currNod->prev);

    }
    isInitOk = 1;
}
void move_string_task(UINT16 *frame_buff,int isMove)
{
    struct timeval t_v;
    static unsigned long t_t=0;
    static UNICODE_STRING_INFO *strinf=NULL;//{8,220,320,240,TRANSPARENT_COLOR,RGB888_TO_RGB565(0xffffff),0x618};
    static MOVE_CAPTION *currNode=NULL;
    static int oldx=0,oldy=0;
    move_string_init();

    if(NULL == currNode)
    {
        currNode = str_move_head;
        if(NULL == currNode) return ;
        oldx = currNode->str_info.x;
        oldy = currNode->str_info.y;
    }
    else
    {
        if(abs(currNode->str_info.x) >= (lcd_size.width-oldx))
        {
            currNode->str_info.x  = oldx;
            currNode->str_info.y  = oldy;
            currNode = currNode->next;
            oldx = currNode->str_info.x;
            oldy = currNode->str_info.y;
        }
    }
    if(NULL == currNode->string_addr)
    {
        printf("\033[32m curr Node String address is NULL \n\033[0m");
        return ;
    }
    gettimeofday(&t_v,NULL);
    strinf = &currNode->str_info;
    strinf->y  = 220;
    if((t_v.tv_sec-t_t) >= 1)
    {
        strinf->x -= 6;
        
        printf("x:%d y:%d t_t:%ld sec:%ld sec-t_t:%ld\n",strinf->x,strinf->y,t_t,t_v.tv_sec,t_v.tv_sec-t_t);
        if(NULL != currNode->callbak_func)
            currNode->callbak_func(currNode);
        gp_string_draw_unicode(frame_buff,currNode->string_addr,strinf);
        t_t = t_v.tv_sec;	

    }
    else 
    {
        gp_string_draw_unicode(frame_buff,currNode->string_addr,strinf);
    }

}
