/*
 *  linux/drivers/net/gp_eth.c
 *
 *  GPL64000 On-Chip ethernet driver.
 *
 *  Copyright (c) 2010 by Generalplus Inc. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <linux/mii.h>
#include <linux/skbuff.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/kthread.h>
#include <linux/version.h>
#include <linux/random.h>
#include <linux/platform_device.h>	/* For platform_driver framework */
//#include <asm/addrspace.h>
#include <asm/uaccess.h>
#include <asm/cacheflush.h>
//#include <asm/cacheops.h>
#include <mach/hardware.h>
#include <mach/io.h>
#include "gp_eth.h"
#include <mach/irqs.h>
#include <mach/hal/hal_sdma.h>
#include <mach/hal/regmap/reg_scu.h>
//#include <mach/gp_pin_grp.h>
#include <mach/hal/hal_gpio.h>
#include <asm/system.h>
#include <mach/gp_board.h>
#include <linux/moduleparam.h>
#include <mach/clk/gp_clk_core.h>

#define gp_mac_alloc(dma_handle, size) \
	dma_alloc_coherent(NULL, size, dma_handle, GFP_KERNEL)
#define gp_mac_free(dma_handle, ptr) \
	dma_free_coherent(NULL, sizeof(*ptr), ptr, dma_handle)


/* jerry@101220 : modified for cache flush and invalidate */	
#if 0
static __inline__ void dma_cache_inv (unsigned long _start, unsigned long _size)
{
    
	const void *start = (void __force *)_start;
	dmac_inv_range(start, start + _size);

}
#else
#define dma_cache_inv(_start,_size)      dma_cache_maint((void *)_start, _size, DMA_FROM_DEVICE)//do {} while (0)
#endif
#if 0
static __inline__ void dma_cache_wback (unsigned long _start, unsigned long _size)
{
	const void *start = (void __force *)_start;
	dmac_clean_range(start, start + _size);
}
#else
#define dma_cache_wback(_start,_size)      dma_cache_maint((void *)_start, _size, DMA_TO_DEVICE)//  do {} while (0)
#endif
#define dma_cache_wback_inv(_start,_size)    do {} while (0)

#define P2ADDR(a)    ((unsigned long)(a) & 0x1fffffff)//(((unsigned long)(a) & 0x1fffffff) | 0xa0000000)
#define P1ADDR(a)    (((unsigned long)(a) & 0x1fffffff) | 0x80000000)

#define ETHERNET_MAC_INTERFACE_MII  0
#define ETHERNET_MAC_INTERFACE_RMMII 1


//#define ETH_DEBUG
#ifdef ETH_DEBUG
#define DBPRINTK(fmt,args...) printk(KERN_DEBUG fmt,##args)
#else
#define DBPRINTK(fmt,args...) do {} while(0)
#endif

#define errprintk(fmt,args...)  printk(KERN_ERR fmt,##args);
#define infoprintk(fmt,args...) printk(KERN_INFO fmt,##args);

#define DRV_NAME    "gp_eth"
#define DRV_VERSION    "1.0"
#define DRV_AUTHOR    "Jerry Chang <jerrychang@generalplus.com>"
#define DRV_DESC    "GPL83000 On-chip Ethernet driver"

MODULE_AUTHOR(DRV_AUTHOR);
MODULE_DESCRIPTION(DRV_DESC);
MODULE_LICENSE("GPL");
MODULE_PARM_DESC(debug, "i");
MODULE_PARM_DESC(hwaddr,"Mac Address");


/*
 * Local variables
 */
static struct net_device *netdev;
static char * hwaddr = NULL;
static int debug = -1;
static struct mii_if_info mii_info;

static struct net_dma_desc_t *tx_list_head;
static struct net_dma_desc_t *tx_list_tail;
static struct net_dma_desc_t *rx_list_head;
static struct net_dma_desc_t *rx_list_tail;
static struct net_dma_desc_t *current_rx_ptr;
static struct net_dma_desc_t *current_tx_ptr;
static struct net_dma_desc_t *tx_desc;
static struct net_dma_desc_t *rx_desc;

//static unsigned int toe_rx_buf;

static unsigned int start_vaddr_tx_buf;
static unsigned int start_vaddr_rx_buf;
static unsigned int current_vaddr_tx_buf;
static  unsigned int current_vaddr_rx_buf;
static unsigned int end_vaddr_tx_buf;
static  unsigned int end_vaddr_rx_buf;

#define ETH_BUS_BUG
//#define MDC_MDIO_USING_GPIO //this is fixed in 32900B ver B IC, MDIO/MDCLK works

#ifdef MDC_MDIO_USING_GPIO
#include <mach/gp_gpio.h>
static int mdc_handle;
static int mdio_handle;
#endif

// Module Parameter
module_param(hwaddr,charp,0);

#define COUNTER_MAX 1000000000

/*
 * Local routines
 */
static irqreturn_t gp_eth_interrupt(int irq, void *dev_id);
static int link_check_thread (void *data);


static inline unsigned char str2hexnum(unsigned char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
       return c - 'A' + 10;
    return 0; /* foo */
}

static inline void str2eaddr(unsigned char *ea, unsigned char *str)
{
    int i;

    for (i = 0; i < 6; i++) {
        unsigned char num;

        if((*str == '.') || (*str == ':'))
            str++;
        num = str2hexnum(*str++) << 4;
        num |= (str2hexnum(*str++));
        ea[i] = num;
    }
}

static int ethaddr_cmd = 0;
static unsigned char ethaddr_hex[6];

static int __init ethernet_addr_setup(char *str)
{
    if (!str) {
            printk("ethaddr not set in command line\n");
        return -1;
    }
    ethaddr_cmd = 1;
    str2eaddr(ethaddr_hex, str);

    return 0;
}

__setup("ethaddr=", ethernet_addr_setup);

static int get_mac_address(struct net_device *dev)
{
    int i;
    unsigned char flag0=0;
    unsigned char flag1=0xff;
   
    dev->dev_addr[0] = 0xff;
    if (hwaddr != NULL) {
        /* insmod gp-ethc.o hwaddr=00:ef:a3:c1:00:10 */
        str2eaddr(dev->dev_addr, hwaddr);
    } else if (ethaddr_cmd) {
        /* linux command line: ethaddr=00:ef:a3:c1:00:10 */
        for (i=0; i<6; i++)
            dev->dev_addr[i] = ethaddr_hex[i];
    } else {

    }

    /* check whether valid MAC address */
    for (i=0; i<6; i++) {
        flag0 |= dev->dev_addr[i];
        flag1 &= dev->dev_addr[i];
    }
    if ((dev->dev_addr[0] & 0xC0) || (flag0 == 0) || (flag1 == 0xff)) {
        printk("WARNING: There is not MAC address, use default ..\n");
        dev->dev_addr[0] = 0x00;
        dev->dev_addr[1] = 0xef;
        dev->dev_addr[2] = 0xa3;
        dev->dev_addr[3] = 0xc1;
        dev->dev_addr[4] = 0x00;
        dev->dev_addr[5] = 0x03;
    }

    return 0;
}

/*---------------------------------------------------------------------*/

static u32 gp_eth_curr_mode(struct net_device *dev);

/*
 * Link check routines
 */
static void start_check(struct net_device *dev)
{

    struct gp_eth_private *np = netdev_priv(dev);//(struct gp_eth_private *)P2ADDR(netdev_priv(dev));
    struct task_struct *t;

    np->thread_die = 0;
    init_waitqueue_head(&np->thr_wait);
    init_completion(&np->thr_exited);

    t = kthread_create(link_check_thread,(void *)dev, dev->name);
    if (IS_ERR(t))
        errprintk("%s: Unable to start kernel thread\n",dev->name);
    np->thread = t;
}

static int close_check(struct net_device *dev)
{

    struct gp_eth_private *np = netdev_priv(dev);//(struct gp_eth_private *)P2ADDR(netdev_priv(dev));
    int ret = 0;

	printk("[Jerry] check close 1\n");
    if (np->thread != NULL) {
        np->thread_die = 1;
        wmb();
	printk("[Jerry] check close 2\n");	
        send_sig(SIGTERM, np->thread, 1);
        if (ret) {
            errprintk("%s: Unable to signal thread\n", dev->name);
            return 1;
        }
	printk("[Jerry] check close 3\n");		
        wait_for_completion (&np->thr_exited);
    }
	printk("[Jerry] check close 4\n");	
    return 0;
}

static int link_check_thread(void *data)
{
    struct net_device *dev=(struct net_device *)data;
    struct gp_eth_private *np = netdev_priv(dev);//(struct gp_eth_private *)P2ADDR(netdev_priv(dev));


    unsigned char current_link;
    unsigned long timeout;

    daemonize("%s", dev->name);
    spin_lock_irq(&current->sighand->siglock);
    sigemptyset(&current->blocked);
    recalc_sigpending();
    spin_unlock_irq(&current->sighand->siglock);

    strncpy(current->comm, dev->name, sizeof(current->comm) - 1);
    current->comm[sizeof(current->comm) - 1] = '\0';

    while (1) {
        timeout = 3*HZ;
        do {
            timeout = interruptible_sleep_on_timeout(&np->thr_wait, timeout);
            /* make swsusp happy with our thread */
//            if (current->flags & PF_FREEZE)
//                refrigerator(PF_FREEZE);
        } while (!signal_pending(current) && (timeout > 0));

        if (signal_pending (current)) {
            spin_lock_irq(&current->sighand->siglock);
            flush_signals(current);
            spin_unlock_irq(&current->sighand->siglock);
        }

        if (np->thread_die)
            break;
       
        current_link = mii_link_ok(&mii_info);
        if (np->link_state != current_link) {
            if (current_link) {
                infoprintk("%s: Ethernet Link OK!\n", dev->name);
                gp_eth_curr_mode(dev);
                netif_carrier_on(dev);
            } else {
                errprintk("%s: Ethernet Link offline!\n", dev->name);
                netif_carrier_off(dev);
            }
        }
        np->link_state = current_link;

    }
    complete_and_exit(&np->thr_exited, 0);    
}

