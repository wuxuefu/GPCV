
#define SNDCTL_DSP_SET_AGC		_IOWR('P', 75, int)


typedef struct audio_agc_setting
{
  int PGA_low_bound;
  int PGA_upper_bound;
  int level_low_bound;
  int level_upper_bound;
} audio_agc_setting;

void I2SRx_init_mic_agc(void);
int	I2SRx_Mic_AGC(unsigned int AGCbufferAddr, int AGCbufferSize);
void agc_para_setting(audio_agc_setting *para);
