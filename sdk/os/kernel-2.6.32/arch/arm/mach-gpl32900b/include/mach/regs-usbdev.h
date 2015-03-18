/*
 * arch/arm/mach-spmp8000/include/mach/regs-usbdev.h
 *
 * Copyright (C) 2013 GeneralPlus
 *
 * USB device - USB device regsters.
 *
 */

#define USB_NAK_EP_EP0 		0x0
#define USB_NAK_EP_BULK_IN 	0x1
#define USB_NAK_EP_BULK_OUT 0x2

#define USB_ACK_EP_EP0 		0x0
#define USB_ACK_EP_BULK_IN 	0x1
#define USB_ACK_EP_BULK_OUT 0x2

#define UDC_BASE			IO3_ADDRESS(0x6000)

#define  UDC_DMA_CS	 	(*(volatile unsigned *)(UDC_BASE+0x000))
#define  UDC_DMA_DA		(*(volatile unsigned *)(UDC_BASE+0x004))

//#define  UDC_DMA_ND		(*(volatile unsigned *)(UDC_BASE+0x00C))
//#define  UDC_DMA_CDA		(*(volatile unsigned *)(UDC_BASE+0x014))
#define  UDC_CS				(*(volatile unsigned *)(UDC_BASE+0x080))
#define  UDC_IE				(*(volatile unsigned *)(UDC_BASE+0x084))	  
#define  UDC_IF				(*(volatile unsigned *)(UDC_BASE+0x088))	
#define  UDC_CIS  			(*(volatile unsigned *)(UDC_BASE+0x08C))

#define  UDC_EP5CTL				(*(volatile unsigned *)(UDC_BASE+0x0140))
#define  UDC_EP5HDLEN			(*(volatile unsigned *)(UDC_BASE+0x0144))
#define  UDC_EP5HDCTRL			(*(volatile unsigned *)(UDC_BASE+0x014C))
#define  UDC_EP5FRAMCTL			(*(volatile unsigned *)(UDC_BASE+0x0148))
#define  UDC_EP5EN				(*(volatile unsigned *)(UDC_BASE+0x0150))
#define  UDC_DVIDEO_REFCLOCK	(*(volatile unsigned *)(UDC_BASE+0x0200))
#define  UDC_LLCFS				(*(volatile unsigned *)(UDC_BASE+0x0320))																																																																																																																																																																														
#define  UDC_MSTC     			(*(volatile unsigned *)(UDC_BASE+0x0328))																																																																																																																																																																														
#define  UDC_EP12C      		(*(volatile unsigned *)(UDC_BASE+0x0330))																																																																																																																																																																														
#define  UDC_EP12PPC			(*(volatile unsigned *)(UDC_BASE+0x0334))																																																																																																																																																																														
#define  UDC_EP0SCS  			(*(volatile unsigned *)(UDC_BASE+0x0344))																																																																																																																																																																														
#define  UDC_EP0SDP   			(*(volatile unsigned *)(UDC_BASE+0x0348))																																																																																																																																																																														
#define  UDC_EP0CS    			(*(volatile unsigned *)(UDC_BASE+0x034C))																																																																																																																																																																														
#define  UDC_EP0DC      		(*(volatile unsigned *)(UDC_BASE+0x0350))																																																																																																																																																																														
#define  UDC_EP0DP      		(*(volatile unsigned *)(UDC_BASE+0x0354))																																																																																																																																																																														
#define  UDC_EP1SCS    			(*(volatile unsigned *)(UDC_BASE+0x0358))																																																																																																																																																																														
#define  UDC_EP1SDP    			(*(volatile unsigned *)(UDC_BASE+0x035C))																																																																																																																																																																																											
#define  UDC_EP12FS    			(*(volatile unsigned *)(UDC_BASE+0x0364))																																																																																																																																																																																											
#define  UDC_EP12FCL 			(*(volatile unsigned *)(UDC_BASE+0x0368))																																																																																																																																																																																																							
#define  UDC_EP12FCH 			(*(volatile unsigned *)(UDC_BASE+0x036C))																																																																																																																																																																																																							
                        																														
#define  UDC_EP12FDP  			(*(volatile unsigned *)(UDC_BASE+0x0370))																														
#define  UDC_EP3CS      		(*(volatile unsigned *)(UDC_BASE+0x0374))																														
#define  UDC_EP3DC      		(*(volatile unsigned *)(UDC_BASE+0x0378))																														
#define  UDC_EP3DP      		(*(volatile unsigned *)(UDC_BASE+0x037C))																														
#define  UDC_EP0ONKC  			(*(volatile unsigned *)(UDC_BASE+0x0384))																														
#define  UDC_EP0INAKCN  		(*(volatile unsigned *)(UDC_BASE+0x0388))																														
#define  UDC_EP1INAKCN  		(*(volatile unsigned *)(UDC_BASE+0x038C))																														
#define  UDC_EP2INAKCN  		(*(volatile unsigned *)(UDC_BASE+0x0390))																														
#define  UDC_LLCSET0  			(*(volatile unsigned *)(UDC_BASE+0x03B0))																														
#define  UDC_LLCSET1 			(*(volatile unsigned *)(UDC_BASE+0x03B4))																														
#define  UDC_LLCS    			(*(volatile unsigned *)(UDC_BASE+0x03B8))																														
#define  UDC_LLCSTL 			(*(volatile unsigned *)(UDC_BASE+0x03BC))																														
#define  UDC_LLCSET2			(*(volatile unsigned *)(UDC_BASE+0x03C0))																														
#define  UDC_LCS3				(*(volatile unsigned *)(UDC_BASE+0x03C8))																														
#define  USBPATTERN_GEN			(*(volatile unsigned *)(UDC_BASE+0x03F0))																														
#define  UDC_LCADDR	   			(*(volatile unsigned *)(UDC_BASE+0x03F4))																														
                        																														
#define  UDC_LLCIF				(*(volatile unsigned *)(UDC_BASE+0x0400))																														
#define  UDC_LLCIE				(*(volatile unsigned *)(UDC_BASE+0x0404))																														
#define  UDC_LLCIS				(*(volatile unsigned *)(UDC_BASE+0x0408))																														

