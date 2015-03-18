#include <mach/kernel.h>
#include <mach/module.h>
#include <mach/audiodrv/soundcard.h>
#include <mach/hal/hal_dac.h>
#include <mach/hal/hal_i2s.h>
#include <mach/hal/hal_i2s_hdmi.h>
#include <mach/hal/hal_apbdma1.h>
#include <mach/hal/regmap/reg_scu.h>
#include <mach/hal/regmap/reg_i2s.h>

#define AUDIO_PLL_DISABLE			(0<<0)
#define AUDIO_PLL_ENABLE			(1<<0)
#define AUDIO_APLL_SEL_48K			(0<<1)
#define AUDIO_APLL_SEL_44K			(1<<1)
#define AUDIO_I2S_SCLK_FROM_IOPAD	(0<<2)
#define AUDIO_I2S_SCLK_FROM_APLL	(1<<2)
#define AUDIO_APLL_XTAL_ENABLE		(1<<3)
#define AUDIO_APLL_AS_MASK			(0x3F<<4)
#define AUDIO_CODEC_SEL_INTERAL		(0<<10)
#define AUDIO_CODEC_SEL_EXTERAL		(1<<10)
#define AUDIO_APLL_RESET			(1<<11)

static scuaReg_t *pscuaReg = (scuaReg_t *)LOGI_ADDR_SCU_A_REG;
static int i2stx_en=0;
static int i2srx_en=0;
static int clock_en=0;

/**********************************************************
*  apbdma Section
***********************************************************/
void audio_apbdma_clear(int ch)
{
	gpHalApbdma1ClearIRQFlag(ch);
}

int audio_ocpdma_clear()
{
	return gpHalOcpdmaClearIRQFlag();
}

void audio_apbdma_setbuf(int ch, int buf_num, char* addr, int ln)
{
	gpHalApbdma1SetBuf(ch, buf_num, addr, ln);
}

void audio_ocpdma_setbuf(int buf_num, char* addr, int ln)
{
	gpHalOcpdmaSetBuf(buf_num, addr, ln);
}

int audio_apbdma_checkbuf(int ch)
{
	return gpHalApbdma1BufStatus(ch);
}

void audio_apbdma_en(int ch)
{
	gpHalApbdma1En(ch);
}

void audio_ocpdma_en(void)
{
	gpHalOcpdmaIntEnable();
}

void audio_apbdma_diable(int ch)
{
	gpHalApbdma1Disable(ch);
}

/**********************************************************
*  i2s Section
***********************************************************/
void audio_i2stx_setfmt(int fmt)
{
	if( fmt==AFMT_S16_LE ) {
		gpHalI2sFmtSet(IISTX, 1);
	} else {
		gpHalI2sFmtSet(IISTX, 0);
	}
}

void audio_i2srx_setfmt(int fmt)
{
	if( fmt==AFMT_S16_LE ) {
		gpHalI2sFmtSet(IISRX, 1);
	} else {
		gpHalI2sFmtSet(IISRX, 0);
	}
}

void audio_i2stx_chlset(int ch)
{
	gpHalI2sChlSet(IISTX, ch);
}

void audio_i2srx_chlset(int ch)
{
	gpHalI2sChlSet(IISRX, ch);
}

void audio_i2stx_en(void)
{
	if(i2stx_en==0) {
		gpHalI2sFifoClr(IISTX);
		gpHalI2sIntEn(IISTX);
		gpHalI2sEn(IISTX);
	}
	i2stx_en = 1;
}

void audio_i2stx_disable(void)
{
	if(i2stx_en==1) {
		gpHalI2sFifoClr(IISTX);
		gpHalI2sIntDisable(IISTX);
		gpHalI2sDisable(IISTX);
	}
	i2stx_en=0;
}

void audio_i2srx_en(void)
{
	if(i2srx_en==0) {
		gpHalI2sFifoClr(IISRX);
		gpHalI2sIntEn(IISRX);
		gpHalI2sEn(IISRX);
	}
	i2srx_en=1;
}

void audio_i2srx_disable(void)
{
	if(i2srx_en==1) {
		gpHalI2sFifoClr(IISRX);
		gpHalI2sIntDisable(IISRX);
		gpHalI2sDisable(IISRX);
	}
	i2srx_en=0;
}

void audio_i2shdmi_disable(void)
{
	gpHalI2sHdmiDisable();
}

void audio_i2stx_set(void)
{
	int iiscr;
	iiscr =	IISTX_FIRSTFRAME_L | IISTX_FRAME_POLARITY_L | IISTX_EDGEMODE_FALLING
			| IISTX_SENDMODE_MSB | IISTX_MODE_ALIGN_LEFT | IISTX_VDMODE_16
			| IISTX_FSMODE_32 | IISTX_MODE_I2S | IISTX_MASTER_MODE
			| IISTX_IRT_POLARITY_HIGH | IISTX_EN_OVWR | IISTX_MERGE;
	
	iiscr = iiscr & (~IISTX_SLAVE_MODE);
	gpHalI2sCtlSet(IISTX, iiscr);
}

