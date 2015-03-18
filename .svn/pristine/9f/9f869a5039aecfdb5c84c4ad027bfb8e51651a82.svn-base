/**************************************************************************
 *                                                                        *
 *         Copyright (c) 2010 by Generalplus Inc.                         *
 *                                                                        *
 *  This software is copyrighted by and is the property of Generalplus    *
 *  Inc. All rights are reserved by Generalplus Inc.                      *
 *  This software may only be used in accordance with the                 *
 *  corresponding license agreement. Any unauthorized use, duplication,   *
 *  distribution, or disclosure of this software is expressly forbidden.  *
 *                                                                        *
 *  This Copyright notice MUST not be removed or modified without prior   *
 *  written consent of Generalplus Technology Co., Ltd.                   *
 *                                                                        *
 *  Generalplus Inc. reserves the right to modify this software           *
 *  without notice.                                                       *
 *                                                                        *
 *  Generalplus Inc.                                                      *
 *  3F, No.8, Dusing Rd., Hsinchu Science Park,                           *
 *  Hsinchu City 30078, Taiwan, R.O.C.                                    *
 *                                                                        *
 **************************************************************************/
/**
 * @file pmdump.c
 * @brief Dump Register for PM
 * @author Chris Wang
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <mach/typedef.h>
#include <mach/hal/sysregs.h>
#include <mach/hardware.h>

#if 1
	#define DEBUG	printf
	#define INFO	printf
#else
	#define DEBUG(...)
	#define INFO	printf
#endif

#ifndef READ32
#define READ32(_reg_)           (*((volatile UINT32 *)(_reg_)))
#endif

#ifndef WRITE32
#define WRITE32(_reg_, _value_) (*((volatile UINT32 *)(_reg_)) = (_value_))
#endif


#define XTAL_RATE       27000000   //27 MHz
#define RTC_RATE        32768      //32.768KHz
#define USBPHY_RATE     96000000   //96 MHz
#define FCK_48KRATE     96000000   //96 MHz
#define FCK_44K_RATE    96000000   //96 MHz

const static char *usage_str = "\
\nUsage: pmdump op\
\n       [op]\
\n          all  : all information\
\n          pm   : all information for pm\
\n";


char *scuAname[] = { 
                     "A00-CHECK RD" , "LCD CTRL"    , "A02-CHECK RD", "USB0"         , /* 00-03 */
                     "USB1"         , "A05-CHECK RD", "A06-CHECK RD", "A07-CHECK RD" , /* 04-07 */

                     "A08-CHECK RD" , "APBDMA_A"    , "CMOS CTRL"   , "NAND0"        , /* 08-11 */
                     "A12-CHECK RD" , "BCH"         , "A13-CHECK RD", "A14-CHECK RD" , /* 12-15 */

                     "AAHBM212"     , "I2S"         , "I2SRX"       , "SAADC"        , /* 16-19 */
                     "NAND_ABT"     , "REALTIME_ABT", "RTABT212"    , "A23-CHECK RD" , /* 20-23 */

                     "A24-CHECK RD" , "SPU"         , "SCA"         , "OVG"          , /* 24-27 */
                     "MIPI"         , "CDSP"        , "AES"        , "A31-CHECK RD"  , /* 28-31 */
                   };
char *scuBname[] = { 
                     "B00-CHECK RD" , "B01-CHECK RD", "AHB2AHB"     , "B03-CHECK RD" , /* 00-03 */
                     "VIC0"         , "VIC1"        , "B06-CHECK RD", "B07-CHECK RD" , /* 04-07 */

                     "B08-CHECK RD" , "TIMER0"      , "TIMER1"      , "B11-CHECK RD" , /* 08-11 */
                     "ARM_I2C"      , "RAND"        , "GPIO"        , "RTC"          , /* 12-15 */

                     "B16-CHECK RD" , "B17-CHECK RD", "B18-CHECK RD", "B19-CHECK RD" , /* 16-19 */
                     "B20-CHECK RD" , "B21-CHECK RD", "B22-CHECK RD", "B23-CHECK RD" , /* 20-23 */

                     "B24-CHECK RD" , "B25-CHECK RD", "B26-CHECK RD", "B27-CHECK RD" , /* 24-27 */
                     "B28-CHECK RD" , "B29-CHECK RD", "B30-CHECK RD", "B31-CHECK RD" , /* 28-31 */
                   };

char *scuCname[] = { 
                     "C00-CHECK RD" , "DMAC0-SDMA"  , "DMAC1-XDMA"  , "C01-CHECK RD" , /* 00-03 */
                     "DRAM CTRL"    , "C00-CHECK RD", "C00-CHECK RD", "APBDMA_C"     , /* 04-07 */

                     "C08-CHECK RD" , "C09-CHECK RD", "C10-CHECK RD", "MS"           , /* 08-11 */
                     "INT_MEM"      , "UART_C0"     , "C14-CHECK RD", "UART_C2"      , /* 12-15 */

                     "SSP0"         , "SSP1"        , "SD0"         , "SD1"          , /* 16-19 */
                     "SYS_I2C"      , "Scaling"     , "2dscaleabt"  , "TI2C"         , /* 20-23 */

                     "SYS_A"        , "CXMP212"     , "CXMD212"     , "CIR"          , /* 24-27 */
                     "C28-CHECK RD" , "EFUSE"       , "C30-CHECK RD", "C31-CHECK RD" , /* 28-31 */
                   };

char *scuDname[] = { 
                     "PPU_SPR"      , "Ceva L2 ram" , "D02-CHECK RD", "D03-CHECK RD" , /* 00-03 */
                     "D04-CHECK RD" , "D05-CHECK RD", "D06-CHECK RD", "D07-CHECK RD" , /* 04-07 */

                     "D08-CHECK RD" , "D09-CHECK RD", "D10-CHECK RD", "D11-CHECK RD" , /* 08-11 */
                     "D12-CHECK RD" , "D13-CHECK RD", "D14-CHECK RD", "D15-CHECK RD" , /* 12-15 */

                     "D16-CHECK RD" , "D17-CHECK RD", "D18-CHECK RD", "D19-CHECK RD" , /* 16-19 */
                     "D20-CHECK RD" , "D21-CHECK RD", "D22-CHECK RD", "D23-CHECK RD" , /* 20-23 */

                     "PPU"          , "PPU_REG"     , "PPU_TFT"     , "PPU_STN" , /* 24-27 */
                     "PPU_TV"       , "PPU_FB"      , "D30-CHECK RD", "D31-CHECK RD" , /* 28-31 */
                   };


unsigned long spll = 0 ;
unsigned long spll2 = 0 ;
unsigned long apll = 0 ;
unsigned long arm = 0 ;
unsigned long arm_apb = 0 ;
unsigned long arm_ahb = 0 ;
unsigned long ceva = 0 ;
unsigned long ceva_ahb = 0 ;
unsigned long ceva_apb = 0 ;
unsigned long fclk_ref_ceva = 0 ;

unsigned long sys = 0 ;
unsigned long sys_rt = 0 ;
unsigned long sys_ahb = 0 ;
unsigned long sys_apb = 0 ;

int dumpPLL(unsigned int addr , int arg) 
{

/* 
P_SCUB_SPLL_CFG0
    0x90005004[15]
    SPLL_PD
    SPLL power down
    1: power down
    0: power up     
     
     
     
P_SCUB_SPLL_CFG0
0x90005004[30:26]
    SPLL_M
    SPLL clock input divider

P_SCUB_SPLL_CFG0
0x90005004[9:2]
    SPLL_N
    SPLL feedback divider

P_SCUB_SPLL_CFG0
0x90005004[1:0]
    SPLL_R
    SPLL post divider

fspll = (CLKIN/M)*N*R*[(1-MR%)~(1+MR%)]
CLKIN = 27MHz
M[4:0] = 1~31
N[7:0] = 32~212
R[1:0] = (0, x) --> R = 8
R[1:0] = (1, x) --> R = 4      
    */

    unsigned int regvalue = 0 ;
    unsigned int M,N,R;

    INFO("[--------------------------------------------------------------------------]\n");

    regvalue = READ32(addr+0x5004) ;
    
	M = (regvalue & 0x7C000000) >> 26;
	N = (regvalue & 0x3FC) >> 2;
	
	if ((regvalue & 0x2) == 0) {
		R = 8;
	}
	else {
		R = 4;
	}
    spll = ((XTAL_RATE/M) * N * R) ;
    if ( regvalue & ( 1<<15) ) {
        spll = 0 ;
        if ( arg != 1 ) {
            INFO("[SPLL ] OFF\n");
        }
    } else {
        INFO("[SPLL ] ON %4d Mhz\n", spll / 1000000);
    }

/*
P_SCUB_SPLL_CFG1
    0x9000505C[27]
    SPLL2_PD
    SPLL2 power down
    1: power down
    0: power up
 
P_SCUB_SPLL_CFG2
0x90005060[4:0]
    SPLL2_M
    SPLL2 clock input divider

P_SCUB_SPLL_CFG1
0x9000505C[7:0]
    SPLL2_N
    SPLL2 feedback divider

P_SCUB_SPLL_CFG0
0x90005004[11:10]
    SPLL2_R
    SPLL2 post divider

fspll2 = (CLKIN/M)*N*R*[(1-MR%)~(1+MR%)]
CLKIN = 27MHz
M[4:0] = 1~31
N[7:0] = 32~212
R[1:0] = (0, x) --> R = 8
R[1:0] = (1, x) --> R = 4
*/

    regvalue = READ32(addr+0x505C) ;

	M = ( READ32(addr+0x5060) & 0x1F);
	N = (regvalue & 0xFF);
	
	if ((READ32(addr+0x5004) & 0x800) == 0) {
		R = 8;
	}
	else {
		R = 4;
	}
	spll2 = ((XTAL_RATE/M) * N * R) ;
    if ( regvalue & ( 1<<27) ) {
        spll2 = 0 ;
        if ( arg != 1 ) {
            INFO("[SPLL2] OFF\n");
        }
    } else {
        INFO("[SPLL2] ON %4d Mhz\n", spll2 / 1000000);
    }



/*
P_SCUA_APLL_CFG
    0x93007044[0]
    When P=0, APLL power down. When P=1, APLL power up
 
P_SCUA_APLL_CFG
0x93007044[1]
    S=0 for 48KHz Audio, and S=1 for 44.1KHz Audio
    If S=0, the fAPLL is 73.728MHz.
    If S=1, the fAPLL is 67.7376MHz.
*/

    regvalue = READ32(addr+0x3007044) ;

    if ( regvalue & (1<<0) ) {
        if ( regvalue & ( 1<<1) ) {
            apll = 67737600 ;
        } else {
            apll = 73728000 ;
        }
        INFO("[APLL ] ON %4d Mhz\n", apll / 1000000);
    } else {
        apll = 0 ;
        if ( arg != 1 ) {
            INFO("[APLL ] OFF\n");
        }
    }

    return 0;
}

int dumpPeripheralClock(unsigned int addr , int arg) 
{
    
    unsigned int regvalue0 = 0 ;
    int i = 0 ;

    INFO("[--------------------------------------------------------------------------]\n");

    /*
    System A Clock Enable
    */
    regvalue0 = READ32(addr+0x3007004) ;
    for ( i = 0 ; i < 32 ; i++ ) {
        if ( scuAname[i] != "" ) {
            if ( regvalue0 & (1<<i) ) {
                INFO("[SYSTEM A] Clock %s ON\n", scuAname[i]);
            }
        }
    }


    /*
    System B Clock Enable
    */
    regvalue0 = READ32(addr+0x0005020) ;


    for ( i = 0 ; i < 32 ; i++ ) {
        if ( scuBname[i] != "" ) {
            if ( regvalue0 & (1<<i) ) {
                INFO("[SYSTEM B] Clock %s ON\n", scuBname[i]);
            }
        }
    }

    /*
    System C Clock Enable
    */
    regvalue0 = READ32(addr+0x2005004) ;

    for ( i = 0 ; i < 32 ; i++ ) {
        if ( scuCname[i] != "" ) {
            if ( regvalue0 & (1<<i) ) {
                INFO("[SYSTEM C] Clock %s ON\n", scuCname[i]);
            }

        }
    }


    /*
    System D Clock Enable
    */
    regvalue0 = READ32(addr+0x3007018) ;

    for ( i = 0 ; i < 32 ; i++ ) {
        if ( scuDname[i] != "" ) {
            if ( regvalue0 & (1<<i) ) {
                INFO("[SYSTEM D] Clock %s ON\n", scuDname[i]);
            }
        }
    }

    return 0;
}


