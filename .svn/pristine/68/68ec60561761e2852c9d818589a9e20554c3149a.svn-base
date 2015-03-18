#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>

#include "CloudDog.h"

//#include "../openplatform/sdk/include/dbgs.h"
char string[7][500]={ //0 启动安防开机协议
                     {0x41,0x54,0x55,0x08,0x00,0x20,0x00,0x01,0x13,'\n'},
                     //1 GPS 信息
                     {0x41,0x54,0x55,0x21,0x00,0x10,0x41,0x3A,0x0B,0x23,0x00, 0x4E,0x7A,0xCE,0xAD,0x00,0x45,0x1C,0x32,0x00,0x00,0x00,0x00, 0xDB,0x07,0x03,0x07,0x11,0x37,0x23,0x1F,0x01,0x00,0x11,'\n'},
                     //2 固定报警信息
                     {0x41,0x54,0x55,0x3C,0x00,0x11,0x02,0x4C,0x01,0x3C,0x2B,0x8A,0x00,0x00,0x4D,0x52,0xB9,0x65,0x33,0x00,0x30,0x00,0x30,0x00,0x73,0x7C,0x09,0x67,0xEF,0x95,0xA2,0x7E,0x6F,0x70,0xCD,0x62,0x67,0x71,0x2C,0x00,0x50,0x96,0x1F,0x90,0x36,0x00,0x30,0x00,0x6C,0x51,0xCC,0x91,0x2C,0x00,0xF7,0x8B,0xE8,0x6C,0x0F,0x61,0x63,'\n'},
                     //3 雷达报警信息
                     {0x41,0x54,0x55,0x0E,0x00,0x12,0x20,0x02,0xFF,0xFF,0x24,0x2C,0x00,0x00,0x7A,'\n'},
                     //4 功能菜单信息获取
                     {0x41,0x54,0x55,0x1E,0x00,0x13,0x00,0x06,0x32,0x78,0x01,0x0A,0x00,0xB6,0x00,0x53,0x96,0x75,0x30,0x30,0x35,0x75,0x08,0x31,0x14,0x72,0x36,0x89,0x04,0x00,0x76,'\n'},
                     //5 天气预报
                     {0x41,0x54,0x55,0x27,0x00,0x18,0xAA,0xF1,0x6D,0x33,0x57,0x02,0x5E,0x20,0x00,0x32,0x00,0x36,0x00,0x2D,0x00,0x33,0x00,0x32,0x00,0x44,0x64,0x0F,0x6C,0xA6,0x5E,0x20,0x00,0x35,0x96,0xE8,0x96,0x20,0x00,0xE5,'\n'},
                     //6 投诉键协议
                     {0x41,0x54,0x55,0x07,0x00,0x19,0x01,0x0B,0x0D,0x0A,'\n'},
                    };
char stringLength[7]={sizeof(SECURITY_BOOT),sizeof(GPS_INFO),sizeof(FIXED_ALARM),\
                      sizeof(RADAR_ALARM),sizeof(MENU_INFO),sizeof(WEATHER),sizeof(COMPLAIN)};
      
int main(int argc, char *argv[])
{
	int i=0;
	printf("========welcom to extended app =============argc:%d====\r\n",argc);
	for(i=0;i<argc;i++)
		printf("argv[%d]:%s\n",i,argv[i]);
    printf("UINT8:%ld  UINT16:%ld UINT32:%ld  UINT64:%ld\n",\
            sizeof(UINT8),sizeof(UINT16),sizeof(UINT32),sizeof(UINT64));
    printf("START_PACKETSize:%ld UINT32:%ld GPS_INFOSize:%ld SECURITY_BOOTSize:%ld\n",\
            sizeof(START_PACKET),sizeof(UINT32),sizeof(GPS_INFO),sizeof(SECURITY_BOOT));

    int j=0;
    DATA_TYPES type;
    void *getPack=NULL;
    for(j=0;j<7;j++)
    {
        i=0;
         printf("\n--------------------------ID:%d  Size:%d--------------------------------\n",j,stringLength[j]);
        getPack = getDataStruct(string[j],&type);
        switch (type) {
        case TYPE_SECURITY_BOOT:
            putDataStruct(getPack,type);
            break;
        default:
            break;
        }
        printf("\n*************************************************************************\n\n\n");
    }
    //Dsp_Open();
    printf("\n\n\n\n\n\n");
    return 0;
}

