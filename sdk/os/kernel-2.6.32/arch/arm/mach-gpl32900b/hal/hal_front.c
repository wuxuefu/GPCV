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
#include <mach/kernel.h>
#include <mach/module.h>
#include <mach/hal/hal_front.h>
#include <mach/hal/hal_gpio.h>
#include <mach/hal/sysregs.h>
#include <mach/hal/regmap/reg_front.h>
#include <mach/hal/regmap/reg_mipi.h>


/**************************************************************************
 *                           C O N S T A N T S                            *
 **************************************************************************/


/**************************************************************************
 *                              M A C R O S                               *
 **************************************************************************/
#define FRONT_CLKEN		(1<<10)
#define FRONT_CNT_EN	(1<<8)

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/


/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/


/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/


/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static frontReg_t *pFrontReg = (frontReg_t *)(LOGI_ADDR_FRONT_REG);

/**************************************************************************
 *             F U N C T I O N    I M P L E M E N T A T I O N S           *
 **************************************************************************/

/**************************************************************************************
Function Name	:	gpHalFrontReset
Purposes		:	To reset Front
Desprictions	:
Arguments		:	None
Returns			:	None
See also		:
***************************************************************************************/
void gpHalFrontReset(void)
{
	pFrontReg->frontReset = 0x01;
	pFrontReg->frontReset = 0x00;
}


/**************************************************************************************
Function Name	:	gpHalFrontStart
Purposes		:	To start Front
Desprictions	:
Arguments		:	None
Returns			:	None
See also			:
***************************************************************************************/
void gpHalFrontStart(void)
{
	pFrontReg->frontValidCtrl |= 0x04;
	pFrontReg->frontVdUpdate = 1;
	pFrontReg->frontTgSignalGating = 0x00;
	pFrontReg->frontClkGatingDisable = 0x0f;
	pFrontReg->front2xckGatingDisable = 0x01;
}


/**************************************************************************************
Function Name	:	gpHalFrontStop
Purposes		:	To stop Front
Desprictions	:
Arguments		:	None
Returns			:	None
See also		:
***************************************************************************************/
void gpHalFrontStop(void)
{
	pFrontReg->front2xckGatingDisable = 0x00;
	pFrontReg->frontClkGatingDisable = 0x00;
	pFrontReg->frontTgSignalGating = 0x1f;
	pFrontReg->frontVdUpdate = 0;
}


/**************************************************************************************
Function Name	:	gpHalFrontInputPathSet
Purposes		:	Select input path
Desprictions	:
Arguments		:	mode : 0: Normal , 1: mipi
Returns			:	None
See also		:
***************************************************************************************/
void gpHalFrontInputPathSet(UINT8 path)
{
	if (path)
		pFrontReg->frontMipiEnable |= 0x10;
	else
		pFrontReg->frontMipiEnable &= 0xEF;
}


/**************************************************************************************
Function Name	:	gpHalFrontInputModeSet
Purposes		:	Select input mode
Desprictions	:
Arguments		:	mode : 0: RAW , 1: YUV
Returns			:	None
See also		:
***************************************************************************************/
void gpHalFrontInputModeSet(UINT8 mode)
{
	if (mode) {
		pFrontReg->frontYuvMode |= 0x10;
		pFrontReg->frontYuvCtrl |= (3 << 5); //Tvreshhen, Tvreshven
	} else {
		pFrontReg->frontYuvMode &= 0xEF;
		pFrontReg->frontYuvCtrl &= ~(3 << 5); //Tvreshhen, Tvreshven
	}
}


/**************************************************************************************
Function Name	:	gpHalFrontYuvSubEn
Purposes		:	Y/U/V sub 128
Desprictions	:
Arguments		:	
					ySub : 0: disable, 1:enable
					uSub : 0: disable, 1:enable
					vSub : 0: disable, 1:enable
Returns			:	None
See also		:
***************************************************************************************/
void gpHalFrontYuvSubEn(UINT32 ySub, UINT32 uSub, UINT32 vSub)
{
	pFrontReg->frontYuvMode &= 0xF8;
	pFrontReg->frontYuvMode |= (UINT8)(((vSub<<2)|(uSub<<1)|ySub)&0x07);
}


/**************************************************************************************
Function Name	:	gpHalFrontDataTransMode
Purposes		:	Select data transfer mode
Desprictions	:
Arguments		:	mode : 0: CCIR601 , 1: CCIR656
Returns			:	None
See also		:
***************************************************************************************/
void gpHalFrontDataTransMode(UINT8 type)
{
	if (type)
		pFrontReg->frontYuvCtrl |= 0x10;
	else
		pFrontReg->frontYuvCtrl &= 0xEF;
}

UINT8 gpHalFrontDataTransModeGet(void)
{
	return (pFrontReg->frontYuvCtrl >> 4) & 0x01;
}