int dumpMainClock(unsigned int addr, int arg) 
{

/*
P_SCUB_ARM_RATIO
    0x900050D0[5:0]
    ARM_RATIO

P_SCUB_ARM_AHB_RATIO
    0x900050D4[5:0]
    ARM_AHB_RATIO

P_SCUB_ARM_APB_RATIO
    0x900050D8[5:0]
    ARM_APB_RATIO

P_SCUB_SPLL_CFG1
    0x9000505C[24]
    CLKSEL
    0: ARM clock use SPLL
    1: ARM clock use SPLL2

P_SCUB_SPLL_CFG0
    0x90005004[18:16]
    ARM_CLKSEL
    Clock selection to the ARM clock
    ARM_CLKSEL : fclk_ref_arm
    0: 27MHz
    1: 32768Hz
    2: fspll/2
    3: fspll/3
    4~7: fspll

fclk_arm = fclk_ref_arm/(ARM_RATIO+1)
fclk_arm_ahb = fclk_arm/(ARM_AHB_RATIO+1)
fclk_arm_apb = fclk_arm_ahb/(ARM_APB_RATIO+1)
*/
    INFO("[--------------------------------------------------------------------------]\n");

    unsigned int regvalue0 = 0 ;
    unsigned int M,N,R;
    unsigned int enable = 1 ;
    regvalue0 = READ32(addr+0x0005058) ;

    if ( regvalue0 & ( 1<<0) ) {
        INFO("[ARM] WFI Keep ARM Clock Enable\n");
    } else {
        if ( arg != 1 ) {
            INFO("[ARM] WFI Keep ARM Clock Disable\n");
        }
    }


    regvalue0 = READ32(addr+0x000505C) ;
    if ( regvalue0 & ( 1<<24) ) {
        INFO("[ARM] USE SPLL2\n");
        arm = spll2 ;
    } else {
        INFO("[ARM] USE SPLL\n");
        arm = spll ;
    }

    regvalue0 = (READ32(addr+0x0005004) & 0x70000 ) >> 16;

    switch (regvalue0)
    {
      case 1:	  	
          arm =  RTC_RATE ;
          break;
      case 2:
          arm = arm / 2;
          break;			
      case 3:
          arm = arm / 3;
          break;
      case 4:
      case 5:
      case 6:			
      case 7:
          arm = arm ;
          break;
      default:
        arm = XTAL_RATE;
        break;
    }

    N = READ32(addr+0x00050D0) & 0x3F;
    arm = arm / ( N + 1) ;
    INFO("[ARM] ARM     %4d Mhz\n", arm / 1000000);

    R = READ32(addr+0x00050D4) & 0x3F;
    arm_ahb = arm / ( R + 1) ;
    INFO("[ARM] ARM_AHB %4d Mhz\n", arm_ahb / 1000000);

    M = READ32(addr+0x00050D8) & 0x3F;
    arm_apb = arm_ahb / ( M + 1) ;
    INFO("[ARM] ARM_APB %4d Mhz\n", arm_apb / 1000000);

/*
 
P_SCUC_CEVA_CNT_EN
    0x9200511C[0]
    CEVA_CNT_EN
    CEVA Counter Enable, enable ceva clock 
    1: Enable
    0: Disable
0x9200511C[1]
    CEVA_AHB
    _CNT_EN
    CEVA AHB Counter Enable, enable ceva ahb clock
    1: Enable
    0: Disable
0x9200511C[2]
    CEVA_APB
    _CNT_EN
    CEVA APB Counter Enable, ebale ceva apb clock
    1: Enable
    0: Disable 
 
 
P_SCUC_CEVA_RATIO
0x92005110[5:0]
CEVA_RATIO

P_SCUC_CEVA_AHB_RATIO
0x92005114[5:0]
CEVA_AHB_RATIO

P_SCUC_CEVA_APB_RATIO
0x92005118[5:0]
CEVA_APB_RATIO

P_SCUB_SPLL_CFG0
0x90005004[14:12]
CEVA_CLKSEL
0: 27, 1: RTC, 2: DIV2, 3: DIV3, 4: DIV1

CEVA_CLKSEL : fclk_ref_ceva
0: 27MHz
1: 32768Hz
2: fspll/2
3: fspll/3
4~7: fspll

fclk_ceva= fclk_ref_ceva/(CEVA_RATIO+1)
fclk_ceva_ahb = fclk_ceva/(CEVA_AHB_RATIO+1)
fclk_ceva_apb = fclk_ceva_ahb/(CEVA_APB_RATIO+1) 
*/ 




    regvalue0 = READ32(addr+0x200511C) ;
    enable = 1 ;
    if ( regvalue0 & 0x80000007) {
        if ( regvalue0 & ( 1<<0) ) {
            INFO("[CEVA] CEVA Counter Enable, enable ceva clock\n");
        }

        if ( regvalue0 & ( 1<<1) ) {
            INFO("[CEVA] CEVA_AHB Counter Enable, enable ceva ahb clock\n");
        }

        if ( regvalue0 & ( 1<<2) ) {
            INFO("[CEVA] CEVA APB Counter Enable, enable ceva apb clock\n");
        }

        if ( regvalue0 & ( 1<<31) ) {
            INFO("[CEVA] CEVA Check Counter enable\n");
        }
    } else {
        enable = 0 ;
        if ( arg != 1 ) {
            INFO("[CEVA] CEVA Check Counter disable , ceva/ahb/apb clock diable\n");
        }
    }


    if ( enable) {
        INFO("[CEVA] USE SPLL\n");
    }
    ceva = spll ;

    regvalue0 = (READ32(addr+0x0005004) & 0x7000 ) >> 12;

    switch (regvalue0)
    {
      case 1:	  	
          ceva =  RTC_RATE ;
          break;
      case 2:
          ceva = ceva / 2;
          break;			
      case 3:
          ceva = ceva / 3;
          break;
      case 4:
      case 5:
      case 6:			
      case 7:
          ceva = ceva ;
          break;
      default:
        ceva = XTAL_RATE;
        break;
    }

    fclk_ref_ceva = ceva ;

    N = READ32(addr+0x2005110) & 0x3F;
    ceva = ceva / ( N + 1) ;
    if ( (enable) && (arg != 1) ) {
        INFO("[CEVA] CEVA     %4d Mhz\n", ceva / 1000000);
    }

    R = READ32(addr+0x2005114) & 0x3F;
    ceva_ahb = ceva / ( R + 1) ;
    if ( (enable) && (arg != 1) ) {
        INFO("[CEVA] CEVA_AHB %4d Mhz\n", ceva_ahb / 1000000);
    }

    M = READ32(addr+0x2005118) & 0x3F;
    ceva_apb = ceva_ahb / ( M + 1) ;
    if ( (enable) && (arg != 1) ) {
        INFO("[CEVA] CEVA_APB %4d Mhz\n", ceva_apb / 1000000);
    }


/*
P_SCUB_SYS_CNT_EN
    0x900050DC[0]
    SYS Counter Enable

P_SCUB_SYS_CNT_EN
    0x900050DC[2]
    SYS AHB Counter Enable

P_SCUB_SYS_CNT_EN
    0x900050DC[3]
    SYS APB Counter Enable 
 
 
P_SCUC_SYS_RATIO
    0x92005100[5:0]
    SYS_RATIO

P_SCUC_SYS_RT_RATIO
    0x92005104[5:0]
    SYS_RT_RATIO
    The rt clock ratio for fabric register

P_SCUC_SYS_AHB_RATIO
    0x92005108[5:0]
    SYS_AHB_RATIO

P_SCUC_SYS_APB_RATIO
    0x9200510C[5:0]
    SYS_APB_RATIO

fclk_sys = fclk_ref_ceva/(SYS_RATIO+1)
fclk_sys_ahb = fclk_sys/(SYS_AHB_RATIO+1)
fclk_sys_apb = fclk_sys_ahb/(SYS_APB_RATIO+1) 
 
*/


    regvalue0 = READ32(addr+0x00050DC) ;
    enable = 1 ;
    if ( regvalue0 & 0x0000000F) {
        if ( regvalue0 & ( 1<<0) ) {
            INFO("[SYS] SYS Counter Enable\n");
        }

        if ( regvalue0 & ( 1<<1) ) {
            INFO("[SYS] SYS RT Counter Enable\n");
        }

        if ( regvalue0 & ( 1<<2) ) {
            INFO("[SYS] SYS AHB Counter Enable\n");
        }

        if ( regvalue0 & ( 1<<3) ) {
            INFO("[SYS] SYS APB Counter Enable\n");
        }

    } else {
        enable = 0 ;
        if ( arg != 1 ) {
            INFO("[SYS] SYS Counter disable\n");
        }
    }



    if ( (enable) && (arg != 1) ) {
        INFO("[SYS] USE CEVA %4d Mhz\n",ceva / 1000000);
    }
    sys = fclk_ref_ceva ;

    N = READ32(addr+0x2005100) & 0x3F;
    sys = sys / ( N + 1) ;
    if ( (enable) && (arg != 1) ) {
        INFO("[SYS] SYS     %4d Mhz\n", sys / 1000000);
    }

    R = READ32(addr+0x2005104) & 0x3F;
    sys_rt = sys / ( R + 1) ;
    if ( (enable) && (arg != 1) ) {
        INFO("[SYS] SYS_RT %4d Mhz\n", sys_rt / 1000000);
    }

    R = READ32(addr+0x2005108) & 0x3F;
    sys_ahb = sys / ( R + 1) ;
    if ( (enable) && (arg != 1) ) {
        INFO("[SYS] SYS_AHB %4d Mhz\n", sys_ahb / 1000000);
    }

    M = READ32(addr+0x200510C) & 0x3F;
    sys_apb = sys_ahb / ( M + 1) ;
    if ( (enable) && (arg != 1) ) {
        INFO("[SYS] SYS_APB %4d Mhz\n", sys_apb / 1000000);
    }


    return 0;
}


int dumpUSB(unsigned int addr, int arg) 
{

    INFO("[--------------------------------------------------------------------------]\n");

/*
P_SCUA_USBPHY_CFG
0x9300703C[2] 
 
P_SCUA_PERI_CLKEN
0x93007004[3] 
 
USB phy  = 120MHz 
fclk_sys_ahb, must >=48MHz 
 
 
P_SCUA_USBPHY_CFG
0x9300703C[2]
 
P_SCUA_PERI_CLKEN
0x93007004[4]
 
USB phy  = 120MHz 
fclk_sys_ahb, must >=48MHz 
 
*/

    unsigned int regvalue0 = 0 ;

    regvalue0 = READ32(addr+0x3007004) ;

    if ( regvalue0 & ( 1<<3) ) {
        INFO("[USB0] Clock Enable %4d Mhz\n" , sys_ahb / 1000000);
    } else {
        if ( arg != 1 ) {
            INFO("[USB0] Clock Disable\n");
        }
    }

    if ( regvalue0 & ( 1<<4) ) {
        INFO("[USB1] Clock Enable %4d Mhz\n", sys_ahb / 1000000);
    } else {
        if ( arg != 1 ) {
            INFO("[USB1] Clock Disable\n");
        }
    }

    regvalue0 = READ32(addr+0x300703C) ;

    if ( ( ( regvalue0 & (0x3<<8) ) >> 8 ) == 0x01 ){
        if ( arg != 1 ) {
            INFO("[USB0] PHY Force Suspend\n");
        }
    } else {
        INFO("[USB0] PHY Working %4d Mhz\n",USBPHY_RATE/ 1000000);
    }

    if ( (( regvalue0 & (0x3<<12) ) >> 12 ) == 0x01 ){
        if ( arg != 1 ) {
            INFO("[USB1] PHY Force Suspend\n");
        }
    } else {
        INFO("[USB1] PHY Working %4d Mhz\n",USBPHY_RATE/ 1000000);
    }
    
    return 0;
}


int dumpTVDAC(unsigned int addr, int arg) 
{
/*
P_SCUA_VDAC_CFG
0x93007040[0]
When P=1, VDAC power down. When P=0, VDAC power up
0x93007040[4]
0: Enable YPbPr
1: Disable YPbPr 
 
 
 
P_SCUA_SYS_SEL
0x930070E0[5]
PPU_CLK27
PPU 27MHz clock enable,for TV1 or YPBPR 480i
1: Enable
0: Disable

P_SCUA_SYS_SEL
0x930070E0[4]
PPU_CLK74
PPU 74.25MHz clock enable,for YPBPR 720P
1: Enable
0: Disable 
 
 
 
P_SCUA_VDAC_CFG
0x93007040[5]
0: Disable 74.25MHz clock
1: Enable 74.25MHz clock  
= 0時, 看下面這個
P_SCUA_SYS_SEL
0x930070E0[5][4]
*/
    INFO("[--------------------------------------------------------------------------]\n");

    unsigned int regvalue0 = 0 ;
    unsigned int vdac_digital_clk = 0 ;

    regvalue0 = READ32(addr+0x3007040) ;

    if ( regvalue0 & ( 1<<0) ) {

        if ( regvalue0 & ( 1<<4) ) {
            INFO("[VDAC] 1 VDAC Enable\n");
        } else {
            INFO("[VDAC] 3 VDAC Enable\n");
        }

    } else {
        if ( arg != 1 ) {
            INFO("[VDAC] Disable\n");
        }
    }
    

    regvalue0 = READ32(addr+0x30070E0) ;

    if ( regvalue0 & ( 1<<4) ) {
        INFO("[VDAC] Digital Clock 74.25MHz Enable\n");
    } 

    if ( regvalue0 & ( 1<<5) ) {
        INFO("[VDAC] Digital Clock 27MHz Enable\n");
    } 

    if ( READ32(addr+0x3007040) & ( 1<<5) ) {
        if ( regvalue0 & ( 1<<4) ) {
            INFO("[VDAC] Clock 74.25MHz Enable\n");
        } 
    } else {
        if ( regvalue0 & ( 1<<5) ) {
            INFO("[VDAC] Clock 27MHz Enable\n");
        } 
    }

    regvalue0 = READ32(addr+0x3007018) ;

    if ( regvalue0 & ( 1<<28) ) {
        INFO("[TV1] Enable\n");
    } else {
        if ( arg != 1 ) {
            INFO("[TV1] Disable\n");
        }
    }
    
    INFO("[TV0] ?????????????????????????????\n");    
    return 0;
}