SINT32 SPI_Flash_update(void)
{
	int spi_fd,sdc_fd,read_sdc;
	UINT8 file_path[128];
	int ret;
	char spi_addr[4];
	int freq = 12000000;	
	UINT32 addr,block,file_size;
	char* pInput=NULL;
	int i,j;
	char led_flash = 1;
	char buffer[4];
	char buffer1[256];
	UINT32 loop_num;
	float  value,old_value;
	DIR *fd;
	struct dirent *dir;	
	int file_num = 0;
	int name_len=0;
	printf("SPI_Flash_update Enter\n");
	printf("============STEP 1==============\n");
	if(dv_set.sd_check == 0)
	{
		printf("NO SDC\n");
		return 0;
	}
	printf("============STEP 2==============\n");
#if 0
	strcpy(file_path, Memory_path);
	fd = opendir(file_path);
	if(fd == NULL){
		return 0;
	}
	while((dir = readdir(fd)) != NULL){
		if((strcmp(dir->d_name,".") == 0)||(strcmp(dir->d_name,"..") == 0)){
			continue;
		}
		printf("find name = %s\n",dir->d_name);
		if(dir->d_type != DT_DIR){
			if(strncmp(dir->d_name,"CVR_firmware_mass",17)==0)
			{
				file_num = 1;
				/*name_len = strlen(dir->d_name);
				printf("name_len = %d\n",name_len);
				if(name_len>17+5&&name_len<=33+5)
				{
					for(i=0;i<32;i++)
						Global_User_Optins.item.version[i]=0;
					strncpy(Global_User_Optins.item.version,(dir->d_name)+13+5,name_len-13-5-4);
					printf("version = %s\n",Global_User_Optins.item.version);
				}
				else
				{
					printf("version is too long\n");
				}*/
				break;
			}			
			else if(strncmp(dir->d_name,"CVR_firmware",12)==0)
			{
				file_num = 2;
				/*name_len = strlen(dir->d_name);
				printf("name_len = %d\n",name_len);
				if(name_len>17&&name_len<=33)
				{
					for(i=0;i<32;i++)
						Global_User_Optins.item.version[i]=0;
					strncpy(Global_User_Optins.item.version,(dir->d_name)+13,name_len-13-4);
					printf("version = %s\n",Global_User_Optins.item.version);
				}
				else
				{
					printf("version is too long\n");
				}*/
				break;
			}
				
		}
	}
	Get_cmputer_build_time();
	printf("version = %s\n",Global_User_Optins.item.version);
	if(file_num == 0)
	{
		closedir(fd);
		return 0;
	}
	strcat(file_path, "/");
	strcat(file_path, dir->d_name);
	closedir(fd);	
#else
#if (AUTO_UPDATE == 0)
	if(Power_key_Check() == 1) {
		file_num = checkUpdateFile(file_path, 20);
	}
	else {
		return 0;
	}
#else
	file_num = checkUpdateFile(file_path, 6);
#endif
	Get_cmputer_build_time();
	printf("version = %s\n",Global_User_Optins.item.version);
	if(file_num == 0)
	{
		return 0;
	}
#endif
	
	printf("Open file[%s] ",file_path);
	sdc_fd = open(file_path,O_RDWR);
	if(sdc_fd<0){	
		printf("Fail\n");
		return 0;
	}
	else
	{
		printf("OK\n");
	}
	file_size = lseek(sdc_fd,0,SEEK_END);
	lseek(sdc_fd,0,SEEK_SET);
	printf("file size = [%d]\n",file_size);
	if(file_size <=0)
	{
		close(sdc_fd);
		return 0;
	}
	pInput = gpChunkMemAlloc(file_size);
	if(pInput < 0)
	{
		close(sdc_fd);
		printf("ChunkMem Fail\n");
		return 0;
	}
	else
	{
		printf("pInput = [0x%x]\n",pInput);
	}
	ret = read(sdc_fd,pInput,file_size);
	if(ret != file_size)
	{
		close(sdc_fd);
		gpChunkMemFree(pInput);
		printf("read file Fail\n");		
	}
	close(sdc_fd);	
	printf("============STEP 3==============\n");
	spi_fd = open("/dev/spi",O_RDWR);
	if(spi_fd<0){		
		printf("Can't open [/dev/spi0]\n");		
		return 0;	
	}
	ioctl(spi_fd, SPI_IOCTL_SET_CHANNEL, 0);	//Set SPI CS0		
	ioctl(spi_fd, SPI_IOCTL_SET_FREQ, freq);		
	ioctl(spi_fd, SPI_IOCTL_SET_DMA, 0);	
	printf("============STEP 4==============\n");
	printf("Chip Erase\n");
	if(dv_set.display_mode == SP_DISP_OUTPUT_LCD) {
		LCD_Backlight_Set(1);
	}
	UPgrade_UI(0,0);
	UPgrade_UI(0,0);
	UPgrade_UI(1,0);
	UPgrade_UI(1,0);	
	spi_addr[0] = 0x06;		//erase commmand
	ioctl(spi_fd, SPI_IOCTL_CS_ENABLE, 0);		
	write(spi_fd, spi_addr, 1);		
	ioctl(spi_fd, SPI_IOCTL_CS_DISABLE, 0);			
					
	spi_addr[0] = 0x60;		//erase commmand
	ioctl(spi_fd, SPI_IOCTL_CS_ENABLE, 0);		
	write(spi_fd, spi_addr, 1);		
	ioctl(spi_fd, SPI_IOCTL_CS_DISABLE, 0);
	j=0;
	name_len = 0;
	do{
		LED_Set(NORMAL_LED,led_flash);
		led_flash = !led_flash;

		spi_addr[0] = 0x05;		
		ioctl(spi_fd, SPI_IOCTL_CS_ENABLE, 0);		
		write(spi_fd, spi_addr, 1);
		read(spi_fd, buffer, 1);	
		ioctl(spi_fd, SPI_IOCTL_CS_DISABLE, 0);	
		usleep(100000); //100ms
		j++;
		if(j%5 == 0)
		{
			printf("j=%4d\b\b",j);
			name_len ++;
			if(name_len > 4)
				name_len = 0;
			UPgrade_UI(5,name_len);	
		}
		if(j >= 500)
		{
			printf("SPI ERASE TIMEOUT!\n");
			close(spi_fd);
			gpChunkMemFree(pInput);					
			return 0;
		}
	}while(buffer[0]& SPI_FLASH_SR_WIP != 0);				

	printf("============STEP 5==============\n");
	UPgrade_UI(2,0);
	UPgrade_UI(2,0);
	old_value = value = 0; 
	addr = 0;
	loop_num = file_size/256;
	if(file_size%256 != 0)
	{
		loop_num += 1;
	}
	printf("loop_num = [%d]\n",loop_num);


	for(i=0;i<loop_num;i++)
	{
		value = (i/(float)loop_num)*100;
		//printf("write addr = [0x%x]value =%x\n",addr,(int)value);
		if((int)old_value != (int)value)
		{
			LED_Set(NORMAL_LED,led_flash);
			led_flash = !led_flash;

			UPgrade_UI(3,(int)value);
			old_value = value;
			printf("%3d\b\b",(int)value);
		}
		spi_addr[0] = 0x06;		
		ioctl(spi_fd, SPI_IOCTL_CS_ENABLE, 0);		
		write(spi_fd, spi_addr, 1);		
		ioctl(spi_fd, SPI_IOCTL_CS_DISABLE, 0);			
		spi_addr[0] = 0x02;		
		spi_addr[1] = ((addr>>16) & 0xFF);		
		spi_addr[2] = ((addr>>8) & 0xFF);		
		spi_addr[3] = (addr & 0xFF);		  			
		ioctl(spi_fd, SPI_IOCTL_CS_ENABLE, 0);		
		write(spi_fd, spi_addr, 4);
		write(spi_fd, pInput +i*256, 256);		
		ioctl(spi_fd, SPI_IOCTL_CS_DISABLE, 0);	
		j=0;
		do{
			spi_addr[0] = 0x05;		
			ioctl(spi_fd, SPI_IOCTL_CS_ENABLE, 0);		
			write(spi_fd, spi_addr, 1);
			read(spi_fd, buffer, 1);	
			ioctl(spi_fd, SPI_IOCTL_CS_DISABLE, 0);	
			/*j++;
			if(j >= 10000)
			{
				printf("SPI write TIMEOUT!\n");
				close(spi_fd);
				gpChunkMemFree(pInput);					
				return -1;
			}*/
		}while(buffer[0]& SPI_FLASH_SR_WIP != 0);			
		addr += 256;
		
	}	
	/*addr = 0;
	strcpy(file_path, Memory_path);
	strcat(file_path, "/GPL329XXB_SPI_ROM_read.bin");	
	read_sdc = open(file_path,O_CREAT | O_RDWR);
	for(i=0;i<loop_num;i++)
	{
		printf("read addr = [0x%x]\n",addr);
		spi_addr[0] = 0x03;		
		spi_addr[1] = ((addr>>16) & 0xFF);		
		spi_addr[2] = ((addr>>8) & 0xFF);		
		spi_addr[3] = (addr & 0xFF);				
		ioctl(spi_fd, SPI_IOCTL_CS_ENABLE, 0);		
		write(spi_fd, spi_addr, 4);
		read(spi_fd, buffer1, 256);	
		ioctl(spi_fd, SPI_IOCTL_CS_DISABLE, 0);	
		write(read_sdc,buffer1,256);
		addr += 256;
	}
	close(read_sdc);
	*/
	
	close(spi_fd);
	gpChunkMemFree(pInput);
	if(file_num == 2)
	{
		remove(file_path);
		system("sync");
	}
	UPgrade_UI(4,0);
	UPgrade_UI(4,0);
	LED_Set(NORMAL_LED,1);
	//Global_User_Optins.item.ifdirty = 1;
	//ap_state_config_store();
	printf("update finish,plut out sdc!\n");
	while(dv_set.sd_check == 1);
	return 1;
}

