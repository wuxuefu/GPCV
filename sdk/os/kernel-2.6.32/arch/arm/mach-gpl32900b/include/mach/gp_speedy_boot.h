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
 *  No.19, Industry E. Rd. IV, Hsinchu Science Park,                      *
 *  Hsinchu City 30077, Taiwan, R.O.C                                     *
 *                                                                        *
 **************************************************************************/
/**
 * @file    gp_speedy_boot.h
 * @brief   GPlus Speedy Boot header definition
 * @author  Roger hsu
 * @date    2012-10-24
 */

#ifndef GP_SPEEDY_BOOT_H
#define GP_SPEEDY_BOOT_H

/**************************************************************************
 *                          C O N S T A N T S                          *
 **************************************************************************/
//GP Speedy Boot
#define CHECKSUM_VERIFY			1
#define PARAM_OFFSET		64
#define SIGNATURE_OFFSET	0x00
#define PARAM_PTR_OFFSET	0x04
#define CHECKSUM_OFFSET		0x08

#define CPU_CONTEXT_PHY_ADDR	0x1000
#define SAVED_CONTEXT_PHY_ADDR	(CPU_CONTEXT_PHY_ADDR + 0x1000)
#define CONTEXT_SIZE			0x14000

#define SPEEDY_STATE_NORMAL       0
#define SPEEDY_STATE_SUSPEND      1
#define SPEEDY_STATE_RESUME       2

#define SPEEDY_SHRINK_NONE        0
#define SPEEDY_SHRINK_ALL         1
#define SPEEDY_SHRINK_LIMIT1      2
#define SPEEDY_SHRINK_LIMIT2      3

#if 1
#define SPEEDY_HIBDRV_OFFSET		0x00000000
#else 
//for GPL32900 old code
#define SPEEDY_HIBDRV_VIRT		0xfca00000
#define SPEEDY_HIBDRV_PHYS		0xb2000000
#define SPEEDY_HIBDRV_SIZE		0x00008000
#endif


#define SPEEDY_PFN_IS_NOSAVE

#define SPEEDY_RESERVEAREA_TOP    0x000f0000
#define SPEEDY_RESERVEAREA_END    0x00200000 
  
#define speedy_pfn_valid(pfn)                                             \
   ({                                                                  \
       unsigned long start = PFN_DOWN(SPEEDY_RESERVEAREA_TOP);     \
       unsigned long end = PFN_UP(SPEEDY_RESERVEAREA_END);         \
                                                                       \
       pfn_valid(pfn) && !(pfn >= start && pfn < end);                 \
   }) 

#define SPEEDY_SNAPSHOT_NUM      2
/* dummy info */
#define SPEEDY_BOOTFLAG_AREA0		0x00000000	/* snapshot number */
#define SPEEDY_BOOTFLAG_SIZE0		0x00000000
#define SPEEDY_BOOTFLAG_AREA1		0x00000001	/* snapshot number */
#define SPEEDY_BOOTFLAG_SIZE1		0x00000000
#define SPEEDY_SNAPSHOT_AREA0		0x00000000	/* snapshot number */
#define SPEEDY_SNAPSHOT_SIZE0		0x00000000
#define SPEEDY_SNAPSHOT_AREA1		0x00000001	/* snapshot number */
#define SPEEDY_SNAPSHOT_SIZE1		0x00000000

#define SPEEDY_SAVEAREA                                      \
   /* Save area 0 */                                       \
{                                                          \
   SPEEDY_DEV_EXT,              /* Bootflag dev     */      \
   SPEEDY_BOOTFLAG_AREA0,        /* Bootflag area    */      \
   SPEEDY_BOOTFLAG_SIZE0,        /* Bootflag size    */      \
   SPEEDY_DEV_EXT,              /* Snapshot dev     */      \
   SPEEDY_SNAPSHOT_AREA0,        /* Snapshot area    */      \
   SPEEDY_SNAPSHOT_SIZE0,        /* Snapshot size    */      \
},                                                         \
   /* Save area 1 */                                       \
{                                                          \
   SPEEDY_DEV_EXT,              /* Bootflag dev     */      \
   SPEEDY_BOOTFLAG_AREA1,        /* Bootflag area    */      \
   SPEEDY_BOOTFLAG_SIZE1,        /* Bootflag size    */      \
   SPEEDY_DEV_EXT,              /* Snapshot dev     */      \
   SPEEDY_SNAPSHOT_AREA1,        /* Snapshot area    */      \
   SPEEDY_SNAPSHOT_SIZE1,        /* Snapshot size    */      \
},                                                         \