int dumpTFTDisplay(unsigned int addr, int arg) 
{

    unsigned int regvalue0 = 0 ;
    unsigned int lcdclk = 0 ;

    INFO("[--------------------------------------------------------------------------]\n");

/*
P_SCUA_PERI_CLKEN
0x93007004[1]

P_SCUA_LCD_CLK_CFG
0x93007080[8]
LCD_CLK_EN
0: clk_lcd is off. 
1: clk_lcd is on 
 
 
P_SCUA_LCD_CLK_CFG
0x93007080[18:16]
LCD_CLK_SEL
Select LCD clock source , 
xx1:LCD clock source is from XTAL 27MHz
000:LCD clock source is from SPLL
110: LCD clock source is from USBPHY 96MHz(USB must enable)

P_SCUA_LCD_CLK_CFG
0x93007080[7:0]
LCD_CLK_RATIO

fclk_lcd = fclk_ref_ceva/(LCD_CLK_RATIO+1) 
 
*/

    regvalue0 = READ32(addr+0x3007004) ;

    if ( regvalue0 & ( 1<<1) ) {
        INFO("[TFT0] Enable\n");
    } else {
        if ( arg != 1 ) {
            INFO("[TFT0] Disable\n");
        }
    }


    regvalue0 = READ32(addr+0x3007080) ;

    if ( regvalue0 & ( 1<<8) ) {
        regvalue0 = READ32(addr+0x3007080) ;
        if ( ( ( regvalue0 & (0x1<<16) ) >> 16 ) == 0x1 ) {
            lcdclk = XTAL_RATE ;
        } else if (( ( regvalue0 & (0x7<<16) ) >> 16 ) == 0x000 ) {
            lcdclk = fclk_ref_ceva ;
        } else if ( ( ( regvalue0 & (0x7<<16) ) >> 16 ) == 0x006 ) {
            lcdclk = USBPHY_RATE ;
        }
        
        regvalue0 = READ32(addr+0x3007080) & 0xFF;

        lcdclk = lcdclk / (regvalue0+1) ;

        INFO("[TFT0] LCD Clock %4d Mhz Enable\n",lcdclk/1000000);
    } else {
        if ( arg != 1 ) {
            INFO("[TFT0] LCD Clock Disable\n");
        }
    }



    regvalue0 = READ32(addr+0x3007018) ;

    if ( regvalue0 & ( 1<<26) ) {
        INFO("[TFT1] Enable\n");
        INFO("[TFT1] LCD Clock ?????????????????????????????\n");
    } else {
        if ( arg != 1 ) {
            INFO("[TFT1] Disable\n");
            INFO("[TFT1] LCD Clock ?????????????????????????????\n");
        }
    }

    return 0;
}


int dumpI2S(unsigned int addr, int arg) 
{

    INFO("[--------------------------------------------------------------------------]\n");
    unsigned int regvalue0 = 0 ;
    unsigned int clk = 0 ;
/*
P_SCUA_PERI_CLKEN
0x93007004[17]

P_SCUA_APLL_CFG
0x93007044[31:24]
DA_RATIO
If DA_RATIO is set as ‘0’, MCLK for I2STX will be disabled.
 
 
P_SCUA_APLL_CFG
0x93007044[2]
When F=1, SCLK for I2S is sourced from APLL.
When F=0, SCLK for I2S is sourced from I/O PAD(不要使用)

P_SCUA_APLL_CFG
0x93007044[31:24]
DA_RATIO

fi2s_mclk = fAPLL/DA_RATIO
 
 
*/
    regvalue0 = READ32(addr+0x3007004) ;

    if ( regvalue0 & ( 1<<17) ) {
        INFO("[I2STX] Enable\n");
    } else {
        if ( arg != 1 ) {
            INFO("[I2STX] Disable\n");
        }
    }
    
    regvalue0 = ( ( READ32(addr+0x3007044) & ( 0xFF << 24 ) ) >> 24 ) ;
    if ( regvalue0 == 0 ) {
        if ( arg != 1 ) {
            INFO("[I2STX] Clock Disable\n");
        }
    } else {
        clk = apll / regvalue0 ;
        INFO("[I2STX] Clock %4d Mhz Enable\n",clk/1000000);
    }

/*
"P_SCUA_PERI_CLKEN
0x93007004[18]"
 
 
"P_SCUA_PERI_CLKEN
0x93007004[18]

P_SCUA_APLL_CFG
0x93007044[23:16]
AD_RATIO
If AD_RATIO is set as ‘0’, MCLK for I2SRX will be disabled."
 
 
"P_SCUA_APLL_CFG
0x93007044[2]
When F=1, SCLK for I2S is sourced from APLL.
When F=0, SCLK for I2S is sourced from I/O PAD(不要使用)

P_SCUA_APLL_CFG
0x93007044[1]
S=0 for 48KHz Audio, and S=1 for 44.1KHz Audio
If S=0, the fAPLL is 73.728MHz.
If S=1, the fAPLL is 67.7376MHz.

P_SCUA_APLL_CFG
0x93007044[23:16]
AD_RATIO

fi2srx_mclk = fAPLL/AD_RATIO"
 
*/

    regvalue0 = READ32(addr+0x3007004) ;

    if ( regvalue0 & ( 1<<18) ) {
        INFO("[I2SRX] Enable\n");
    } else {
        if ( arg != 1 ) {
            INFO("[I2SRX] Disable\n");
        }
    }

    regvalue0 = ( ( READ32(addr+0x3007044) & ( 0xFF << 16 ) ) >> 16 ) ;
    if ( regvalue0 == 0 ) {
        if ( arg != 1 ) {
            INFO("[I2SRX] Clock Disable\n");
        }
    } else {
        clk = apll / regvalue0 ;
        INFO("[I2SRX] Clock %4d Mhz Enable\n",clk/1000000);
    }

    return 0;
}


int dumpCIS(unsigned int addr, int arg) 
{

/* 
P_SCUA_CSI_CLK_CFG
0x93007084[8]
CSI_CLK_EN
0: clk_csi is off. 
1: clk_csi is on

CSI0/CSI1/CDSP皆可透過此port輸出 
 
P_SCUA_CSI_CLK_CFG
0x93007084[16]
CSI_CLK_SEL
Select CSI clock source , 
0:CSI clock source is from SPLL
1: CSI clock source is from USBPHY 96MHz(USB must enable)


P_SCUA_CSI_CLK_CFG
0x93007084[7:0]
CSI_CLK_RATIO
Set the ratio for clk_csi

fclk_csi = fclk_ref_ceva/(CSI_CLK_RATIO+1)
此clock為sensor MCLK output 
 
*/ 
    unsigned int regvalue0 = 0 ;
    unsigned int clk = 0 ;
    
    INFO("[--------------------------------------------------------------------------]\n");
    regvalue0 = READ32(addr+0x3007084) ;
    if ( regvalue0 & ( 1<<8) ) {
        INFO("[CSI] Enable\n");
    } else {
        if ( arg != 1 ) {
            INFO("[CSI] Disable\n");
        }
    }

    if ( regvalue0 & ( 1<<16) ){
        clk = USBPHY_RATE ;
    } else {
        clk = fclk_ref_ceva ;
    }
        
    regvalue0 = READ32(addr+0x3007084) & 0xFF;
    clk = clk / (regvalue0+1) ;
    
    if ( arg != 1 ) {
        INFO("[CSI] MCLK Clock %4d Mhz Enable\n",clk/1000000);
    }
    
/*
 
"P_SCUA_PERI_CLKEN
0x93007004[10]

P_SCUA_SYS_SEL
0x930070E0[15]
SEN0_CLK
Sensor 0 clock enable
1: Enable
0: Disable"
 
*/
  
    regvalue0 = READ32(addr+0x3007004) ;
    if ( regvalue0 & ( 1<<10) ) {
        INFO("[CSI0] Enable\n");
    } else {
        if ( arg != 1 ) {
            INFO("[CSI0] Disable\n");
        }
    }
      
/*
 
P_SCUC_PERI_CLKEN
0x92005004[27]

P_SCUA_SYS_SEL
0x930070E0[6]
Sensor 1 clock enable
1: Enable
0: Disable 
*/
        
    regvalue0 = READ32(addr+0x2005004) ;
    if ( regvalue0 & ( 1<<27) ) {
        INFO("[CSI1] Enable\n");
    } else {
        if ( arg != 1 ) {
            INFO("[CSI1] Disable\n");
        }
    }

    regvalue0 = READ32(addr+0x30070E0) ;
    if ( regvalue0 & ( 1<<6) ) {
        INFO("[CSI1] Clock Enable ??????MHz\n");
    } else {
        if ( arg != 1 ) {
            INFO("[CSI1] Clock Disable\n");
        }
    }


/*
 
P_SCUA_PERI_CLKEN
0x93007004[29]

P_SCUA_SYS_SEL
0x930070E0[8]
CDSP clock enable 
1: Enable
0: Disable

上面這個若設disable, 下面這個don't care
P_SCUA_CDSP_PCLK
0x930070EC[8]
0: CDSP clock is off. 
1: CDSP clock is on 
 
若CDSP有用到sensor input + 後處理功能
clock是吃sensor PCLK

P_SCUA_CDSP_PCLK
0x930070EC[16]
CDSP_CLK_SEL
Select CDSP clock source , 
0:CDSP clock source is from SPLL
1: CDSP clock source is from USBPHY 96MHz(USB must enable)

P_SCUA_CDSP_PCLK
0x930070EC[7:0]
CDSP_CLK_RATIO
Set the ratio for CDSP clock

fclk_cdsp = fclk_ref_ceva/(CDSP_CLK_RATIO+1)
此clock指的是只用CDSP做後處理時, 提供給CDSP post process unit的clock 
 
*/

    regvalue0 = READ32(addr+0x3007004) ;
    if ( regvalue0 & ( 1<<29) ) {
        INFO("[CDSP] Enable\n");
    } else {
        if ( arg != 1 ) {
            INFO("[CDSP] Disable\n");
        }
    }

    regvalue0 = READ32(addr+0x30070E0) ;
    if ( regvalue0 & ( 1<<8) ) {

        if ( READ32(addr+0x30070EC) & ( 1<<8) ) {
            INFO("[CDSP] Clock Enable\n");
        } else {
            if ( arg != 1 ) {
                INFO("[CDSP] Clock Disable\n");
            }
        }
        

        if ( READ32(addr+0x30070EC) & ( 1<<16) ){
            clk = USBPHY_RATE ;
        } else {
            clk = fclk_ref_ceva ;
        }

        regvalue0 = READ32(addr+0x30070EC) & 0xFF;
        clk = clk / (regvalue0+1) ;
        INFO("[CDSP] Pixel Clock %4d Mhz Enable\n",clk/1000000);


    } else {
        if ( arg != 1 ) {
            INFO("[CDSP] Clock Disable\n");
        }
    }
    
    return 0;
}