#define  UDC_DMA_CS_OFST			0x000
#define  UDC_DMA_DA_OFST			0x004
#define  UDC_DMA_AC_OFST			0x008
#define  UDC_DMA_ADS_OFST			0x00C
#define  UDC_DMA_CDA_OFST			0x014
#define  UDC_CS_OFST				0x080
#define  UDC_IE_OFST				0x084
#define  UDC_IF_OFST				0x088
#define  UDC_CIS_OFST				0x08C
#define  UDC_EP5CTL_OFST			0x140
#define  UDC_EP5HDLEN_OFST      	0x144
#define  UDC_EP5FRAMCTL_OFST		0x148
#define  UDC_EP5HDCTRL_OFST     	0x14C
#define  UDC_EP5EN_OFST				0x150
#define  UDC_EP5RPTR_OFST			0x154
#define  UDC_EP5WPTR_OFST			0x158
#define  UDC_EP5DMAEN_OFST			0x160
#define  UDC_EP7CTL_OFST			0x1C0
#define  UDC_EP7RPTR_OFST			0x1C4
#define  UDC_EP7WPTR_OFST			0x1C8
#define  UDC_EP7DMA_OFST			0x008
#define  UDC_DVIDEO_REFCLOCK_OFST	0x200


#define  UDC_LLCFS_OFST			0x0320
#define  UDC_MSTC_OFST			0x0328
#define  UDC_EP12C_OFST			0x0330
#define  UDC_EP12PPC_OFST		0x0334
#define  UDC_EP0DPVALID_OFST    0x0340
#define  UDC_EP0SCS_OFST		0x0344
#define  UDC_EP0SDP_OFST		0x0348
#define  UDC_EP0CS_OFST			0x034C
#define  UDC_EP0DC_OFST			0x0350
#define  UDC_EP0DP_OFST			0x0354
#define  UDC_EP1SCS_OFST		0x0358
#define  UDC_EP1SDP_OFST		0x035C
#define  UDC_EP12FS_OFST		0x0364
#define  UDC_EP12FCL_OFST		0x0368
#define  UDC_EP12FCH_OFST		0x036C
#define  UDC_EP12FDP_OFST		0x0370
#define  UDC_EP3CS_OFST			0x0374
#define  UDC_EP3DC_OFST			0x0378
#define  UDC_EP3DP_OFST			0x037C
#define  UDC_EP0ONKC_OFST		0x0384
#define  UDC_EP0INAKCN_OFST		0x0388
#define  UDC_EP1INAKCN_OFST		0x038C
#define  UDC_EP2INAKCN_OFST		0x0390
#define  UDC_EP12VALID_OFST 	0x0398
#define  UDC_EP12POFDLB_OFST 	0x039C
#define  UDC_EP12POFDHB_OFST 	0x03A0
#define  UDC_LLCSET0_OFST		0x03B0
#define  UDC_LLCSET1_OFST		0x03B4
#define  UDC_LLCS_OFST			0x03B8
#define  UDC_LLCSTL_OFST		0x03BC
#define  UDC_LLCSET2_OFST		0x03C0
#define  UDC_LCS3_OFST			0x03C8
#define  USBPATTERN_GEN_OFST	0x03F0
#define  UDC_LLCIF_OFST			0x0400
#define  UDC_LLCIE_OFST			0x0404
#define  UDC_LLCIS_OFST			0x0408
#define  UDC_SREQIF_OFST		0x0410
#define  UDC_SREQIE_OFST		0x0414

/* DMA_CS */
#define  DMACS_ONWER_DMA		0x80000000  /*other is CPU*/

#define  DMACS_BURST_FIXADDR	0x00000000 
#define  DMACS_BURST_INCADDR	0x02000000
#define  DMACS_BURST_MASK       0x30000000

#define  DMACS_NO_EOF           0x08000000

#define  DMACS_EOF_DISABLE      0x04000000

#define  DMACS_RESPONSE_OK      0x00000000
#define  DMACS_RESPONSE_EXOK    0x0100000
#define  DMACS_RESPONSE_SLVERR  0x02000000
#define  DMACS_RESPONSE_DECERR  0x03000000
#define  DMACS_RESPONSE_MASK    0x03000000

//#define  DMACS_DMA_EN         0x00800000
#define  DMACS_DMA_EN           0x80000000


#define  DMACS_DMA_MODE_DATA		0x00000000
#define  DMACS_DMA_MODE_SCATTER		0x00400000

#define  DMACS_DMA_READ				0x00000000
//#define  DMACS_DMA_WRITE      	0x00200000
#define  DMACS_DMA_WRITE          	0x10000000


#define  DMACS_DMA_END             	0x00100000

//#define  DMACS_DMA_FLUSH         	0x00080000
#define  DMACS_DMA_FLUSH			0x20000000


//#define  DMACS_DMA_FLUSHEND  		0x00040000
#define  DMACS_DMA_FLUSHEND     	0x40000000

#define DMA_IF_BIT_UDC (0x01<<25)		
#define  DMACS_DMA_BYTECNT_MASK 0x0000FFFF

/* DMA_DA */
/* CIS */
#define CIS_FDISCONN_IF         0x08000000 /*Force USB Disconnect INT*/
#define CIS_DMA_IF              0x02000000 /*DMA finish INT*/
#define CIS_FCONN_IF            0x04000000 /*Force USB Connect INT*/
#define CIS_VBUS_IF             0x01000000 /*VBUS INT*/
//#define CIS_VBUS_STATUS    		0x00000001

/* DMA_ND */
#define DMAND_NDADDR_MASK 		0xFFFFFFFC
#define DMAND_AUTO_ND			0x00000001

/* DMA_CDA */
/* CIE */
#define CIE_FDISCONN_IE  	  0x08000000 /*Force USB Disconnect INT*/      
#define CIE_FCONN_IE          0x04000000 /*DMA finish INT*/
#define CIE_DMA_IE            0x02000000 /*Force USB Connect INT*/
#define CIE_VBUS_IE           0x01000000 /*VBUS INT*/

/* CCS */
#define UDLC_WAKEUP           0x80000000       
#define UDLC_FCONN            0x40000000
#define UDLC_FDISCONN         0x20000000
#define UDLC_SUSPEND_BLK  	  0x10000000
#define VBUS_SAMP_PERIOD_MASK  0x0000FFFF