#ifdef ETH_DEBUG

static void skb_dump(struct sk_buff *skb)
{
    printk("skb ----------- [ 0x%08x ]\n", (unsigned int)skb);
    printk("head = 0x%08x, data = 0x%08x, tail = 0x%08x, end = 0x%08x\n",
           (unsigned int)(skb->head), (unsigned int)(skb->data),
           (unsigned int)(skb->tail), (unsigned int)(skb->end));
    printk("truesize = %d (0x%08x), len = %d (0x%08x)\n",
           skb->truesize, skb->truesize, skb->len, skb->len);
    printk("headroom = %d (0x%08x), tailroom = %d (0x%08x)\n",
           skb_headroom(skb), skb_headroom(skb), skb_tailroom(skb), skb_tailroom(skb));
    printk("---------------------------\n");
}


static void priv_data_dump(struct gp_eth_private *priv)
{
    printk("---- priv [0x%08x] -----------------------\n"
           "tx_ring = 0x%08x, rx_ring = 0x%08x\n"
           "dma_tx_ring = 0x%08x, dma_rx_ring = 0x%08x\n"
           "dma_rx_buf = 0x%08x, vaddr_rx_buf = 0x%08x\n"
           "rx_head = 0x%08x, tx_head = 0x%08x\n"
           "tx_tail = 0x%08x, tx_full = 0x%08x, tx_skb = 0x%08x\n"
           "---------------------------\n",
           (unsigned int)priv,
           (unsigned int)priv->tx_ring, (unsigned int)priv->rx_ring,
           (unsigned int)priv->dma_tx_ring, (unsigned int)priv->dma_rx_ring,
           (unsigned int)priv->dma_rx_buf, (unsigned int)priv->vaddr_rx_buf,
           (unsigned int)priv->rx_head, (unsigned int)priv->tx_head,
           (unsigned int)priv->tx_tail, (unsigned int)priv->tx_full, (unsigned int)priv->tx_skb);
}

static void desc_dump(struct gp_eth_private *np)
{
    int i;
    printk("==================================================\n");

    dma_cache_inv((unsigned int)np->tx_ring, sizeof(gp_desc_t) * NUM_TX_DESCS);
    dma_cache_inv((unsigned int)np->rx_ring, sizeof(gp_desc_t) * NUM_RX_DESCS);

    for (i = 0; i < NUM_TX_DESCS; i++) {
        printk("tx desc %2d :[0x%08x],  addr = 0x%08x,  pkt_size = 0x%08x,  next = 0x%08x\n",
               i, (unsigned int)&(np->tx_ring[i]), (unsigned int)np->tx_ring[i].pkt_addr,
               (unsigned int)np->tx_ring[i].pkt_size, (unsigned int)np->tx_ring[i].next_desc);
    }

    printk("\n");

    for (i = 0; i < 30/*NUM_RX_DESCS*/; i++) {
        printk("rx desc %2d :[0x%08x],  addr = 0x%08x,  pkt_size = 0x%08x,  next = 0x%08x\n",
               i, (unsigned int)&(np->rx_ring[i]), (unsigned int)np->rx_ring[i].pkt_addr,
               (unsigned int)np->rx_ring[i].pkt_size, (unsigned int)np->rx_ring[i].next_desc);
    }

    printk("REG32(ETH_DMA_TDR) = 0x%08x\nREG32(ETH_DMA_RDR) = 0x%08x\n",
           REG32(ETH_DMA_TDR), REG32(ETH_DMA_RDR));

    printk("t pkt_addr = 0x%08x\n", ((gp_desc_t *)P2ADDR(REG32(ETH_DMA_TDR)))->pkt_addr);
    printk("t pkt_size = 0x%08x\n", ((gp_desc_t *)P2ADDR(REG32(ETH_DMA_TDR)))->pkt_size);
    printk("t next_desc = 0x%08x\n", ((gp_desc_t *)P2ADDR(REG32(ETH_DMA_TDR)))->next_desc);
   
    printk("r pkt_addr = 0x%08x\n", ((gp_desc_t *)P2ADDR(REG32(ETH_DMA_RDR)))->pkt_addr);
    printk("r pkt_size = 0x%08x\n", ((gp_desc_t *)P2ADDR(REG32(ETH_DMA_RDR)))->pkt_size);
    printk("r next_desc = 0x%08x\n", ((gp_desc_t *)P2ADDR(REG32(ETH_DMA_RDR)))->next_desc);

    //priv_data_dump(np);
    printk("==================================================\n");
}

static void dma_regs_dump(char *str)
{
    printk("%s", str);
    printk("==================================================\n");
    printk("DMA_TCR = 0x%08x    DMA_TDR = 0x%08x\n", REG32(ETH_DMA_TCR), REG32(ETH_DMA_TDR));
    printk("DMA_TSR = 0x%08x    DMA_RCR = 0x%08x\n", REG32(ETH_DMA_TSR), REG32(ETH_DMA_RCR));
    printk("DMA_RDR = 0x%08x    DMA_RSR = 0x%08x\n", REG32(ETH_DMA_RDR), REG32(ETH_DMA_RSR));
    printk("DMA_IMR = 0x%08x    DMA_IR  = 0x%08x\n", REG32(ETH_DMA_IMR), REG32(ETH_DMA_IR));
    printk("==================================================\n");
}

static void mac_regs_dump(void)
{
    printk("==================================================\n");
    printk("MAC_MCR1 = 0x%08x    MAC_MCR2 = 0x%08x\n", REG32(ETH_MAC_MCR1), REG32(ETH_MAC_MCR2));
    printk("MAC_IPGR = 0x%08x    MAC_NIPGR= 0x%08x\n", REG32(ETH_MAC_IPGR), REG32(ETH_MAC_NIPGR));
    printk("MAC_CWR  = 0x%08x    MAC_MFR  = 0x%08x\n", REG32(ETH_MAC_CWR), REG32(ETH_MAC_MFR));
    printk("MAC_PSR  = 0x%08x    MAC_TR   = 0x%08x\n", REG32(ETH_MAC_PSR), REG32(ETH_MAC_TR));
    printk("MAC_MCFGR= 0x%08x    MAC_MCMDR= 0x%08x\n", REG32(ETH_MAC_MCFGR), REG32(ETH_MAC_MCMDR));
    printk("MAC_MADRR= 0x%08x    MAC_MINDR= 0x%08x\n", REG32(ETH_MAC_MADRR), REG32(ETH_MAC_MINDR));
    printk("MAC_SA0  = 0x%08x    MAC_SA1  = 0x%08x    MAC_SA2  = 0x%08x\n",
           REG32(ETH_MAC_SA0), REG32(ETH_MAC_SA1), REG32(ETH_MAC_SA2));
    printk("==================================================\n");
}

static void fifo_regs_dump(void)
{
    printk("==================================================\n");
    printk("FIFO_CR0 = 0x%08x\n", REG32(ETH_FIFO_CR0));
    printk("FIFO_CR1 = 0x%08x\n", REG32(ETH_FIFO_CR1));
    printk("FIFO_CR2 = 0x%08x\n", REG32(ETH_FIFO_CR2));
    printk("FIFO_CR3 = 0x%08x\n", REG32(ETH_FIFO_CR3));
    printk("FIFO_CR4 = 0x%08x\n", REG32(ETH_FIFO_CR4));
    printk("FIFO_CR5 = 0x%08x\n", REG32(ETH_FIFO_CR5));
    printk("==================================================\n");
}

static void stat_regs_dump(void)
{

    printk("==================================================\n");
    printk("ETH_STAT_TR64= 0x%08x, ETH_STAT_TR127= 0x%08x, ETH_STAT_TR255= 0x%08x, ETH_STAT_TR511= 0x%08x\n",
           REG32(ETH_STAT_TR64), REG32(ETH_STAT_TR127), REG32(ETH_STAT_TR255), REG32(ETH_STAT_TR511));
    printk("ETH_STAT_TR1K= 0x%08x, ETH_STAT_TRMAX= 0x%08x, ETH_STAT_TRMGV= 0x%08x\n",
           REG32(ETH_STAT_TR1K), REG32(ETH_STAT_TRMAX), REG32(ETH_STAT_TRMGV));
   
    printk("------\nETH_STAT_RBYT= 0x%08x, ETH_STAT_RPKT= 0x%08x, ETH_STAT_RFCS= 0x%08x, ETH_STAT_RMCA= 0x%08x\n",
           REG32(ETH_STAT_RBYT), REG32(ETH_STAT_RPKT), REG32(ETH_STAT_RFCS), REG32(ETH_STAT_RMCA));
    printk("ETH_STAT_RDRP= 0x%08x\n", REG32(ETH_STAT_RDRP));

    printk("------\nETH_STAT_TBYT= 0x%08x, ETH_STAT_TPKT= 0x%08x, ETH_STAT_TFCS = 0x%08x, ETH_STAT_TNCL= 0x%08x\n",
           REG32(ETH_STAT_TBYT), REG32(ETH_STAT_TPKT), REG32(ETH_STAT_TFCS), REG32(ETH_STAT_TNCL));
    printk("ETH_STAT_TDRP= 0x%08x\n", REG32(ETH_STAT_TDRP));
    printk("==================================================\n");

}

static void counters_dump(struct gp_eth_private *np)
{
    int i = 0;

    printk("\n");

    do {
        printk("cnts[%d] = %d\n", i, np->carry_counters[i]);
    } while (++i < STAT_CNT_NUM);
}

static void sal_regs_dump(void)
{
    printk("==================================================\n");
    printk("ETH_SAL_AFR = 0x%08x, ETH_SAL_HT1 = 0x%08x, ETH_SAL_HT2 = 0x%08x\n",
           REG32(ETH_SAL_AFR), REG32(ETH_SAL_HT1), REG32(ETH_SAL_HT2));
    printk("==================================================\n");    
}

/*
 * Display ethernet packet header
 * This routine is used for test function
 */