void audio_i2stx_setctl(unsigned int ctl)
{
	gpHalI2sCtlSet(IISTX, ctl);
}

unsigned int audio_i2stx_getctl(void)
{
	return gpHalI2sCtlGet(IISTX);
}

void audio_i2srx_set(void)
{
	int iiscr;
	iiscr = IISRX_FIRSTFRAME_L | IISRX_FRAME_POLARITY_L |
			IISRX_EDGEMODE_FALLING | IISRX_SENDMODE_MSB |
			IISRX_MODE_ALIGN_RIGHT | IISRX_VDMODE_16 | IISRX_FSMODE_16
			| IISRX_MODE_NORMAL | IISRX_SLAVE_MODE |
			IISRX_IRT_POLARITY_HIGH | IISRX_CLRFIFO | IISRX_MERGE | 0x200000;

	iiscr = iiscr & (~IISRX_MASTER_MODE);
	gpHalI2sCtlSet(IISRX, iiscr);
}

void audio_i2shdmi_chlset(int num)
{
	gpHalI2sHdmiChlSet(num);
}

void audio_i2s_hdmi_init(int reset, int ch, int bit, int sample)
{
	gpHalI2sHdmiClkdiv(sample);
	gpHalI2sHdmiInit(reset);
	gpHalI2sHdmiChlSet(2);
	gpHalI2sHdmiChanStatus(ch, bit, sample);
}

void audio_i2shdmi_setfmt(int fmt)
{
	gpHalI2sHdmiSetfmt(fmt);
}

/**********************************************************
*  Mic control Section
***********************************************************/
void audio_mic_ctrl(int enable)
{
	gpHalMicEn(enable);
}
EXPORT_SYMBOL(audio_mic_ctrl);

void audio_mic_volset(long l_vol)
{
	gpHalMicVolSet( 0x1f-(l_vol&0x1f));	
}
EXPORT_SYMBOL(audio_mic_volset);

void audio_mic_volget(long *l_vol)
{
	gpHalMicVolGet(l_vol);
	*l_vol = 0x1f - *l_vol;
}
EXPORT_SYMBOL(audio_mic_volget);


/**********************************************************
*  Linein control Section
***********************************************************/
void audio_linein_ctrl(int enable)
{
	gpHalLininEn(enable);
}
EXPORT_SYMBOL(audio_linein_ctrl);

void audio_linein_volset(long l_vol, long r_vol)
{
	gpHalLininVolSet( 0x1f-(l_vol&0x1f), 0x1f-(r_vol&0x1f));
}
EXPORT_SYMBOL(audio_linein_volset);

void audio_linein_volget(long *l_vol,long *r_vol)
{
	gpHalLininVolGet(l_vol, r_vol);

	*l_vol = 0x1f - *l_vol;
	*r_vol = 0x1f - *r_vol;
}
EXPORT_SYMBOL(audio_linein_volget);


/**********************************************************
*  Headphone control Section
***********************************************************/
void audio_hdphn_volget(long *l_vol,long *r_vol)
{
	gpHalHdphnVolGet(l_vol, r_vol);

	*l_vol = 0x1f - *l_vol;
	*r_vol = 0x1f - *r_vol;
}
EXPORT_SYMBOL(audio_hdphn_volget);

void audio_hdphn_volset(long l_vol,long r_vol)
{
	gpHalHdphnVolSet( 0x1f-(l_vol&0x1f), 0x1f-(r_vol&0x1f));
}
EXPORT_SYMBOL(audio_hdphn_volset);

void audio_hdphn_muteget(long *l_mute, long *r_mute)
{
	gpHalHdphnMuteGet(l_mute, r_mute);
}
EXPORT_SYMBOL(audio_hdphn_muteget);

void audio_hdphn_muteset(long l_mute, long r_mute)
{
	gpHalHdphnMuteSet(l_mute, r_mute);
}
EXPORT_SYMBOL(audio_hdphn_muteset);

unsigned int audio_hpins_get(void)
{
	return gpHalHpinsGet();
}
EXPORT_SYMBOL(audio_hpins_get);

void audio_hpins_set(unsigned int hpins)
{
	gpHalHpinsSet(hpins);
}
EXPORT_SYMBOL(audio_hpins_set);

unsigned int audio_adins_get(void)
{
	return gpHalAdinsGet();
}
EXPORT_SYMBOL(audio_adins_get);