/**************************************************************************************
Function Name	:	gpHalFrontYuvOrderSet
Purposes		:	Select YUV data in sequence
Desprictions	:
Arguments		:	mode : 0: UYVY , 1: YVYU , 2: VYUY , 3: YUYV
Returns			:	None
See also		:
***************************************************************************************/
void gpHalFrontYuvOrderSet(UINT32 mode)
{
	pFrontReg->frontYuvCtrl &= 0xFC;
	pFrontReg->frontYuvCtrl |= ((UINT8)(mode & 0x03));
}


/**************************************************************************************
Function Name	:	gpHalFrontSyncPolaritySet
Purposes		:	Set sync signal polarity for internal use
Desprictions	:
Arguments		:	hPol : 0: normal , 1: inverse
					vPol : 0: normal , 1: inverse
Returns			:	None
See also			:
***************************************************************************************/
void gpHalFrontSyncPolaritySet(UINT8 hPol, UINT8 vPol, UINT8 in1den)
{
	if(hPol) {
		hPol = 1;
	}
	if(vPol) {
		vPol = 1;
	}
	pFrontReg->frontValidCtrl = (vPol << 5) | (hPol << 4);
	
	if(in1den) {
		in1den = 1;
	}
	pFrontReg->frontSyncPolarity = (in1den << 4);
}

void gpHalFrontSyncPolarityGet(UINT8 *hPol, UINT8 *vPol, UINT8 *in1den)
{
	*hPol = ((pFrontReg->frontSyncPolarity >> 1) & 0x01) ? 0 : 1;
	*vPol = (pFrontReg->frontSyncPolarity & 0x01) ? 0 : 1;
	*in1den = (pFrontReg->frontSyncPolarity >> 4) & 0x01;
}

/**************************************************************************************
Function Name	:	gpHalFrontInputGate
Purposes		:	Gate TG input signal to zero
Desprictions	:
Arguments		:	tggz : 0: normal , 1: gating to zero
						bit[0]: Exthdi
						bit[1]: Extvdi
						bit[2]: Tvhvalidi
						bit[3]: Tvvvalidi
						bit[4]: Tvdvalidi
Returns			:	None
See also		:
***************************************************************************************/
void gpHalFrontInputGate(UINT32 tggz)
{
	pFrontReg->frontTgSignalGating = (UINT8)tggz;
	pFrontReg->rsvA5d[0] = (UINT8)((tggz >> 8) & 0x01);
}


/**************************************************************************************
Function Name	:	gpHalFrontIntCfg
Purposes		:	Enable Front interrupt control
Desprictions	:
Arguments		:	tggz : 0: disable , 1: enable
						bit[0]: VD Rising (equal num) interrupt enable
						bit[1]: VD Falling (equal num) interrupt enable
						bit[2]: Mipi input Done interrupt enable
						bit[3]: Snap Done interrupt enable
						bit[4]: Do Cdsp Done interrupt enable
						bit[5]: Hd count eq Inthnum interrupt enable
						bit[6]: frontvvalid rising interrupt enable
						bit[7]: frontvvalid falling interrupt enable
						bit[8]: VD Rising (every) interrupt enable
						bit[9]: VD Falling (every) interrupt enable
					vdrintnum : Define How many VD's rising occurs VD Rising interrupt
					vdfintnum : Define How many VD's falling occurs VD Falling interrupt
					hdfintnum : Define How many HD's falling occurs HD Falling interrupt
Returns			:	None
See also		:
***************************************************************************************/
void gpHalFrontIntCfg(UINT32 interrupten, UINT32 vdrintnum, UINT32 vdfintnum, UINT32 hdfintnum)
{
	if (vdfintnum > 15) {
		vdfintnum = 15;
	}
	if(vdfintnum == 0) {
		vdfintnum =1;
	}
	if (vdrintnum > 15) {
		vdrintnum = 15;
	}
	if (vdrintnum == 0) {
		vdrintnum = 1;
	}

	pFrontReg->frontVdIntNumSet = (UINT8)((vdfintnum << 4) |vdrintnum);
	if(hdfintnum > 4095) {
		hdfintnum = 4095;
	}
	if(hdfintnum == 0) {
		hdfintnum = 1;
	}

	pFrontReg->frontHdIntNumSetL = (UINT8)(hdfintnum & 0xff);
	pFrontReg->frontHdIntNumSetH = (UINT8)(hdfintnum>>8);
	pFrontReg->frontIntEnable = (UINT8)(interrupten & 0xff);
	pFrontReg->frontVdIntEnable = (UINT8)((interrupten>>8) & 0x03);
}