static void eth_dbg_rx(struct sk_buff *skb, int len)
{
    int i, j;

    printk("R: %02x:%02x:%02x:%02x:%02x:%02x <- %02x:%02x:%02x:%02x:%02x:%02x len/SAP:%02x%02x [%d]\n",
           (u8)skb->data[0], (u8)skb->data[1], (u8)skb->data[2], (u8)skb->data[3], (u8)skb->data[4],
             (u8)skb->data[5], (u8)skb->data[6], (u8)skb->data[7], (u8)skb->data[8], (u8)skb->data[9],
             (u8)skb->data[10], (u8)skb->data[11], (u8)skb->data[12], (u8)skb->data[13], len);

    for (j = 0; len > 0; j += 16, len -= 16) {
        printk("    %03x: ",j);
        for (i = 0; i < 16 && i < len; i++) {
            printk("%02x ", (u8)skb->data[i + j]);
        }
        printk("\n");
      }
    return;
}
 
static void eth_dbg_tx(struct sk_buff *skb, int len)
{

      int i, j;
   
      printk("T: %02x:%02x:%02x:%02x:%02x:%02x <- %02x:%02x:%02x:%02x:%02x:%02x len/SAP:%02x%02x [%d]\n",
             (u8)skb->data[0], (u8)skb->data[1], (u8)skb->data[2], (u8)skb->data[3],
             (u8)skb->data[4], (u8)skb->data[5], (u8)skb->data[6], (u8)skb->data[7],
             (u8)skb->data[8], (u8)skb->data[9], (u8)skb->data[10], (u8)skb->data[11],
             (u8)skb->data[12], (u8)skb->data[13], len);
 
      for (j = 0; len > 0; j += 16, len -= 16) {
          printk("    %03x: ",j);
          for (i = 0; i < 16 && i < len; i++) {
              printk("%02x ", (u8)skb->data[i+j]);
          }
          printk("\n");
      }
      return;
}



#endif // ETH_DEBUG

/*
 * MII operation routines
 */
static inline void mii_wait(void)
{
    int i;
//    for (i = 0; i < MAX_WAIT; i++, mdelay(1)) {
    for (i = 0; i < MAX_WAIT; i++, mdelay(20)) { /* Cynthia, Test, 2010-05-14 */
        if (!__mac_mii_is_busy())
            return ;
    }

//    printk("\nMAC_MCMDR= 0x%04x    MAC_MADRR= 0x%04x    MAC_MINDR = 0x%04x\n",
//           REG16(ETH_MAC_MCMDR),  REG16(ETH_MAC_MADRR), REG16(ETH_MAC_MINDR));

    if (i == MAX_WAIT)
        printk("MII wait timeout\n");
}

#ifdef MDC_MDIO_USING_GPIO
static void mdio_output(int level)
{
	gp_gpio_set_direction(mdio_handle, GPIO_DIR_OUTPUT);
	gp_gpio_set_output(mdio_handle, 1, 0);

	udelay(10);
	gp_gpio_set_output(mdc_handle, 0, 0);
	gp_gpio_set_output(mdio_handle, level, 0);
	udelay(10);
	gp_gpio_set_output(mdc_handle, 1, 0);

}

static unsigned int mdio_input(void)
{
	unsigned int value;

	gp_gpio_set_direction(mdio_handle, GPIO_DIR_INPUT);

	udelay(10);
	gp_gpio_set_output(mdc_handle, 0, 0);
	gp_gpio_get_value(mdio_handle, &value);
	udelay(10);
	gp_gpio_set_output(mdc_handle, 1, 0);

	return value;
}
#endif

static int mdio_read(struct net_device *dev,int phy_id, int location)
{
	int retval = 0;
#ifdef MDC_MDIO_USING_GPIO
	int i;
	unsigned int regVal;

	//MSG("\n");

	/* Preamble : 32 bit 1 */
	gp_gpio_set_direction(mdio_handle, GPIO_DIR_OUTPUT);
	gp_gpio_set_output(mdio_handle, 1, 0);
	for (i=0; i<32; i++) {
		mdio_output(1);
	}

	/* SFD : 2 bit 01 */
	mdio_output(0);
	mdio_output(1);

	/* Op code : 2 bit 10 */
	mdio_output(1);
	mdio_output(0);

	/* Phy address : 5 bit */
	for (i=0; i<5; i++) {
		mdio_output((phy_id >> (4 - i)) & 1);
	}

	/* Reg address : 5 bit */
	for (i=0; i<5; i++) {
		mdio_output((location >> (4 - i)) & 1);
	}

	/* Turn Around : 2 bit */
	mdio_input();
	mdio_input();

	/* Data : 16 bit */
	for (i=0; i<16; i++) {
		regVal = mdio_input();
		retval = (retval << 1) | (regVal & 1);
	}
	
#else
    __mac_send_mii_read_cmd(phy_id, location, MII_NO_SCAN);
    mii_wait();
    retval = __mac_mii_read_data();
#endif
    return retval;
   
}

static void mdio_write(struct net_device *dev,int phy_id, int location, int data)
{
#ifdef MDC_MDIO_USING_GPIO
	printk("Not implement yet ..............\n");
#else
    __mac_send_mii_write_cmd(phy_id, location, data);
    mii_wait();
#endif
}


/*
 * Search MII phy
 */
static int gp_search_mii_phy(struct net_device *dev)
{
    struct gp_eth_private *np = netdev_priv(dev);//(struct gp_eth_private *)P2ADDR(netdev_priv(dev));
    int phy, phy_idx = 0;
	int mii_status = 0xFFFF;

    np->valid_phy = 0xff;
    for (phy = 0; phy < 32; phy++) {
        mii_status = mdio_read(dev, phy, 1);
	
        if (mii_status != 0xffff  &&  mii_status != 0x0000) {
            np->phys[phy_idx] = phy;
            np->ecmds[phy_idx].speed=SPEED_100;
            np->ecmds[phy_idx].duplex=DUPLEX_FULL;
            np->ecmds[phy_idx].port=PORT_MII;
            np->ecmds[phy_idx].transceiver=XCVR_INTERNAL;
            np->ecmds[phy_idx].phy_address=np->phys[phy_idx];
            np->ecmds[phy_idx].autoneg=AUTONEG_ENABLE;
            np->ecmds[phy_idx].advertising=(ADVERTISED_10baseT_Half |
                            ADVERTISED_10baseT_Full |
                            ADVERTISED_100baseT_Half |
                            ADVERTISED_100baseT_Full);
            phy_idx++;
		printk("[%d]1st mii_status = %x\n",phy,mii_status);	
        }
    }
    if (phy_idx == 1) {
        np->valid_phy = np->phys[0];
        np->phy_type = 0;
    }
    if (phy_idx != 0) {
        phy = np->valid_phy;
        np->advertising = mdio_read(dev,phy, 4);
		
		// Some PHY need read twice
		for (phy = 0; phy < 32; phy++) {
			mii_status = mdio_read(dev, phy, 1);
			if (mii_status != 0xffff  &&  mii_status != 0x0000) {
				printk("[%d]2nd mii_status = %x\n",phy,mii_status);	
			}
		}
		
    }
    return phy_idx;
}

#if 1 // multicast

/*
 * CRC calc for Destination Address for gets hashtable index
 */
#define POLYNOMIAL 0x04c11db7UL
static u16 gp_hashtable_index(u8 *addr)
{
    u32 crc = 0xffffffff, msb;
    int  i, j;
    u32  byte;
    for (i = 0; i < 6; i++) {
        byte = *addr++;
        for (j = 0; j < 8; j++) {
            msb = crc >> 31;
            crc <<= 1;
            if (msb ^ (byte & 1)) crc ^= POLYNOMIAL;
            byte >>= 1;
        }
    }
    return ((int)(crc >> 26));

}

/*
 * Multicast filter and config multicast hash table
 */
#define MULTICAST_FILTER_LIMIT 64

static void gp_set_multicast_list(struct net_device *dev)
{
#if 0
    int i, hash_index;
    u32 hash_h, hash_l, hash_bit;



    if (dev->flags & IFF_PROMISC) {
        /* Accept any kinds of packets */
        __sal_set_mode(AFR_PRO);
        __sal_set_hash_table(0xffffffff, 0xffffffff);

        printk("%s: Enter promisc mode!\n",dev->name);
    } else  if ((dev->flags & IFF_ALLMULTI) || (dev->mc_count > MULTICAST_FILTER_LIMIT)) {
        /* Accept all multicast packets */
        __sal_set_mode(AFR_PRM);
        __sal_set_hash_table(0xffffffff, 0xffffffff);
        //printk("%s: Enter allmulticast mode!   %d \n",dev->name,dev->mc_count);
    } else if (dev->flags & IFF_MULTICAST) {
        /* Update multicast hash table */
        struct dev_mc_list *mclist;
        __sal_get_hash_table(hash_h, hash_l);

        for (i = 0, mclist = dev->mc_list; mclist && i < dev->mc_count;
             i++, mclist = mclist->next)
        {
            hash_index = gp_hashtable_index(mclist->dmi_addr);
            hash_bit=0x00000001;
            hash_bit <<= (hash_index & 0x1f);
            if (hash_index > 0x1f)
                hash_h |= hash_bit;
            else
                hash_l |= hash_bit;

#ifdef ETH_DEBUG
            DBPRINTK("----------------------------\n");
            {
                int j;
                for (j=0;j<mclist->dmi_addrlen;j++)
                    printk("%2.2x:",mclist->dmi_addr[j]);
                printk("\n");
            }
#endif
            //printk("dmi.addrlen => %d\n",mclist->dmi_addrlen);
            //printk("dmi.users   => %d\n",mclist->dmi_users);
            //printk("dmi.gusers  => %d\n",mclist->dmi_users);
        }
        __sal_set_hash_table(hash_h, hash_l);

        __sal_set_mode(AFR_AMC);

        //printk("This is multicast hash table high bits [%4.4x]\n",readl(ETH_SAL_HT1));
        //printk("This is multicast hash table low  bits [%4.4x]\n",readl(ETH_SAL_HT2));
        printk("%s: Enter multicast mode!\n",dev->name);
    }
#endif		
}
#endif // multicast