/* LCIE */
#define UDLC_RESETN_IE 		0x80000 /*?? Reset release Int*/
#define UDLC_SCONF_IE 		0x40000 /*Set Configuration Int*/
#define UDLC_RESUME_IE 		0x20000 /*Resume Int*/
#define UDLC_SUSPEND_IE 	0x10000 /*Suspend Int*/
#define UDLC_DMA_IE 		0x08000 /*DMA data finish Int*/
#define UDLC_EP3I_IE		0x04000 /*EP3 In transaction Int*/
#define UDLC_PIPO_IE		0x02000 /*Ping-pong fifo swap Int*/
#define UDLC_HCS_IE			0x01000 /*Clear Stall Int*/
#define UDLC_EP2N_IE		0x00800 /*EP2 NAK Int*/
#define UDLC_EP1N_IE		0x00400 /*EP1 NAK Int*/
#define UDLC_EP0N_IE		0x00200 /*EP0 NAK Int*/
#define UDLC_HSS_IE			0x00100 /*Stall Int*/
#define UDLC_EP2O_IE		0x00080 /*EP2 Out Int*/
#define UDLC_EP1I_IE		0x00040 /*EP1 In Int*/
#define UDLC_EP1SI_IE		0x00020 /*??*/
#define UDLC_EP0I_IE		0x00010 /*EP0 In Int*/
#define UDLC_EP0O_IE		0x00008 /*EP0 Out Int*/
#define UDLC_EP0S_IE		0x00004 /*EP0 Setup Int*/
#define UDLC_ESUSP_IE		0x00002 /*Suspend Int*/
#define UDLC_RESET_IE		0x00001 /*Reset Int*/

/* LCIS */
#define UDLC_RESETN_IF 		0x80000
#define UDLC_SCONF_IF 		0x40000
#define UDLC_RESUME_IF 		0x20000
#define UDLC_SUSPEND_IF 	0x10000
#define UDLC_DMA_IF 		0x08000
#define UDLC_EP3I_IF		0x04000
#define UDLC_PIPO_IF		0x02000
#define UDLC_HCS_IF			0x01000
#define UDLC_EP2N_IF		0x00800
#define UDLC_EP1N_IF		0x00400
#define UDLC_EP0N_IF		0x00200
#define UDLC_HSS_IF			0x00100
#define UDLC_EP2O_IF		0x00080
#define UDLC_EP1I_IF		0x00040
#define UDLC_EP1SI_IF		0x00020
#define UDLC_EP0I_IF		0x00010
#define UDLC_EP0O_IF		0x00008
#define UDLC_EP0S_IF		0x00004 
#define UDLC_ESUSP_IF		0x00002
#define UDLC_RESET_IF		0x00001

/* LCIF */
#define UDLC_RESETN_IS 		0x80000
#define UDLC_SCONF_IS 		0x40000
#define UDLC_RESUME_IS 		0x20000
#define UDLC_SUSPEND_IS 	0x10000
#define UDLC_DMA_IS 		0x08000
#define UDLC_EP3I_IS		0x04000
#define UDLC_PIPO_IS		0x02000
#define UDLC_HCS_IS			0x01000
#define UDLC_EP2N_IS		0x00800
#define UDLC_EP1N_IS		0x00400
#define UDLC_EP0N_IS		0x00200
#define UDLC_HSS_IS			0x00100
#define UDLC_EP2O_IS		0x00080
#define UDLC_EP1I_IS		0x00040
#define UDLC_EP1SI_IS		0x00020
#define UDLC_EP0I_IS		0x00010
#define UDLC_EP0O_IS		0x00008
#define UDLC_EP0S_IS		0x00004 
#define UDLC_ESUSP_IS		0x00002
#define UDLC_RESET_IS		0x00001

/* LCFS */
#define CLR_EP2_OVLD      0x00020
#define SET_EP1_IVLD      0x00010
#define MSDC_CMD_VLD      0x00008
#define CUR_FIFO_EMPTY 	  0x00004
#define LCFS_EP2_OVLD     0x00002
#define LCFS_EP1_IVLD     0x00001
/* MSTC */
#define CP_CBW_TAG_MASK 0xFF

/* EP12C */
#define EP12C_MSDC_CMD_VLD      0x80
#define EP12C_EP2_OVLD          0x40
#define EP12C_EP1_IVLD          0x20
#define EP12C_SET_EP1_IVLD      0x10
#define EP12C_CLR_EP2_OVLD      0x08
#define EP12C_RESET_PIPO        0x04
#define EP12C_ENABLE_BULK       0x02
#define EP12C_DIR_IN            0x01

/* EP12PPC */
#define EP12PPC_N_EP2_OVLD		0x80
#define EP12PPC_P_EP1_IVLD		0x40
#define EP12PPC_EP2_OVLD		0x20
#define EP12PPC_EP1_IVLD		0x10
#define EP12PPC_DIR_IN			0x08
#define EP12PPC_CURR_BUF_NUM	0x04
#define EP12PPC_SW_BUF       	0x02
#define EP12PPC_AUTO_SW_EN      0x01

/* EP0SCS */
#define EP0SCS_SFIO_UPDATE	0x40
#define EP0SCS_SFIFO_VALID	0x20

/* EP0SDP */
#define EP0SDP_SET_DATA_MSK   0xFF       

/* EP0CS */
#define EP0CS_OUT_EMPTY		0x80  
#define EP0CS_OVLD			0x40
#define EP0CS_CLR_EP0_OVLD  0x20
#define EP0CS_IVLD			0x10
#define EP0CS_SET_EP0_IVLD	0x08
#define EP0CS_SFIFO_UPDATE	0x04
#define EP0CS_SFIFO_VALID	0x02
#define EP0CS_DIR_IN		0x01

/* EP0DC */
#define EP0_DATA_CNTR_MASK	0x7F

/* EP1SCS */
#define EP1SCS_FIFO_CNTR_MSK	0xF0
#define EP1SCS_CLR_IVLD			0x08
#define EP1SCS_RESET_FIFO		0x04
#define EP1SCS_IVLD				0x02
#define EP1SCS_SET_IVLD			0x01
/* EP1SDP */
/* EP12FS */
#define  EP12FS_N_MSDC_CMD		0x80
#define  EP12FS_A_FIFO_EMPTY	0x40
#define  EP12FS_N_EP2_OVLD		0x20
#define  EP12FS_P_EP1_IVLD		0x10
#define  EP12FS_MSDC_CMD_VLD	0x08
#define  EP12FS_FIFO_EMPTY		0x04
#define  EP12FS_EP2_OVLD        0x02
#define  EP12FS_EP1_IVLD        0x01

/* EP12FCL */
#define  EP12FCL_FIFO_CNTRL_MASK	0xFF

/* EP12FCH */
#define  EP12FCH_RESET_CNTR			0x04
#define  EP12FCH_FIFO_CNTRH_MASK	0x03
/* EP12FDP */
/* EP3CS */
#define EP3CS_IVLD        0x08
#define EP3CS_CLR_IVLD    0x04
#define EP3CS_SET_IVLD    0x02
#define EP3CS_IN_EN       0x01