/**************************************************************************************
Function Name	:	gpHalFrontIntEventClr
Purposes		:	Clear front interrupt event 
Desprictions	:
Arguments		:	clrintevt : 0: keep value , 1: clear value
						bit[0]: VD Rising (equal num) event
						bit[1]: VD Falling (equal num) event
						bit[2]: Mipi input Done event
						bit[3]: Snap Done event
						bit[4]: Do Cdsp Done event
						bit[5]: Hd count eq Inthnum event
						bit[6]: frontvvalid rising event
						bit[7]: frontvvalid falling event
						bit[8]: VD Rising (every) event
						bit[9]: VD Falling (every) event
Returns			:	None
See also		:
***************************************************************************************/
void gpHalFrontIntEventClr(UINT32 clrintevt)
{
	pFrontReg->frontIntEvent= (UINT8)(clrintevt & 0xff);
	pFrontReg->frontVdIntEvent = (UINT8)((clrintevt >> 8) & 0x03);
}


/**************************************************************************************
Function Name	:	gpHalFrontIntEventGet
Purposes		:	Read front interrupt event 
Desprictions	:
Arguments		:	pintevtvalue : out
						bit[0]: VD Rising (equal num) event
						bit[1]: VD Falling (equal num) event
						bit[2]: Mipi input Done event
						bit[3]: Snap Done event
						bit[4]: Do Cdsp Done event
						bit[5]: Hd count eq Inthnum event
						bit[6]: frontvvalid rising event
						bit[7]: frontvvalid falling event
						bit[8]: VD Rising (every) event
						bit[9]: VD Falling (every) event
Returns			:	None
See also		:
***************************************************************************************/
void gpHalFrontIntEventGet(UINT32 *pintevtvalue)
{
	*pintevtvalue = (((UINT32)pFrontReg->frontVdIntEvent << 8) | (UINT32)pFrontReg->frontIntEvent);
}


/**************************************************************************************
Function Name	:	gpHalFrontSnapTrigger
Purposes		:	Trigger front to snap image 
Desprictions	:
Arguments		:	number : Number of images to snap (1~16) 
Returns			:	None
See also		:
***************************************************************************************/
void gpHalFrontSnapTrigger(UINT32 number)
{
	if (number == 0)
		number = 1;
	if (number >= 16)
		number = 0;
		
	pFrontReg->frontSnapCtrl = (UINT8)(0x10 | number);
}


/**************************************************************************************
Function Name	:	gpHalFrontFlashSet
Purposes		:	Control flah light
Desprictions	:
Arguments		:	width : pulse width, (unit 2 pixel clock, 16 bits) 
					linenum : the number of lines after vd falling edge to otput pulse
					mode :
						bit[0]: 0: Immediate mode , 1: Synchronize-to-linenum mode
						bit[3]:polarity
Returns			:	None
See also		:
***************************************************************************************/
void gpHalFrontFlashSet(UINT32 width, UINT32 linenum, UINT32 mode)
{
	pFrontReg->frontFlashWidthL = (UINT8)width;
	pFrontReg->frontFlashWidthH = (UINT8)(width >> 8);
	if (linenum > 0x1fff)
		linenum = 0x1ff;
	pFrontReg->frontFlashDelayL = (UINT8)linenum;
	pFrontReg->frontFlashDelayH = (UINT8)(linenum >> 8);
	pFrontReg->frontFlashCtrl = ((UINT8)mode & 0x09);
} 


/**************************************************************************************
Function Name	:	gpHalFrontFlashTrigger
Purposes		:	Trigger flah light
Desprictions	:
Arguments		:	None
Returns			:	None
See also		:
***************************************************************************************/
void gpHalFrontFlashTrigger(void)
{
	pFrontReg->frontFlashCtrl |= 0x10;
} 


/**************************************************************************************
Function Name	:	gpHalFrontRoiSet
Purposes		:	Adjust offset/size of image from sensor 
Desprictions	:
Arguments		:	roi : region of image 
						roi.hSize: horizontal size
						roi.hOffset: horizontal offset
						roi.vSize: vertical size
						roi.vOffset: verticaloffset
					syncVdEn: 0: immediate change value ,  1: synchronizeto the next Vsync
Returns			:	None
See also		:
***************************************************************************************/
void gpHalFrontRoiSet(gpHalFrontRoi_t roi, UINT8 syncVdEn)
{
	if (syncVdEn) {
		pFrontReg->frontHVSyncVd = 0x03;
	}
	else {
		pFrontReg->frontHVSyncVd = 0x00;
	}

	//front size set
	pFrontReg->frontHSizeL = (UINT8)(roi.hSize & 0xff);
	pFrontReg->frontHSizeH = (UINT8)(roi.hSize >> 8);
	pFrontReg->frontHOffsetL = (UINT8)(roi.hOffset & 0xff);
	pFrontReg->frontHOffsetH = (UINT8)(roi.hOffset >> 8);

	pFrontReg->frontVSizeL = (UINT8)(roi.vSize & 0xff);
	pFrontReg->frontVSizeH = (UINT8)(roi.vSize >> 8);
	pFrontReg->frontVOffsetL = (UINT8)(roi.vOffset & 0xff);
	pFrontReg->frontVOffsetH = (UINT8)(roi.vOffset >> 8);

	//mipi size set
	pFrontReg->frontMipiHOffsetL = (UINT8)(roi.hOffset & 0xff);
	pFrontReg->frontMipiHOffsetH = (UINT8)(roi.hOffset >> 8);
	pFrontReg->frontMipiVOffsetL = (UINT8)(roi.vOffset & 0xff);
	pFrontReg->frontMipiVOffsetH = (UINT8)(roi.vOffset >> 8);

	pFrontReg->frontMipiHSizeL = (UINT8)(roi.hSize & 0xff);
	pFrontReg->frontMipiHSizeH = (UINT8)(roi.hSize >> 8);	
	pFrontReg->frontMipiVSizeL = (UINT8)(roi.vSize & 0xff);
	pFrontReg->frontMipiVSizeH = (UINT8)(roi.vSize >> 8);
} 