int dumpMIPI(unsigned int addr, int arg)
{
	/*
 
"P_MIPI_GLB_CSR
0x9300D000[0]
MIPI_ENABLE
MIPI transfer turn on.
When MIPI_ENABLE=0; The MIPI_RX_TOP will go to initial state; during the initial state, all the MIPI bus line states will be ignored.
0: Disable
1: Enable"
 
*/
	unsigned int regvalue0 = 0 ;
    unsigned int clk = 0 ;
    
    regvalue0 = READ32(addr+0x300D000) ;
    if ( regvalue0 & ( 1<<0) ) {
        INFO("[MIPI] Analog Enable\n");
    } else {
        if ( arg == 1 ) {
            INFO("[MIPI] Analog Disable\n");
        }
    }
    
/*
"P_SCUA_PERI_CLKEN
0x93007004[28]

P_SCUA_MIPI_2CH_PCLK
0x930070F0[8]
0: MIPI clock is off. 
1: MIPI clock is on
此clock只當用到MIPI 2nd ch才有需要"
*/
	
	regvalue0 = READ32(addr+0x3007004) ;
    if ( regvalue0 & ( 1<<28) ) {
        INFO("[MIPI] Enable\n");
    } else {
        if ( arg == 1 ) {
            INFO("[MIPI] Disable\n");
        }
    }
    
    regvalue0 = READ32(addr+0x30070F0) ;
    if ( regvalue0 & ( 1<<8) ) {
        INFO("[MIPI] 2CH Clock Enable\n");
        
        if ( regvalue0 & ( 1<<16) ){
            clk = USBPHY_RATE ;
        } else {
            clk = fclk_ref_ceva ;
        }

        regvalue0 = READ32(addr+0x30070F0) & 0xFF;
        clk = clk / (regvalue0+1) ;
        INFO("[MIPI] 2CH Clock %4d Mhz Enable\n",clk/1000000);
        
    } else {
        if ( arg == 1 ) {
            INFO("[MIPI] 2CH Clock Disable\n");
        }
        
        regvalue0 = READ32(addr+0x3007084) ;
    	if ( regvalue0 & ( 1<<8) ) {
        	INFO("[MIPI] 1CH Enable\n");
        	
        	if ( regvalue0 & ( 1<<16) ){
        		clk = USBPHY_RATE ;
    		} else {
        		clk = fclk_ref_ceva ;
    		}
        
    		regvalue0 = READ32(addr+0x3007084) & 0xFF;
    		clk = clk / (regvalue0+1) ;
    
    		if ( arg != 1 ) {
        		INFO("[MIPI] MCLK %4d Mhz Enable\n",clk/1000000);
  			}
    	} else {
        	if ( arg != 1 ) {
            	INFO("[MIPI] 1CH Disable\n");
        	}
    	}
    }
} 

int dumpSPU(unsigned int addr, int arg) 
{

    unsigned int regvalue0 = 0 ;
    unsigned int clk = 0 ;
/* 
 
"P_SCUA_PERI_CLKEN
0x93007004[25]

P_SCUA_SYS_SEL
0x930070E0[7]
SPU_CLK
SPU 27MHZ clock enable 
1: Enable
0: Disable"
 
"P_SCUA_SYS_SEL
0x930070E0[25:24]
SPU_CLK_SEL
SPU clock select:
SPUMCLK = fclk_sys/ (SPU_CLK_SEL+1)"
 
 
*/
    INFO("[--------------------------------------------------------------------------]\n");
    regvalue0 = READ32(addr+0x3007004) ;
    if ( regvalue0 & ( 1<<25) ) {
        INFO("[SPU] Enable\n");
    } else {
        if ( arg == 1 ) {
            return 0;
        }
        INFO("[SPU] Disable\n");
    }

    regvalue0 = READ32(addr+0x30070E0) ;

    if ( regvalue0 & ( 1<<7) ) {
        clk = XTAL_RATE ;
    } else {
        clk = spll ;
    }

    regvalue0 = (READ32(addr+0x30070E0) & (0x3<<24) ) >> 24;
    clk = clk / (regvalue0+1) ;
    INFO("[SPU] Clock %4d Mhz Disable/Enable ???????????????????????\n",clk/1000000);
    return 0;
}

int dumpPPU(unsigned int addr, int arg) 
{

    unsigned int regvalue0 = 0 ;
    unsigned int clk = 0 ;
    INFO("[--------------------------------------------------------------------------]\n");
/*
"internal system RAM and 
ppu sprite RAM (16KB)
不用 IRAM 的時候
應該也可以關掉 clock 才是"
 
"P_SCUA_PERI_CLKEN2
0x93007018[0]"
 
*/
    regvalue0 = READ32(addr+0x3007018) ;
    if ( regvalue0 & ( 1<<0) ) {
        INFO("[PPU_SPR] Enable\n");
        INFO("[PPU_SPR] Clock Disable/Enable ????????????MHz\n");
    } else {
        if ( arg != 1 ) {
            INFO("[PPU_SPR] Disable\n");
            INFO("[PPU_SPR] Clock Disable/Enable ????????????MHz\n");
        }
    }

/*
"P_SCUA_PERI_CLKEN2
0x93007018[24]"
*/
    regvalue0 = READ32(addr+0x3007018) ;
    if ( regvalue0 & ( 1<<24) ) {
        INFO("[PPU] Enable\n");
        INFO("[PPU] Clock Disable/Enable ????????????MHz\n");
    } else {
        if ( arg != 1 ) {
            INFO("[PPU] Disable\n");
            INFO("[PPU] Clock Disable/Enable ????????????MHz\n");
        }
    }

/*
"P_SCUA_PERI_CLKEN2
0x93007018[25]"
*/
    regvalue0 = READ32(addr+0x3007018) ;
    if ( regvalue0 & ( 1<<25) ) {
        INFO("[PPU_REG] Enable\n");
        INFO("[PPU_REG] Clock Disable/Enable ????????????MHz\n");
    } else {
        if ( arg != 1 ) {
            INFO("[PPU_REG] Disable\n");
            INFO("[PPU_REG] Clock Disable/Enable ????????????MHz\n");
        }
    }

/*
"P_SCUA_PERI_CLKEN2
0x93007018[26]"
*/
    regvalue0 = READ32(addr+0x3007018) ;
    if ( regvalue0 & ( 1<<26) ) {
        INFO("[PPU_TFT] Enable\n");
        INFO("[PPU_TFT] Clock Disable/Enable ????????????MHz\n");
    } else {
        if ( arg != 1 ) {
            INFO("[PPU_TFT] Disable\n");
            INFO("[PPU_TFT] Clock Disable/Enable ????????????MHz\n");
        }
    }


/*
"P_SCUA_PERI_CLKEN2
0x93007018[27]"
*/
    regvalue0 = READ32(addr+0x3007018) ;
    if ( regvalue0 & ( 1<<27) ) {
        INFO("[PPU_STN] Enable\n");
        INFO("[PPU_STN] Clock Disable/Enable ????????????MHz\n");
    } else {
        if ( arg != 1 ) {
            INFO("[PPU_STN] Disable\n");
            INFO("[PPU_STN] Clock Disable/Enable ????????????MHz\n");
        }
    }


/*
"P_SCUA_PERI_CLKEN2
0x93007018[29]"
*/
    regvalue0 = READ32(addr+0x3007018) ;
    if ( regvalue0 & ( 1<<29) ) {
        INFO("[PPU_FB] Enable\n");
    } else {
        if ( arg == 1 ) {
            return 0;
        }
        INFO("[PPU_FB] Disable\n");
    }
    INFO("[PPU_FB] Clock Disable/Enable ????????????MHz\n");
    return 0;
}


int dumpAPBDMA(unsigned int addr, int arg) 
{

    unsigned int regvalue0 = 0 ;
    unsigned int clk = 0 ;
/*
"P_SCUC_PERI_CLKEN
0x92005004[7]

P_APBDMA0_CTRL0
0x92B0006C[7]
P_APBDMA0_CTRL1
0x92B00070[7]
P_APBDMA0_CTRL2
0x92B00074[7]
P_APBDMA0_CTRL3
0x92B00078[7]
DMA channel enable.
0 = Disable DMA channel.
1 = Enable DMA channel."
*/
    INFO("[--------------------------------------------------------------------------]\n");
    regvalue0 = READ32(addr+0x2005004) ;
    if ( regvalue0 & ( 1<<7) ) {
        INFO("[APBDMA0] Enable\n");
        INFO("[APBDMA0] Main Clock %4d Mhz Enable\n",sys/1000000);
        INFO("[APBDMA0] Bus Clock %4d Mhz Enable\n",sys_ahb/1000000);
    } else {
        if ( arg != 1 ) {
            INFO("[APBDMA0] Disable\n");
            INFO("[APBDMA0] Main Clock %4d Mhz Enable\n",sys/1000000);
            INFO("[APBDMA0] Bus Clock %4d Mhz Enable\n",sys_ahb/1000000);
        }
    }
    
/*
"P_SCUA_PERI_CLKEN
0x93007004[9]

P_APBDMA1_CTRL0
0x9301006C[7]
P_APBDMA1_CTRL1
0x93010070[7]
Channel Enable
0:Disable DMA Channel
1:Enable DMA Channel"

*/
    regvalue0 = READ32(addr+0x3007004) ;
    if ( regvalue0 & ( 1<<9) ) {
        INFO("[APBDMA1] Enable\n");
    } else {
        if ( arg == 1 ) {
            return 0;
        }
        INFO("[APBDMA1] Disable\n");
    }
    INFO("[APBDMA1] Main Clock %4d Mhz Enable\n",sys/1000000);
    INFO("[APBDMA1] Bus Clock %4d Mhz Enable\n",sys_apb/1000000);
    return 0;
}


int dumpNAND(unsigned int addr, int arg) 
{

/*
P_SCUA_PERI_CLKEN
0x93007004[11]

P_NF_CSR
0x93008000[0]
Nand-flash function enable 
0: Nand Disable, 1:Nand Enable 
 
P_NF_AC_TIMING
0x93008008
[3:0] CLE pulse width
[7:4] ALE pulse width
[11:8] ACT pulse width
[15:12] REC pulse width
[19:16] WAIT
[25:20] Ready Status Pulse width

NAND cycle time is referenced with Sys AHB Clock

fclk_sys_ahb 
 
*/ 
    unsigned int regvalue0 = 0 ;
    unsigned int clk = 0 ;
    INFO("[--------------------------------------------------------------------------]\n");
    regvalue0 = READ32(addr+0x3007004) ;
    if ( regvalue0 & ( 1<<11) ) {
        INFO("[NAND] Enable\n");
    } else {
        if ( arg == 1 ) {
            return 0;
        }
        INFO("[NAND] Disable\n");
    }
    INFO("[NAND] Main Clock %4d Mhz Enable\n",sys_ahb/1000000);
    INFO("[NAND] Bus Clock %4d Mhz Enable\n",sys_ahb/1000000);
    //INFO("[NAND] OutPut Clock %4d Khz ????????????????\n",sys_apb/1000000);


/*
"P_SCUA_PERI_CLKEN
0x93007004[13]"
fclk_sys
*/
    regvalue0 = READ32(addr+0x3007004) ;
    if ( regvalue0 & ( 1<<13) ) {
        INFO("[BCH] Enable\n");
    } else {
        if ( arg == 1 ) {
            return 0;
        }
        INFO("[BCH] Disable\n");
    }
    INFO("[BCH] Main Clock %4d Mhz Enable\n",sys/1000000);
    INFO("[BCH] Bus Clock %4d Mhz Enable\n",sys/1000000);

    return 0;
}

int dumpSAADC(unsigned int addr, int arg) 
{

/*
"P_SCUA_PERI_CLKEN
0x93007004[19]"

fclk_sys_apb

*/
    unsigned int regvalue0 = 0 ;
    //unsigned int clk = 0 ;
    INFO("[--------------------------------------------------------------------------]\n");
    regvalue0 = READ32(addr+0x3007004) ;
    if ( regvalue0 & ( 1<<19) ) {
        INFO("[SAADC] Enable\n");
    } else {
        if ( arg != 1 ) {
            INFO("[SAADC] Disable\n");
            INFO("[SAADC] Main Clock %4d Mhz Enable\n",sys_apb/1000000);
    		INFO("[SAADC] Bus Clock %4d Mhz Enable\n",sys_apb/1000000);
        }   
    }
    

/*
P_AUD_LININCTL(0x9301F024)[0]
ENLININ
Line in power control, high active
0: disable
1: enable
P_AUD_ADCCTL(0x9301F028)[0]
ENADL
Left channel ADC power control, high active
0: disable
1: enable
P_AUD_ADCCTL(0x9301F028)[1]
ENADR
Right channel ADC power control, high active
0: disable
1: enable
P_AUD_PWCTRL(0x9301F020)[1]
ENVREF
VREF power control, high active
0: disable
1: enable
*/
	regvalue0 = READ32(addr+0x301F024) ;
    if ( regvalue0 & ( 1<<0) ) {
        INFO("[SAADC] Line In Power Enable\n");
    } else {
        if ( arg != 1 ) {
            INFO("[SAADC] Line In Power Disable\n");
        }   
    }
    
    regvalue0 = READ32(addr+0x301F028) ;
    if ( regvalue0 & ( 1<<0) ) {
        INFO("[SAADC] Left Channel Power Enable\n");
    } else {
        if ( arg != 1 ) {
            INFO("[SAADC] Left Channel Power Disable\n");
        }   
    }
    
    regvalue0 = READ32(addr+0x301F028) ;
    if ( regvalue0 & ( 1<<1) ) {
        INFO("[SAADC] Right Channel Power Enable\n");
    } else {
        if ( arg != 1 ) {
            INFO("[SAADC] Right Channel Power Disable\n");
        }   
    }
    
    regvalue0 = READ32(addr+0x301F020) ;
    if ( regvalue0 & ( 1<<1) ) {
        INFO("[SAADC] VREF Enable\n");
    } else {
        if ( arg != 1 ) {
            INFO("[SAADC] VREF Disable\n");
        }   
    }
    
    return 0;
}