/* EP3DC */
#define EP3_DATA_CNTR_MASK  0x7F
/* EP3DP */

/* EP0ONAKCN */
#define EP0_OUT_NAK_CNT_MASK 0xFF
/* EP0INAKCN */
#define EP0_IN_NAK_CNT_MASK 0xFF
/* EP1INAKCN */
#define EP1_IN_NAK_CNT_MASK 0xFF
/* EP2INAKCN */
#define EP2_OUT_NAK_CNT_MASK 0xFF

/* LCSET0 */
#define LCSET0_CLR_SUSP_CNT		0x80
#define LCSET0_SIM_MODE			0x40
#define LCSET0_DISC_SUSP_EN		0x20
#define LCSET0_CPU_WKUP_EN		0x10
#define LCSET0_PWR_PART_N      	0x08
#define LCSET0_PWR_SUSP_N      	0x04
#define LCSET0_ISSUE_RESUME   	0x02
#define LCSET0_SOFT_DISC        0x01
/* LCSET1 */
#define LCSET0_NO_SET_SUSP_OPT		0x80
#define LCSET0_NO_STOP_CHIRP        0x40
#define LCSET0_INTER_PACKET_DLY     0x20
#define LCSET0_FORCE_FULLSP         0x10
#define LCSET0_VBUS_LOW_AUTO_DISC  	0x08
#define LCSET0_DISC_AUTO_DPDMPD     0x04
#define LCSET0_SUPP_RWAKE         	0x02
#define LCSET0_SELF_POWER         	0x01

/*LCS*/
#define LCS_DISC_CONN_STATUS			0x80
#define LCS_HOST_CONFIGED				0x40
#define LCS_LNK_SUSP					0x20
#define LCS_HOST_ALLOW_RWAKE			0x10
#define LCS_CURR_LINESTATE_SE0         	0x00
#define LCS_CURR_LINESTATE_F_J         	0x04
#define LCS_CURR_LINESTATE_F_K			0x08
#define LCS_CURR_LINESTATE_H_SQUELCH	0x04
#define LCS_CURR_LINESTATE_MASK 		0xC0
#define LCS_CURR_SPEED_F       			0x02                         
#define LCS_VBUS_HIGH                   0x01

/* LCSTL */
#define LCSTL_CLREP3STL   0x80
#define LCSTL_CLREP2STL   0x40
#define LCSTL_CLREP1STL   0x20
#define LCSTL_CLREP0STL   0x10
#define LCSTL_SETEP3STL   0x08
#define LCSTL_SETEP2STL   0x04
#define LCSTL_SETEP1STL   0x02
#define LCSTL_SETEP0STL   0x01

/*LCSET2 */
#define LCSET2_SUSP_REF  		0x8
#define LCSET2_USBC_EP1S_FST  	0x4
#define LCSET2_USBC_STOP_SPEED  0x2
#define LCSET2_USBC_SUPP_EP3  	0x1

/* Data port valid byte */
#define DATA_PORT_4BYTE_VALID   0x0F
#define DATA_PORT_3BYTE_VALID   0x07
#define DATA_PORT_2BYTE_VALID   0x03
#define DATA_PORT_1BYTE_VALID   0x01

enum
{
	USB_STATE_CBW = 0,
	USB_STATE_BULKIN,
    USB_STATE_BULKOUT,
    USB_STATE_CSW,
};
/* LCS2 */
/* PATTER*/

/* For new S222 USB Device Controller driver, Eugene, 2013/11/12 */
#define		BIT0		0x00000001
#define		BIT1		0x00000002
#define		BIT2		0x00000004
#define		BIT3		0x00000008
#define		BIT4		0x00000010
#define		BIT5		0x00000020
#define		BIT6		0x00000040
#define		BIT7		0x00000080
#define		BIT8		0x00000100
#define		BIT9		0x00000200
#define		BIT10		0x00000400
#define		BIT11		0x00000800
#define		BIT12		0x00001000
#define		BIT13		0x00002000
#define		BIT14		0x00004000
#define		BIT15		0x00008000
#define		BIT16		0x00010000
#define		BIT17		0x00020000
#define		BIT18		0x00040000
#define		BIT19		0x00080000
#define		BIT20		0x00100000
#define		BIT21		0x00200000
#define		BIT22		0x00400000
#define		BIT23		0x00800000
#define		BIT24		0x01000000
#define		BIT25		0x02000000
#define		BIT26		0x04000000
#define		BIT27		0x08000000
#define		BIT28		0x10000000
#define		BIT29		0x20000000
#define		BIT30		0x40000000
#define		BIT31		0x80000000

/********************* Define UDC_DMA_CS_OFST bit mask (Offset + 0x000, USB device EP12 DMA control status) *****************/
#define MASK_USBD_EP12_DMA_EN				BIT31
#define MASK_USBD_EP12_DMA_FLUSH			BIT30
#define MASK_USBD_EP12_DMA_FIFO_FLUSH		BIT29
#define MASK_USBD_EP12_DMA_WRITE			BIT28
#define MASK_USBD_EP12_DMA_MODIFY_EN		BIT27
#define MASK_USBD_EP12_DMA_COUNT_ALIGN		BIT26

/********************* Define UDC_EP7DMA_OFST bit mask (Offset + 0x008, USB device EP7 DMA control) *****************/
#define MASK_USBD_EP7_DMA_EN				BIT31
#define MASK_USBD_EP7_DMA_FLUSH				BIT30

/********************* Define UDC_CS_OFST bit mask (Offset + 0x080, USB device controller control status) **/
#define MASK_USBD_UDCS_DISCONNECT			BIT31	/* Force USB PHY disconnect from USB bus */
#define MASK_USBD_UDCS_USB_CLK_EN			BIT30	/* USB clock enable status signal */
#define MASK_USBD_UDCS_PARTIAL				BIT29	/* USB PHY partial control signal, 0: control by suspend signal, 1: clock enable */
#define MASK_USBD_UDCS_SUSPENDM				BIT28	/* USB PHY suspend control signal */
#define MASK_USBD_UDCS_FORCE_DISCONNECT		BIT27	/* Write 1 to force USB bus disconnect */
#define MASK_USBD_UDCS_FORCE_CONNECT		BIT26	/* Write 1 to force USB bus connect */
#define MASK_USBD_UDCS_PHYCLK_SEL			BIT25	/* 1: slow clk, 0: phy clk */
#define MASK_USBD_UDCS_PHYCLK_READY			BIT24	/* phy clk is ready(whole OP input) */
#define MASK_USBD_UDCS_VBUS_PRE				BIT17	/* USB VBUS signal sync to system domain */
#define MASK_USBD_UDCS_VBUS					BIT16	/* USB VBUS */

