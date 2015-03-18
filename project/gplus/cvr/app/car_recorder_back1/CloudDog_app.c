#include "CloudDog.h"
#include "ap_state_config.h"
#include "font.h"
#include "disp.h"
#include "CloudDog_Table.c"


#define __MODULE__          "CloudDog_app"
#define __DBGLVL__          4 // 0=OFF, 1=ERROR, 2=WAENING 3=MSG 4=ALL
#define __DBGNAME__         printf
#include "dbgs.h"
extern HANDLE hDisp;
extern UINT8 HDMIxNum;
extern dv_set_t dv_set;
static gp_size_t lcd_size={0};
static void *gp_dogDispBuf=NULL;
static void *packData=NULL;
static gp_point_t pointOffset={0};
/////////////////////////////////////////////////
//          overall 
////////////////////////////////////////////////
extern UINT8 Playmode_flag;
extern UINT8 cur_draw_flag;
extern UINT8 foreground_draw_flag;


static SINT32 gp_Icon_draw_Index(UINT8 index);
static void gp_Icon_draw_test(int isShow)
{
    if(0==isShow)
        return ;

    static UINT8 compass=0,gpsNum=0, unmId=0,radar=0,speedWrn=0,spPorint=0,warning=0;
    compass++;
    gpsNum++;
    unmId++;
    radar++;
    speedWrn++;
    spPorint++;
    warning++;

    //printf("compass:%d gpsNum:%d unmId:%d radar:%d speedWrn:%d spPorint:%d warning:%d\n",\
        compass,gpsNum,unmId,radar,speedWrn,spPorint,warning);
    if(compass >(DOG_ICON_R_COMPASS_8-DOG_ICON_R_COMPASS_1))
        compass=0;
    if(gpsNum > (DOG_ICON_R_GPS_NUM20-DOG_ICON_R_GPS_NUM1))
        gpsNum =0;
    if(radar > (DOG_ICON_R_RADAR4_K-DOG_ICON_R_RADAR0_X))
        radar =0;
    if(speedWrn > (DOG_ICON_R_SPEED_W_NULL -DOG_ICON_R_SPEED_10))
        speedWrn=0;
    if(warning > (DOG_ICON_R_WARNING_110-DOG_ICON_R_WARNING_02))
        warning =0;
    if(spPorint > (DOG_ICON_R_SPEED_POINTER_39-DOG_ICON_R_SPEED_POINTER_0))
        spPorint=0;
    gp_Icon_draw_Index(DOG_ICON_R_COMPASS_1+compass);
    //gp_Icon_draw_Index(DOG_ICON_R_GPS_NUM1+gpsNum);
    gp_Icon_draw_Index(DOG_ICON_R_RADAR0_X+radar);
    gp_Icon_draw_Index(DOG_ICON_R_SPEED_10+speedWrn);
    gp_Icon_draw_Index(DOG_ICON_R_WARNING_02+warning);
    gp_Icon_draw_Index(DOG_ICON_R_SPEED_POINTER_0+spPorint);
    
    int g,s,b,q;
    q=unmId/1000;
    b=unmId/100%10;
    s=unmId/10%10;
    g=unmId%10;
    gp_Icon_draw_Index(DOG_ICON_R_NUM_0+q);
    
    pointOffset.x = 160-NUM_X;
    gp_Icon_draw_Index(DOG_ICON_R_NUM_0+b);
    
    pointOffset.x = 176-NUM_X;
    gp_Icon_draw_Index(DOG_ICON_R_NUM_0+s);
    
    pointOffset.x = 192-NUM_X;
    gp_Icon_draw_Index(DOG_ICON_R_NUM_0+g);
    pointOffset.x = 0;
    pointOffset.y = 0;
    
}
UINT16 * gp_get_dog_dispInfo(void)
{
    //如果使用独立线程就要开启
#if 1
    dispGetResolution(hDisp, &lcd_size);	
    gp_dogDispBuf= (UINT16*)dispGetFramebuffer(hDisp);
#endif
     return (UINT16 *)gp_dogDispBuf;
}
static UINT16 *gp_clear_disp_buf(void)
{
    //get display buf info
    UINT16* addr1;
    addr1 = gp_get_dog_dispInfo();
    #if 1
    
    if((dv_set.sd_check!=1)||(dv_set.sdc_full !=0)||(dv_set.no_power_flag !=0)||(dv_set.sdc_error== 1))
    {
        for (int y = 0; y <lcd_size.height; y++) {		
        	for (int x = 0; x < lcd_size.width; x++) {	
        		*addr1++ = TRANSPARENT_COLOR;		
        	}	
        }
        foreground_draw_flag++;
    }
	if(dv_set.ui_show == 0)
	{
		dispFlip(hDisp);
		cur_draw_flag = DV_EXIT_MODE;
		foreground_draw_flag=0;		
		return NULL;
	}
    gp_Icon_draw_Index(DOG_ICON_BACKGROUND);

    #else
    //memset(addr1,TRANSPARENT_COLOR,lcd_size.height*lcd_size.width);
    #endif
    return addr1;
}
static SINT32 gp_Icon_draw_Index(UINT8 index)
{
    int x=0,y=0,icon_width=0,icon_height=0;
    UINT16 tran_color=0,background_color=0;
    iconManager_t *pIconItem=&Icon_Manager_CloudDog[index];
    void *destbuf = gp_dogDispBuf;
    x =      pIconItem->x_pos*HDMIxNum+pointOffset.x;
    y =      pIconItem->y_pos*HDMIxNum+pointOffset.y;
    icon_width =  pIconItem->icon_width;
    icon_height = pIconItem->icon_height;
    tran_color =  pIconItem->tran_color;
    background_color = TRANSPARENT_COLOR;
    
//////////////////////////////////////////////////////////////////////////
    UINT16 i;
	SINT32 fd;
	UINT32 file_size;
	UINT32  * addr=NULL;
	char *fileType=NULL;
	static char icon_file_path[256];
	if(NULL == destbuf)
	{
	    __err("Not Display Buff index:%d\n",index);
	    return -1;
	}
    //open index Bin
	if(index < DOG_ICON_NUM_MAX)
	{
		if(pIconItem->addr == NULL)
		{
    		strcpy(icon_file_path, ICON_PATH_DOG);
    		strcat(icon_file_path, pIconItem->icon_name);	
            fileType = &icon_file_path[strlen(icon_file_path)-4];
    		if(0==strcmp(fileType,".bmp"))
    		{
    		    addr = (UINT32  *)openBmpFile(icon_file_path);
    		    if(NULL != addr)
    		    {
    		       BITMAPINFOHEADER *fileHead = (BITMAPINFOHEADER *)getBimpFileHeadInfo();
                   pIconItem->icon_width      = fileHead->biWidth;
                   pIconItem->icon_height     = abs(fileHead->biHeight);
    		    }
                pIconItem->addr = addr;
    		}
    		else if(0==strcmp(fileType,".bin"))
    		{
                fd = open(icon_file_path,O_RDONLY);
        		if(fd >=0)
        		{
        			file_size = lseek(fd,0,SEEK_END);
        			lseek(fd,0,SEEK_SET);
        			addr = (UINT32 *)malloc(file_size);	
        			read(fd,addr,file_size);
        			close(fd);	
        			pIconItem->addr = addr;
        		}
        		else printf("\033[32mopen Bin Fail\n\033[0m");
        		
    		}

		}
		else 
		{
            addr = pIconItem->addr;
		}
	}	

    if(addr != NULL)
	{
        gp_block_cpy(destbuf,x,y,lcd_size.width, lcd_size.height,addr,icon_width,icon_height,tran_color,background_color);
		return 0;
	}else
	{
        __inf("open file %s  :0x%X  DispAddr:0x%X  ICON index = %d X=%d,Y=%d LcdW:%d LcdH:%d\n", \
	        icon_file_path,addr,destbuf,index,x,y,lcd_size.width,lcd_size.height);
	}

	return -1;
}
static void gp_dog_icon_commInfo(void)
{
    int battayLeve=dv_set.battery_state;
    if(battayLeve > 3)
        battayLeve = 3;
    else if(battayLeve<0)
        battayLeve =0;
    gp_Icon_draw_Index(DOG_ICON_R_BAT_0+battayLeve);
}
static void gp_dog_icon_GpsInfo(PGPS_INFO arg)
{
    
    PGPS_INFO gpsInfo = arg;
    int gsm_level=0;
    //Gps status
    #if 0
    if('A'==gpsInfo->gpsStatus)
         gp_Icon_draw_Index(DOG_ICON_R_GPS_ON);
    else gp_Icon_draw_Index(DOG_ICON_R_GPS_OFF);
    #else
        gp_Icon_draw_Index(DOG_ICON_R_GPS_NUM1);
    #endif
    //GSM Signal
    gsm_level=gpsInfo->gsmSignal*(5-1)/31 + 1;
    if(gsm_level>4)
        gsm_level = 4;
    gp_Icon_draw_Index(DOG_ICON_R_SIGNAL_0+gsm_level);

}