static inline int gp_phy_reset(struct net_device *dev)
{

    struct gp_eth_private *np = netdev_priv(dev);//(struct gp_eth_private *)P2ADDR(netdev_priv(dev));

    unsigned int mii_reg0;
    unsigned int count;
   
    mii_reg0 = mdio_read(dev,np->valid_phy, MII_BMCR);
    mii_reg0 |= MII_CR_RST;

    mdio_write(dev, np->valid_phy, MII_BMCR, mii_reg0);    //reset phy
    for (count = 0; count < 2000; count++) {
        mdelay(1);
        mii_reg0 = mdio_read(dev,np->valid_phy, MII_BMCR);
        if (!(mii_reg0 & MII_CR_RST))
            break;  //reset completed
    }

    if (count == 2000)
        return 1;     //phy error
    else
        return 0;
}


/*
 * Get current mode of eth phy
 */
static u32 gp_eth_curr_mode(struct net_device *dev)
{

    struct gp_eth_private *np = netdev_priv(dev);//(struct gp_eth_private *)P2ADDR(netdev_priv(dev));

    unsigned int mii_reg17;
    u32 flag = 0;

    mii_reg17 = mdio_read(dev,np->valid_phy,MII_DSCSR);
    np->media = mii_reg17>>12;
    if (np->media==8) {
        infoprintk("%s: Current Operation Mode is [100M Full Duplex]",dev->name);
        flag = 0;
        np->full_duplex=1;
    }
    if (np->media==4) {
        infoprintk("%s: Current Operation Mode is [100M Half Duplex]",dev->name);
        flag = 0;
        np->full_duplex=0;
    }
    if (np->media==2) {
        infoprintk("%s: Current Operation Mode is [10M Full Duplex]",dev->name);
        np->full_duplex=1;
    }
    if (np->media==1) {
        infoprintk("%s: Current Operation Mode is [10M Half Duplex]",dev->name);
        np->full_duplex=0;
    }
    printk("\n");
    return flag;
}

static void config_mac(struct net_device *dev)
{

    struct gp_eth_private *np = netdev_priv(dev);//(struct gp_eth_private *)P2ADDR(netdev_priv(dev));
    u32    mac_cfg_1 = 0, mac_cfg_2 = 0;

    // Enable tx & rx flow control, enable receive
    mac_cfg_1 = MCR1_TFC | MCR1_RFC | MCR1_RE;

#if 0//def USE_RMII
    // Enable RMII
    mac_cfg_1 |= 1 << 13;
#endif

    // Enable loopack mode
    //mac_cfg_1 |= MCR1_LB;

    /* bit 7    bit 6        bit 5
       MCR2_ADPE    MCR2_VPE    MCR2_PCE
       x        x        0        No pad, check CRC
       0        0        1        Pad to 60B, append CRC
       x        1        1        Pad to 64B, append CRC
       1        0        1        if un-tagged, Pad to 60B, append CRC
                              if VLAN tagged, Pad to 64B, append CRC
     
       if set MCR2_PCE(bit 5)
          MCR2_CE(bit 4) must be set.
     
       We need to pad frame to 60B and append 4-byte CRC.
     */

    mac_cfg_2 = MCR2_PCE | MCR2_CE;

    // Pure preamble enforcement
    //mac_cfg_2 |= MCR2_PPE;

    // Frame length checking
    mac_cfg_2 |= MCR2_FLC;

    if (np->full_duplex) {
        mac_cfg_2 |= MCR2_FD;
        __mac_set_IPGR(0x15);
       
    } else {
       
        __mac_set_IPGR(0x12);
    }

    REG16(ETH_MAC_MCR1) = mac_cfg_1;
    REG16(ETH_MAC_MCR2) = mac_cfg_2;

    __mac_set_NIPGR1(0x0c);
    __mac_set_NIPGR2(0x12);

    //mac_regs_dump();
}

static void config_fifo(void)
{
    int    i;

    __fifo_reset_all();

    REG32(ETH_FIFO_CR0) |= 0x80000000;

    /* 4k rx FiFo */

    __fifo_set_fr_threshold(0x180);
    __fifo_set_high_wm(0x200);
    __fifo_set_low_wm(0x40);

    /* 4k tx FiFo */
/*
    __fifo_set_ft_threshold(0x0200);
    __fifo_set_ft_high_wm(0x0300);
*/
    /* for 2k FiFo both tx */
    __fifo_set_ft_threshold(0x0180);
    __fifo_set_ft_high_wm(0x01b0);

    __fifo_set_XOFF_RTX(4);
    __fifo_set_pause_control();

    REG32(ETH_FIFO_CR4) &= 0x0000;
    REG32(ETH_FIFO_CR5) |= 0xffff;

    __fifo_set_drop_cond(RSV_CRCE);
    __fifo_set_dropdc_cond(RSV_CRCE);
#if 0
    __fifo_set_drop_cond(RSV_MP);
    __fifo_set_dropdc_cond(RSV_MP);

    __fifo_set_drop_cond(RSV_BP);
    __fifo_set_dropdc_cond(RSV_BP);
#endif
    __fifo_set_drop_cond(RSV_LCE);
    __fifo_set_dropdc_cond(RSV_LCE);

    __fifo_enable_all_modules();

    for (i = 0;
         i < MAX_WAIT && !__fifo_all_enabled();
         i++, udelay(100)) {
        ;
    }

    if (i == MAX_WAIT) {
        printk("config_fifo: Wait time out !\n");
        return;
    }
    //fifo_regs_dump();
}

#if 0
static void config_sal(void)
{
    /* SAL config */
    __sal_set_mode(AFR_AMC | AFR_ABC);
    __sal_set_hash_table(0, 0);
}
#endif
	
/*
static void config_stat(void)
{
    __stat_disable();
    __stat_clear_counters();
    // clear carry registers
    REG32(ETH_STAT_CAR1) = REG32(ETH_STAT_CAR1);
    REG32(ETH_STAT_CAR2) = REG32(ETH_STAT_CAR2);
    __stat_disable_carry_irq();
    __stat_enable_carry_irq();
    //printk("CARM1 = 0x%08x, CARM2 = 0x%08x\n", REG32(ETH_STAT_CARM1), REG32(ETH_STAT_CARM2));

    __stat_enable();
    printk("Enable eth stat module.\n");

}
*/

static int autonet_complete(struct net_device *dev)
{
    int    i;
    struct gp_eth_private *np = netdev_priv(dev);//(struct gp_eth_private *)P2ADDR(netdev_priv(dev));

    for (i = 0;
         i < MAX_WAIT && !(mdio_read(dev, np->valid_phy, MII_BMSR) & 0x0020);
         i++, udelay(10)) {
        ;
    }

    if (i == MAX_WAIT) {
        printk("%s: Autonet time out!\n", dev->name);
        return -1;
    }

    return 0;
}

static void config_phy(struct net_device *dev)
{

    struct gp_eth_private *np = netdev_priv(dev);//(struct gp_eth_private *)P2ADDR(netdev_priv(dev));
    u32    mii_anlpar, phy_mode;
	
    mii_anlpar = mdio_read(dev, np->valid_phy, MII_ANLPAR);

    if (mii_anlpar != 0xffff) {
        mii_anlpar = mdio_read(dev, np->valid_phy, MII_ANLPAR); // read twice to make data stable
        if ((mii_anlpar & 0x0100) || (mii_anlpar & 0x01C0) == 0x0040) {
            np->full_duplex = 1;
        }
        phy_mode = mii_anlpar;

		// ==> Set 10M/100M
        if (phy_mode & MII_ANLPAR_100M)
           REG32(ETH_MAC_PSR) |= PSR_OS;

        printk("%s: Setting %s %s-duplex based on MII tranceiver #%d\n",
               dev->name, (phy_mode & MII_ANLPAR_100M) ? "100Mbps" : "10Mbps",
               np->full_duplex ? "full" : "half", np->mii_phy_cnt);

    } else {
        printk("%s: config_phy: MII_ANLPAR is 0xFFFF, may be error ???\n", dev->name);
    }

}

/*
 * Ethernet device hardware init
 * This routine initializes the ethernet device hardware and PHY
 */
static int gp_init_hw(struct net_device *dev)
{

    struct gp_eth_private *np = netdev_priv(dev);//(struct gp_eth_private *)P2ADDR(netdev_priv(dev));
    struct ethtool_cmd ecmd;
    int i;
	unsigned int regVal;

	#define P_SCUC_MAC			((volatile unsigned int *)(0xfd00514C))

	regVal = *P_SCUC_MAC;
	regVal |= 1;
#ifdef ETH_BUS_BUG
	regVal &= ~(1 << 31);
#else
	regVal |= (1 << 31);
#endif
	*P_SCUC_MAC = regVal;

    __eth_disable_irq();
    __mac_set_mii_clk(0x7);
    __mac_reset();
printk("ETH_MAC_MCR1 = %x\n",REG32(ETH_MAC_MCR1));
#if 0
    /* MII operation */
    if (gp_phy_reset(dev)) {
        errprintk("Ethernet PHY device does not reset!\n");
        //return operation not permitted
        return -EPERM;
    }
#endif

    __eth_set_mac_address(dev->dev_addr[0], dev->dev_addr[1],
                  dev->dev_addr[2], dev->dev_addr[3],
                  dev->dev_addr[4], dev->dev_addr[5]);

    printk("%s: GPL83000 On-Chip ethernet (MAC ", dev->name);
    for (i = 0; i < 5; i++) {
        printk("%2.2x:", dev->dev_addr[i]);
    }
    printk("%2.2x, IRQ %d)\n", dev->dev_addr[i], dev->irq);

REG32(ETH_MAC_MCFGR) |= 0x1c;

    np->mii_phy_cnt = gp_search_mii_phy(dev);
    printk("%s: Found %d PHY on GP MAC\n", dev->name, np->mii_phy_cnt);
//  New code, could make it work on 100M mode ...
    if (autonet_complete(dev))
        printk("ETH Auto-Negotiation failed\n");
   
    config_phy(dev);
//  old configuration
    mii_info.phy_id = np->valid_phy;
    mii_info.dev = dev;
    mii_info.mdio_read = &mdio_read;
    mii_info.mdio_write = &mdio_write;

    ecmd.speed = SPEED_100;
    ecmd.duplex = DUPLEX_FULL;
    ecmd.port = PORT_MII;
    ecmd.transceiver = XCVR_INTERNAL;
    ecmd.phy_address = np->valid_phy;
    ecmd.autoneg = AUTONEG_ENABLE;


    config_mac(dev);

    config_fifo();

/* jerry@101108: this IP without PE-SAL register */
    //config_sal();

//    config_stat();
#ifdef ETH_DEBUG
	stat_regs_dump();
#endif
    /* Set base address of TX and RX descriptors */
    __eth_set_rx_desc_addr(np->dma_rx_ring);
    __eth_set_tx_desc_addr(np->dma_tx_ring);

    /* Burst length: 4, 8 or 16 */
    //__eth_dma_set_burst_len(BURST_LEN_4);

    //dma_regs_dump("set burst length\n");

    /* Clear status registers */
    __eth_clear_rx_flags();
    __eth_clear_rx_pkt_cnt();
    __eth_clear_tx_flags();
    __eth_clear_tx_pkt_cnt();

    __eth_enable_irq();
    __eth_dma_rx_enable();

    return 0;
}

