#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/major.h>

#include <mach/gp_display.h>
#include <mach/gp_display1.h>
#include <mach/gp_display2.h>
#include <mach/gp_board.h>
#include <mach/audio/soundcard.h>
#include "mach/typedef.h"
#include "config/sysconfig.h"
#include "diag.h"
#include "version.h"
#include "icver.h"

static gp_icver_t gp_ver={0};

extern UINT32 insmod(const char *path_fmt, ...);

#define	PATH_GAMMA	"/etc/disp/DISPGAM.BIN"
#define	PATH_COLOR	"/etc/disp/COLOR.BIN"

static UINT32
sysMknod(
	char *name,
	mode_t mode
)
{
	FILE *file;
	char line[128];
	UINT32 minor;
	char *pEnd;

	file = fopen ("/proc/misc", "r");
	if (file == NULL) {
		DIAG_ERROR("[%s:%d], Open Fail!\n", __FUNCTION__, __LINE__);
		return SP_FAIL;
	}

	while (fgets(line, sizeof(line), file) != NULL) {
		minor = (UINT32)strtol(line, &pEnd, 10);
		pEnd += 1;
		if (strncmp(pEnd, name, strlen(name)) == 0) {
			memset(&line, 0, sizeof(line));
			snprintf(line, sizeof(line), "/dev/%s", name);
			mknod(line, S_IFCHR|0777, makedev(MISC_MAJOR, minor));
			break;
		}
	}

	fclose ( file );

	return SP_OK;
}

#if 0 // gp_board static linked with kernel
static UINT32
boardCoreInit(
	void
)
{
	if (insmod("gp_board") != SP_OK) {
		DIAG_ERROR("insmod gp_board error!\n");
		return SP_FAIL;
	}

	return SP_OK;
}
#endif

static UINT32
boardInit(
	void
)
{
	if (insmod("/lib/modules/board_config.ko") != SP_OK) {
		DIAG_ERROR("insmod board_config error!\n");
		return SP_FAIL;
	}

	return SP_OK;
}


static UINT32
rtcInit(
	void
)
{
#ifdef SYSCONFIG_INTERNAL_RTC
	if (insmod("gp_rtc_module") != SP_OK) {
		DIAG_ERROR("insmod external_rtc error!\n");
		return SP_FAIL;
	}
#else
	if (insmod("/lib/modules/external_rtc.ko") != SP_OK) {
		DIAG_ERROR("insmod external_rtc error!\n");
		return SP_FAIL;
	}
#endif
	return SP_OK;
}

static UINT32
powerInit(
	void
)
{
#ifdef SYSCONFIG_POWER
	if (insmod("/lib/modules/gp_power.ko") != SP_OK) {
		DIAG_ERROR("insmod gp_power error!\n");
		return SP_FAIL;
	}
#endif
	return SP_OK;
}

static UINT32
batteryInit(
	void
)
{
#ifdef SYSCONFIG_BATTERY
	if (insmod("/lib/modules/gp_battery.ko") != SP_OK) {
		DIAG_ERROR("insmod gp_battery error!\n");
		return SP_FAIL;
	}
#endif
	return SP_OK;
}

static UINT32
gsensorInit(
	void
)
{
#ifdef SYSCONFIG_G_SENSOR
	DIAG_ERROR("load gsensor ko\n");
	if (insmod("/lib/modules/common/%s.ko",SYSCONFIG_G_SENSOR_DEVICE) != SP_OK) {
		DIAG_ERROR("insmod gp_gsensor error!\n");
		return SP_FAIL;
	}
#endif
	return SP_OK;
}

static UINT32
asensorInit(
	void
)
{
#ifdef SYSCONFIG_A_SENSOR
	DIAG_ERROR("load asensor ko\n");
	if (insmod("/lib/modules/common/%s.ko",SYSCONFIG_A_SENSOR_DEVICE) != SP_OK) {
		DIAG_ERROR("insmod gp_gsensor error!\n");
		return SP_FAIL;
	}
#endif
	return SP_OK;
}
static UINT32 uartModeInit(void)
{/*

#define SYSCONFIG_UART_FUNC

#define SYSCONFIG_UART_FUNC_DEVICE "CloudDog"

*/
 #ifdef SYSCONFIG_UART_FUNC
	DIAG_ERROR("load asensor ko\n");
	if (insmod("/lib/modules/common/%s.ko",SYSCONFIG_UART_FUNC_DEVICE) != SP_OK) {
		DIAG_ERROR("insmod gp_gsensor error!\n");
		return SP_FAIL;
	}
#endif
	return SP_OK;
}
static UINT32
inputInit(
	void
)
{
#ifdef SYSCONFIG_KEY
	if (insmod("/lib/modules/key_driver.ko") != SP_OK) {
		DIAG_ERROR("insmod key_driver error!\n");
		return SP_FAIL;
	}
#endif

#ifdef SYSCONFIG_TOUCH
	if (insmod("/lib/modules/touch_driver.ko") != SP_OK) {
		DIAG_ERROR("insmod touch_driver error!\n");
		return SP_FAIL;
	}
#endif

	return SP_OK;
}
static UINT32
touchpanelInit(
        void
)
{
#ifdef SYSCONFIG_TOUCH_PANEL
    if(insmod("/lib/modules/common/%s.ko", SYSCONFIG_TOUCH_PANEL_DEVICE) != SP_OK) {
        DIAG_ERROR("insmod touch panel error!\n");
        return SP_FAIL;
    }
#endif
    return SP_OK;
}