/**************************************************************************************
Function Name	:	gpHalFrontRoiGet
Purposes		:	Get offset/size of image from sensor 
Desprictions	:
Arguments		:	roi : region of image 
						roi->hSize: horizontal size
						roi->hOffset: horizontal offset
						roi->vSize: vertical size
						roi->vOffset: verticaloffset
Returns			:	None
See also		:
***************************************************************************************/
void gpHalFrontRoiGet(gpHalFrontRoi_t* roi)
{
	roi->hSize = ((((UINT16)pFrontReg->frontHSizeH) << 8) | (UINT16)pFrontReg->frontHSizeL);
	roi->hOffset = ((((UINT16)pFrontReg->frontHOffsetH) << 8) | (UINT16)pFrontReg->frontHOffsetL);
	roi->vSize = ((((UINT16)pFrontReg->frontVSizeH) << 8) | (UINT16)pFrontReg->frontVSizeL);
	roi->vOffset = ((((UINT16)pFrontReg->frontVOffsetH) << 8) | (UINT16)pFrontReg->frontVOffsetL);
} 


/**************************************************************************************
Function Name	:	gpHalFrontVdUpdateEn
Purposes		:	Enable vd update function 
Desprictions	:
Arguments		:	enable : 0: disable , 1: enable
Returns			:	None
See also		:
***************************************************************************************/
void gpHalFrontVdUpdateEn(UINT32 enable)
{
	pFrontReg->frontVdUpdate = (UINT8)enable;
} 


/**************************************************************************************
Function Name	:	gpHalFrontReshapeSet
Purposes		:	Reshape internal sync signal 
Desprictions	:
Arguments		:	reshapeCtl :
						reshapeCtl.mode: 0: RAW , 1:YUV
						reshapeCtl.hReshapeEn: 0: hd reshape disable , 1: hd reshape enable
						reshapeCtl.vReshapeEn: 0: vd reshape disable , 1: vd reshape enable
						reshapeCtl.vReshapeClkType:	0: H-sync ; 1: pclk	
						reshapeCtl.vBackOffEn
						reshapeCtl.hRise
						reshapeCtl.hFall
						reshapeCtl.vRise
						reshapeCtl.vFall
Returns			:	None
See also		:
***************************************************************************************/
void gpHalFrontReshapeSet(gpHalFrontReshape_t reshapeCtl)
{
	pFrontReg->frontYuvCtrl &= 0x9F;
	pFrontReg->frontReshapeEnable = 0;

	if(reshapeCtl.hReshapeEn){
		pFrontReg->frontReshapeHRiseL = ((UINT8)reshapeCtl.hRise & 0xff);
		pFrontReg->frontReshapeHRiseH = (UINT8)(reshapeCtl.hRise >> 8);
		pFrontReg->frontReshapeHFallL = ((UINT8)reshapeCtl.hFall & 0xff);
		pFrontReg->frontReshapeHFallH = (UINT8)(reshapeCtl.hFall >> 8);
	}
	
	if(reshapeCtl.vReshapeEn){
		pFrontReg->frontReshapeVRiseL = ((UINT8)reshapeCtl.vRise & 0xff);
		pFrontReg->frontReshapeVRiseH = (UINT8)(reshapeCtl.vRise >> 8);
		pFrontReg->frontReshapeVFallL = ((UINT8)reshapeCtl.vFall & 0xff);
		pFrontReg->frontReshapeVFallH = (UINT8)(reshapeCtl.vFall >> 8);
	}
	
	if (reshapeCtl.mode == 0){ //RAW
		pFrontReg->frontReshapeEnable = ((reshapeCtl.vBackOffEn << 5) | (reshapeCtl.vReshapeClkType << 4) | 
										(reshapeCtl.vReshapeEn << 1) | (reshapeCtl.hReshapeEn));
	} else{ //YUV	
		pFrontReg->frontReshapeEnable = ((reshapeCtl.vBackOffEn << 5) | (reshapeCtl.vReshapeClkType << 4));
		pFrontReg->frontYuvCtrl |= ((reshapeCtl.vReshapeEn << 6) | (reshapeCtl.hReshapeEn << 5));
	}
} 