static void gp_eth_read_stats(struct gp_eth_private *np, int carry1, int carry2)
{
	int i=0;
	//int tmp;

	printk("ETH_FIFO_RAR7 :");
	for(i = 0 ; i < 50 ; i ++)
	{
		REG32(ETH_FIFO_RAR6) |= (RAR2_HT_R_REQ + i);
		//printk("addr_0x68 : %x\n",REG32(ETH_FIFO_RAR6));
		printk("%8x  ", REG32(ETH_FIFO_RAR7));
		REG32(ETH_FIFO_RAR6) &= (~ (RAR2_HT_R_REQ + i));
		
	}
	printk("\n");
#if 0
	printk("ETH_FIFO_RAR6 ([19:16]) :");
	for(i = 0 ; i < 50 ; i ++)
	{
		REG32(ETH_FIFO_RAR6) |= (RAR2_HT_R_REQ + i);
		tmp = (REG32(ETH_FIFO_RAR6) >> 16) & 0xf;
		printk("%8x  ", tmp);
		REG32(ETH_FIFO_RAR6) &= (~ (RAR2_HT_R_REQ + i));		
	}
	
	printk("\n");


for(i = 0 ; i < 10 ; i++)
{
	mdelay(100);
	printk("DMA_RSR: %x\n",REG32(ETH_DMA_RSR) );
	
}
#endif
    return ;

    if (carry1 != 0) {
        if (carry1 & CAR1_RPK) np->carry_counters[CNT_RPKT]++;
        if (carry1 & CAR1_RBY) np->carry_counters[CNT_RBYT]++;
        if (carry1 & CAR1_RFC) np->carry_counters[CNT_RFCS]++;
        if (carry1 & CAR1_RDR) np->carry_counters[CNT_RDRP]++;
        if (carry1 & CAR1_RMC) np->carry_counters[CNT_RMCA]++;
        printk("carry1 = 0x%08x\n", carry1);
    }

    if (carry2 != 0) {
        if (carry2 & CAR2_TPK) np->carry_counters[CNT_TPKT]++;
        if (carry2 & CAR2_TBY) np->carry_counters[CNT_TBYT]++;
        if (carry2 & CAR2_TFC) np->carry_counters[CNT_TFCS]++;
        if (carry2 & CAR2_TDP) np->carry_counters[CNT_TDRP]++;
        if (carry2 & CAR2_TNC) np->carry_counters[CNT_TNCL]++;
        printk("carry2 = 0x%08x\n", carry2);
    }

    np->stats.rx_packets    = REG32(ETH_STAT_RPKT) + (np->carry_counters[CNT_RPKT] << 18);
    np->stats.tx_packets    = REG32(ETH_STAT_TPKT) + (np->carry_counters[CNT_TPKT] << 18);
    np->stats.rx_bytes    = REG32(ETH_STAT_RBYT) + (np->carry_counters[CNT_RBYT] << 24);
    np->stats.tx_bytes    = REG32(ETH_STAT_TBYT) + (np->carry_counters[CNT_TBYT] << 24);
    np->stats.rx_errors    = REG32(ETH_STAT_RFCS) + (np->carry_counters[CNT_RFCS] << 12);
    np->stats.tx_errors    = REG32(ETH_STAT_TFCS) + (np->carry_counters[CNT_TFCS] << 12);
    np->stats.rx_dropped    = REG32(ETH_STAT_RDRP) + (np->carry_counters[CNT_RDRP] << 12);
    np->stats.tx_dropped    = REG32(ETH_STAT_TDRP) + (np->carry_counters[CNT_TDRP] << 12);
    np->stats.multicast    = REG32(ETH_STAT_RMCA) + (np->carry_counters[CNT_RMCA] << 18);
    np->stats.collisions    = REG32(ETH_STAT_TNCL) + (np->carry_counters[CNT_TNCL] << 13);

    //counters_dump(np);

}

static struct tasklet_struct gp_eth_rx_done_task;
static void eth_rxready(unsigned long data);

static int gp_eth_open(struct net_device *dev)
{

    struct gp_eth_private *np = netdev_priv(dev);//(struct gp_eth_private *)P2ADDR(netdev_priv(dev));
    int retval, i;

    retval = request_irq(dev->irq, gp_eth_interrupt, 0, dev->name, dev);
    if (retval) {
        errprintk("%s: Unable to get IRQ %d .\n", dev->name, dev->irq);
        return -EAGAIN;
    }


tasklet_init(&gp_eth_rx_done_task, eth_rxready, (unsigned long)dev);
/* jerry : change descriptors as non-cacheable*/	
#if 0	
    for (i = 0; i < NUM_RX_DESCS; i++) {
        np->rx_ring[i].pkt_addr    = cpu_to_le32(np->dma_rx_buf + i * RX_BUF_SIZE);
	//printk("rx_ring %d addr =%x \n",i,np->rx_ring[i].pkt_addr );
        np->rx_ring[i].pkt_size    = cpu_to_le32(EMPTY_FLAG_MASK);
        np->rx_ring[i].next_desc= cpu_to_le32(np->dma_rx_ring + (i+1) * sizeof (gp_desc_t));
    }
    np->rx_ring[NUM_RX_DESCS - 1].next_desc = cpu_to_le32(np->dma_rx_ring);

    for (i = 0; i < NUM_TX_DESCS; i++) {
        np->tx_ring[i].pkt_addr    = cpu_to_le32(np->dma_tx_buf + i * TX_BUF_SIZE);//cpu_to_le32(0);
        //printk("tx_ring %d addr =%x \n",i,np->tx_ring[i].pkt_addr );
        np->tx_ring[i].pkt_size    = cpu_to_le32(EMPTY_FLAG_MASK);
        np->tx_ring[i].next_desc= cpu_to_le32(np->dma_tx_ring + (i+1) * sizeof (gp_desc_t));
    }
    np->tx_ring[NUM_TX_DESCS - 1].next_desc = cpu_to_le32(np->dma_tx_ring);
#else

	/* init tx_list */
	tx_list_head = tx_list_tail = tx_desc;

	for (i = 0; i < NUM_TX_DESCS; i++) {
		struct net_dma_desc_t *t = tx_desc + i;
		gp_desc_t *d = &(t->desc);
	
		//printk("[%d] t: 0x%x, d: 0x%x, ",i,t,d);
		d->pkt_addr = cpu_to_le32(np->dma_tx_buf + i * TX_BUF_SIZE);
		d->pkt_size = cpu_to_le32(EMPTY_FLAG_MASK);
		d->next_desc = cpu_to_le32(np->dma_tx_ring + (i+1) * sizeof (struct net_dma_desc_t));;
		if(i == NUM_TX_DESCS - 1)
		{
			d->next_desc = cpu_to_le32(np->dma_tx_ring);
		}
		//printk("pkt_addr = 0x%x, pkt_size = 0x%x, next_desc = 0x%x\n",d->pkt_addr,d->pkt_size,d->next_desc);
		tx_list_tail->next = t;
		tx_list_tail = t;
	}
	tx_list_tail->next = tx_list_head;	/* tx_list is a circle */
	current_tx_ptr = tx_list_head;

	/* init rx_list */
	rx_list_head = rx_list_tail = rx_desc;

	for (i = 0; i < NUM_RX_DESCS; i++) {
		struct net_dma_desc_t *r = rx_desc + i;
		gp_desc_t *d = &(r->desc);
	
		//printk("[%d] r: 0x%x, d: 0x%x, ",i,r,d);
		d->pkt_addr = cpu_to_le32(np->dma_rx_buf + i * RX_BUF_SIZE);
		d->pkt_size = cpu_to_le32(EMPTY_FLAG_MASK);
		d->next_desc = cpu_to_le32(np->dma_rx_ring + (i+1) * sizeof (struct net_dma_desc_t));;
		if(i == NUM_RX_DESCS - 1)
		{
			d->next_desc = cpu_to_le32(np->dma_rx_ring);
		}
		//printk("pkt_addr = 0x%x, pkt_size = 0x%x, next_desc = 0x%x\n",d->pkt_addr,d->pkt_size,d->next_desc);
		rx_list_tail->next = r;
		rx_list_tail = r;
	}
	
	rx_list_tail->next = rx_list_head;	/* rx_list is a circle */
	current_rx_ptr = rx_list_head;	

#endif	
    np->rx_head = 0;
    np->tx_head = np->tx_tail = 0;

    //desc_dump(dev);

    for (i = 0; i < STAT_CNT_NUM; i++) {
        np->carry_counters[i] = 0;
    }

    gp_init_hw(dev);

    dev->trans_start = jiffies;
    netif_start_queue(dev);
    start_check(dev);

    return 0;
}