int dumpMIC(unsigned int addr, int arg) 
{

/*
P_SCUA_PERI_CLKEN
0x93007004[18]
fclk_sys_apb

*/
    unsigned int regvalue0 = 0 ;
    //unsigned int clk = 0 ;
    INFO("[--------------------------------------------------------------------------]\n");
    regvalue0 = READ32(addr+0x3007004) ;
    if ( regvalue0 & ( 1<<18) ) {
        INFO("[MIC] Enable\n");
    } else {
        if ( arg != 1 ) {
            INFO("[MIC] Disable\n");
            INFO("[MIC] Main Clock %4d Mhz Enable\n",sys_apb/1000000);
    		INFO("[MIC] Bus Clock %4d Mhz Enable\n",sys_apb/1000000);
        }   
    }
    

/*
P_AUD_PWCTRL(0x9301F020)[3]
ENMICBIAS
MIC bias-voltage output power control, high active
0: disable
1: enable
P_AUD_PWCTRL(0x9301F020)[4]
ENMIC
MIC power control, high active
0: disable
1: enable
P_AUD_ADCCTL(0x9301F028)[0]
ENADL
Left channel ADC power control, high active
0: disable
1: enable
P_AUD_ADCCTL(0x9301F028)[1]
ENADR
Right channel ADC power control, high active
0: disable
1: enable
P_AUD_PWCTRL(0x9301F020)[1]
ENVREF
VREF power control, high active
0: disable
1: enable
*/
	regvalue0 = READ32(addr+0x301F020) ;
    if ( regvalue0 & ( 1<<3) ) {
        INFO("[MIC] Bias-Voltage Output Power Enable\n");
    } else {
        if ( arg != 1 ) {
            INFO("[MIC] Bias-Voltage Output Power Disable\n");
        }   
    }
    
    regvalue0 = READ32(addr+0x301F020) ;
    if ( regvalue0 & ( 1<<4) ) {
        INFO("[MIC] Power Enable\n");
    } else {
        if ( arg != 1 ) {
            INFO("[MIC] Power Disable\n");
        }   
    }
    
    regvalue0 = READ32(addr+0x301F028) ;
    if ( regvalue0 & ( 1<<0) ) {
        INFO("[MIC] Left Channel Power Enable\n");
    } else {
        if ( arg != 1 ) {
            INFO("[MIC] Left Channel Power Disable\n");
        }   
    }
    
    regvalue0 = READ32(addr+0x301F028) ;
    if ( regvalue0 & ( 1<<1) ) {
        INFO("[MIC] Right Channel Power Enable\n");
    } else {
        if ( arg != 1 ) {
            INFO("[MIC] Right Channel Power Disable\n");
        }   
    }
    
    regvalue0 = READ32(addr+0x301F020) ;
    if ( regvalue0 & ( 1<<1) ) {
        INFO("[MIC] VREF Enable\n");
    } else {
        if ( arg != 1 ) {
            INFO("[MIC] VREF Disable\n");
        }   
    }
    
    return 0;
}

int dumpFM(unsigned int addr, int arg) 
{

    unsigned int regvalue0 = 0 ;

/*
P_AUD_LININCTL(0x9301F024)[0]
ENLININ
Line in power control, high active
0: disable
1: enable
P_AUD_PWCTRL(0x9301F020)[1]
ENVREF
VREF power control, high active
0: disable
1: enable
P_DAC_ HDPHN(0x9301F038)[0]
ENHPL
Left channel headphone amplifier power control.
0: Disable
1: Enable
P_DAC_ HDPHN(0x9301F038)[1]
ENHPR
Right channel headphone amplifier power control.
0: Disable
1: Enable
*/  
    
	regvalue0 = READ32(addr+0x301F024) ;
    if ( regvalue0 & ( 1<<0) ) {
        INFO("[FM] Line In Power Enable\n");
    } else {
        if ( arg != 1 ) {
            INFO("[FM] Line In Power Disable\n");
        }   
    }
    
    regvalue0 = READ32(addr+0x301F020) ;
    if ( regvalue0 & ( 1<<1) ) {
        INFO("[FM] VREF Enable\n");
    } else {
        if ( arg != 1 ) {
            INFO("[FM] VREF Disable\n");
        }   
    }
    
    regvalue0 = READ32(addr+0x301F038) ;
    if ( regvalue0 & ( 1<<0) ) {
        INFO("[FM] Left Channel Headphone Power Enable\n");
    } else {
        if ( arg != 1 ) {
            INFO("[FM] Left Channel Headphone Power Disable\n");
        }   
    }
    
    regvalue0 = READ32(addr+0x301F038) ;
    if ( regvalue0 & ( 1<<1) ) {
        INFO("[FM] Right Channel Headphone Power Enable\n");
    } else {
        if ( arg != 1 ) {
            INFO("[FM] Right Channel Headphone Power Disable\n");
        }   
    }
    
    return 0;
}

int dumpOVG(unsigned int addr, int arg) 
{

/*
"P_SCUA_PERI_CLKEN
0x93007004[27]"

fclk_sys
 
*/
    unsigned int regvalue0 = 0 ;
    //unsigned int clk = 0 ;
    INFO("[--------------------------------------------------------------------------]\n");
    regvalue0 = READ32(addr+0x3007004) ;
    if ( regvalue0 & ( 1<<27) ) {
        INFO("[OVG] Enable\n");
    } else {
        if ( arg == 1 ) {
            return 0;
        }
        INFO("[OVG] Disable\n");
    }
    INFO("[OVG] Main Clock %4d Mhz Enable\n",sys/1000000);
    INFO("[OVG] Bus Clock %4d Mhz Enable\n",sys/1000000);

    return 0;
}

int dumpAES(unsigned int addr, int arg) 
{
/*
"P_SCUA_PERI_CLKEN
0x93007004[30]"
 
fclk_sys_ahb
 
*/
    unsigned int regvalue0 = 0 ;
    //unsigned int clk = 0 ;
    INFO("[--------------------------------------------------------------------------]\n");
    regvalue0 = READ32(addr+0x3007004) ;
    if ( regvalue0 & ( 1<<30) ) {
        INFO("[AES] Enable\n");
    } else {
        if ( arg == 1 ) {
            return 0;
        }
        INFO("[AES] Disable\n");
    }
    INFO("[AES] Main Clock %4d Mhz Enable\n",sys_ahb/1000000);
    INFO("[AES] Bus Clock %4d Mhz Enable\n",sys_ahb/1000000);

    return 0;
}

int dumpCEVAL2(unsigned int addr, int arg) 
{
/*
"ceva L2 cache and 
OVG ram (32KB)"
 
"P_SCUA_PERI_CLKEN2
0x93007018[1]"
 
fclk_sys
 
*/
    unsigned int regvalue0 = 0 ;
    unsigned int clk = 0 ;
    INFO("[--------------------------------------------------------------------------]\n");
    regvalue0 = READ32(addr+0x3007018) ;
    if ( regvalue0 & ( 1<<1) ) {
        INFO("[CEVAL2] Enable\n");
    } else {
        if ( arg == 1 ) {
            return 0;
        }
        INFO("[CEVAL2] Disable\n");
    }
    INFO("[CEVAL2] Main Clock %4d Mhz Enable\n",sys/1000000);
    return 0;
}

int dumpVIC(unsigned int addr, int arg) 
{

/*
"P_SCUB_PERI_CLKEN
0x90005020[4]"
fclk_arm_ahb
*/
    unsigned int regvalue0 = 0 ;
    //unsigned int clk = 0 ;
    INFO("[--------------------------------------------------------------------------]\n");
    regvalue0 = READ32(addr+0x0005020) ;
    if ( regvalue0 & ( 1<<4) ) {
        INFO("[VIC0] Enable\n");
        INFO("[VIC0] Main Clock %4d Mhz Enable\n",arm_ahb/1000000);
        INFO("[VIC0] Bus Clock %4d Mhz Enable\n",arm_ahb/1000000);
    } else {
        if ( arg != 1 ) {
            INFO("[VIC0] Disable\n");
            INFO("[VIC0] Main Clock %4d Mhz Enable\n",arm_ahb/1000000);
            INFO("[VIC0] Bus Clock %4d Mhz Enable\n",arm_ahb/1000000);
        }
    }

/*
"P_SCUB_PERI_CLKEN
0x90005020[5]"
fclk_arm_ahb
*/
    regvalue0 = READ32(addr+0x0005020) ;
    if ( regvalue0 & ( 1<<5) ) {
        INFO("[VIC1] Enable\n");
    } else {
        if ( arg == 1 ) {
            return 0;
        }
        INFO("[VIC1] Disable\n");
    }
    INFO("[VIC1] Main Clock %4d Mhz Enable\n",arm_ahb/1000000);
    INFO("[VIC1] Bus Clock %4d Mhz Enable\n",arm_ahb/1000000);
    return 0;
}

int dumpARMTimer(unsigned int addr, int arg) 
{

/*
"P_SCUB_PERI_CLKEN
0x90005020[9]"
fclk_arm_apb
*/
    unsigned int regvalue0 = 0 ;
    //unsigned int clk = 0 ;
    INFO("[--------------------------------------------------------------------------]\n");
    regvalue0 = READ32(addr+0x0005020) ;
    if ( regvalue0 & ( 1<<9) ) {
        INFO("[ARMTimer] Enable\n");
    } else {
        if ( arg == 1 ) {
            return 0;
        }
        INFO("[ARMTimer] Disable\n");
    }
    INFO("[ARMTimer] Main Clock %4d Mhz Enable\n",arm_apb/1000000);
    INFO("[ARMTimer] Bus Clock %4d Mhz Enable\n",arm_apb/1000000);

    return 0;
}

int dumpWDT(unsigned int addr, int arg) 
{

/*
"P_SCUB_PERI_CLKEN
0x90005020[10]"
fclk_arm_apb
*/
    unsigned int regvalue0 = 0 ;
    //unsigned int clk = 0 ;
    INFO("[--------------------------------------------------------------------------]\n");
    regvalue0 = READ32(addr+0x0005020) ;
    if ( regvalue0 & ( 1<<10) ) {
        INFO("[WDT] Enable\n");
    } else {
        if ( arg == 1 ) {
            return 0;
        }
        INFO("[WDT] Disable\n");
    }
    INFO("[WDT] Main Clock %4d Mhz Enable\n",arm_apb/1000000);
    INFO("[WDT] Bus Clock %4d Mhz Enable\n",arm_apb/1000000);

    return 0;
}

int dumpRAND(unsigned int addr, int arg) 
{
/*
"P_SCUB_PERI_CLKEN
0x90005020[13]"
fclk_arm_apb
*/
    unsigned int regvalue0 = 0 ;
    //unsigned int clk = 0 ;
    INFO("[--------------------------------------------------------------------------]\n");
    regvalue0 = READ32(addr+0x0005020) ;
    if ( regvalue0 & ( 1<<13) ) {
        INFO("[RAND] Enable\n");
    } else {
        if ( arg == 1 ) {
            return 0;
        }
        INFO("[RAND] Disable\n");
    }
    INFO("[RAND] Main Clock %4d Mhz Enable\n",arm_apb/1000000);
    INFO("[RAND] Bus Clock %4d Mhz Enable\n",arm_apb/1000000);

    return 0;
}

int dumpGPIO(unsigned int addr, int arg) 
{

/*
"P_SCUB_PERI_CLKEN
0x90005020[14]"
fclk_arm_apb
*/
    unsigned int regvalue0 = 0 ;
    //unsigned int clk = 0 ;
    INFO("[--------------------------------------------------------------------------]\n");
    regvalue0 = READ32(addr+0x0005020) ;
    if ( regvalue0 & ( 1<<14) ) {
        INFO("[GPIO] Enable\n");
    } else {
        if ( arg == 1 ) {
            return 0;
        }
        INFO("[GPIO] Disable\n");
    }
    INFO("[GPIO] Main Clock %4d Mhz Enable\n",arm_apb/1000000);
    INFO("[GPIO] Bus Clock %4d Mhz Enable\n",arm_apb/1000000);

    return 0;
}