/********************* Define UDC_IE_OFST bit mask (Offset + 0x084, USB device controller interrupt enable) **/
#define MASK_USBD_UDC_IE_EPAB_DMA			BIT30	/* EPAB DMA finish interrupt enable */
#define MASK_USBD_UDC_IE_EP89_DMA			BIT29	/* EP89 DMA finish interrupt enable */
#define MASK_USBD_UDC_IE_AUDIO_DMA			BIT28	/* AUDIO DMA finish interrupt enable */
#define MASK_USBD_UDC_IE_FORCE_DISC			BIT27	/* Force disconnect finish interrupt enable */
#define MASK_USBD_UDC_IE_FORCE_CONN			BIT26	/* Force connect finish interrupt enable */
#define MASK_USBD_UDC_IE_DMA				BIT25	/* DMA interrupt enable */
#define MASK_USBD_UDC_IE_VBUS				BIT24	/* VBUS interrupt enable */
#define MASK_USBD_UDC_IE_RESETN				BIT19	/* UDC USB_RESET END interrupt enable */
#define MASK_USBD_UDC_IE_SCONF				BIT18	/* UDC HOST set configuration interrupt enable */
#define MASK_USBD_UDC_IE_RESUME				BIT17	/* UDC BUS RESUME interrupt enable */
#define MASK_USBD_UDC_IE_SUSPEND			BIT16	/* UDC BUS SUSPEND interrupt enable */
#define MASK_USBD_UDC_IE_EP1INN				BIT15	/* UDC DMA & EP1_IN END interrupt enable */
#define MASK_USBD_UDC_IE_EP3I				BIT14	/* UDC EP3_IN transaction interrupt enable */
#define MASK_USBD_UDC_IE_PIPO				BIT13	/* UDC PING-PONG_FIFO_SWAP interrupt enable */
#define MASK_USBD_UDC_IE_HCS				BIT12	/* UDC HOST clear stall interrupt enable */
#define MASK_USBD_UDC_IE_EP2N				BIT11	/* UDC EP2_NAK interrupt enable */
#define MASK_USBD_UDC_IE_EP1N				BIT10	/* UDC EP1_NAK interrupt enable */
#define MASK_USBD_UDC_IE_EP0N				BIT9	/* UDC EP0_NAK interrupt enable */
#define MASK_USBD_UDC_IE_HSS				BIT8	/* UDC HOST_SET_STALL interrupt enable */
#define MASK_USBD_UDC_IE_EP2O				BIT7	/* UDC EP2_OUT transaction interrupt enable */
#define MASK_USBD_UDC_IE_EP1I				BIT6	/* UDC EP1_IN transaction interrupt enable */
#define MASK_USBD_UDC_IE_EP1SI				BIT5	/* UDC EP1S_IN transaction interrupt enable */
#define MASK_USBD_UDC_IE_EP0I				BIT4	/* UDC EP0_OUT transaction interrupt enable */
#define MASK_USBD_UDC_IE_EP0O				BIT3	/* UDC EP0_IN transaction interrupt enable */
#define MASK_USBD_UDC_IE_EP0S				BIT2	/* UDC EP0_SETUP transaction interrupt enable */
#define MASK_USBD_UDC_IE_SUSP				BIT1	/* UDC USB_SUSPEND_DIFF interrupt enable */
#define MASK_USBD_UDC_IE_RESET				BIT0	/* UDC USB_RESET interrupt enable */

/********************* Define UDC_IF_OFST bit mask (0x088, USB device controller interrupt flag) ******/
#define MASK_USBD_UDC_IF_EPAB_DMA			BIT30	/* EPAB DMA finish interrupt flag */
#define MASK_USBD_UDC_IF_EP89_DMA			BIT29	/* EP89 DMA finish interrupt flag */
#define MASK_USBD_UDC_IF_AUDIO_DMA			BIT28	/* AUDIO DMA finish interrupt flag */
#define MASK_USBD_UDC_IF_FORCE_DISC			BIT27	/* Force Disconnect finish flag, write 1 clear */
#define MASK_USBD_UDC_IF_FORCE_CONN			BIT26	/* Force connect finish flag, write 1 clear */
#define MASK_USBD_UDC_IF_DMA				BIT25	/* DMA interrupt flag, write 1 clear */
#define MASK_USBD_UDC_IF_VBUS				BIT24	/* USB VBUS toggle interrupt flag, write 1 clear */
#define MASK_USBD_UDC_IF_RESETN				BIT19	/* UDC USB_RESET END interrupt flag */
#define MASK_USBD_UDC_IF_SCONF				BIT18	/* UDC HOST Set Configuration interrupt flag */
#define MASK_USBD_UDC_IF_RESUME				BIT17	/* UDC USB BUS RESUME interrupt flag */
#define MASK_USBD_UDC_IF_SUSPEND			BIT16	/* UDC USB BUS SUSPEND interrupt flag */
#define MASK_USBD_UDC_IF_EP1INN				BIT15	/* UDC DMA & EP1_IN END interrupt flag */
#define MASK_USBD_UDC_IF_EP3I				BIT14	/* UDC EP3_IN transaction interrupt flag */
#define MASK_USBD_UDC_IF_PIPO				BIT13	/* UDC PING-PONG_FIFO_SWAP interrupt flag */
#define MASK_USBD_UDC_IF_HCS				BIT12	/* UDC HOST_CLEAR_STALL interrupt flag */
#define MASK_USBD_UDC_IF_EP2N				BIT11	/* UDC EP2_NAK interrupt flag */
#define MASK_USBD_UDC_IF_EP1N				BIT10	/* UDC EP1_NAK interrupt flag */
#define MASK_USBD_UDC_IF_EP0N				BIT9	/* UDC EP0_NAK interrupt flag */
#define MASK_USBD_UDC_IF_HSS				BIT8	/* UDC HOST_SET_STALL interrupt flag */
#define MASK_USBD_UDC_IF_EP2O				BIT7	/* UDC EP2_OUT transaction interrupt flag */
#define MASK_USBD_UDC_IF_EP1I				BIT6	/* UDC EP1_IN transaction interrupt flag */
#define MASK_USBD_UDC_IF_EP1SI				BIT5	/* UDC EP1S_IN transaction interrupt flag */
#define MASK_USBD_UDC_IF_EP0I				BIT4	/* UDC EP0_OUT transaction interrupt flag */
#define MASK_USBD_UDC_IF_EP0O				BIT3	/* UDC EP0_IN transaction interrupt flag */
#define MASK_USBD_UDC_IF_EP0S				BIT2	/* UDC EP0_SETUP transaction interrupt flag */
#define MASK_USBD_UDC_IF_SUSP				BIT1	/* UDC USB_SUSPEND_DIFF interrupt flag */
#define MASK_USBD_UDC_IF_RESET				BIT0	/* UDC USB_RESET interrupt flag */