static int gp_eth_close(struct net_device *dev)
{
printk("[Jerry] gp_eth_close...\n");	
    netif_stop_queue(dev);
    close_check(dev);

    __eth_disable();
   printk("[Jerry] check close 5\n");
    free_irq(dev->irq, dev);
	printk("[Jerry] check close 6\n");
    return 0;
}

/*
 * Get the current statistics.
 * This may be called with the device open or closed.
 */
static struct net_device_stats * gp_eth_get_stats(struct net_device *dev)
{

    struct gp_eth_private *np = netdev_priv(dev);//(struct gp_eth_private *)P2ADDR(netdev_priv(dev));


//    unsigned int    flags;

//    spin_lock_irqsave(&np->lock, flags);
    gp_eth_read_stats(np, 0, 0);

//    spin_unlock_irqrestore(&np->lock, flags);
    return &np->stats;
}

/*
 * ethtool routines
 */
static int gp_ethtool_ioctl(struct net_device *dev, void *useraddr)
{

    struct gp_eth_private *np = netdev_priv(dev);//(struct gp_eth_private *)P2ADDR(netdev_priv(dev));
    u32 ethcmd;

    /* dev_ioctl() in ../../net/core/dev.c has already checked
      capable(CAP_NET_ADMIN), so don't bother with that here.  */

    if (get_user(ethcmd, (u32 *)useraddr))
        return -EFAULT;

    switch (ethcmd) {

    case ETHTOOL_GDRVINFO: {
        struct ethtool_drvinfo info = { ETHTOOL_GDRVINFO };
        strcpy (info.driver, DRV_NAME);
        strcpy (info.version, DRV_VERSION);
        strcpy (info.bus_info, "OCS");
        if (copy_to_user (useraddr, &info, sizeof (info)))
            return -EFAULT;
        return 0;
    }

    /* get settings */
    case ETHTOOL_GSET: {
        struct ethtool_cmd ecmd = { ETHTOOL_GSET };
        spin_lock_irq(&np->lock);
        mii_ethtool_gset(&mii_info, &ecmd);
        spin_unlock_irq(&np->lock);
        if (copy_to_user(useraddr, &ecmd, sizeof(ecmd)))
            return -EFAULT;
        return 0;
    }
    /* set settings */
    case ETHTOOL_SSET: {
        int r;
        struct ethtool_cmd ecmd;
        if (copy_from_user(&ecmd, useraddr, sizeof(ecmd)))
            return -EFAULT;
        spin_lock_irq(&np->lock);
        r = mii_ethtool_sset(&mii_info, &ecmd);
        spin_unlock_irq(&np->lock);
        return r;
    }
    /* restart autonegotiation */
    case ETHTOOL_NWAY_RST: {
        return mii_nway_restart(&mii_info);
    }
    /* get link status */
    case ETHTOOL_GLINK: {
        struct ethtool_value edata = {ETHTOOL_GLINK};
        edata.data = mii_link_ok(&mii_info);
        if (copy_to_user(useraddr, &edata, sizeof(edata)))
            return -EFAULT;
        return 0;
    }
   /* get message-level */
    case ETHTOOL_GMSGLVL: {
        struct ethtool_value edata = {ETHTOOL_GMSGLVL};
        edata.data = debug;
        if (copy_to_user(useraddr, &edata, sizeof(edata)))
            return -EFAULT;
        return 0;
    }
    /* set message-level */
    case ETHTOOL_SMSGLVL: {
        struct ethtool_value edata;
        if (copy_from_user(&edata, useraddr, sizeof(edata)))
            return -EFAULT;
        debug = edata.data;
        return 0;
    }

    default:
        break;
    }

    return -EOPNOTSUPP;
}

/*
 * Config device
 */
static int gp_eth_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{

    struct gp_eth_private *np = netdev_priv(dev);//(struct gp_eth_private *)P2ADDR(netdev_priv(dev));
    struct mii_ioctl_data *data, rdata;

    switch (cmd) {
    case SIOCETHTOOL:
        return gp_ethtool_ioctl(dev, (void *) rq->ifr_data);
    case SIOCGMIIPHY:
    case SIOCDEVPRIVATE:
        data = (struct mii_ioctl_data *)&rq->ifr_data;
        data->phy_id = np->valid_phy;
    case SIOCGMIIREG:
    case SIOCDEVPRIVATE+1:
        data = (struct mii_ioctl_data *)&rq->ifr_data;
        data->val_out = mdio_read(dev,np->valid_phy, data->reg_num & 0x1f);
        return 0;
    case SIOCSMIIREG:
    case SIOCDEVPRIVATE+2:
        data = (struct mii_ioctl_data *)&rq->ifr_data;
        if (!capable(CAP_NET_ADMIN))
            return -EPERM;
        mdio_write(dev,np->valid_phy, data->reg_num & 0x1f, data->val_in);
        return 0;
    case READ_COMMAND:    
        data = (struct mii_ioctl_data *)rq->ifr_data;
        if (copy_from_user(&rdata,data,sizeof(rdata)))
            return -EFAULT;
        rdata.val_out = mdio_read(dev,rdata.phy_id, rdata.reg_num & 0x1f);
        if (copy_to_user(data,&rdata,sizeof(rdata)))
            return -EFAULT;
        return 0;
    case WRITE_COMMAND:
        if (np->phy_type==1) {
            data = (struct mii_ioctl_data *)rq->ifr_data;
            if (!capable(CAP_NET_ADMIN))
                return -EPERM;
            if (copy_from_user(&rdata,data,sizeof(rdata)))
                return -EFAULT;
            mdio_write(dev,rdata.phy_id, rdata.reg_num & 0x1f, rdata.val_in);
        }
        return 0;
    case GETDRIVERINFO:
        if (np->phy_type==1) {
            data = (struct mii_ioctl_data *)rq->ifr_data;
            if (copy_from_user(&rdata,data,sizeof(rdata)))
                return -EFAULT;
            rdata.val_in = 0x1;
            rdata.val_out = 0x00d0;
            if (copy_to_user(data,&rdata,sizeof(rdata)))
                return -EFAULT;
        }
        return 0;
    default:
        return -EOPNOTSUPP;
    }
    return 0;
}

/*
 * Received one packet
 */
// static int timeout_count = 0;
static unsigned long g_counter_isr = 0;
static unsigned long g_counter_dsr = 0;
static void eth_rxready(unsigned long data/*struct net_device *dev, int counter*/)
{

    struct net_device *dev = (struct net_device *)data;
    struct gp_eth_private *np = netdev_priv(dev);//(struct gp_eth_private *)P2ADDR(netdev_priv(dev));
    gp_desc_t    *rx_current_desc;
    struct sk_buff    *skb;
    unsigned char    *pkt_ptr;
    u32        pkt_len;
    //int counter_bak = counter;
    int i = 0;
    int counter = (g_counter_isr - g_counter_dsr);
#if 0	
    if (0 == counter || counter >= 2) {
        printk("%s: eth_rxready ... BUG_ON 1 ...counter =%d\n", dev->name, counter);
        BUG_ON(1);
    }	
#endif	

	if (counter < 0) {
		counter += COUNTER_MAX;	
	}

	
    while (counter > 0) {

	/* jerry : change descriptors as non-cacheable*/		
	#if 0
        rx_current_desc = &(np->rx_ring[np->rx_head]);
	 dma_cache_inv((unsigned int)rx_current_desc, sizeof(gp_desc_t));
	#else
	rx_current_desc = &(current_rx_ptr->desc);
	//printk("cu : %x \n",rx_current_desc->next_desc);
	#endif

	//if(__desc_get_empty_flag(le32_to_cpu(rx_current_desc->pkt_size))) {
	if(__desc_get_empty_flag(rx_current_desc->pkt_size)) {

		
 		break;
	     #if 0
	     printk("%s: ?????????? fix me ??? %s, %d\n", dev->name, __FUNCTION__, __LINE__);
            printk("counter_bak = %d, counter = %d, rx_current_desc = 0x%x, next->desc = 0x%x\n",\
             	counter_bak, counter,(unsigned int)rx_current_desc,rx_current_desc->next_desc);
            return;
	     #endif
        }

	 i++;
	 
	#if 0
        pkt_ptr = (unsigned char *)((rx_current_desc->pkt_addr) | 0xc0000000);
	#endif
        pkt_ptr = (unsigned char  *)current_vaddr_rx_buf;
        pkt_len = (__desc_get_pkt_size(le32_to_cpu(rx_current_desc->pkt_size))) - 4;

        np->stats.rx_packets++;
        np->stats.rx_bytes += pkt_len;

        skb = dev_alloc_skb(pkt_len + 2);
        if (skb == NULL) {
            printk("%s: Memory squeeze, dropping. dev_alloc_skb(0x%08x)\n",
                   dev->name, pkt_len + 2);
            np->stats.rx_dropped++;
	/* move	to isr */			
            //__eth_reduce_pkt_recv_cnt();
        }
        skb->dev = dev;
        skb_reserve(skb, 2); /* 16 byte align */

	 #if 0
        dma_cache_inv((unsigned int)pkt_ptr, pkt_len);
        #else
        memcpy(skb->data, pkt_ptr, pkt_len);
	#endif

	current_vaddr_rx_buf += RX_BUF_SIZE;
	if(current_vaddr_rx_buf == end_vaddr_rx_buf)
		current_vaddr_rx_buf = 	start_vaddr_rx_buf;
	
        skb_put(skb, pkt_len);
        skb->protocol = eth_type_trans(skb,dev);

        netif_rx(skb);    /* pass the packet to upper layers */
        dev->last_rx = jiffies;
        rx_current_desc->pkt_size = EMPTY_FLAG_MASK;

        //dma_cache_wback((unsigned int)rx_current_desc, sizeof(gp_desc_t));
	current_rx_ptr = current_rx_ptr->next;
	/* move	to isr */
	//__eth_reduce_pkt_recv_cnt();

	g_counter_dsr++;
	if (g_counter_dsr > COUNTER_MAX) {
		g_counter_dsr -= COUNTER_MAX;	
	}

	counter = (g_counter_isr - g_counter_dsr);
	if (counter < 0) {
		counter += COUNTER_MAX;	
	}
	
        np->rx_head == NUM_RX_DESCS - 1 ? np->rx_head = 0 : np->rx_head++;
        BUG_ON(np->rx_head >= NUM_RX_DESCS);
    }

#if 0
    if (i == 0) {
        printk("%s: eth_rxready ... BUG_ON 2\n", dev->name);
        BUG_ON(1);
    } 
#endif	


}