#define SPEEDY_DEV_SHIFT          16
#define SPEEDY_DEV_MASK           (0xff << SPEEDY_DEV_SHIFT)
#define SPEEDY_DEV_EXT            (0x7f << SPEEDY_DEV_SHIFT)
#define SPEEDY_HIBDRV_VIRT        speedy_hibdrv_buf

/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/
typedef struct speedy_savetbl_s {
    unsigned int start;
    unsigned int end;
}speedy_savetbl_t;

typedef struct speedy_info_s {
	int start_us;
    int mode;
    int compress;
    int oneshot;
    int silent;					//for enable debug message from proc fs
    int halt;
    int stat;					// 0 : hibernate flow, 1 : boot flow
    int retry;
    unsigned int bootflag_dev;
    unsigned int bootflag_area;//management start address
    unsigned int bootflag_size;//management size(1 block)
    unsigned int snapshot_dev;
    unsigned int snapshot_area;//image start address
    unsigned int snapshot_size;//image size
    unsigned int v2p_offset;
    unsigned int lowmem_size;
    int page_shift;
    int zonetbl_num;
    int exttbl_num;
    int dramtbl_num;
    int preload_exttbl;
    speedy_savetbl_t *zonetbl;
    speedy_savetbl_t *exttbl;
    speedy_savetbl_t *dramtbl;
    unsigned int maxarea;
    unsigned int maxsize;
    unsigned int lowmem_maxarea;
    unsigned int lowmem_maxsize;
    unsigned int interface_addr;
    unsigned int checksum;
    unsigned int speedy_buf_addr;	//pass none reserved memory to bootloader
    unsigned int speedy_buf_size;
    unsigned int total_saved_size;
    unsigned int compress_size;
    unsigned int signature;
    int restore;
    int erase_sig;
    int reserve[4];
}speedy_info_t;

struct speedy_savearea {
    unsigned int bootflag_dev;
    unsigned int bootflag_area;
    unsigned int bootflag_size;
    unsigned int snapshot_dev;
    unsigned int snapshot_area;
    unsigned int snapshot_size;
};

struct speedy_ops {
    int (*drv_load)(void *buf, size_t size);
    int (*drv_init)(void);
    int (*device_suspend_early)(void);
    int (*device_suspend)(void);
    int (*pre_snapshot)(void);
    int (*snapshot)(void);
    void (*post_snapshot)(void);
    void (*device_resume)(void);
    void (*device_resume_late)(void);
    void (*drv_uninit)(void);
    void (*putc)(char c);
    void (*progress)(int val);
    int (*erase_signature)(int index);
};


/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
enum speedy_progress {
    SPEEDY_PROGRESS_INIT,
    SPEEDY_PROGRESS_SYNC,
    SPEEDY_PROGRESS_FREEZE,
    SPEEDY_PROGRESS_SHRINK,
    SPEEDY_PROGRESS_SUSPEND,
    SPEEDY_PROGRESS_SAVE,
    SPEEDY_PROGRESS_SAVEEND,
    SPEEDY_PROGRESS_RESUME,
    SPEEDY_PROGRESS_THAW,
    SPEEDY_PROGRESS_EXIT,
    SPEEDY_PROGRESS_CANCEL,
};

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/
extern int pm_device_down;
extern int speedy_shrink;
extern int speedy_swapout_disable;
extern int speedy_separate_pass;
extern int speedy_canceled;
extern speedy_info_t speedy_param;
extern unsigned char *speedy_hibdrv_buf;

/**************************************************************************
 *               F U N C T I O N    D E C L A R A T I O N S               *
 **************************************************************************/
int speedy_set_savearea(unsigned long start, unsigned long end);
void speedy_save_cancel(void);

#ifdef CONFIG_PM_SPEEDY_DEBUG
int speedy_printf(const char *fmt, ...);
void speedy_putc(char c);
#else
#define speedy_printf(fmt...)
#define speedy_putc(c)
#endif

//void dumpbuf(unsigned char *buf,int size);

int swsusp_page_is_saveable(struct zone *zone, unsigned long pfn);
int hibdrv_snapshot(void);
int speedy_register_machine(struct speedy_ops *ops);
int speedy_unregister_machine(struct speedy_ops *ops);

#if 0 //def CONFIG_MTD
int speedy_mtd_load(int mtdno, void *buf, size_t size);
int speedy_mtd_load_nm(const char *mtdname, void *buf, size_t size);
#endif

#endif  /* GP_SPEEDY_BOOT_H */
