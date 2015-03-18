#ifndef _AUDIO_UTIL_H_
#define _AUDIO_UTIL_H_

enum {
	I2S_TX = 0,
	HDMI_TX = 1
};
enum {
	DMA_BUFFER_A = 1,
	DMA_BUFFER_B = 2
};

/* GPL32900B use only */
#define SNDCTL_DSP_GP_INIT		_IOWR('P', 7, int)

void audio_clock_enable(void);
void audio_clock_disable(void);
void audio_power_ctrl(int enable);
int audio_freq_set(int freq, int type);

void audio_suspend(void);

void audio_apbdma_clear(int ch);
int audio_apbdma_checkbuf(int ch);
void audio_apbdma_setbuf(int ch, int buf_num, char* addr, int ln);
void audio_apbdma_en(int ch);
void audio_apbdma_diable(int ch);
void audio_i2shdmi_disable(void);

int audio_ocpdma_clear(void);
void audio_ocpdma_setbuf(int buf_num, char* addr, int ln);
void audio_ocpdma_en(void);

void audio_i2stx_setfmt(int fmt);
void audio_i2srx_setfmt(int fmt);
void audio_i2stx_en(void);
void audio_i2stx_disable(void);
void audio_i2srx_en(void);
void audio_i2srx_disable(void);
void audio_i2stx_set(void);
unsigned int audio_i2stx_getctl(void);
void audio_i2stx_setctl(unsigned int ctl);
void audio_i2srx_set(void);
void audio_i2stx_chlset(int ch);
void audio_i2srx_chlset(int ch);

void audio_hdphn_volset(long l_vol,long r_vol);
void audio_hdphn_volget(long *l_vol,long *r_vol);
void audio_hdphn_muteget(long *l_mute, long *r_mute);
void audio_hdphn_muteset(long l_mute, long r_mute);

void audio_mic_ctrl(int enable);
void audio_mic_volset(long l_vol);
void audio_mic_volget(long *l_vol);

void audio_linein_ctrl(int enable);
void audio_linein_volset(long l_vol, long r_vol);
void audio_linein_volget(long *l_vol,long *r_vol);

unsigned int audio_hpins_get(void);
void audio_hpins_set(unsigned int hpins);
unsigned int audio_adins_get(void);

void audio_LINEOUT_muteset(long l_mute,long r_mute);
int audio_WAVE_ismute(void);

/* Compatible with previous version	*/
void audio_WAVE_ctrl(unsigned int enable);
void audio_LINEIN_ctrl(unsigned int enable);
void audio_LINEIN_volset(long l_vol,long r_vol);
void audio_HPINS_set(unsigned int hpins);

void audio_i2s_hdmi_init(int reset, int ch, int bit, int sample);
void audio_i2shdmi_setfmt(int fmt);
void audio_i2shdmi_chlset(int num);

#endif