/********************* Define UDC_EP5CTL_OFST bit mask (Offset + 0x140, USB device EP5 control) *****************/
#define MASK_USBD_EP5_FLUSH					BIT1
#define MASK_USBD_EP5_CTL_EN				BIT0

/********************* Define rEP5FRAMCTL bit mask (Offset + 0x148, USB device EP5 control) *****************/
#define MASK_USBD_EP5_CTL_EMPTY				BIT7

/********************* Define UDC_EP5EN_OFST bit mask (Offset + 0x150, USB device EP5 enable control) *****************/
#define MASK_USBD_EP5_EN					BIT0

/********************* Define UDC_EP5DMAEN_OFST bit mask (Offset + 0x160, USB device EP5 DMA control) *****************/
#define MASK_USBD_EP5_DMA_EN				BIT0

/********************* Define UDC_EP7CTL_OFST bit mask (Offset + 0x1C0, USB device EP7 control) *****************/
#define MASK_USBD_EP7_BUF_FLUSH				BIT7	/* write 1 and write 0 manually, not auto */
#define MASK_USBD_EP7_CTL_VLD				BIT3
#define MASK_USBD_EP7_CTL_EN				BIT0

/********************* Define UDC_EP12C_OFST bit mask (Offset + 0x330, USB device endpoint1/2 control) **********/
#define MASK_USBD_EP12_C_MSDC_CMD_VLD		BIT7
#define MASK_USBD_EP12_C_EP2_OVLD			BIT6
#define MASK_USBD_EP12_C_EP1_IVLD			BIT5
#define MASK_USBD_EP12_C_SET_EP1_IVLD		BIT4
#define MASK_USBD_EP12_C_CLR_EP2_OVLD		BIT3
#define MASK_USBD_EP12_C_RESET_PIPO_FIFO	BIT2
#define MASK_USBD_EP12_C_EP12_ENA			BIT1
#define MASK_USBD_EP12_C_EP12_DIR			BIT0

/********************* Define UDC_EP12PPC_OFST bit mask (Offset + 0x334, USB device endpoint1/2 Ping-Pong FIFO control) **********/
#define MASK_USBD_EP12_FIFO_C_A_EP2_OVLD	BIT7
#define MASK_USBD_EP12_FIFO_C_A_EP1_IVLD	BIT6
#define MASK_USBD_EP12_FIFO_C_EP2_OVLD		BIT5
#define MASK_USBD_EP12_FIFO_C_EP1_IVLD		BIT4
#define MASK_USBD_EP12_FIFO_C_EP12_DIR		BIT3
#define MASK_USBD_EP12_FIFO_C_CUR_BUF		BIT2
#define MASK_USBD_EP12_FIFO_C_SWITCH_BUF	BIT1
#define MASK_USBD_EP12_FIFO_C_ASE   		BIT0

/********************* Define UDC_EP0CS_OFST bit mask (0x34C, USB device endpoint0 control status) ******/
#define MASK_USBD_EP0_CS_EP0_OUT_EMPTY		BIT7
#define MASK_USBD_EP0_CS_EP0_OVLD			BIT6
#define MASK_USBD_EP0_CS_CLE_EP0_OUT_VLD	BIT5
#define MASK_USBD_EP0_CS_EP0_IVLD			BIT4
#define MASK_USBD_EP0_CS_SET_EP0_IN_VLD		BIT3
#define MASK_USBD_EP0_CS_EP0_SFIFO_UPDATE	BIT2
#define MASK_USBD_EP0_CS_EP0_SFIFO_VALID	BIT1
#define MASK_USBD_EP0_CS_EP0_DIR			BIT0

/********************* Define UDC_EP1SCS_OFST bit mask (0x358, USB device endpoint1 special control status) ******/
#define MASK_USBD_EP1_CLR_IN_VALID			BIT3
#define MASK_USBD_EP1_RESET_EP1S_FIFO		BIT2
#define MASK_USBD_EP1_SET_EP1S_IN_VALID		BIT0

/********************* Define UDC_EP12FCH_OFST bit mask (0x36C, USB device endpoint1/2 PING data count high byte) ******/
#define MASK_USBD_EP12_RESET_PING_CNTR		BIT2

/********************* Define UDC_EP12POFDHB_OFST bit mask (0x3A0, USB device endpoint1/2 PONG data count high byte) ******/
#define MASK_USBD_EP12_RESET_PONG_CNTR		BIT2

/********************* Define UDC_LLCSET0_OFST bit mask (0x3B0, USB deivce linker layer controller setting 0) **********/
#define MASK_USBD_UDLC_CS0_CLR_SUSP_COUNTER			BIT7
#define MASK_USBD_UDLC_CS0_SIM_MODE					BIT6
#define MASK_USBD_UDLC_CS0_DIS_SUSP_EN				BIT5
#define MASK_USBD_UDLC_CS0_CPU_WAKE_UP_EN			BIT4
#define MASK_USBD_UDLC_CS0_PWR_PARTIAL_N			BIT3
#define MASK_USBD_UDLC_CS0_PWR_SUSP_N				BIT2
#define MASK_USBD_UDLC_CS0_ISSUE_RESUME				BIT1
#define MASK_USBD_UDLC_CS0_SOFT_DISCONNECT			BIT0

/********************* Define UDC_LLCS_OFST bit mask (0x3B8, USB deivce linker layer controller status) **********/
#define MASK_USBD_UDLC_CS_DISC_CONNECT_STATUS		BIT7
#define MASK_USBD_UDLC_CS_H_HOST_CONFIGED			BIT6
#define MASK_USBD_UDLC_CS_H_LNK_SUSPENDM			BIT5
#define MASK_USBD_UDLC_CS_H_ALLOW_RWAKE				BIT4
#define MASK_USBD_UDLC_CS_CURR_SPEED				BIT1
#define MASK_USBD_UDLC_CS_VBUS_HIGH					BIT0