static UINT32
wdtInit(
	void
)
{
	if (insmod("gp_wdt_module") != SP_OK) {
		DIAG_ERROR("insmod gp_wdt_module error!\n");
		return SP_FAIL;
	}
}

static UINT32
timerInit(
	void
)
{
	if (insmod("gp_timer_module") != SP_OK) {
		DIAG_ERROR("insmod gp_tc_module error!\n");
		return SP_FAIL;
	}
}

static UINT32
apbdma0Init(
	void
)
{
	if (insmod("gp_apbdma0_module") != SP_OK) {
		DIAG_ERROR("insmod gp_apbdma0_module error!\n");
		return SP_FAIL;
	}
}

static UINT32
sdmaInit(
	void
)
{
	if (insmod("gp_sdma_module") != SP_OK) {
		DIAG_ERROR("insmod gp_sdma_module error!\n");
		return SP_FAIL;
	}
}

static UINT32
i2cInit(
	void
)
{
	if(insmod("gp_i2c_bus_module") != SP_OK){
		DIAG_ERROR("insmod I2C bus error!\n");
		return SP_FAIL;
	}
}

static UINT32
ti2cInit(
	void
)
{
	if(insmod("gp_ti2c_bus_module") != SP_OK){
		DIAG_ERROR("insmod TI2C bus error!\n");
		return SP_FAIL;
	}
}

static UINT32
pwmInit(
	void
)
{
	if (insmod("gp_pwm_module") != SP_OK) {
		DIAG_ERROR("insmod gp_pwm_module error!\n");
		return SP_FAIL;
	}
}

static UINT32
adcInit(
	void
)
{
#ifdef SYSCONFIG_INTERNAL_ADC
	if (insmod("gp_adc_module") != SP_OK) {
		DIAG_ERROR("insmod gp_adc_module error!\n");
		return SP_FAIL;
	}
#endif
    return SP_OK;
}

static UINT32
dc2dcInit(
	void
)
{
#ifdef SYSCONFIG_DC2DC
	if (insmod("gp_dc2dc_module") != SP_OK) {
		DIAG_ERROR("insmod gp_dc2dc_module error!\n");
		return SP_FAIL;
	}
#endif
	return SP_OK;
}