/**************************************************************************************
Function Name	:	gpHalFrontSiggenSet
Purposes		:	Config signal generation 
Desprictions	:
Arguments		:	enable : 0: disable , 1: enable
					type : 0: RGB , 1: YUV
					mode :
						0: Still, White
						1: Still, Yellow
						2: Still, Cyan
						3: Still, Green
						4: Still, Magenta
						5: Still, Red
						6: Still, Blue
						7: Still, Black
						8: Gray level
						9: hcnt
						10: vcnt
						11: hcnt-17 (0,1,2,3¡K¡K.)
						12: Still vertical color bar
						13: Still horizontal color bar
						14: R, G, B, and W flashing
						15: Moving horizontal color bar
Returns			:	None
See also		:
***************************************************************************************/
void gpHalFrontSiggenSet(UINT32 enable, UINT32 type, UINT32 mode)
{
	pFrontReg->frontSignalGen = (UINT8)(((enable & 0x01) << 7)|((type & 0x01)   << 6)|(mode & 0x1F));
} 


void gpHalFrontVdWait(UINT32 mode, UINT32 number)
{
	UINT32 i;
	UINT32 timeOutCnt = 0x000FFFFF;
	UINT8 tmp0;

	if(mode) {
		for(i=0;i<number;i++) {
			timeOutCnt = 0x000FFFFF;
			do {
				tmp0 = pFrontReg->frontSyncSignalState;
				tmp0 = tmp0 & 0x02; 
				timeOutCnt--;
			} while(tmp0 == 0x02 && timeOutCnt > 0);

			timeOutCnt = 0x000FFFFF;
			do {
				tmp0 = pFrontReg->frontSyncSignalState;
				tmp0 = tmp0 & 0x02; 
				timeOutCnt--;
			} while(tmp0 == 0x00 && timeOutCnt > 0);
		}
	}
	else {
		for(i=0;i<number;i++) {
			timeOutCnt = 0x000FFFFF;
			do {
				tmp0 = pFrontReg->frontSyncSignalState;
				tmp0 = tmp0 & 0x02; 
				timeOutCnt--;
			} while(tmp0 == 0x00 && timeOutCnt > 0);

			timeOutCnt = 0x000FFFFF;
			do {
				tmp0 = pFrontReg->frontSyncSignalState;
				tmp0 = tmp0 & 0x02; 
				timeOutCnt--;
			} while(tmp0 == 0x02 && timeOutCnt > 0);
		}
	} 
} 


/**************************************************************************************
*                                                                                     *
*  Function Name : hwFrontProbeSet                                                    *
*                                                                                     *
*  Purposes      :                                                                    *
*    To set probe signal of front                                                     *
*                                                                                     *
*  Desprictions  :                                                                    *
*    To set probe siganl of front                                                     *
*                                                                                     *
*  Arguments     :                                                                    *
*    enable : in, 0 -> Disable                                                        *
*                 1 -> Enable                                                         *
*    mode   : in, 0 -> {sifbusy, prefetch, cdssclk, cdsdo,                            *
*                       i2cdo, i2cclk, i2cwrdata1, i2cwrdata0}                        *
*                 1 -> sifcpudata                                                     *
*                 2 : {frontrgb[9], frontrgb[7], tvvvalid, tvhvalid, frontvdvalid, fronthdvalid, frontvd, fronthd};
*                 3 : {clk2x, clk, frontrgb[9], evdi, ehdi, tvhvalid601, tvvd, tvhd};
*                 4 : {fronty[7:5], clk, frontvdvalid, fronthdvalid, frontvd, fronthd};
*                 5 : {frontu[7:5], frontvd, fronthd, clk, frontvdvalid, fronthdvalid};
*                 6 : probefifo;
*                 7 : probemipi;
*                 8 : {sencpudata[6:0], cclk};
*                 9 : {frontrgb[5], frontrgb[1],  frontvdvalid, fronthdvalid, frontvd, fronthd, clk2x, clk};
*                 10: {extrgb[9:4], extvdi, exthdi};
*                 11: {frontrgb[9:6], frontvdvalid, fronthdvalid, frontvd, fronthd};
*                 12: pg0probe;
*                 13: pg1probe;
*                 14: {mipidata[23], mipidata[15], mipidata[7], mipivvalid, mipihvalid, extrgb[9], extrgb[2], gvclk3};
*                 15: {mipidata[16], mipidata[8],  mipidata[0], mipivvalid, mipihvalid, extrgb[8], extrgb[3], gvclk3};
*                                                                                     *
*  Returns       :                                                                    *
*    None                                                                             *
*                                                                                     *
*  See also      :                                                                    *
*                                                                                     *
***************************************************************************************/
void gpHalFrontProbeSet(UINT32 enable, UINT32 mode)
{
#if 0
	UINT32 tmp0;
	frontSenReg_t *pfrontSenReg;   
	GpioProbeReg_t   *pGpioProbeReg;
    
#if 1   
	if (mode ==2){
		pGpioProbeReg = (GpioProbeReg_t  *) (0x10000000);  
	
		// [02/14/2007] lanzhu marked for 2M sensor 
		//pGpioProbeReg->proben =0x01;
		pGpioProbeReg->probsel =0x09;
	
	}
 #endif

	pfrontSenReg = (frontSenReg_t *)(0x10000000);   
	if ((mode ==0) | (mode == 1)) {
		tmp0 = mode;    
	}
	else {
		tmp0 = (mode - 2) | 0x10;
	}
	pfrontSenReg->probemode = tmp0;
#endif
} 


