#include <mach/gp_tv.h>

//  ypbpr mode
#define TV_480I                         0
#define TV_720P                         1

//  nTvStd
#define TVSTD_NTSC_M                    0
#define TVSTD_NTSC_J                    1
#define TVSTD_NTSC_N                    2
#define TVSTD_PAL_M                     3
#define TVSTD_PAL_B                     4
#define TVSTD_PAL_N                     5
#define TVSTD_PAL_NC                    6
#define TVSTD_NTSC_J_NONINTL            7
#define TVSTD_PAL_B_NONINTL             8
//  nResolution
#define TV_QVGA                         0
#define TV_HVGA                         1
#define TV_D1                           2
#define TV_HVGA_576                     3
#define TV_D1_576                       4
//  nNonInterlace
#define TV_INTERLACE                    0
#define TV_NON_INTERLACE                1

extern void tv1_init(void);
extern void tv1_start(signed int nTvStd, signed int nResolution, signed int nNonInterlace);
extern void tv1_color_set(BUFFER_COLOR_FORMAT buffer_color_mode);
extern void tv1_reverse_set(signed int mode);
extern void tv1_buffer_set(unsigned int buffer_ptr);
extern signed int tv1_buffer_ck(void);
extern void tv1_ypbpr_set(unsigned int mode,unsigned enable);
extern signed int tv1_irq_wait_poll(unsigned int Timeout_sec,unsigned int Timeout_usec);