/*
 * Tx timeout routine
 */
static void gp_eth_tx_timeout(struct net_device *dev)
{
    gp_init_hw(dev);
    netif_wake_queue(dev);
}

/*
 * One packet was transmitted
 */
static void eth_txdone(struct net_device *dev, int counter)
{

    struct gp_eth_private *np = netdev_priv(dev);//(struct gp_eth_private *)P2ADDR(netdev_priv(dev));
    int tx_tail = np->tx_tail;

    while (counter-- > 0) {

#if NUM_RX_DESCS == 1
        int entry = 0;
#else
        int entry = tx_tail % NUM_TX_DESCS;
#endif

        /* Free the original skb */
        if (np->tx_skb[entry]) 
	 {
            np->stats.tx_packets++;
			
		/* jerry : change descriptors as non-cacheable */	
		#if 0	
            np->stats.tx_bytes += le32_to_cpu(np->tx_ring[entry].pkt_size);		
		#endif
            dev_kfree_skb_irq(np->tx_skb[entry]);

            np->tx_skb[entry] = 0;
        } 

	 else {
            //printk("%s: ?????????? fix me ??? %s, %d\n", dev->name, __FUNCTION__, __LINE__);
            //desc_dump(dev);
            //dma_regs_dump("");
        }

        tx_tail++;
	
	
        __eth_reduce_pkt_sent_cnt();
    }

    if (np->tx_full && (tx_tail + NUM_TX_DESCS > np->tx_head + 1)) {
        /* The ring is no longer full */
        np->tx_full = 0;
        netif_start_queue(dev);
    }
    np->tx_tail = tx_tail;
	
	//printk("tx_tail = %d\n",np->tx_tail);
}

/*
 * Update the tx descriptor
 */

static void load_tx_packet(struct net_device *dev, char *buf, u32 length, struct sk_buff *skb)
{

    struct gp_eth_private *np = netdev_priv(dev);//(struct gp_eth_private *)P2ADDR(netdev_priv(dev));
    int entry = np->tx_head % NUM_TX_DESCS;
    //int i;
    u32 pkt_size = 0;

//printk("B: 0x%x,  V: 0x%x, S: 0x%x, L: %d",np->tx_ring[entry].pkt_addr,bus_to_virt(np->tx_ring[entry].pkt_addr),buf,length);
//skb_dump(skb);
	
	/* jerry : change descriptors as non-cacheable */
	#if 0
	memcpy(phys_to_virt(np->tx_ring[entry].pkt_addr),(void *)buf,length);
  //  np->tx_ring[entry].pkt_addr = cpu_to_le32(virt_to_bus(buf));
    	np->tx_ring[entry].pkt_size = cpu_to_le32(length & ~EMPTY_FLAG_MASK);	
  	dma_cache_maint(&np->tx_ring[entry], sizeof(gp_desc_t), DMA_TO_DEVICE);
	#else
	gp_desc_t    *tx_current_desc;	
	tx_current_desc = &(current_tx_ptr->desc);
	memcpy((void *)current_vaddr_tx_buf,(void *)buf,length);
	current_vaddr_tx_buf += TX_BUF_SIZE;
	if(current_vaddr_tx_buf == end_vaddr_tx_buf)
		current_vaddr_tx_buf = 	start_vaddr_tx_buf;

	
	current_tx_ptr = current_tx_ptr->next ;

	dmb();
	//printk("p: 0x%x, v: 0x%x  \n",tx_current_desc->pkt_addr, phys_to_virt(tx_current_desc->pkt_addr));
    	tx_current_desc->pkt_size = cpu_to_le32(length & ~EMPTY_FLAG_MASK);		

	
	pkt_size = tx_current_desc->pkt_size;
	#endif

	np->tx_skb[entry] = skb;
	np->stats.tx_bytes += length;

	
//printk("pkt_addr = %x, pkt_size = %x\n",np->tx_ring[entry].pkt_addr,np->tx_ring[entry].pkt_size);
    /* Notice us when will send a packet which begin with address: xxx1(binary) */
#if 0	
    if (unlikely(np->tx_ring[entry].pkt_addr & 0x1)) {
        skb_dump(skb);
	
        printk("desc.pktaddr = 0x%08x\n", np->tx_ring[entry].pkt_addr);
	 
        for (i = -2; i < 16; i++) {
            printk("%02x ", *((unsigned char *)le32_to_cpu(bus_to_virt(np->tx_ring[entry].pkt_addr)) + i));
        }
        printk("\n");
        for (i = -2; i < 16; i++) {
            printk("%02x ", *((unsigned char *)le32_to_cpu(buf) + i));
        }
        printk("\n");
        for (i = -2; i < 16; i++) {
            printk("%02x ", *((unsigned char *)(bus_to_virt(np->tx_ring[entry].pkt_addr)) + i));
        }
	printk("\n");
       
    }
#endif	
}

/*
 * Transmit one packet
 */
static int gp_eth_send_packet(struct sk_buff *skb, struct net_device *dev)
{

    struct gp_eth_private *np = netdev_priv(dev);//(struct gp_eth_private *)P2ADDR(netdev_priv(dev));
    u32 length;

	//printk("[Jerry] gp_eth_send_packet start..\n");
    if (np->tx_full) {
	 printk("tx buf is full!\n");
        return 0;
    }

    length = (skb->len < ETH_ZLEN) ? ETH_ZLEN : skb->len;
	//printk("tx length = %d\n",length);

    load_tx_packet(dev, (char *)skb->data, length, skb);
    

    spin_lock_irq(&np->lock);

    np->tx_head++;
//printk("tx_head = %d\n",np->tx_head);
    /* Start TX */
//printk("Start TX...\n");
    __eth_dma_tx_enable();

    /* for timeout */
    dev->trans_start = jiffies;

	
    if (np->tx_tail + NUM_TX_DESCS > np->tx_head + 1) {
        np->tx_full = 0;
    } else {
        np->tx_full = 1;
        netif_stop_queue(dev);
    }

    spin_unlock_irq(&np->lock);
//printk("tx_head: %d ,  tx_tail: %d , tx full: %d\n\n",np->tx_head, np->tx_tail,np->tx_full);	
    return 0;
}

/*
 * Interrupt service routine
 */
static irqreturn_t gp_eth_interrupt(int irq, void *dev_id)
{
    struct net_device    *dev = (struct net_device *)dev_id;
    struct gp_eth_private *np = netdev_priv(dev);//(struct gp_eth_private *)P2ADDR(netdev_priv(dev));
    unsigned int    tx_sts;
    unsigned int    rx_sts;
    unsigned int    int_sts;
    unsigned int    counter;
//    unsigned int    carry1 = 0;
//    unsigned int    carry2 = 0;

    spin_lock(&np->lock);

    //__eth_disable_irq();

    /* Read tx & rx state reg, which indicate action */
      
    int_sts = REG32(ETH_DMA_IR);	
	
    //printk("sts=[tx:%#8x rx:%#8x] \n",tx_sts,rx_sts);
	
    /* Rx PKTRECV: one frame has already been received. Receive frame until there is no PKTRECV */
	if(int_sts & IMR_PKTRECV){
	 rx_sts = REG32(ETH_DMA_RSR);
	 //printk("sts=[rx:%#8x] \n",rx_sts);
    //if (rx_sts & RSR_PKTRECV) {
        /* To reduce counter and PKTRECV bit,
         * __eth_reduce_pkt_recv_cnt() must called after each round */
        counter = (rx_sts & RSR_PKTCNT_MASK) >> 16;
	 //__eth_disable_rx_irq();
	 //printk("a:%x ",REG32(ETH_DMA_IMR));
        if (counter != 0) {
	     //printk("(rx)interrupt...\n");
	     //eth_rxready(dev, counter);
	     	 
	     while(counter-- > 0) 
	     {
			__eth_reduce_pkt_recv_cnt();
			g_counter_isr++; 
			if (g_counter_isr > COUNTER_MAX) {
				g_counter_isr -= COUNTER_MAX;	
			}			
	     }

	     
	     counter =  __eth_get_rx_pkt_cnt();	 		 
	     
	     tasklet_schedule(&gp_eth_rx_done_task);
	 

        }
        //} else {
        //    printk("*");
            //printk("%s: Packet has been received but count is 0\n", dev->name);
            //BUG_ON(1);
        //}
    }

    /* Tx PKTSENT: one or more frames have already been sent. Current tx round has completed. */
	if(int_sts & IMR_PKTSENT){	

  	 tx_sts = REG32(ETH_DMA_TSR);
    //if (tx_sts & TSR_PKTSENT) {
        /* To clear counter and PKTSENT bit,
         * __eth_reduce_pkt_sent_cnt() was called inside */
         	
        counter = (tx_sts & TSR_PKTCNT_MASK) >> 16;
	 
        if (counter != 0) {
            eth_txdone(dev, counter);
		 //counter =  __eth_get_tx_pkt_cnt();
        }
        //} else {
        //    printk("$");
            //printk("%s: Packet has been sent but counter is 0\n", dev->name);
            //BUG_ON(1);
        //}
    }	

    /* Fatal bus error */
	if (int_sts & IMR_TBUSERR || int_sts & IMR_RBUSERR) {
    //if (tx_sts & TSR_BUSERR || rx_sts & RSR_BUSERR) {
        __eth_disable();
    __eth_clear_tx_flags(); /* Clear UNDERRUN and BUSERROR */
    __eth_clear_rx_flags(); /* Clear OVERFLOW and BUSERROR */		
        printk("%s: Fatal bus error occurred, sts=[%#8x %#8x], device stopped.\n", dev->name, REG32(ETH_DMA_TSR), REG32(ETH_DMA_RSR));
        goto _exit_irq;
    }

    /* Rx overflow */
	if (int_sts & IMR_OVERFLOW) {	
    //if (rx_sts & RSR_OVERFLOW) {
        printk("RX_OVERFLOW\n");
        	//__eth_dma_rx_disable();
	    //__eth_clear_rx_flags(); /* Clear OVERFLOW and BUSERROR */		
	 __eth_dma_rx_enable();
    }
#if 0
    /* Tx underrun, it occurred after every tx round */
	if (int_sts & IMR_UNDERRUN) {
    //if (tx_sts & TSR_UNDERRUN) {
	__eth_clear_tx_flags(); /* Clear UNDERRUN and BUSERROR */
	
        //printk("TX_UNDERRUN\n");
    }
#endif

#if 0	
	if( !(int_sts & IMR_ALL_IRQ) ){
    //if ( !(tx_sts & TSR_FLAGS) && !(rx_sts & RSR_FLAGS) ) {
	#if 0
        carry1 = REG32(ETH_STAT_CAR1);
        carry2 = REG32(ETH_STAT_CAR2);
        REG32(ETH_STAT_CAR1) = carry1;
        REG32(ETH_STAT_CAR2) = carry2;
	#endif
	printk("%s: Enter unknown state, device stopped\n",dev->name);
	//printk("tx_sts :%x,  rx_sts : %x\n",tx_sts,rx_sts);
        //goto _exit_irq;
    }
#endif	
_exit_irq:

    //__eth_enable_irq();

    spin_unlock(&np->lock);

    return IRQ_HANDLED;
}