void gpHalFrontVValidWait(UINT32 mode, UINT32 number)
{
	UINT32 i;
	UINT32 timeOutCnt = 0x000FFFFF;
	UINT8 tmp0;

	if(mode) {
		for(i=0;i<number;i++) {
			timeOutCnt = 0x000FFFFF;
			do {
				tmp0 = pFrontReg->frontSyncSignalState;
				tmp0 = tmp0 & 0x08; 
				timeOutCnt--;
			} while(tmp0 == 0x08 && timeOutCnt > 0);

			timeOutCnt = 0x000FFFFF;
			do {
				tmp0 = pFrontReg->frontSyncSignalState;
				tmp0 = tmp0 & 0x08; 
				timeOutCnt--;
			} while(tmp0 == 0x00 && timeOutCnt > 0);
		}
	}
	else {
		for(i=0;i<number;i++) {
			timeOutCnt = 0x000FFFFF;
			do {
				tmp0 = pFrontReg->frontSyncSignalState;
				tmp0 = tmp0 & 0x08; 
				timeOutCnt--;
			} while(tmp0 == 0x00 && timeOutCnt > 0);

			timeOutCnt = 0x000FFFFF;
			do {
				tmp0 = pFrontReg->frontSyncSignalState;
				tmp0 = tmp0 & 0x08; 
				timeOutCnt--;
			} while(tmp0 == 0x08 && timeOutCnt > 0);
		}
	}
} 

#if 0
/**
* @brief		cdsp sensor interface reset
* @return	none
*/
void 
gpHalCdspFrontReset(
	void
)
{
	pCdspRegFront->cdspFrontGCLK |= 0x100;
	pCdspRegFront->cdspFrontGCLK &= ~0x100;
}