/**********************************************************
*  power & frequency control Section
***********************************************************/
int audio_freq_set(int freq, int type)
{
	int ratio=0, reg;
	
	pscuaReg->scuaApllCfg |= AUDIO_PLL_ENABLE | AUDIO_APLL_XTAL_ENABLE | AUDIO_I2S_SCLK_FROM_APLL;
	
	if ((264600 % freq) == 0) {				/* 11025, 14700, 22050, 29400, 44100... Hz*/
		pscuaReg->scuaApllCfg |= AUDIO_APLL_SEL_44K;
		ratio = 264600 / freq - 1;
	}
	else if ((288000 % freq) == 0) {		/* 8000, 16000, 24000, 32000, 48000... Hz*/
		pscuaReg->scuaApllCfg &= (~AUDIO_APLL_SEL_44K);
		ratio = 288000 / freq - 1;
	}

	if( ratio<=0 || ratio>255 ) {
		printk("not support\n");
		return -1;
	}
	
	reg = pscuaReg->scuaApllCfg;
	reg &= ~(AUDIO_APLL_AS_MASK | AUDIO_CODEC_SEL_EXTERAL);
	reg |= 0x120 | AUDIO_APLL_RESET;
	
	if(!type) {
		reg = ((reg&(~0xff0000))|(ratio<<16));
	} else {
		reg = (reg&(~0xff000000))|(ratio<<24);
	}
	pscuaReg->scuaApllCfg = reg;
	reg &= ~AUDIO_APLL_RESET;
	pscuaReg->scuaApllCfg = reg;
	
	pscuaReg->scuaCodecCfg &= ~0x200;
	
	if(type) {
		gpHalDacFeqSet(freq);
	}
	return 0;
}

void audio_clock_enable(void)
{
	if(clock_en==0) {
		gpHalI2sClkEn(1);
		gpHalHdmiI2sClkEn(1);
		gpHalDacClkEn(1);
		gpHalApbdma1ClkEn(1);
	}
	clock_en=1;
}

void audio_clock_disable(void)
{
	if(clock_en==1) {
		gpHalI2sClkEn(0);
		gpHalHdmiI2sClkEn(0);
		gpHalDacClkEn(0);
		gpHalApbdma1ClkEn(0);
	}
	clock_en=0;
}

void audio_power_ctrl(int enable)
{
	int i;
	if(enable) {
		/* enable related clock: i2s, dac, apbdma */
		audio_clock_enable();
				
		/* if apbdma has opened in 2nd boot loader, reset it */
		gpHalApbdma1Rst(0);
		
		/* give default freqency 44100 */
		audio_freq_set(48000, 1);

	//	gpHalLinoutVolset();
		gpHalHdphnEn(enable);
	//	gpHalAdcSetVref();
		gpHalVrefCtrl(enable, 0);
		gpHalAudadcEn(enable);
		gpHalAuddacEn(enable);
	} else {
		audio_hdphn_volset(0x1f, 0x1f);
		gpHalAudadcEn(enable);
		gpHalVrefCtrl(enable, 1);
		for( i=0; i<1000; i++)
			udelay(1000);						//1s delay here
		
		gpHalHdphnDischarge();		
		gpHalHdphnEn(enable);
		
		pscuaReg->scuaApllCfg &= ~(AUDIO_PLL_ENABLE | AUDIO_APLL_XTAL_ENABLE);

		audio_clock_disable();
	}
}

void audio_suspend(void)
{
	audio_hdphn_volset(0x1f, 0x1f);			//mute
	gpHalAuddacEn(0);
	
	pscuaReg->scuaApllCfg &= ~(AUDIO_PLL_ENABLE | AUDIO_APLL_XTAL_ENABLE);

	audio_clock_disable();
}

void audio_dac_ctrl(int enable)
{
	gpHalAuddacEn(enable);
}

int audio_WAVE_ismute(void)
{
	long l_vol, r_vol;
	audio_hdphn_volget(&l_vol, &r_vol);

	if (l_vol == 0 && r_vol == 0){
		return 1;
	}
	return 0;
}

/* Compatible with previous version	*/

void audio_LINEOUT_muteset(long l_mute,long r_mute)
{
}
EXPORT_SYMBOL(audio_LINEOUT_muteset);

void audio_WAVE_ctrl(unsigned int enable)
{
	audio_dac_ctrl(enable);
}
EXPORT_SYMBOL(audio_WAVE_ctrl);

void audio_LINEIN_ctrl(unsigned int enable)
{
	audio_linein_ctrl(enable);
}
EXPORT_SYMBOL(audio_LINEIN_ctrl);

void audio_LINEIN_volset(long l_vol,long r_vol)
{
	audio_linein_volset(l_vol, r_vol);
}
EXPORT_SYMBOL(audio_LINEIN_volset);

void audio_HPINS_set(unsigned int hpins)
{
	audio_hpins_set(hpins);
}
EXPORT_SYMBOL(audio_HPINS_set);