static SINT32 gp_disp_dog_Icon(void)
{   
    const void *currPackData = packData;// Read Only- pointer

    if(NULL==currPackData)
    {
        __err("not CloudDog Pack Data\n");
        return -1;
    }    
    gp_get_dog_dispInfo();
    gp_dog_icon_commInfo();
    gp_Icon_draw_test(1);
    //resolve data pack
    START_PACKET *startPack= (START_PACKET *)currPackData;
    switch (startPack->type) {
        case TYPE_SECURITY_BOOT:
            putDataStruct(currPackData,startPack->type);
            break;
        case TYPE_GPS_INFO:
            gp_dog_icon_GpsInfo((PGPS_INFO)currPackData);
            break;
        default:
            break;
        }

}
void *cloudDog_task(void)
{
    UINT16 *pRetValue=NULL;
	if(dv_set.change_disp != 0)
		return;

        //if((Playmode_flag==CMD_VIDEO_MODE) && (DISABLE!=dv_set.ui_show) && (dv_set.dv_UI_flag==DV_FOREGROUND_MODE) )
        {
            //if((dv_set.sd_check!=1)||(dv_set.sdc_full !=0)||(dv_set.no_power_flag !=0)||(dv_set.sdc_error== 1))
            {
                pRetValue = gp_clear_disp_buf();
                if(NULL==pRetValue)
                    return NULL;
                gp_disp_dog_Icon();
            }
        }

	if(foreground_draw_flag>=2)
	{
		foreground_draw_flag = 0;
		cur_draw_flag = DV_FOREGROUND_MODE;
	}	
    dispFlip(hDisp);
    disp_OSD0_Set_Alpha(hDisp,100);

    printf("%s() UI_Flag:%d Playmode_flag:%d ui_show:%d\n",__func__,dv_set.dv_UI_flag,Playmode_flag,dv_set.ui_show);

    //disp_OSD0_Set_Alpha(hDisp,0);	
}
void *CloudDog_Thread(void *arg)
{
    DATA_TYPES type;
    UINT8 msg_id,ret;
    dv_set.extended_app = EXTAPP_CLOUD_DOG;//EXTAPP_NULL;//EXTAPP_CLOUD_DOG;
        
    while((DISABLE==dv_set.ui_show) || (EXTAPP_NULL==dv_set.extended_app))
    {
        sleep(2);
        switch(dv_set.extended_app)
        {
            case EXTAPP_CLOUD_DOG:
                        __wrn("wait ui show\n");
            break;
            case EXTAPP_GPS:
            case EXTAPP_TOUCH:
            default: break;
        }
    };
    while(1)
    {
        
        packData = getDataStruct(cloudDogString[1],&type);
        cloudDog_task();
        //sleep(1);
       usleep(500000); //500ms
    }

}