/**
 * @brief	cdsp set front sensor input format
 * @param	format [in]: input format
 * @return	0: success, other: fail
*/
SINT32 
gpHalCdspFrontSetInputFormat(
	UINT32 format
)
{	
	switch(format)
	{
		case C_SDRAM_FMT_RAW8:
			pCdspReg1->cdspDataFmt |= 0x320;
			pCdspReg1->cdspDataFmt |= 1 << 4;	/* Raw8 */
			pCdspRegFront->cdspTgZero = 0x1FF;
			gpHalCdspSetMuxPath(0);
			break;
		case C_SDRAM_FMT_RAW10:
			pCdspReg1->cdspDataFmt |= 0x320;
			pCdspReg1->cdspDataFmt &= ~(1 << 4);/* Raw10 */
			pCdspRegFront->cdspTgZero = 0x1FF;
			gpHalCdspSetMuxPath(0);
			break;
		case C_SDRAM_FMT_VY1UY0:
			pCdspReg1->cdspDataFmt |= 0x320;
			pCdspReg1->cdspDataFmt |= 1 << 4;
			pCdspRegFront->cdspTgZero = 0x1FF;
			gpHalCdspSetMuxPath(1);
			break;
		case C_FRONT_FMT_RAW8:
			pCdspReg1->cdspDataFmt |= 0x320;
			pCdspReg1->cdspDataFmt |= 1 << 4;
			pCdspRegFront->cdspFrontCtrl3 &= ~0xE00;
			pCdspRegFront->cdspTgZero = 0x1FC;
			gpHalCdspSetMuxPath(0);
			break;
		case C_FRONT_FMT_RAW10:
			pCdspReg1->cdspDataFmt |= 0x320;
			pCdspReg1->cdspDataFmt &= ~(1 << 4);
			pCdspRegFront->cdspFrontCtrl3 &= ~0xE00;
			pCdspRegFront->cdspTgZero = 0x1FC;
			gpHalCdspSetMuxPath(0);
			break;
		case C_FRONT_FMT_UY1VY0:
			pCdspReg1->cdspDataFmt |= 0x320;
			pCdspReg1->cdspDataFmt |= 1 << 4;
			pCdspRegFront->cdspFrontCtrl3 &= ~0x03;
			pCdspRegFront->cdspFrontCtrl3 |= 0xE00;
			pCdspRegFront->cdspTgZero = 0x1E0;
			gpHalCdspSetMuxPath(1);
			break;
		case C_FRONT_FMT_Y1VY0U:
			pCdspReg1->cdspDataFmt |= 0x320;
			pCdspReg1->cdspDataFmt |= 1 << 4;
			pCdspRegFront->cdspFrontCtrl3 &= ~0x03;
			pCdspRegFront->cdspFrontCtrl3 |= 0xE01;
			pCdspRegFront->cdspTgZero = 0x1E0;
			gpHalCdspSetMuxPath(1);
			break;
		case C_FRONT_FMT_VY1UY0:
			pCdspReg1->cdspDataFmt |= 0x320;
			pCdspReg1->cdspDataFmt |= 1 << 4;
			pCdspRegFront->cdspFrontCtrl3 &= ~0x03;
			pCdspRegFront->cdspFrontCtrl3 |= 0xE02;
			pCdspRegFront->cdspTgZero = 0x1E0;
			gpHalCdspSetMuxPath(1);
			break;
		case C_FRONT_FMT_Y1UY0V:		
			pCdspReg1->cdspDataFmt |= 0x320;
			pCdspReg1->cdspDataFmt |= 1 << 4;
			pCdspRegFront->cdspFrontCtrl3 &= ~0x03;
			pCdspRegFront->cdspFrontCtrl3 |= 0xE03;
			pCdspRegFront->cdspTgZero = 0x1E0;
			gpHalCdspSetMuxPath(1);
			break;	
		case C_MIPI_FMT_RAW8:
			pCdspReg1->cdspDataFmt |= 0x320;
			pCdspReg1->cdspDataFmt |= 1 << 4;
			pCdspRegFront->cdspFrontCtrl3 &= ~0xE00;
			gpHalCdspSetMuxPath(0);
			pCdspRegFront->cdspTgZero = 0x1FF;
			pCdspRegFront->cdspFrontGCLK |= 0x3F;
			pCdspRegFront->cdspMipiCtrl = 0x0000;
			break;
		case C_MIPI_FMT_RAW10:
			pCdspReg1->cdspDataFmt |= 0x320;
			pCdspReg1->cdspDataFmt &= ~(1 << 4);
			pCdspRegFront->cdspFrontCtrl3 &= ~0xE00;
			gpHalCdspSetMuxPath(0);
			pCdspRegFront->cdspTgZero = 0x1FF;
			pCdspRegFront->cdspFrontGCLK |= 0x3F;
			pCdspRegFront->cdspMipiCtrl = 0x0000;
			break;
		case C_MIPI_FMT_Y1VY0U:
			pCdspReg1->cdspDataFmt |= 0x320;
			pCdspReg1->cdspDataFmt |= 1 << 4;
			pCdspRegFront->cdspFrontCtrl3 &= ~0x03;
			pCdspRegFront->cdspFrontCtrl3 |= 0xE60;
			gpHalCdspSetMuxPath(1);
			pCdspRegFront->cdspTgZero = 0x1FF;
			pCdspRegFront->cdspFrontGCLK |= 0x3F;
			pCdspRegFront->cdspMipiCtrl = 0x0001;
			break;	
		default:
			return -1;
	}
	return 0;
}

/**
* @brief	cdsp set front sensor input size
* @param	hoffset [in]: h offset
* @param	voffset [in]: v offset
* @param	hsize [in]: h size
* @param	vsize [in]: v size
* @return	none
*/
void 
gpHalCdspFrontSetFrameSize(
	UINT32 hoffset, 
	UINT32 voffset, 
	UINT32 hsize,	
	UINT32 vsize
)
{
	hsize &= 0xFFF;
	vsize &= 0xFFF;
	if(hoffset == 0) hoffset = 1;
	if(voffset == 0) voffset = 1;
	pCdspRegFront->cdspFrameHSet = (hoffset << 16)|hsize;
	pCdspRegFront->cdspFrameVSet = (voffset << 16)|vsize;
}

/**
* @brief	cdsp set front harizontal reshape
* @param	HReshEn [in]: h reshape enable
* @param	Hrise [in]: h rise size
* @param	Hfall [in]: h fall size
* @return	none
*/
void 
gpHalCdspFrontSetHReshape(
	UINT32 HReshEn, 
	UINT32 Hrise,	
	UINT32 Hfall
)
{
	pCdspRegFront->cdspHSyncFrEdge = ((Hfall & 0x1FFF) << 16) | (Hrise & 0x1FFF);
	pCdspRegFront->cdspFrontCtrl2 &= ~0x01;
	pCdspRegFront->cdspFrontCtrl2 |= (HReshEn & 0x1);
}