static const struct net_device_ops gp_netdev_ops = {

    .ndo_open = gp_eth_open,
    .ndo_stop = gp_eth_close,
    .ndo_start_xmit = gp_eth_send_packet,
    .ndo_get_stats = gp_eth_get_stats,
    .ndo_set_multicast_list = gp_set_multicast_list,
    .ndo_do_ioctl = gp_eth_ioctl,
    .ndo_tx_timeout = gp_eth_tx_timeout,
   
};


/**                                                                         
 * @brief   ethernet device release                                   
 */                                                                         
static void gp_eth_device_release(struct device *dev)                       
{                                                                           
}  

static struct platform_device gp_eth_device = {
	.name	= "gp-eth",
	.id	= -1,
	.dev	= {                                                                 
		.release = gp_eth_device_release,                                       
	},  
};

#ifdef CONFIG_PM
static int gp_eth_suspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

static int gp_eth_resume(struct platform_device *pdev)
{
	return 0;
}
#else
#define gp_eth_suspend 	NULL
#define gp_eth_resume 	NULL
#endif


static struct platform_driver gp_eth_driver = {
	.suspend = gp_eth_suspend,
	.resume = gp_eth_resume,
	.driver	= {
		.owner	= THIS_MODULE,
		.name	= "gp-eth"
	},
};


static int __init gp_eth_init(void)
{
    struct net_device *dev;
    struct gp_eth_private *np;
    int err;
	unsigned int regVal;

	/* clock */
	//#define P_SCUC_PERI_CLKEN	((volatile unsigned int *)(0xfd005004))
	//regVal = *P_SCUC_PERI_CLKEN;
	//regVal |= (1 << 13 | 1 << 22 | 1 << 30);
	//*P_SCUC_PERI_CLKEN = regVal;
	gp_enable_clock( (int*)"EMAC", 1 );

	/* input enable */
	#define P_MAC_INPUT_ENABLE	((volatile unsigned int *)(0xfc005100))
	regVal = *P_MAC_INPUT_ENABLE;
	regVal|= (1 << 6 | 1 << 7 | 1 << 18 | 1 << 19);
	*P_MAC_INPUT_ENABLE = regVal;

	//printk("[jerry] check 1\n");

    dev = alloc_etherdev(sizeof(struct gp_eth_private));
    if (!dev) {
        printk(KERN_ERR "%s: Alloc_etherdev failed\n", DRV_NAME);
        return -ENOMEM;
    }
    netdev = dev;

	//printk("[jerry] check 2\n");

    np = netdev_priv(dev);//(struct gp_eth_private *)P2ADDR(netdev_priv(dev));
	//printk("[jerry] check 3\n");	
    memset(np, 0, sizeof(struct gp_eth_private));


	
	//printk("[jerry] check 4\n");
    np->vaddr_rx_buf = (u32)dma_alloc_coherent(NULL, NUM_RX_DESCS * RX_BUF_SIZE,
                              &np->dma_rx_buf, GFP_KERNEL);

    np->vaddr_tx_buf = (u32)dma_alloc_coherent(NULL, NUM_TX_DESCS * TX_BUF_SIZE,
                              &np->dma_tx_buf, GFP_KERNEL);

	printk("[jerry] check 5 ==> vaddr_rx_buf = 0x%x , vaddr_tx_buf = 0x%x\n",np->vaddr_rx_buf,np->vaddr_tx_buf);

    if (!np->vaddr_rx_buf || !np->vaddr_tx_buf ) {
        printk(KERN_ERR "%s: Cannot alloc dma buffers\n", DRV_NAME);
        unregister_netdev(dev);
        free_netdev(dev);
        return -ENOMEM;
    }

/* jerry : change descriptors as non-cacheable*/	
#if 0
    np->dma_rx_ring = (unsigned int)(np->rx_ring) & 0xfffffff;
    np->dma_tx_ring = (unsigned int)(np->tx_ring) & 0xfffffff;
#else	

	start_vaddr_tx_buf = np->vaddr_tx_buf;
	start_vaddr_rx_buf = np->vaddr_rx_buf;

	current_vaddr_tx_buf = start_vaddr_tx_buf;
	current_vaddr_rx_buf = start_vaddr_rx_buf;

	end_vaddr_tx_buf = start_vaddr_tx_buf + (NUM_TX_DESCS * TX_BUF_SIZE);
	end_vaddr_rx_buf = start_vaddr_rx_buf + (NUM_RX_DESCS * RX_BUF_SIZE);

/* put descriptor on sram or dram*/	
#if 0
	tx_desc = ioremap(0xa0000000, 0x800);
	np->dma_tx_ring = 0xa0000000;
	rx_desc = ioremap(0xa0002000, 0x800);
	np->dma_rx_ring = 0xa0002000;

//	toe_rx_buf = (unsigned int) ioremap(0xa0004000, 0x800);
//	np->dma_toe_chksum = 0xa0004000;
#else
	tx_desc = gp_mac_alloc(&np->dma_tx_ring,
				sizeof (struct net_dma_desc_t) * NUM_TX_DESCS);

    	rx_desc = gp_mac_alloc(&np->dma_rx_ring,
				sizeof (struct net_dma_desc_t) * NUM_RX_DESCS);
#endif

	//printk("[jerry] net_dma_desc_t size = %d\n",sizeof (struct net_dma_desc_t));
#endif

    np->full_duplex = 1;
    np->link_state = 1;
	
	printk("[jerry] check 6, dma_rx_ring = 0x%x,  dma_tx_ring = 0x%x\n",np->dma_rx_ring,np->dma_tx_ring);

    spin_lock_init(&np->lock);

    ether_setup(dev); // ?? the function has already been called in alloc_etherdev
    dev->irq = IRQ_EMAC;
   
    dev->netdev_ops = &gp_netdev_ops; // Cynthia zhzhao added 20100514

    dev->watchdog_timeo = ETH_TX_TIMEOUT;
    
    dev->flags &= ~(IFF_MULTICAST); // Not support multicast
	//printk("[jerry] check 7\n");

    // configure MAC address
    get_mac_address(dev);
	//printk("[jerry] check 8\n");
    if ((err = register_netdev(dev)) != 0) {
        printk("%s: Cannot register net device, error %d\n",
                DRV_NAME, err);
        free_netdev(dev);
        return -ENOMEM;
    }
	
	/* FIXME : Add pin mux */
#ifdef MDC_MDIO_USING_GPIO
	mdc_handle = gp_gpio_request(MK_GPIO_INDEX(0, 0, 22, 2), NULL);
	gp_gpio_set_function(mdc_handle, 0);
	gp_gpio_set_direction(mdc_handle, GPIO_DIR_OUTPUT);
	gp_gpio_set_output(mdc_handle, 1, 0);

	mdio_handle = gp_gpio_request(MK_GPIO_INDEX(0, 0, 22, 3), NULL);
	gp_gpio_set_function(mdio_handle, 0);
	gp_gpio_set_direction(mdio_handle, GPIO_DIR_OUTPUT);
	gp_gpio_set_output(mdio_handle, 1, 0);
#else
	gpHalGpioSetPadGrp((3 << 16) | (22 << 8));
#endif
	gpHalGpioSetPadGrp((3 << 16) | (23 << 8));
	gpHalGpioSetPadGrp((3 << 16) | (36 << 8));
	gpHalGpioSetPadGrp((3 << 16) | (54 << 8));
	gpHalGpioSetPadGrp((3 << 16) | (55 << 8));
	gpHalGpioSetPadGrp((3 << 16) | (56 << 8));

    platform_device_register(&gp_eth_device);
    platform_driver_register(&gp_eth_driver);

    return 0;
}

static void __exit gp_eth_exit(void)
{
    struct net_device *dev = netdev;
    struct gp_eth_private *np = netdev_priv(dev);//(struct gp_eth_private *)P2ADDR(netdev_priv(dev));


    unregister_netdev(dev);
    dma_free_coherent(NULL, NUM_RX_DESCS * RX_BUF_SIZE,
                 (void *)np->vaddr_rx_buf, np->dma_rx_buf);
    dma_free_coherent(NULL, NUM_TX_DESCS * TX_BUF_SIZE,
                 (void *)np->vaddr_tx_buf, np->dma_tx_buf);
	
    free_netdev(dev);

    platform_device_unregister(&gp_eth_device);
    platform_driver_unregister(&gp_eth_driver);
    
    gp_enable_clock( (int*)"EMAC", 0 );
	
	
}    

module_init(gp_eth_init);
module_exit(gp_eth_exit);