/********************* Define UDC_LLCSTL_OFST bit mask (0x3BC, USB deivce linker layer stall control) ************/
#define MASK_USBD_UDLC_STL_CLREPBSTL				BIT15	/* Clear EPB stall */
#define MASK_USBD_UDLC_STL_CLREPASTL				BIT14	/* Clear EPA stall */
#define MASK_USBD_UDLC_STL_CLREP9STL				BIT13	/* Clear EP9 stall */
#define MASK_USBD_UDLC_STL_CLREP8STL				BIT12	/* Clear EP8 stall */
#define MASK_USBD_UDLC_STL_CLREP3STL				BIT11	/* Clear EP3 stall */
#define MASK_USBD_UDLC_STL_CLREP2STL				BIT10	/* Clear EP2 stall */
#define MASK_USBD_UDLC_STL_CLREP1STL				BIT9	/* Clear EP1 stall */
#define MASK_USBD_UDLC_STL_CLREP0STL				BIT8	/* Clear EP0 stall */
#define MASK_USBD_UDLC_STL_SETEPBSTL				BIT7	/* Set EPB stall */
#define MASK_USBD_UDLC_STL_SETEPASTL				BIT6	/* Set EPA stall */
#define MASK_USBD_UDLC_STL_SETEP9STL				BIT5	/* Set EP9 stall */
#define MASK_USBD_UDLC_STL_SETEP8STL				BIT4	/* Set EP8 stall */
#define MASK_USBD_UDLC_STL_SETEP3STL				BIT3	/* Set EP3 stall */
#define MASK_USBD_UDLC_STL_SETEP2STL				BIT2	/* Set EP2 stall */
#define MASK_USBD_UDLC_STL_SETEP1STL				BIT1	/* Set EP1 stall */
#define MASK_USBD_UDLC_STL_SETEP0STL				BIT0	/* Set EP0 stall */

/********************* Define UDC_LLCIF_OFST bit mask (Offset + 0x400, flag of USB deivce linker layer controller) ******/
#define MASK_USBD_UDLC_IF_EP7IEND			BIT25	/* UDLC EP7 DMA IN transaction end flag */
#define MASK_USBD_UDLC_IF_EP5IEND			BIT24	/* UDLC EP5 DMA IN transaction end flag */
#define MASK_USBD_UDLC_IF_EP7I				BIT23	/* UDLC EP7 IN transaction interrupt flag */
#define MASK_USBD_UDLC_IF_EP6I				BIT22	/* UDLC EP6 IN transaction interrupt flag */
#define MASK_USBD_UDLC_IF_EP5I				BIT21	/* UDLC EP5 IN transaction interrupt flag */
#define MASK_USBD_UDLC_IF_EP4I				BIT20	/* UDLC EP4 IN transaction interrupt flag */
#define MASK_USBD_UDLC_IF_RESETN			BIT19	/* UDLC USB_RESET END interrupt flag */
#define MASK_USBD_UDLC_IF_SCONF				BIT18	/* UDLC HOST Set Configuration interrupt flag */
#define MASK_USBD_UDLC_IF_RESUME			BIT17	/* UDLC USB BUS RESUME interrupt flag */
#define MASK_USBD_UDLC_IF_SUSPEND			BIT16	/* UDLC USB BUS SUSPEND interrupt flag */
#define MASK_USBD_UDLC_IF_EP1INN			BIT15	/* UDLC DMA & EP1_IN END interrupt flag */
#define MASK_USBD_UDLC_IF_EP3I				BIT14	/* UDLC EP3_IN transaction interrupt flag */
#define MASK_USBD_UDLC_IF_PIPO				BIT13	/* UDLC PING-PONG_FIFO_SWAP interrupt flag */
#define MASK_USBD_UDLC_IF_HCS				BIT12	/* UDLC HOST_CLEAR_STALL interrupt flag */
#define MASK_USBD_UDLC_IF_EP2N				BIT11	/* UDLC EP2_NAK interrupt flag */
#define MASK_USBD_UDLC_IF_EP1N				BIT10	/* UDLC EP1_NAK interrupt flag */
#define MASK_USBD_UDLC_IF_EP0N				BIT9	/* UDLC EP0_NAK interrupt flag */
#define MASK_USBD_UDLC_IF_HSS				BIT8	/* UDLC HOST_SET_STALL interrupt flag */
#define MASK_USBD_UDLC_IF_EP2O				BIT7	/* UDLC EP2_OUT transaction interrupt flag */
#define MASK_USBD_UDLC_IF_EP1I				BIT6	/* UDLC EP1_IN transaction interrupt flag */
#define MASK_USBD_UDLC_IF_EP1SI				BIT5	/* UDLC EP1S_IN transaction interrupt flag */
#define MASK_USBD_UDLC_IF_EP0I				BIT4	/* UDLC EP0_IN transaction interrupt flag */
#define MASK_USBD_UDLC_IF_EP0O				BIT3	/* UDLC EP0_OUT transaction interrupt flag */
#define MASK_USBD_UDLC_IF_EP0S				BIT2	/* UDLC EP0_SETUP transaction interrupt flag */
#define MASK_USBD_UDLC_IF_SUSP				BIT1	/* UDLC USB_SUSPEND_DIFF interrupt flag */
#define MASK_USBD_UDLC_IF_RESET				BIT0	/* UDLC USB_RESET interrupt flag */