/**
* @brief	cdsp set front vertical reshape
* @param	VReshEn [in]: v reshape enable
* @param	Vrise [in]: v rise size
* @param	Vfall [in]: v fall size
* @return	none
*/
void 
gpHalCdspFrontSetVReshape(
	UINT32 VReshEn, 
	UINT32 Vrise,	
	UINT32 Vfall
)
{
	pCdspRegFront->cdspVSyncFrEdge = ((Vfall & 0x1FFF) << 16) | (Vrise & 0x1FFF);
	pCdspRegFront->cdspFrontCtrl2 &= ~0x02;
	pCdspRegFront->cdspFrontCtrl2 |= (VReshEn & 0x1) << 1;
}

/**
* @brief	cdsp set front h/v polarity
* @param	CCIR656En [in]: 0: CCIR601, 1: CCIR656
* @param	hpolarity [in]: 0: HACT, 1: LACT
* @param	vpolarity [in]: 0: HACT, 1: LACT
* @param	sync_en [in]: 0: disable, 1: enable
* @return	none
*/
void 
gpHalCdspFrontSetInterface(
	UINT32 CCIR656En,
	UINT32 hsync_act, 
	UINT32 vsync_act,
	UINT32 sync_en
)
{
	UINT32 reg_value = pCdspRegFront->cdspFrontCtrl3;
	
	if(CCIR656En)
	{
		reg_value |= (1 << 4);
		reg_value &= ~(1 << 6 | 1 << 5); //use Tvctr interface
		hsync_act = vsync_act = 0;
	}
	else 
	{	//CCIR601 & Href
		reg_value &= ~(1 << 4);	
		reg_value |= (1 << 6 | 1 << 5); //use hoffset/voffset
	}

	pCdspRegFront->cdspFrontCtrl3 = reg_value;
	reg_value = pCdspRegFront->cdspFrontCtrl1;
	if(hsync_act) 
	{
		reg_value |= 1 << 4;
		reg_value &= ~(1 << 8);
	}
	else
	{
		reg_value &= ~(1 << 4);
		reg_value |= (1 << 8);
	}
	
	if(vsync_act)
	{
		reg_value |= 1 << 5;
		reg_value &= ~(1 << 9);
	}
	else
	{
		reg_value &= ~(1 << 5);
		reg_value |= (1 << 9);
	}
	
	if(sync_en)
		reg_value |= 1 << 12;
	else
		reg_value &= ~(1 << 12);
	
	pCdspRegFront->cdspFrontCtrl1 = reg_value;
}

void 
gpHalCdspFrontGetInterface(
	UINT8 *CCIR656En,
	UINT8 *hsync_act, 
	UINT8 *vsync_act,
	UINT8 *sync_en
)
{
	UINT32 reg = pCdspRegFront->cdspFrontCtrl3;

	if(reg & (1<< 4)) {
		*CCIR656En = 1;
	} else {
		*CCIR656En = 0;
	}
		
	reg = pCdspRegFront->cdspFrontCtrl1;
	if(reg & (1 << 4)) {
		*hsync_act = 1;
	} else {
		*hsync_act = 0;
	}

	if(reg & (1 << 5)) {
		*vsync_act = 1;
	} else {
		*vsync_act = 0;
	}

	if(reg & (1 << 12)) {
		*sync_en = 1;
	} else {
		*sync_en = 0;
	}
}

void 
gpHalCdspFrontSetInterlace(
	UINT32 field,
	UINT32 interlace
)
{
	UINT32 reg_value = pCdspReg1->cdspTvMode;
	
	if(interlace)
	{
		reg_value |= (1 << 0);
	}
	else
	{
		reg_value &= ~(1 << 0);
	}

	if(field)
	{
		reg_value |= (1<<2) | (1<<3);
	}
	else
	{
		reg_value &= ~((1<<3) | (1<<2));
	}

	pCdspReg1->cdspTvMode = reg_value;
}

/**
* @brief	cdsp set front mipi sensor input size
* @param	hoffset [out]: h offset
* @param	voffset [out]: v offset
* @param	hsize [out]: h size
* @param	vsize [out]: v size
* @return	none
*/
void 
gpHalCdspFrontSetMipiFrameSize(
	UINT32 hoffset, 
	UINT32 voffset, 
	UINT32 hsize,	
	UINT32 vsize
)
{
	hoffset &= 0xFFF;
	voffset &= 0xFFF;
	hsize &= 0xFFF;
	vsize &= 0xFFF;	
	pCdspRegFront->cdspMipiHVOffset = (voffset << 12) | hoffset;
	pCdspRegFront->cdspMipiHVSize = (vsize << 12) | hsize;
}
#endif