int dumpRTC(unsigned int addr, int arg) 
{

/*
"P_SCUB_PERI_CLKEN
0x90005020[15]"
fclk_arm_apb
*/
    unsigned int regvalue0 = 0 ;
    //unsigned int clk = 0 ;
    INFO("[--------------------------------------------------------------------------]\n");
    regvalue0 = READ32(addr+0x0005020) ;
    if ( regvalue0 & ( 1<<15) ) {
        INFO("[RTC] Enable\n");
    } else {
        if ( arg == 1 ) {
            return 0;
        }
        INFO("[RTC] Disable\n");
    }
    INFO("[RTC] Main Clock %4d Mhz Enable\n",arm_apb/1000000);
    INFO("[RTC] Bus Clock %4d Mhz Enable\n",arm_apb/1000000);

    return 0;
}

int dumpSDMA(unsigned int addr, int arg) 
{

/* 
"P_SCUC_PERI_CLKEN
0x92005004[1]"
 
*/ 
    unsigned int regvalue0 = 0 ;
    //unsigned int clk = 0 ;
    INFO("[--------------------------------------------------------------------------]\n");
    regvalue0 = READ32(addr+0x2005004) ;
    if ( regvalue0 & ( 1<<1) ) {
        INFO("[SDMA] Enable\n");
    } else {
        if ( arg == 1 ) {
            return 0;
        }
        INFO("[SDMA] Disable\n");
    }
    INFO("[SDMA] Main Clock %4d Mhz Enable\n",sys/1000000);
    INFO("[SDMA] Bus Clock %4d Mhz Enable\n",sys/1000000);
    return 0;
}

int dumpXDMA(unsigned int addr, int arg) 
{

/* 
"P_SCUC_PERI_CLKEN
0x92005004[2]"
 
*/ 
    unsigned int regvalue0 = 0 ;
    //unsigned int clk = 0 ;
    INFO("[--------------------------------------------------------------------------]\n");
    regvalue0 = READ32(addr+0x2005004) ;
    if ( regvalue0 & ( 1<<2) ) {
        INFO("[XDMA] Enable\n");
    } else {
        if ( arg == 1 ) {
            return 0;
        }
        INFO("[XDMA] Disable\n");
    }
    INFO("[XDMA] Main Clock %4d Mhz Enable\n",sys/1000000);
    INFO("[XDMA] Bus Clock %4d Mhz Enable\n",sys/1000000);
    return 0;
}

int dumpDRAM(unsigned int addr, int arg) 
{

/* 
"P_SCUC_PERI_CLKEN
0x92005004[4]"
 
*/ 
    unsigned int regvalue0 = 0 ;
    //unsigned int clk = 0 ;
    INFO("[--------------------------------------------------------------------------]\n");
    regvalue0 = READ32(addr+0x2005004) ;
    if ( regvalue0 & ( 1<<4) ) {
        INFO("[DRAM] Enable\n");
    } else {
        if ( arg == 1 ) {
            return 0;
        }
        INFO("[DRAM] Disable\n");
    }
    INFO("[DRAM] Main Clock %4d Mhz Enable\n",sys/1000000);
    INFO("[DRAM] Bus Clock %4d Mhz Enable\n",sys/1000000);

    return 0;
}

int dumpMS(unsigned int addr, int arg) 
{

    unsigned int regvalue0 = 0 ;
    unsigned int clk = 0 ;
    INFO("[--------------------------------------------------------------------------]\n");
/*
"P_SCUC_PERI_CLKEN
0x92005004[11]" 
 
fclk_sys_apb
 
*/
    regvalue0 = READ32(addr+0x2005004) ;
    if ( regvalue0 & ( 1<<11) ) {
        INFO("[MS] Enable\n");
    } else {
        if ( arg == 1 ) {
            return 0;
        }
        INFO("[MS] Disable\n");
    }
    INFO("[MS] Main Clock %4d Mhz Enable\n",sys_apb/1000000);
    INFO("[MS] Bus Clock %4d Mhz Enable\n",sys_apb/1000000);
    //INFO("[MS] OutPut Clock %4d Khz ????????????????\n",sys_apb/1000000);
    return 0;
}

int dumpUART(unsigned int addr, int arg) 
{

    unsigned int regvalue0 = 0 ;
    unsigned int clk = 0 ;
    INFO("[--------------------------------------------------------------------------]\n");
/* 
"P_SCUC_PERI_CLKEN
0x92005004[13]

P_SCUA_UART_CFG
0x93007094[8]
UART_CLK_EN
0: clk_uart is off. 
1: clk_uart is on"
 
 
"P_SCUA_UART_CFG
0x93007094[17]
UART0_CLK
Select UART0 clock source , 
0:SPLL
1:XTAL 27MHz and bypass clock divider

P_SCUA_UART_CFG
0x93007094[7:0]
UART_CLK_RATIO
Set the ratio for clk_uart,only valid whe clock select from SPLL.

fuclk = fclk_ref_ceva/(UART_CLK_RATIO+1)


P_UART0_DLL
0x92B04000[7:0]
DLL
P_UART0_DLM
0x92B04004[7:0]
DLM
P_UART0_SPR
0x92B04024[3:0]
BT
baud  rate = ( fuclk/(BT+1) ) / ([DLM, DLL] + 1)"
 
*/ 
    regvalue0 = READ32(addr+0x2005004) ;
    if ( regvalue0 & ( 1<<13) ) {
        INFO("[UART0] Enable\n");
        INFO("[UART0] Bus Clock %4d Mhz Enable\n",sys_apb/1000000);
    } else {
        if ( arg != 1 ) {
            INFO("[UART0] Disable\n");
            INFO("[UART0] Bus Clock %4d Mhz Enable\n",sys_apb/1000000);
        }
    }


    regvalue0 = READ32(addr+0x3007094) ;
    if ( regvalue0 & ( 1<<17) ) {
        clk = XTAL_RATE ;
    } else {
        clk = fclk_ref_ceva ;
    }
    clk = clk / ( (READ32(addr+0x3007094) & 0xFF ) + 1 ) ;

    regvalue0 = READ32(addr+0x3007094) ;
    if ( regvalue0 & ( 1<<8) ) {
        //INFO("[UART0] Main Clock (baud rate) %d???????????????????\n",clk);
        INFO("[UART0] UART Clock Clock Enable %4d Mhz\n",clk/1000000);
    } else {
        if ( arg != 1 ) {
            INFO("[UART0] Clock Disable\n");
        }
    }

/*
"P_SCUC_PERI_CLKEN
0x92005004[15]

P_SCUA_UART_CFG
0x93007094[8]
UART_CLK_EN
0: clk_uart is off. 
1: clk_uart is on"
 
"P_SCUA_UART_CFG
0x93007094[18]
UART2_CLK
Select UART2 clock source , 
0:SPLL
1:XTAL 27MHz and bypass clock divider

P_SCUA_UART_CFG
0x93007094[7:0]
UART_CLK_RATIO
Set the ratio for clk_uart,only valid whe clock select from SPLL.

fuclk = fclk_ref_ceva/(UART_CLK_RATIO+1)


P_UART1_DLL
0x92B06000[7:0]
DLL
P_UART1_DLM
0x92B06004[7:0]
DLM
P_UART1_SPR
0x92B06024[3:0]
BT

baud  rate = ( fuclk/(BT+1) ) / ([DLM, DLL] + 1)"
 
*/

    regvalue0 = READ32(addr+0x2005004) ;
    if ( regvalue0 & ( 1<<15) ) {
        INFO("[UART1] Enable\n");
        INFO("[UART1] Bus Clock %4d Mhz Enable\n",sys_apb/1000000);
    } else {
        if ( arg != 1 ) {
            INFO("[UART1] Disable\n");
            INFO("[UART1] Bus Clock %4d Mhz Enable\n",sys_apb/1000000);
        }
    }

    regvalue0 = READ32(addr+0x3007094) ;
    if ( regvalue0 & ( 1<<18) ) {
        clk = XTAL_RATE ;
    } else {
        clk = fclk_ref_ceva ;
    }
    clk = clk / ( (READ32(addr+0x3007094) & 0xFF ) + 1 ) ;

    regvalue0 = READ32(addr+0x3007094) ;
    if ( regvalue0 & ( 1<<8) ) {
        //INFO("[UART1] Main Clock (baud rate) %d???????????????????\n",clk);
        INFO("[UART1] UART Clock Clock Enable %4d Mhz\n",clk/1000000);
    } else {
        if ( arg == 1 ) {
            return 0;
        }
        INFO("[UART1] Clock Disable\n");
    }

    return 0;
}

int dumpSPI(unsigned int addr, int arg) 
{

    unsigned int regvalue0 = 0 ;
    //unsigned int clk = 0 ;
    INFO("[--------------------------------------------------------------------------]\n");
/*
"P_SCUC_PERI_CLKEN
0x92005004[16]" 
 
fclk_sys_apb
 
*/
    regvalue0 = READ32(addr+0x2005004) ;
    if ( regvalue0 & ( 1<<16) ) {
        INFO("[SPI0] Enable\n");
        INFO("[SPI0] Main Clock %4d Mhz Enable\n",sys_apb/1000000);
        INFO("[SPI0] Bus Clock %4d Mhz Enable\n",sys_apb/1000000);
        //INFO("[SPI0] OutPut Clock %4d Khz ????????????????\n",sys_apb/1000000);
    } else {
        if ( arg != 1 ) {
            INFO("[SPI0] Disable\n");
            INFO("[SPI0] Main Clock %4d Mhz Enable\n",sys_apb/1000000);
            INFO("[SPI0] Bus Clock %4d Mhz Enable\n",sys_apb/1000000);
        }
    }

/*
"P_SCUC_PERI_CLKEN
0x92005004[17]" 
 
fclk_sys_apb
 
*/
    regvalue0 = READ32(addr+0x2005004) ;
    if ( regvalue0 & ( 1<<17) ) {
        INFO("[SPI1] Enable\n");
    } else {
        if ( arg == 1 ) {
            return 0;
        }
        INFO("[SPI1] Disable\n");
    }
    INFO("[SPI1] Main Clock %4d Mhz Enable\n",sys_apb/1000000);
    INFO("[SPI1] Bus Clock %4d Mhz Enable\n",sys_apb/1000000);
    //INFO("[SPI1] OutPut Clock %4d Khz ????????????????\n",sys_apb/1000000);

    return 0;
}

int dumpSDC(unsigned int addr, int arg) 
{

    unsigned int regvalue0 = 0 ;
    //unsigned int clk = 0 ;
    INFO("[--------------------------------------------------------------------------]\n");
/*
"P_SCUC_PERI_CLKEN
0x92005004[18]" 
 
fclk_sys_apb
 
*/
    regvalue0 = READ32(addr+0x2005004) ;
    if ( regvalue0 & ( 1<<18) ) {
        INFO("[SDC0] Enable\n");
        INFO("[SDC0] Main Clock %4d Mhz Enable\n",sys_apb/1000000);
        INFO("[SDC0] Bus Clock %4d Mhz Enable\n",sys_apb/1000000);
        //INFO("[SDC0] OutPut Clock %4d Khz ????????????????\n",sys_apb/1000000);
    } else {
        if ( arg != 1 ) {
            INFO("[SDC0] Disable\n");
            INFO("[SDC0] Main Clock %4d Mhz Enable\n",sys_apb/1000000);
            INFO("[SDC0] Bus Clock %4d Mhz Enable\n",sys_apb/1000000);
        }
    }
/*
"P_SCUC_PERI_CLKEN
0x92005004[19]" 
 
fclk_sys_apb
 
*/
    regvalue0 = READ32(addr+0x2005004) ;
    if ( regvalue0 & ( 1<<19) ) {
        INFO("[SDC1] Enable\n");
    } else {
        if ( arg == 1 ) {
            return 0;
        }
        INFO("[SDC1] Disable\n");
    }
    INFO("[SDC1] Main Clock %4d Mhz Enable\n",sys_apb/1000000);
    INFO("[SDC1] Bus Clock %4d Mhz Enable\n",sys_apb/1000000);
    //INFO("[SDC1] OutPut Clock %4d Khz ????????????????\n",sys_apb/1000000);
    return 0;
}

int dumpI2C(unsigned int addr, int arg) 
{
/*
"P_SCUC_PERI_CLKEN
0x92005004[20]" 
 
fclk_sys_apb
 
*/
    unsigned int regvalue0 = 0 ;
    //unsigned int clk = 0 ;
    INFO("[--------------------------------------------------------------------------]\n");
    regvalue0 = READ32(addr+0x2005004) ;
    if ( regvalue0 & ( 1<<20) ) {
        INFO("[I2C] Enable\n");
    } else {
        if ( arg == 1 ) {
            return 0;
        }
        INFO("[I2C] Disable\n");
    }
    
    INFO("[I2C] Main Clock %4d Mhz Enable\n",sys_apb/1000000);
    INFO("[I2C] Bus Clock %4d Mhz Enable\n",sys_apb/1000000);
    //INFO("[I2C] OutPut Clock %4d Khz ????????????????\n",sys_apb/1000000);
    return 0;
}