/********************* Define UDC_LLCIE_OFST bit mask (Offset + 0x404, flag enable of USB deivce linker layer controller) ******/
#define MASK_USBD_UDLC_IE_EP7IEND			BIT25	/* UDLC EP7 DMA IN transaction end flag */
#define MASK_USBD_UDLC_IE_EP5IEND			BIT24	/* UDLC EP5 DMA IN transaction interrupt enable */
#define MASK_USBD_UDLC_IE_EP7I				BIT23	/* UDLC EP7 IN transaction interrupt enable */
#define MASK_USBD_UDLC_IE_EP6I				BIT22	/* UDLC EP6 IN transaction interrupt enable */
#define MASK_USBD_UDLC_IE_EP5I				BIT21	/* UDLC EP5 IN transaction interrupt enable */
#define MASK_USBD_UDLC_IE_EP4I				BIT20	/* UDLC EP4 IN transaction interrupt enable */
#define MASK_USBD_UDLC_IE_RESETN			BIT19	/* UDLC USB_RESET END interrupt enable */
#define MASK_USBD_UDLC_IE_SCONF				BIT18	/* UDLC HOST set configuration interrupt enable */
#define MASK_USBD_UDLC_IE_RESUME			BIT17	/* UDLC USB BUS RESUME interrupt enable */
#define MASK_USBD_UDLC_IE_SUSPEND			BIT16	/* UDLC USB BUS SUSPEND interrupt enable */
#define MASK_USBD_UDLC_IE_EP1INN			BIT15	/* UDLC DMA & EP1_IN END interrupt enable */
#define MASK_USBD_UDLC_IE_EP3I				BIT14	/* UDLC EP3_IN transaction interrupt enable */
#define MASK_USBD_UDLC_IE_PIPO				BIT13	/* UDLC PING-PONG_FIFO_SWAP interrupt enable */
#define MASK_USBD_UDLC_IE_HCS				BIT12	/* UDLC HOST_CLEAR_STALL interrupt enable */
#define MASK_USBD_UDLC_IE_EP2N				BIT11	/* UDLC EP2_NAK interrupt enable */
#define MASK_USBD_UDLC_IE_EP1N				BIT10	/* UDLC EP1_NAK interrupt enable */
#define MASK_USBD_UDLC_IE_EP0N				BIT9	/* UDLC EP0_NAK interrupt enable */
#define MASK_USBD_UDLC_IE_HSS				BIT8	/* UDLC HOST_SET_STALL interrupt enable */
#define MASK_USBD_UDLC_IE_EP2O				BIT7	/* UDLC EP2_OUT transaction interrupt enable */
#define MASK_USBD_UDLC_IE_EP1I				BIT6	/* UDLC EP1_IN transaction interrupt enable */
#define MASK_USBD_UDLC_IE_EP1SI				BIT5	/* UDLC EP1S_IN transaction interrupt enable */
#define MASK_USBD_UDLC_IE_EP0I				BIT4	/* UDLC EP0_IN transaction interrupt enable */
#define MASK_USBD_UDLC_IE_EP0O				BIT3	/* UDLC EP0_OUT transaction interrupt enable */
#define MASK_USBD_UDLC_IE_EP0S				BIT2	/* UDLC EP0_SETUP transaction interrupt enable */
#define MASK_USBD_UDLC_IE_SUSP				BIT1	/* UDLC USB_SUSPEND_DIFF interrupt enable */
#define MASK_USBD_UDLC_IE_RESET				BIT0	/* UDLC USB_RESET interrupt enable */

/********************* Define UDC_SREQIF_OFST bit mask (0x410, standard request inerrupt flag) ******/
#define MASK_USBD_STDREQ_IF_SET_DESC		BIT14	/* Set descriptor interrupt flag */
#define MASK_USBD_STDREQ_IF_GET_DESC		BIT13	/* Get descriptor interrupt flag */
#define MASK_USBD_STDREQ_IF_GET_CONF		BIT12	/* Get configuration interrupt flag */
#define MASK_USBD_STDREQ_IF_GET_STS			BIT11	/* Get status interrupt flag */
#define MASK_USBD_STDREQ_IF_GET_INTF		BIT10	/* Get interface interrupt flag */
#define MASK_USBD_STDREQ_IF_SET_INTF		BIT9	/* Set interface interrupt flag */
#define MASK_USBD_STDREQ_IF_SET_ADDR		BIT8	/* Set address interrupt flag */
#define MASK_USBD_STDREQ_IF_SET_CONF		BIT7	/* Set configuration interrupt flag */
#define MASK_USBD_STDREQ_IF_SET_TEST		BIT6	/* Set feature interrupt flag */
#define MASK_USBD_STDREQ_IF_SET_REMOTE		BIT5	/* Set remote interrupt flag */
#define MASK_USBD_STDREQ_IF_SET_EPX_STALL	BIT4	/* Set feature interrupt flag (other EP stall) */
#define MASK_USBD_STDREQ_IF_SET_EP0_STALL	BIT3	/* Set feature interrupt flag (EP0 stall) */
#define MASK_USBD_STDREQ_IF_CLR_REMOTE_IF	BIT2	/* Clear feature interrupt flag (device remote wakeup) */
#define MASK_USBD_STDREQ_IF_CLR_EPX_STALL	BIT1	/* Clear feature interrupt flag (other EP stall) */
#define MASK_USBD_STDREQ_IF_CLR_EP0_STALL	BIT0	/* Clear feature interrupt flag (EP0 stall) */

/********************* Define UDC_SREQIE_OFST bit mask (0x414, standard request inerrupt enable) ******/
#define MASK_USBD_STDREQ_IE_SET_DESC		BIT14	/* Set descriptor interrupt enable */
#define MASK_USBD_STDREQ_IE_GET_DESC		BIT13	/* Get descriptor interrupt enable */
#define MASK_USBD_STDREQ_IE_GET_CONF		BIT12	/* Get configuration interrupt enable */
#define MASK_USBD_STDREQ_IE_GET_STS			BIT11	/* Get status interrupt enable */
#define MASK_USBD_STDREQ_IE_GET_INTF		BIT10	/* Get interface interrupt enable */
#define MASK_USBD_STDREQ_IE_SET_INTF		BIT9	/* Set interface interrupt enable */
#define MASK_USBD_STDREQ_IE_SET_ADDR		BIT8	/* Set address interrupt enable */
#define MASK_USBD_STDREQ_IE_SET_CONF		BIT7	/* Set configuration interrupt enable */
#define MASK_USBD_STDREQ_IE_SET_TEST		BIT6	/* Set feature interrupt enable */
#define MASK_USBD_STDREQ_IE_SET_REMOTE		BIT5	/* Set remote interrupt enable */
#define MASK_USBD_STDREQ_IE_SET_EPX_STALL	BIT4	/* Set feature interrupt enable (other EP stall) */
#define MASK_USBD_STDREQ_IE_SET_EP0_STALL	BIT3	/* Set feature interrupt enable (EP0 stall) */
#define MASK_USBD_STDREQ_IE_CLR_REMOTE_IF	BIT2	/* Clear feature interrupt enable (device remote wakeup) */
#define MASK_USBD_STDREQ_IE_CLR_EPX_STALL	BIT1	/* Clear feature interrupt enable (other EP stall) */
#define MASK_USBD_STDREQ_IE_CLR_EP0_STALL	BIT0	/* Clear feature interrupt enable (EP0 stall) */