static UINT32
displayInit(
	void
)
{
	int dispDev;
	int outputCnt;
	int res_ptr;
	int pllsel;
	int showbg;
	
	if(strcmp(SYSCONFIG_PLATFORM, "gplus.cvr_turnkey_evm") == 0 ||
		strcmp(SYSCONFIG_PLATFORM, "gplus.cvr_turnkey_demo0") == 0 ||
		strcmp(SYSCONFIG_PLATFORM, "gplus.cvr_turnkey_demo1") == 0 ) {
		pllsel = 2;
		showbg = 0;
	} else {
		pllsel = 0;
		showbg = 1;		
	}

#ifdef SYSCONFIG_DISP0
	outputCnt = 0;
	/* install display module */
	if(pllsel == 0) {
		if (insmod("gp_display pll_sel=\"%s\"", "PLL0") != SP_OK) {
			DIAG_ERROR("insmod display error!\n");
			return SP_FAIL;
		}
	} else if(pllsel == 2) {
		if (insmod("gp_display pll_sel=\"%s\"", "PLL2") != SP_OK) {
			DIAG_ERROR("insmod display error!\n");
			return SP_FAIL;
		}
	}
	sysMknod("disp0", S_IFCHR);	

	/* install panel module */
	if ((strcmp(SYSCONFIG_DISP0_PANEL, "None") != 0) && (insmod("/lib/modules/common/%s.ko", SYSCONFIG_DISP0_PANEL) == SP_OK)) {
		outputCnt ++;
	}

	/* install tv module */
	#ifdef SYSCONFIG_DISP0_TVOUT
	if( gp_ver.major == MACH_GPL32900 ) {
		if (insmod("tv_ntsc") == SP_OK) {
			outputCnt ++;
		}
	}
	
	if( gp_ver.major == MACH_GPL32900B ) {
		if (insmod("gp_tv") == SP_OK) {
			outputCnt ++;
		}	
	}
	#endif

	#ifdef SYSCONFIG_DISP0_HDMI
	if(pllsel == 0) {
		if (insmod("gp_hdmi pll_sel=\"%s\"", "PLL0") == SP_OK)
			outputCnt ++;
	} else if(pllsel == 2) {
		if (insmod("gp_hdmi pll_sel=\"%s\"", "PLL2") == SP_OK)
			outputCnt ++;		
	}
	#endif

	if (!outputCnt) {
		DIAG_ERROR("Error!! No output device of display installed!\n");
		return SP_FAIL;
	}
if(showbg) {
	/* Initial display device */
	dispDev = open("/dev/disp0",O_RDWR);
	if(dispDev < 0) {
		DIAG_ERROR("Error!! open /dev/disp0/\n");
		return SP_FAIL;
	}

	ioctl(dispDev, DISPIO_SET_INITIAL, NULL);
	
	/* Download Display Gamma Curve */
	res_ptr = open( PATH_GAMMA,O_RDONLY);
	if(res_ptr < 0) {
		DIAG_ERROR("Error!! can't ");
		DIAG_ERROR(PATH_GAMMA);
		DIAG_ERROR("\n");
	}
	else {
		gp_disp_gammatable_t	pGammaTable_R;
		gp_disp_gammatable_t	pGammaTable_G;
		gp_disp_gammatable_t	pGammaTable_B;
		
		pGammaTable_R.id = SP_DISP_GAMMA_R;
		read( res_ptr, pGammaTable_R.table, 256 );
		pGammaTable_G.id = SP_DISP_GAMMA_G;
		read( res_ptr, pGammaTable_G.table, 256 );
		pGammaTable_B.id = SP_DISP_GAMMA_B;
		read( res_ptr, pGammaTable_B.table, 256 );

		ioctl(dispDev, DISPIO_SET_GAMMA_ENABLE, 0 );
		ioctl(dispDev, DISPIO_SET_GAMMA_PARAM, &pGammaTable_R );
		ioctl(dispDev, DISPIO_SET_GAMMA_PARAM, &pGammaTable_G );
		ioctl(dispDev, DISPIO_SET_GAMMA_PARAM, &pGammaTable_B );
		ioctl(dispDev, DISPIO_SET_GAMMA_ENABLE, 1 );
		close(res_ptr);
	}

	/* Download Display Color Matrix */
	res_ptr = open( PATH_COLOR, O_RDONLY);
	if(res_ptr < 0) {
		DIAG_ERROR("Error!! can't ");
		DIAG_ERROR(PATH_COLOR);
		DIAG_ERROR("\n");
	}
	else {
		gp_disp_colormatrix_t	colorMatrix;
		uint32_t	parameter;
		
		read( res_ptr, &parameter, sizeof(uint32_t));
		colorMatrix.a00 = (uint16_t) parameter;
		read( res_ptr, &parameter, sizeof(uint32_t));
		colorMatrix.a01 = (uint16_t) parameter;
		read( res_ptr, &parameter, sizeof(uint32_t));
		colorMatrix.a02 = (uint16_t) parameter;
		read( res_ptr, &parameter, sizeof(uint32_t));
		colorMatrix.a10 = (uint16_t) parameter;
		read( res_ptr, &parameter, sizeof(uint32_t));
		colorMatrix.a11 = (uint16_t) parameter;
		read( res_ptr, &parameter, sizeof(uint32_t));
		colorMatrix.a12 = (uint16_t) parameter;
		read( res_ptr, &parameter, sizeof(uint32_t));
		colorMatrix.a20 = (uint16_t) parameter;
		read( res_ptr, &parameter, sizeof(uint32_t));
		colorMatrix.a21 = (uint16_t) parameter;
		read( res_ptr, &parameter, sizeof(uint32_t));
		colorMatrix.a22 = (uint16_t) parameter;
		read( res_ptr, &parameter, sizeof(uint32_t));
		colorMatrix.b0 = (uint16_t) parameter;
		read( res_ptr, &parameter, sizeof(uint32_t));
		colorMatrix.b1 = (uint16_t) parameter;
		read( res_ptr, &parameter, sizeof(uint32_t));
		colorMatrix.b2 = (uint16_t) parameter;
			
		ioctl(dispDev, DISPIO_SET_CMATRIX_PARAM, &colorMatrix );
		close(res_ptr);
	}	
	
	close(dispDev);
}
	#ifdef SYSCONFIG_DISP0_FB
		#ifdef SYSCONFIG_FB_PARAMETER
		if (insmod("gp_fb fbparam=\"%s\"", SYSCONFIG_FB_PARAMETER) != SP_OK) {
		#else
		if (insmod("gp_fb main_disp=\"%s\"", SYSCONFIG_DISP0_PANEL) != SP_OK) {
		#endif
			DIAG_ERROR("insmod fb fail\n");
			return SP_FAIL;
		}
	#endif
#endif

#ifdef SYSCONFIG_DISP1
	outputCnt = 0;
	/* install display1 module */
	if (insmod("gp_display1") != SP_OK) {
		DIAG_ERROR("insmod display1 error!\n");
		return SP_FAIL;
	}
	sysMknod("disp1", S_IFCHR);

	/* install panel module */
	if ((strcmp(SYSCONFIG_DISP1_PANEL, "None") != 0) && (insmod("/lib/modules/common/%s.ko", SYSCONFIG_DISP1_PANEL) == SP_OK)) {
		outputCnt ++;
	}

	if (!outputCnt) {
		DIAG_ERROR("Error!! No output device of display1 installed!\n");
		return SP_FAIL;
	}

	/* Initial display1 device */
	dispDev = open("/dev/disp1",O_RDWR);
	if(dispDev < 0) {
		DIAG_ERROR("Error!! open /dev/disp0/\n");
		return SP_FAIL;
	}

	ioctl(dispDev, DISPIO_SET_INITIAL, NULL);
	close(dispDev);

	#ifdef SYSCONFIG_DISP1_FB
		if (insmod("gp_fb1") != SP_OK) {
			DIAG_ERROR("insmod fb1 fail\n");
			return SP_FAIL;
		}
	#endif
#endif

#ifdef SYSCONFIG_DISP2
	outputCnt = 0;
	/* install display module */
	if (insmod("gp_display2") != SP_OK) {
		DIAG_ERROR("insmod display2 error!\n");
		return SP_FAIL;
	}
	sysMknod("disp2", S_IFCHR);	

	/* install tv module */
	if (insmod("tv1") == SP_OK) {
		outputCnt ++;
	}

	if (!outputCnt) {
		DIAG_ERROR("Error!! No output device of display2 installed!\n");
		return SP_FAIL;
	}

	/* Initial display device */
	dispDev = open("/dev/disp2",O_RDWR);
	if(dispDev < 0) {
		DIAG_ERROR("Error!! open /dev/disp2/\n");
		return SP_FAIL;
	}

	ioctl(dispDev, DISPIO_SET_INITIAL, NULL);
	close(dispDev);

	#ifdef SYSCONFIG_DISP2_FB
		if (insmod("gp_fb2") != SP_OK) {
			DIAG_ERROR("insmod fb2 fail\n");
			return SP_FAIL;
		}
	#endif
#endif

	return SP_OK;
}

static UINT32
backlightEnable(
	UINT32 en
)
{
	gp_board_panel_t *panel_config;
	panel_config = gp_board_get_config("panel", gp_board_panel_t);
	if (panel_config != NULL && panel_config->set_backlight != NULL) {
		panel_config->set_backlight(en);
	}
	else {
		return SP_FAIL;
	}

	return SP_OK;
}

static UINT32
usbInit(
	void
)
{
	if (strcmp(SYSCONFIG_PRODUCT, "gplus.microwindowsUI__gplus.evm32900b") != 0) {
		#ifdef SYSCONFIG_USB
		if (insmod("usbcore") != SP_OK) {
			DIAG_ERROR("insmod usbcore error!\n");
			return SP_FAIL;
		}
		#endif

		#ifdef SYSCONFIG_USB_HOST
		#ifdef SYSCONFIG_USB_HOST_HIGHSPEED_MODE
		/*Only High Speed mode needs to use EHCI controller.*/
		if (insmod("ehci-hcd") != SP_OK) {
			DIAG_ERROR("insmod ehci-hcd error!\n");
			return SP_FAIL;
		}
		/* HOST EHSET test mode.*/
		if (insmod("gp_ehset") != SP_OK) {
			DIAG_ERROR("insmod ehci-hcd error!\n");
			return SP_FAIL;
		}
		#endif
		
		if (insmod("ohci-hcd") != SP_OK) {
			DIAG_ERROR("insmod ohci-hcd error!\n");
			return SP_FAIL;
		}
			
		#endif

		#ifdef SYSCONFIG_USB_HOST_STORAGE
		if (insmod("usb-storage") != SP_OK) {
			DIAG_ERROR("insmod usb storage error!\n");
			return SP_FAIL;
		}
		#endif
	}

#ifdef SYSCONFIG_USB_SLAVE
	if (insmod("spmp_udc") != SP_OK) {
		DIAG_ERROR("insmod spmp_udc error!\n");
		return SP_FAIL;
	}
#endif


#ifdef SYSCONFIG_USB_SLAVE_MSDC
	if (insmod("usb-storage") != SP_OK) {
		DIAG_ERROR("insmod usb mdsc error!\n");
		return SP_FAIL;
	}
#endif


#ifdef SYSCONFIG_USB
	if (insmod("gp_usb") != SP_OK) {
		DIAG_ERROR("insmod USB middleware error!\n");
		return SP_FAIL;
	}
#endif
	return SP_OK;
}

static UINT32
storageInit(
	void
)
{
#ifdef SYSCONFIG_NAND
	if (strcmp(SYSCONFIG_MAINSTORAGE, "gp_nand_module") 
		&& insmod("nand_hal") != SP_OK
		&& insmod("gp_nand_module") != SP_OK) {
		DIAG_ERROR("insmod nand driver error!\n");
		return SP_FAIL;
	}
#ifdef SYSCONFIG_GP_FAST_BOOT
	if (insmod("gp_blk_app") != SP_OK) {
		DIAG_ERROR("insmod nand APP driver error!\n");
		return SP_FAIL;
	}
#endif /* SYSCONFIG_GP_FAST_BOOT */

#endif
/*#ifdef SYSCONFIG_SDIO
#ifdef SYSCONFIG_ARCH_SPMP8050
	if (insmod("gp_sdio") != SP_OK) {
		DIAG_ERROR("insmod sdio driver error!\n");
		return SP_FAIL;
	}
#endif
#endif*/
//#ifdef //SYSCONFIG_SD
#if 1
	insmod("gp_sd");
//	if (strcmp(SYSCONFIG_MAINSTORAGE, "gp_sd") && insmod("gp_sd") != SP_OK) {
//		DIAG_ERROR("insmod sd driver error!\n");
//		return SP_FAIL;
//	}
#endif
#ifdef SYSCONFIG_MS
	if (strcmp(SYSCONFIG_MAINSTORAGE, "gp_mscard") && insmod("gp_mscard") != SP_OK) {
		DIAG_ERROR("insmod ms driver error!\n");
		return SP_FAIL;
	}
#endif
#ifdef SYSCONFIG_CF
	if (strcmp(SYSCONFIG_MAINSTORAGE, "gp_cf") && insmod("gp_cf") != SP_OK) {
		DIAG_ERROR("insmod cf driver error!\n");
		return SP_FAIL;
	}
#endif
#ifdef SYSCONFIG_XD
	if (strcmp(SYSCONFIG_MAINSTORAGE, "gp_xd") && insmod("gp_xd") != SP_OK) {
		DIAG_ERROR("insmod xd driver error!\n");
		return SP_FAIL;
	}
#endif
#ifdef SYSCONFIG_SFLASH
         insmod("gp_spi_module");
	if (strcmp(SYSCONFIG_MAINSTORAGE, "gp_spi_nor_module") && insmod("gp_spi_nor_module") != SP_OK) {
		DIAG_ERROR("insmod sflash driver error!\n");
		return SP_FAIL;
	}
#endif
#ifdef SYSCONFIG_EMMC_NAND
	if (strcmp(SYSCONFIG_MAINSTORAGE, "gp_emmc_nand") && insmod("gp_emmc_nand") != SP_OK) {

		DIAG_ERROR("insmod emmc_nand driver error!\n");
		return SP_FAIL;
	}
#endif
	return SP_OK;
}

static UINT32
audioInit(
	void
)
{
#ifdef SYSCONFIG_SPU
	if (insmod("gp_audio") != SP_OK) {
		DIAG_ERROR("insmod gp_audio error!\n");
		return SP_FAIL;
	}
	if (insmod("gp_mixer") != SP_OK) {
		DIAG_ERROR("insmod gp_mixer error!\n");
		return SP_FAIL;
	}
	if (insmod("gp_spu_module") != SP_OK) {
		DIAG_ERROR("insmod gp_spu_module error!\n");
		return SP_FAIL;
	}
#else 
#ifdef	SYSCONFIG_AUDIO
	if (insmod("gp_audio") != SP_OK) {
		DIAG_ERROR("insmod gp_audio error!\n");
		return SP_FAIL;
	}
	if (insmod("gp_mixer") != SP_OK) {
		DIAG_ERROR("insmod gp_mixer error!\n");
		return SP_FAIL;
	}
#endif
#endif
	return SP_OK;
}

static UINT32
fmInit(
	   void
	   )
{

#ifdef SYSCONFIG_FM
	if (insmod("/lib/modules/fm_driver.ko") != SP_OK) {
		DIAG_ERROR("insmod fm_driver error!\n");
		return SP_FAIL;
	}
#endif
	return SP_OK;
}

static UINT32
cevaInit(
	void
)
{
	if (insmod("gp_ceva_module") != SP_OK) {
		DIAG_ERROR("insmod gp_ceva_module error!\n");
		return SP_FAIL;
	}

	return SP_OK;
}

static UINT32
LBPInit(
	void
)
{
	if (insmod("gp_lbp_module") != SP_OK) {
		DIAG_ERROR("insmod gp_lbp_module error!\n");
		return SP_FAIL;
	}
	
	return SP_OK;
}

static UINT32
On2Init(
	void
)
{
	if (insmod("hx170dec") != SP_OK) {
		DIAG_ERROR("insmod on2 decoder module error!\n");
		return SP_FAIL;
	}
	
	if (insmod("hx280enc") != SP_OK) {
		DIAG_ERROR("insmod on2 encoder module error!\n");
		return SP_FAIL;
	}
	
	if (insmod("memalloc") != SP_OK) {
		DIAG_ERROR("insmod on2 memory module error!\n");
		return SP_FAIL;
	}

	return SP_OK;
}

static UINT32
lineBufferInit(
	void
)
{
	if (insmod("gp_line_buffer_module") != SP_OK) {
		DIAG_ERROR("insmod gp_line_buffer_module error!\n");
		return SP_FAIL;
	}

	return SP_OK;
}

static UINT32
graphicInit(
	void
)
{
#ifdef SYSCONFIG_GRAPHIC_2D
	if (insmod("gp_2d_module") != SP_OK) {
		DIAG_ERROR("insmod gp_2d_module error!\n");
		return SP_FAIL;
	}
#endif
	return SP_OK;
}

static UINT32
scalarInit(
	void
)
{
	if (insmod("gp_scale_module") != SP_OK) {
		DIAG_ERROR("insmod gp_scale_module error!\n");
		return SP_FAIL;
	}
	
	if( gp_ver.major == MACH_GPL32900 )
	{
		if (insmod("gp_scale2_module") != SP_OK) {
			DIAG_ERROR("insmod gp_scale_module error!\n");
			return SP_FAIL;
		}
	}

	if( gp_ver.major == MACH_GPL32900B )
	{	
		if (insmod("gp_scale2_module") != SP_OK) {
			DIAG_ERROR("insmod gp_scale_module error!\n");
			return SP_FAIL;
		}
	}

	return SP_OK;
}

static UINT32
aesInit(
	void
)
{
	if (insmod("gp_aes_module") != SP_OK) {
		DIAG_ERROR("insmod gp_aes_module error!\n");
		return SP_FAIL;
	}
	return SP_OK;
}

#ifdef SYSCONFIG_ARCH_SPMP8050
static UINT32
rotatorInit(
	void
)
{
	if (insmod("gp_rotate_module") != SP_OK) {
		DIAG_ERROR("insmod gp_rotate_module error!\n");
		return SP_FAIL;
	}

	return SP_OK;
}
#endif

static UINT32
ppuInit(
	void
)
{
	
#ifdef SYSCONFIG_PPU
#if (SYSCONFIG_PPU == 1)
	if (insmod("gp_ppu_module") != SP_OK) {
		DIAG_ERROR("insmod gp_ppu_module error!\n");
		return SP_FAIL;
	}
#elif (SYSCONFIG_PPU == 2)
	if (insmod("gp_ppu_simple") != SP_OK) {
		DIAG_ERROR("insmod gp_ppu_simple error!\n");
		return SP_FAIL;
	}
#endif
#endif
	return SP_OK;
}

static UINT32
tvInit(
	void
)
{
#ifdef SYSCONFIG_PPU_TV
	if (insmod("gp_tv_module") != SP_OK) {
		DIAG_ERROR("insmod gp_tv_module error!\n");
		return SP_FAIL;
	}
#endif
	return SP_OK;
}

static UINT32
sensorInit(
	void
)
{
	int count=0;
	int i, j;
	int done[SYSCONFIG_SENSOR_DRIVER_NUM];
	char sensor[SYSCONFIG_SENSOR_DRIVER_NUM][16];
	char port[SYSCONFIG_SENSOR_DRIVER_NUM][16];
	char param[SYSCONFIG_SENSOR_DRIVER_NUM][64];
	
	if(count==0) {
		if (insmod("sensor_mgr_module") != SP_OK) {
			DIAG_ERROR("insmod sensor_mgr_module error!\n");
			return SP_FAIL;
		}
		count++;
	}

	if( gp_ver.major == MACH_GPL32900 )
	{
		/* install mipi core module */
		if (insmod("gp_mipi_module") != SP_OK) {
			DIAG_ERROR("insmod gp_mipi_module error!\n");
			return SP_FAIL;
		}
	}

	if( gp_ver.major == MACH_GPL32900B )
	{
		if (strcmp(SYSCONFIG_PRODUCT, "gplus.microwindowsUI__gplus.evm32900b") != 0) {
			if (insmod("gp_mipi_module") != SP_OK) {	/* install mipi0/1 core module */
				DIAG_ERROR("insmod gp_mipi_module error!\n");
				return SP_FAIL;
			}
		}

	}

#ifdef SYSCONFIG_CSI0
	/* install sensor core module */
	if (insmod("gp_csi_module") != SP_OK) {
		DIAG_ERROR("insmod gp_csi_module error!\n");
		return SP_FAIL;
	}
#endif

#ifdef SYSCONFIG_CSI1
	/* install sensor core module */
	if (insmod("gp_csi1_module") != SP_OK) {
		DIAG_ERROR("insmod gp_csi1_module error!\n");
		return SP_FAIL;
	}
#endif

#ifdef SYSCONFIG_CSI2
	if (insmod("gp_aeawb_module") != SP_OK) {
		DIAG_ERROR("insmod gp_aeawb_module error!\n");
		return SP_FAIL;
	}
	/* install cdsp core module */
	if (insmod("gp_cdsp_module") != SP_OK) {
		DIAG_ERROR("insmod gp_cdsp_module error!\n");
		return SP_FAIL;
	}
#endif

#if defined(SYSCONFIG_SENSOR) || defined(SYSCONFIG_SENSOR_DRIVER_NUM)
	#if (SYSCONFIG_SENSOR_DRIVER_NUM != 0)
	for( i=0; i<SYSCONFIG_SENSOR_DRIVER_NUM; i++ ) {
		done[i]=0;
		if( i==0 )
		{
			strcpy( sensor[i], SYSCONFIG_SENSOR0 );
			strcpy( port[i], SYSCONFIG_SENSOR0_PORT_SEL );
		}
		else if( i==1 )
		{
			strcpy( sensor[i], SYSCONFIG_SENSOR1 );
			strcpy( port[i], SYSCONFIG_SENSOR1_PORT_SEL );
		}
		else if( i==2 )
		{
			strcpy( sensor[i], SYSCONFIG_SENSOR2 );
			strcpy( port[i], SYSCONFIG_SENSOR2_PORT_SEL );
		}
		for( j=0; j<64; j++ )
			param[i][j] = 0;
	}
	for( i=0; i<SYSCONFIG_SENSOR_DRIVER_NUM; i++ ) {
		if( done[i]==1 )
			continue;
		printf("%d\n", i);
		snprintf( param[i], sizeof(param[i]), "%d,%s", i, port[i] );
		for( j=i+1; j<SYSCONFIG_SENSOR_DRIVER_NUM; j++ ) {
			if( strcmp( sensor[i], sensor[j] )==0 ) {
				snprintf( param[i], sizeof(param[i]), "%s,%d,%s", param[i], j, port[j] );
				done[j]=1;
			}
		}
		if (insmod("/lib/modules/common/%s.ko param=\"%s\"",  sensor[i], param[i]) != SP_OK) {
			DIAG_ERROR("insmod sensor device driver0 error!\n");
			return SP_FAIL;
		}
	}
	#endif
#endif /* SYSCONFIG_SENSOR */
	return SP_OK;
}

#ifdef SYSCONFIG_GP_FAST_BOOT
static UINT32
gpFastBootInit(
	void
)
{
	if(insmod("snapshot") != SP_OK) {
		DIAG_ERROR("insmod snapshot driver error!\n");
		return SP_FAIL;
	}
	return SP_OK;
}
#endif /* SYSCONFIG_GP_FAST_BOOT */

static UINT32
usbWifiInit(
	void
)
{
	if (strcmp(SYSCONFIG_USB_WIFI, "None") != 0) {
		if (strcmp(SYSCONFIG_USB_WIFI, "AR9271") == 0) {
			system("insmod /lib/modules/common/compat.ko");
			system("insmod /lib/modules/common/compat_firmware_class.ko");
			system("insmod /lib/modules/common/kfifo.ko");
			system("insmod /lib/modules/common/cfg80211.ko");
			system("insmod /lib/modules/common/mac80211.ko");
			system("insmod /lib/modules/common/ath.ko");
			system("insmod /lib/modules/common/ath9k_hw.ko");
			system("insmod /lib/modules/common/ath9k_common.ko");
			system("insmod /lib/modules/common/ath9k_htc.ko");
		}
		else
			system("modprobe cfg80211");
			system("modprobe mac80211");
			system("modprobe sunrpc");
			if (strcmp(SYSCONFIG_PRODUCT, "gplus.microwindowsUI__gplus.evm32900b") != 0) {
				insmod("/lib/modules/common/%s.ko", SYSCONFIG_USB_WIFI);
			}
	}

	return SP_OK;
}

static UINT32
sysCoreInit(
	void
)
{
#if 0 // gp_board static linked with kernel
	/* board core init */
	boardCoreInit();
#endif
	apbdma0Init();
	
	if( gp_ver.major == MACH_GPL32900 )
	{
		i2cInit();
	}

	if( gp_ver.major == MACH_GPL32900B )
	{
		i2cInit();
		ti2cInit();
	}

	timerInit();
	pwmInit();
	adcInit();
	dc2dcInit();	
	
	/* board config and board init */
	boardInit();

	/* display init */
	displayInit();
//	storageInit();
	sysMknod("board", S_IFCHR);
	sysMknod("chunkmem", S_IFCHR);
	mknod("/dev/null", S_IFCHR|0660, makedev(MEM_MAJOR, 3));

	return SP_OK;
}

static UINT32
sysMainStorageInit(
	void
)
{
	if (!strcmp(SYSCONFIG_MAINSTORAGE, "gp_usb_disk")) {
		usbInit();
	}
	else if (!strcmp(SYSCONFIG_MAINSTORAGE, "gp_nand_module") 
		&& insmod("nand_hal") != SP_OK
		&& insmod("gp_nand_module") != SP_OK
		) {
		DIAG_ERROR("insmod main storage %s driver error!\n", SYSCONFIG_MAINSTORAGE);
		return SP_FAIL;
	}
	else if (strcmp(SYSCONFIG_MAINSTORAGE, "None") && insmod(SYSCONFIG_MAINSTORAGE) != SP_OK) {
		DIAG_ERROR("insmod main storage %s driver error!\n", SYSCONFIG_MAINSTORAGE);
		return SP_FAIL;
	}
	return SP_OK;
}

static UINT32
sysInit(
	void
)
{
	/* board dependent modules */
	rtcInit();
	wdtInit();

	inputInit();

	audioInit();
	fmInit();
	lineBufferInit();
	graphicInit();
	scalarInit();
	if (strcmp(SYSCONFIG_PRODUCT, "gplus.microwindowsUI__gplus.evm32900b") != 0) {
		aesInit();
	}
	sdmaInit();
#ifdef SYSCONFIG_ARCH_SPMP8050
	rotatorInit();
#endif
#ifdef SYSCONFIG_ARCH_SPMP8050
	cevaInit();
#endif
	if( gp_ver.major == MACH_GPL32900 )
	{
		cevaInit();
	}
	
	storageInit();

	if (strcmp(SYSCONFIG_MAINSTORAGE, "gp_usb_disk")) {
		usbInit();
	}

	usbWifiInit();
    touchpanelInit();
#ifdef SYSCONFIG_SDIO
	system("modprobe cfg80211");
	system("modprobe mac80211");
	system("modprobe sunrpc");
#endif
	sensorInit();
	ppuInit();
	//tvInit();

	powerInit();
	batteryInit();
	gsensorInit();
	asensorInit();
#ifdef SYSCONFIG_GP_FAST_BOOT
	gpFastBootInit();
#endif /* SYSCONFIG_GP_FAST_BOOT */
	if( gp_ver.major == MACH_GPL32900B )
	{
		On2Init();
		LBPInit();
	}

	return SP_OK;
}

static UINT32
sysShowVersion(
	void
)
{
	printf("\nOpenplatform Revision:%d\n", __PLATFORM_VER__);
	printf("Kernel Revision:%d\n", __KERNEL_VER__);
	printf("Build time:%s %s\n", __DATE__, __TIME__);
	return 0;
}

static UINT32
audioPowerOff(
	void
)
{
	int audioDev;

	audioDev = open("/dev/dsp",O_WRONLY);
	if (audioDev < 0) {
		DIAG_ERROR("Error!! open /dev/dsp\n");
		return SP_FAIL;
	}
	ioctl(audioDev, SNDCTL_DSP_POWEROFF, NULL);
	close(audioDev);

	return SP_OK;
}
int
main(
	int argc,
	char **argv
)
{
	UINT32 ret;
	UINT32 i;

	gp_ver = gpICVersion();

	if (argc < 2) {
#ifndef SYSCONFIG_MODULE_ALLIN_ROOTFS
		ret = sysInit();
#endif
		//sleep(10);
	}
	else {
		for (i=1; i<argc; i++) {
			if (strcmp(argv[i], "backlight_enable") == 0) {
				backlightEnable(1);
			}
			else if (strcmp(argv[i], "backlight_disable") == 0) {
				backlightEnable(0);
			}
			else if (strcmp(argv[i], "core") == 0) {
				sysCoreInit();
#ifdef SYSCONFIG_MODULE_ALLIN_ROOTFS
				sysInit();
#endif
			}
			else if (strcmp(argv[i], "main_storage") == 0) {
				sysMainStorageInit();
			}
			else if (strcmp(argv[i], "version") == 0) {
				sysShowVersion();
			}
			else if (strcmp(argv[i], "audio_power_off") == 0) {
				audioPowerOff();
			}
		}
	}

	return ret;
}