int dumpSCALER(unsigned int addr, int arg) 
{

    unsigned int regvalue0 = 0 ;
    //unsigned int clk = 0 ;
    INFO("[--------------------------------------------------------------------------]\n");
/*
"P_SCUC_PERI_CLKEN
0x92005004[21]"

*/
    regvalue0 = READ32(addr+0x2005004) ;
    if ( regvalue0 & ( 1<<21) ) {
        INFO("[SCALER1] Enable\n");
        INFO("[SCALER1] Main Clock %4d Mhz Enable\n",sys/1000000);
        INFO("[SCALER1] Bus Clock %4d Mhz Enable\n",sys/1000000);
    } else {
        if ( arg != 1 ) {
            INFO("[SCALER1] Disable\n");
            INFO("[SCALER1] Main Clock %4d Mhz Enable\n",sys/1000000);
            INFO("[SCALER1] Bus Clock %4d Mhz Enable\n",sys/1000000);
        }
    }

/*
"P_SCUA_PERI_CLKEN
0x93007004[26]"
*/
    regvalue0 = READ32(addr+0x3007004) ;
    if ( regvalue0 & ( 1<<26) ) {
        INFO("[SCALER2] Enable\n");
    } else {
        if ( arg == 1 ) {
            return 0;
        }
        INFO("[SCALER2] Disable\n");
    }
    INFO("[SCALER2] Main Clock %4d Mhz Enable\n",sys/1000000);
    INFO("[SCALER2] Bus Clock %4d Mhz Enable\n",sys/1000000);

    return 0;
}

int dumpDC2DC(unsigned int addr, int arg) 
{
/*
"P_SCUB_PWM_CTRL
0x90005140[0]
CLK6M_EN
PWM CLK 6M Enable
0 = Disable
1 = Enable

P_SCUB_PWM_CTRL
0x90005140[1]
PWM0_EN
PWM0 Enable
0 = Disable
1 = Enable

P_SCUB_PWM_CTRL
0x90005140[2]
PWM1_EN
PWM1 Enable
0 = Disable
1 = Enable"
 
6.75MHz
 
*/
    unsigned int regvalue0 = 0 ;
    //unsigned int clk = 0 ;
    INFO("[--------------------------------------------------------------------------]\n");
    regvalue0 = READ32(addr+0x0005140) ;
    if ( regvalue0 & ( 1<<0) ) {
        INFO("[DC2DC] Main Clock 6.75MHz Enable\n");
    } else {
        if ( arg == 1 ) {
            return 0;
        }
        INFO("[DC2DC] Main Clock 6.75MHz Disable\n");
    }
    if ( regvalue0 & ( 1<<1) ) {
        INFO("[DC2DC] PWM0 Enable\n");
    } else {
        INFO("[DC2DC] PWM0 Disable\n");
    }
    if ( regvalue0 & ( 1<<2) ) {
        INFO("[DC2DC] PWM1 Enable\n");
    } else {
        INFO("[DC2DC] PWM1 Disable\n");
    }
    return 0;
}

int dumpIOPull(unsigned int addr, int arg) 
{

/*
[bit3] [bit2] [bit1] [bit0] 
[GPIO/Function 1 = GPIO][Direction 0=OUT] [Input Enable 1=Enable ] [Pull Select 0=DOWN] 
0 , 0 , 0 , 0  = 0 
0 , 0 , 0 , 1  = 1 
0 , 0 , 1 , 0  = CHECK-0  
0 , 0 , 1 , 1  = CHECK-1 
0 , 1 , 0 , 0  = CHECK-2 
0 , 1 , 0 , 1  = CHECK-3 
0 , 1 , 1 , 0  = 6  
0 , 1 , 1 , 1  = 7 
0 , 0 , 0 , 0  = 8 
0 , 0 , 0 , 1  = 9 
0 , 0 , 1 , 0  = ERROR-0  
0 , 0 , 1 , 1  = ERROR-1 
0 , 1 , 0 , 0  = ERROR-2 
0 , 1 , 0 , 1  = ERROR-3 
0 , 1 , 1 , 0  = 14  
0 , 1 , 1 , 1  = 15 
*/

char *GPIOStatePull[] = { 
    "[FUNC][OUT ][PD]" ,
    "[FUNC][OUT ][PU]" ,
    "[FUNC][CHK0][PD]" ,
    "[FUNC][CHK1][PU]" ,
    "[FUNC][CHK2][PD]" ,
    "[FUNC][CHK3][PU]" ,
    "[FUNC][IN  ][PD]" ,
    "[FUNC][IN  ][PU]" ,
    "[GPIO][OUT ][PD]" ,
    "[GPIO][OUT ][PU]" ,
    "[GPIO][ERR0][PD]" ,
    "[GPIO][ERR1][PU]" ,
    "[GPIO][ERR2][PD]" ,
    "[GPIO][ERR3][PU]" ,
    "[GPIO][IN  ][PD]" ,
    "[GPIO][IN  ][PU]" ,
};

char *GPIOStateNoPull[] = { 
    "[FUNC][OUT ][PX]" ,
    "[FUNC][OUT ][PX]" ,
    "[FUNC][CHK0][PX]" ,
    "[FUNC][CHK1][PX]" ,
    "[FUNC][CHK2][PX]" ,
    "[FUNC][CHK3][PX]" ,
    "[FUNC][IN  ][PX]" ,
    "[FUNC][IN  ][PX]" ,
    "[GPIO][OUT ][PX]" ,
    "[GPIO][OUT ][PX]" ,
    "[GPIO][ERR0][PX]" ,
    "[GPIO][ERR1][PX]" ,
    "[GPIO][ERR2][PX]" ,
    "[GPIO][ERR3][PX]" ,
    "[GPIO][IN  ][PX]" ,
    "[GPIO][IN  ][PX]" ,
};
    unsigned int regvalue0 = 0 ;
    unsigned int regvalue1 = 0 ;
    unsigned int regvalue2 = 0 ;
    unsigned int regvalue3 = 0 ;
    unsigned int regvalue4 = 0 ;
    int i = 0 ;
    int index = 0 ;
    INFO("[--------------------------------------------------------------------------]\n");
/*
IOA
*/
    regvalue4 = READ32(addr+0x000A000) ; /* Select the IO port to GPIO function or
                                            other function */
    regvalue0 = READ32(addr+0x000A020) ; /* Direction */
    regvalue1 = READ32(addr+0x0005100) ; /* Input Enable */
    regvalue2 = READ32(addr+0x0005108) ; /* Pull Enable */
    regvalue3 = READ32(addr+0x000510C) ; /* Pull Select */

    for ( i = 31 ; i >= 0 ; i-- ) {
        index = 0 ;
        if (regvalue4 & (1<<i)) {
            index = index | (1 << 3) ;
        }
        if (regvalue0 & (1<<i)) {
            index = index | (1 << 2) ;
        }
        if (regvalue1 & (1<<i)) {
            index = index | (1 << 1) ;
        }
        if (regvalue3 & (1<<i)) {
            index = index | (1 << 0) ;
        }

        if (regvalue2 & (1<<i)) {
            INFO("[IOPULL] IOA%02d %s DIR[%d],IE[%d]\n", i , GPIOStatePull[index], ((regvalue0 & (1<<i)) >> i ) , ((regvalue1 & (1<<i)) >> i ));
        } else {
            INFO("[IOPULL] IOA%02d %s DIR[%d],IE[%d]\n", i , GPIOStateNoPull[index], ((regvalue0 & (1<<i)) >> i ) , ((regvalue1 & (1<<i)) >> i ));
        }
    }

/*
IOB
*/
    regvalue4 = READ32(addr+0x000A004) ; /* Select the IO port to GPIO function or
                                            other function */
    regvalue0 = READ32(addr+0x000A024) ; /* Direction */
    regvalue1 = READ32(addr+0x0005110) ; /* Input Enable */
    regvalue2 = READ32(addr+0x0005118) ; /* Pull Enable */
    regvalue3 = READ32(addr+0x000511C) ; /* Pull Select */
    for ( i = 31 ; i >= 0 ; i-- ) {
        index = 0 ;
        if (regvalue4 & (1<<i)) {
            index = index | (1 << 3) ;
        }
        if (regvalue0 & (1<<i)) {
            index = index | (1 << 2) ;
        }
        if (regvalue1 & (1<<i)) {
            index = index | (1 << 1) ;
        }
        if (regvalue3 & (1<<i)) {
            index = index | (1 << 0) ;
        }

        if (regvalue2 & (1<<i)) {
            INFO("[IOPULL] IOB%02d %s DIR[%d],IE[%d]\n", i , GPIOStatePull[index], ((regvalue0 & (1<<i)) >> i ) , ((regvalue1 & (1<<i)) >> i ));
        } else {
            INFO("[IOPULL] IOB%02d %s DIR[%d],IE[%d]\n", i , GPIOStateNoPull[index], ((regvalue0 & (1<<i)) >> i ) , ((regvalue1 & (1<<i)) >> i ));
        }
    }

/*
IOC
*/
    regvalue4 = READ32(addr+0x000A008) ; /* Select the IO port to GPIO function or
                                            other function */
    regvalue0 = READ32(addr+0x000A028) ; /* Direction */
    regvalue1 = READ32(addr+0x0005120) ; /* Input Enable */
    regvalue2 = READ32(addr+0x0005128) ; /* Pull Enable */
    regvalue3 = READ32(addr+0x000512C) ; /* Pull Select */
    for ( i = 31 ; i >= 0 ; i-- ) {
        index = 0 ;
        if (regvalue4 & (1<<i)) {
            index = index | (1 << 3) ;
        }
        if (regvalue0 & (1<<i)) {
            index = index | (1 << 2) ;
        }
        if (regvalue1 & (1<<i)) {
            index = index | (1 << 1) ;
        }
        if (regvalue3 & (1<<i)) {
            index = index | (1 << 0) ;
        }

        if (regvalue2 & (1<<i)) {
            INFO("[IOPULL] IOC%02d %s DIR[%d],IE[%d]\n", i , GPIOStatePull[index], ((regvalue0 & (1<<i)) >> i ) , ((regvalue1 & (1<<i)) >> i ));
        } else {
            INFO("[IOPULL] IOC%02d %s DIR[%d],IE[%d]\n", i , GPIOStateNoPull[index], ((regvalue0 & (1<<i)) >> i ) , ((regvalue1 & (1<<i)) >> i ));
        }
    }

/*
IOD
*/
    regvalue4 = READ32(addr+0x000A008) ; /* Select the IO port to GPIO function or
                                            other function */
    regvalue0 = READ32(addr+0x000A02C) ; /* Direction */
    regvalue1 = READ32(addr+0x0005130) ; /* Input Enable */
    regvalue2 = READ32(addr+0x0005138) ; /* Pull Enable */
    regvalue3 = READ32(addr+0x000513C) ; /* Pull Select */
    for ( i = 31 ; i >= 0 ; i-- ) {
        index = 0 ;
        if (regvalue4 & (1<<i)) {
            index = index | (1 << 3) ;
        }
        if (regvalue0 & (1<<i)) {
            index = index | (1 << 2) ;
        }
        if (regvalue1 & (1<<i)) {
            index = index | (1 << 1) ;
        }
        if (regvalue3 & (1<<i)) {
            index = index | (1 << 0) ;
        }

        if (regvalue2 & (1<<i)) {
            INFO("[IOPULL] IOD%02d %s DIR[%d],IE[%d]\n", i , GPIOStatePull[index], ((regvalue0 & (1<<i)) >> i ) , ((regvalue1 & (1<<i)) >> i ));
        } else {
            INFO("[IOPULL] IOD%02d %s DIR[%d],IE[%d]\n", i , GPIOStateNoPull[index], ((regvalue0 & (1<<i)) >> i ) , ((regvalue1 & (1<<i)) >> i ));
        }
    }

    return 0;
}

int dumpIODriving(unsigned int addr, int arg) 
{

/*
[bit3] [bit2] [bit1] [bit0] 
[GPIO/Function 1 = GPIO][Input Enable 1=Enable ] [Driving Strength 0=4mA] 
0 , 0 , 0 , 0  = 0 
0 , 0 , 0 , 1  = 1 
0 , 0 , 1 , 0  = CHECK-0  
0 , 0 , 1 , 1  = CHECK-1 
0 , 1 , 0 , 0  = CHECK-2 
0 , 1 , 0 , 1  = CHECK-3 
0 , 1 , 1 , 0  = 6  
0 , 1 , 1 , 1  = 7 
0 , 0 , 0 , 0  = 8 
0 , 0 , 0 , 1  = 9 
0 , 0 , 1 , 0  = ERROR-0  
0 , 0 , 1 , 1  = ERROR-1 
0 , 1 , 0 , 0  = ERROR-2 
0 , 1 , 0 , 1  = ERROR-3 
0 , 1 , 1 , 0  = 14  
0 , 1 , 1 , 1  = 15 
*/

char *GPIOStateDriving[] = { 
    "[FUNC][OUT ][4mA]" ,
    "[FUNC][OUT ][8mA]" ,
    "[FUNC][CHK0][4mA]" ,
    "[FUNC][CHK1][8mA]" ,
    "[FUNC][CHK2][4mA]" ,
    "[FUNC][CHK3][8mA]" ,
    "[FUNC][IN  ][4mA]" ,
    "[FUNC][IN  ][8mA]" ,
    "[GPIO][OUT ][4mA]" ,
    "[GPIO][OUT ][8mA]" ,
    "[GPIO][ERR0][4mA]" ,
    "[GPIO][ERR1][8mA]" ,
    "[GPIO][ERR2][4mA]" ,
    "[GPIO][ERR3][8mA]" ,
    "[GPIO][IN  ][4mA]" ,
    "[GPIO][IN  ][8mA]" ,
};
    unsigned int regvalue0 = 0 ;
    unsigned int regvalue1 = 0 ;
    unsigned int regvalue2 = 0 ;
    unsigned int regvalue3 = 0 ;
    int i = 0 ;
    int index = 0 ;
    INFO("[--------------------------------------------------------------------------]\n");

/*
IOA
*/
    regvalue3 = READ32(addr+0x000A000) ; /* Select the IO port to GPIO function or
                                            other function */
    regvalue0 = READ32(addr+0x000A020) ; /* Direction */
    regvalue1 = READ32(addr+0x0005100) ; /* Input Enable */
    regvalue2 = READ32(addr+0x0005104) ; /* Driving Strength */

    for ( i = 31 ; i >= 0 ; i-- ) {
        index = 0 ;
        if (regvalue3 & (1<<i)) {
            index = index | (1 << 3) ;
        }
        if (regvalue0 & (1<<i)) {
            index = index | (1 << 2) ;
        }
        if (regvalue1 & (1<<i)) {
            index = index | (1 << 1) ;
        }
        if (regvalue2 & (1<<i)) {
            index = index | (1 << 0) ;
        }

        INFO("[IODRV] IOA%02d %s DIR[%d],IE[%d]\n", i , GPIOStateDriving[index] , ((regvalue0 & (1<<i)) >> i ) , ((regvalue1 & (1<<i)) >> i ));

        /* 
        if ( (( regvalue0 & (1<<i) ) == 0) && 
             (( regvalue1 & (1<<i) ) == 0) ) {
            if (regvalue2 & (1<<i)) {
                INFO("[IODRV] IOA%02d 8mA\n", i);
            }
        } 
        */ 
    }

/*
IOB
*/
    regvalue3 = READ32(addr+0x000A004) ; /* Select the IO port to GPIO function or
                                            other function */
    regvalue0 = READ32(addr+0x000A024) ; /* Direction */
    regvalue1 = READ32(addr+0x0005110) ; /* Input Enable */
    regvalue2 = READ32(addr+0x0005114) ; /* Driving Strength */

    for ( i = 31 ; i >= 0 ; i-- ) {
        index = 0 ;
        if (regvalue3 & (1<<i)) {
            index = index | (1 << 3) ;
        }
        if (regvalue0 & (1<<i)) {
            index = index | (1 << 2) ;
        }
        if (regvalue1 & (1<<i)) {
            index = index | (1 << 1) ;
        }
        if (regvalue2 & (1<<i)) {
            index = index | (1 << 0) ;
        }

        INFO("[IODRV] IOB%02d %s DIR[%d],IE[%d]\n", i , GPIOStateDriving[index] , ((regvalue0 & (1<<i)) >> i ) , ((regvalue1 & (1<<i)) >> i ));

        /* 
        if ( (( regvalue0 & (1<<i) ) == 0) && 
             (( regvalue1 & (1<<i) ) == 0) ) {
            if (regvalue2 & (1<<i)) {
                INFO("[IODRV] IOB%02d 8mA\n", i);
            }
        } 
        */ 
    }

/*
IOC
*/
    regvalue3 = READ32(addr+0x000A008) ; /* Select the IO port to GPIO function or
                                            other function */
    regvalue0 = READ32(addr+0x000A028) ; /* Direction */
    regvalue1 = READ32(addr+0x0005120) ; /* Input Enable */
    regvalue2 = READ32(addr+0x0005124) ; /* Driving Strength */

    for ( i = 31 ; i >= 0 ; i-- ) {
        index = 0 ;
        if (regvalue3 & (1<<i)) {
            index = index | (1 << 3) ;
        }
        if (regvalue0 & (1<<i)) {
            index = index | (1 << 2) ;
        }
        if (regvalue1 & (1<<i)) {
            index = index | (1 << 1) ;
        }
        if (regvalue2 & (1<<i)) {
            index = index | (1 << 0) ;
        }

        INFO("[IODRV] IOC%02d %s DIR[%d],IE[%d]\n", i , GPIOStateDriving[index] , ((regvalue0 & (1<<i)) >> i ) , ((regvalue1 & (1<<i)) >> i ));

        /* 

        if ( (( regvalue0 & (1<<i) ) == 0) && 
             (( regvalue1 & (1<<i) ) == 0) ) {
            if (regvalue2 & (1<<i)) {
                INFO("[IODRV] IOC%02d 8mA\n", i);
            }
        } 
        */ 
    }

/*
IOD
*/
    regvalue3 = READ32(addr+0x000A00C) ; /* Select the IO port to GPIO function or
                                            other function */
    regvalue0 = READ32(addr+0x000A02C) ; /* Direction */
    regvalue1 = READ32(addr+0x0005130) ; /* Input Enable */
    regvalue2 = READ32(addr+0x0005134) ; /* Driving Strength */

    for ( i = 31 ; i >= 0 ; i-- ) {
        index = 0 ;
        if (regvalue3 & (1<<i)) {
            index = index | (1 << 3) ;
        }
        if (regvalue0 & (1<<i)) {
            index = index | (1 << 2) ;
        }
        if (regvalue1 & (1<<i)) {
            index = index | (1 << 1) ;
        }
        if (regvalue2 & (1<<i)) {
            index = index | (1 << 0) ;
        }

        INFO("[IODRV] IOD%02d %s DIR[%d],IE[%d]\n", i , GPIOStateDriving[index] , ((regvalue0 & (1<<i)) >> i ) , ((regvalue1 & (1<<i)) >> i ));

        /* 
        if ( (( regvalue0 & (1<<i) ) == 0) && 
             (( regvalue1 & (1<<i) ) == 0) ) {
            if (regvalue2 & (1<<i)) {
                INFO("[IODRV] IOD%02d 8mA\n", i);
            }
        } 
        */ 
    }

    return 0;
}

int dumpAudioDAC(unsigned int addr, int arg) 
{
/*
P_AUD_PWCTRL(0x9301F020)[1]
ENVREF
VREF power control, high active
0: disable
1: enable
P_DAC_ DACCTRL(0x9301F02c)[0]
ENDAL
Audio left channel DAC power control.
0: Disable
1: Enable
P_DAC_ DACCTRL(0x9301F02c)[1]
ENDAR
Audio right channel DAC power control.
0: Disable
1: Enable
P_DAC_ HDPHN(0x9301F038)[0]
ENHPL
Left channel headphone amplifier power control.
0: Disable
1: Enable
P_DAC_ HDPHN(0x9301F038)[1]
ENHPR
Right channel headphone amplifier power control.
0: Disable
1: Enable	
*/
	unsigned int regvalue0 = 0 ;

    regvalue0 = READ32(addr+0x301F020) ;

    if ( regvalue0 & ( 1<<1) ) {
        INFO("[AudioDAC] VREF Enable\n");
    } else {
        if ( arg != 1 ) {
            INFO("[AudioDAC] VREF Disable\n");
        }
    }

    regvalue0 = READ32(addr+0x301F02C) ;

    if ( regvalue0 & ( 1<<0) ) {
        INFO("[AudioDAC] Left Channel Enable\n");
    } else {
        if ( arg != 1 ) {
            INFO("[AudioDAC] Left Channel Disable\n");
        }
    }
    
    if ( regvalue0 & ( 1<<0) ) {
        INFO("[AudioDAC] Right Channel Enable\n");
    } else {
        if ( arg != 1 ) {
            INFO("[AudioDAC] Right Channel Disable\n");
        }
    }
    
    regvalue0 = READ32(addr+0x301F038) ;

    if ( regvalue0 & ( 1<<0) ) {
        INFO("[AudioDAC] Left Channel Headphone Enable\n");
    } else {
        if ( arg != 1 ) {
            INFO("[AudioDAC] Left Channel Headphone Disable\n");
        }
    }
    
    if ( regvalue0 & ( 1<<0) ) {
        INFO("[AudioDAC] Right Channel Headphone Enable\n");
    } else {
        if ( arg != 1 ) {
            INFO("[AudioDAC] Right Channel Headphone Disable\n");
        }
    }
	return 0;	
}


/**
*@brief Show usage string
*/
static int 
show_usage(
	void
)
{
	INFO(usage_str);
	return 0;
}


int main(int argc, char *argv[]) 
{

	DEBUG("[%s] \n", __FUNCTION__);

    unsigned int temp = 0;
    int ret = 0;
    int fd = 0;
    void *map_base, *virt_addr;
    unsigned int page_size, mapped_size, offset_in_page;

    int select = 0;

    if (argc<2) {
		show_usage();
		return ret;
	}

    /*Calculate maping base & size*/
    mapped_size = page_size = getpagesize();	
    offset_in_page = (unsigned int)LOGI_ADDR_REG & (page_size - 1);
    temp = (offset_in_page + LOGI_ADDR_REG_RANGE)/page_size;
    mapped_size = mapped_size*(temp+1);

    fd = open("/dev/mem", (O_RDONLY | O_SYNC));
    if(fd < 0){
        INFO("Cannot open /dev/mem\n");
        return 1;
    }

    map_base = mmap(NULL,
            mapped_size,
            PROT_READ,
            MAP_SHARED,
            fd,
            LOGI_ADDR_REG & ~(off_t)(page_size - 1));

    if (map_base == MAP_FAILED){
        INFO("Mapping fail!\n");
        ret = 1;
        goto FAIL_MAP;
    }

    DEBUG("Memory mapped at address %p, mapped size = %d .\n", map_base, mapped_size);

    virt_addr = (char*)map_base + offset_in_page;

    if (strcmp(argv[1], "all") == 0) {
        select = 0 ;
    }

    if (strcmp(argv[1], "pm") == 0) {
        select = 1 ;
    }

    if ( (strcmp(argv[1], "all") == 0) || (strcmp(argv[1], "pm") == 0) ){
        DEBUG("[%s] dump all\n", __FUNCTION__);
        dumpPLL((unsigned int)virt_addr,select);
        dumpMainClock((unsigned int)virt_addr,select);
        dumpPeripheralClock((unsigned int)virt_addr,select);
        dumpUSB((unsigned int)virt_addr,select);
        dumpTVDAC((unsigned int)virt_addr,select);
        dumpTFTDisplay((unsigned int)virt_addr,select);
        dumpI2S((unsigned int)virt_addr,select);
        dumpCIS((unsigned int)virt_addr,select);
        dumpSPU((unsigned int)virt_addr,select);
        dumpPPU((unsigned int)virt_addr,select);
        dumpAPBDMA((unsigned int)virt_addr,select);
        dumpNAND((unsigned int)virt_addr,select);
        dumpSAADC((unsigned int)virt_addr,select);
        dumpOVG((unsigned int)virt_addr,select);
        dumpAES((unsigned int)virt_addr,select);
        dumpCEVAL2((unsigned int)virt_addr,select);
        dumpVIC((unsigned int)virt_addr,select);
        dumpARMTimer((unsigned int)virt_addr,select);
        dumpWDT((unsigned int)virt_addr,select);
        dumpRAND((unsigned int)virt_addr,select);
        dumpGPIO((unsigned int)virt_addr,select);
        dumpRTC((unsigned int)virt_addr,select);
        dumpSDMA((unsigned int)virt_addr,select);
        dumpXDMA((unsigned int)virt_addr,select);
        dumpDRAM((unsigned int)virt_addr,select);
        dumpMS((unsigned int)virt_addr,select);
        dumpUART((unsigned int)virt_addr,select);
        dumpSPI((unsigned int)virt_addr,select);
        dumpSDC((unsigned int)virt_addr,select);
        dumpI2C((unsigned int)virt_addr,select);
        dumpSCALER((unsigned int)virt_addr,select);
        dumpDC2DC((unsigned int)virt_addr,select);
        dumpIODriving((unsigned int)virt_addr,select);
        dumpIOPull((unsigned int)virt_addr,select);
        dumpAudioDAC((unsigned int)virt_addr,select);
        dumpMIPI((unsigned int)virt_addr,select);
        dumpMIC((unsigned int)virt_addr,select);
        dumpFM((unsigned int)virt_addr,select);
    }










    









    munmap(map_base, mapped_size);

FAIL_MAP:
    close(fd);
    return ret;
}




