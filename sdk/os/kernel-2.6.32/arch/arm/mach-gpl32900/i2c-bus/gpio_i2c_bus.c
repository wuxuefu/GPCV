#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <mach/gp_gpio.h>
#include <mach/hal/hal_gpio.h>
#include <mach/gp_i2c_bus.h>

#define ClkCycle 2 

typedef struct gpio_i2c_data_s {
	int hScl;
	int hSda;
	int addr;		/*slave address*/
	int udelay;	/*clock delay in us*/
	int timeout;
}gpio_i2c_data_t;

/* ----- global defines ----------------------------------------------- */
#ifdef DEBUG
#define bit_dbg(args...) \
	do { \
		printk(args);\
	} while (0)
#else
#define bit_dbg(args...) \
	do {} while (0)
#endif /* DEBUG */

/* ----- global variables ---------------------------------------------	*/

static struct gpio_i2c_data_s gpio_i2c_data = {0};
/* --- setting states on the bus with the right timing: ---------------	*/
void gpio_HalI2cBusInit(void)
{
	gpio_i2c_data.hScl = gp_gpio_request(MK_GPIO_INDEX(1, 3, 13, 17), "i2c_scl"); /*iob17*/
	gpio_i2c_data.hSda = gp_gpio_request(MK_GPIO_INDEX(1, 3, 13, 16), "i2c_sda"); /*iob16*/
	bit_dbg("hScl=%x,hSda=%x\n",gpio_i2c_data.hScl,gpio_i2c_data.hSda);
	gp_gpio_set_input( gpio_i2c_data.hScl, GPIO_PULL_HIGH);
	gp_gpio_set_debounce(gpio_i2c_data.hScl, 100);
	gp_gpio_set_input( gpio_i2c_data.hSda, GPIO_PULL_HIGH);
	gp_gpio_set_debounce(gpio_i2c_data.hSda, 100);
}

void gpio_HalI2cBusUninit(void)
{
	gp_gpio_release(gpio_i2c_data.hScl);
	gp_gpio_release(gpio_i2c_data.hSda);
}


static inline int getsda(struct gpio_i2c_data_s *adap)
{
	int val;
	gp_gpio_get_value(adap->hSda, &val);
	return (val);
}

static inline int getscl(struct gpio_i2c_data_s *adap)
{
	int val;
	gp_gpio_get_value(adap->hScl, &val);
	return (val);
}

static inline void sdalo(struct gpio_i2c_data_s *adap)
{
//	printk("AK00411\n");
	gp_gpio_set_value(adap->hSda, 0);
//	printk("AK00412\n");
	gp_gpio_set_direction(adap->hSda, GPIO_DIR_OUTPUT);
	udelay((adap->udelay + 1) / 2);
//	printk("AK0043\n");
}

static inline void sdahi(struct gpio_i2c_data_s *adap)
{
	gp_gpio_set_direction(adap->hSda, GPIO_DIR_INPUT);
	udelay((adap->udelay + 1) / 2);
}

static inline void scllo(struct gpio_i2c_data_s *adap)
{
	gp_gpio_set_value(adap->hScl, 0);
	gp_gpio_set_direction(adap->hScl, GPIO_DIR_OUTPUT);
	udelay(adap->udelay / 2);
}

/*
 * Raise scl line, and do checking for delays. This is necessary for slower
 * devices.
 */
static int sclhi(struct gpio_i2c_data_s *adap)
{
	unsigned long start;

	gp_gpio_set_direction(adap->hScl, GPIO_DIR_INPUT);

	udelay(adap->udelay/2);
	start = jiffies;
	while (!getscl(adap)) {
		/* This hw knows how to read the clock line, so we wait
		 * until it actually gets high.  This is safer as some
		 * chips may hold it low ("clock stretching") while they
		 * are processing data internally.
		 */
		if (time_after(jiffies, start + adap->timeout))
			return -ETIMEDOUT;
		cond_resched();
	}
	udelay(adap->udelay/2);
	return 0;
}

/* --- other auxiliary functions --------------------------------------	*/
static void i2c_start(struct gpio_i2c_data_s *adap)
{
	/* assert: scl, sda are high */
//	printk("AK0041\n");
	sdalo(adap);
//	printk("AK0042\n");
	udelay(adap->udelay);
//	printk("AK0043\n");
	scllo(adap);
//	printk("AK0044\n");
}

static void i2c_stop(struct gpio_i2c_data_s *adap)
{
	/* assert: scl is low */
	sdalo(adap);
	sclhi(adap);
	sdahi(adap);
	udelay(adap->udelay);
}

/* send a byte without start cond., look for arbitration,
   check ackn. from slave */
/* returns:
 * 1 if the device acknowledged
 * 0 if the device did not ack
 * -ETIMEDOUT if an error occurred (while raising the scl line)
 */
static int i2c_outb(struct gpio_i2c_data_s *adap, unsigned char c)
{
	int i;
	int sb;
	int ack;

	/* assert: scl is low */
	for (i = 7; i >= 0; i--) {
		sb = (c >> i) & 1;
		sb ? sdahi(adap) : sdalo(adap);
		udelay((adap->udelay + 1) / 2);
		if (sclhi(adap) < 0) { /* timed out */
			bit_dbg("i2c_outb: 0x%02x, timeout at bit #%d\n", (int)c, i);
			return -ETIMEDOUT;
		}
		/* FIXME do arbitration here:
		 * if (sb && !getsda(adap)) -> ouch! Get out of here.
		 *
		 * Report a unique code, so higher level code can retry
		 * the whole (combined) message and *NOT* issue STOP.
		 */
		scllo(adap);
	}
	sdahi(adap);
	if (sclhi(adap) < 0) { /* timeout */
		bit_dbg("i2c_outb: 0x%02x, timeout at ack\n", (int)c);
		return -ETIMEDOUT;
	}

	/* read ack: SDA should be pulled down by slave, or it may
	 * NAK (usually to report problems with the data we wrote).
	 */
	ack = !getsda(adap);    /* ack: sda is pulled low -> success */
	bit_dbg("i2c_outb: 0x%02x %s\n", (int)c, ack ? "A" : "NA");

	scllo(adap);
	return ack;
	/* assert: scl is low (sda undef) */
}

static int i2c_inb(struct gpio_i2c_data_s *adap)
{
	/* read byte via i2c port, without start/stop sequence	*/
	/* acknowledge is sent in i2c_read.			*/
	int i;
	unsigned char indata = 0;

	/* assert: scl is low */
	sdahi(adap);
	for (i = 0; i < 8; i++) {
		if (sclhi(adap) < 0) { /* timeout */
			bit_dbg("i2c_inb: timeout at bit #%d\n", 7 - i);
			return -ETIMEDOUT;
		}
		indata <<= 1;
		if (getsda(adap))
			indata |= 0x01;
		scllo(adap);
		udelay(i == 7 ? adap->udelay / 2 : adap->udelay);
	}
	/* assert: scl is low */
	return indata;
}

/* ----- Utility functions
 */
static int sendbytes(struct gpio_i2c_data_s *adap, struct i2c_msg_s *msg)
{
	const unsigned char *temp = msg->buf;
	int count = msg->len;
	unsigned short nak_ok = msg->flags & I2C_M_IGNORE_NAK;
	int retval;
	int wrcount = 0;

	while (count > 0) {
		retval = i2c_outb(adap, *temp);

		/* OK/ACK; or ignored NAK */
		if ((retval > 0) || (nak_ok && (retval == 0))) {
			count--;
			temp++;
			wrcount++;

		/* A slave NAKing the master means the slave didn't like
		 * something about the data it saw.  For example, maybe
		 * the SMBus PEC was wrong.
		 */
		} else if (retval == 0) {
			bit_dbg("sendbytes: NAK bailout.\n");
			return -EIO;

		/* Timeout; or (someday) lost arbitration
		 *
		 * FIXME Lost ARB implies retrying the transaction from
		 * the first message, after the "winning" master issues
		 * its STOP.  As a rule, upper layer code has no reason
		 * to know or care about this ... it is *NOT* an error.
		 */
		} else {
			bit_dbg("sendbytes: error %d\n", retval);
			return retval;
		}
	}
	return wrcount;
}

static int acknak(struct gpio_i2c_data_s *adap, int is_ack)
{

	/* assert: sda is high */
	if (is_ack) {		/* send ack */
		sdalo(adap);
	}
	udelay((adap->udelay + 1) / 2);
	if (sclhi(adap) < 0) {	/* timeout */
		bit_dbg("readbytes: ack/nak timeout\n");
		return -ETIMEDOUT;
	}
	scllo(adap);
	return 0;
}

static int readbytes(struct gpio_i2c_data_s *adap, struct i2c_msg_s *msg)
{
	int inval;
	int rdcount = 0;	/* counts bytes read */
	unsigned char *temp = msg->buf;
	int count = msg->len;
	const unsigned flags = msg->flags;

	while (count > 0) {
		inval = i2c_inb(adap);
		if (inval >= 0) {
			*temp = inval;
			rdcount++;
		} else {   /* read timed out */
			break;
		}

		temp++;
		count--;

		/* Some SMBus transactions require that we receive the
		   transaction length as the first read byte. */
		if (rdcount == 1 && (flags & I2C_M_RECV_LEN)) {
			if (inval <= 0 || inval > 8) {
				if (!(flags & I2C_M_NO_RD_ACK))
					acknak(adap, 0);
				bit_dbg("readbytes: invalid block length (%d)\n", inval);
				return -EREMOTEIO;
			}
			/* The original count value accounts for the extra
			   bytes, that is, either 1 for a regular transaction,
			   or 2 for a PEC transaction. */
			count += inval;
			msg->len += inval;
		}

		bit_dbg("readbytes: 0x%02x %s\n",
			inval,
			(flags & I2C_M_NO_RD_ACK)
				? "(no ack/nak)"
				: (count ? "A" : "NA"));

		if (!(flags & I2C_M_NO_RD_ACK)) {
			inval = acknak(adap, count);
			if (inval < 0)
				return inval;
		}
	}
	return rdcount;
}

int bit_xfer_sw( int handle, struct i2c_msg_s *pmsg)
{
	int ret;
	unsigned short nak_ok;
	unsigned char addr;
	struct gpio_i2c_data_s *adap;
	i2c_bus_attr_t *hd = (i2c_bus_attr_t*)handle;
	gpHalGpioSetPadGrp((3 << 16) | (13 << 8));
	gpio_HalI2cBusInit();
	adap = &gpio_i2c_data;
	adap->addr = hd->slaveAddr;
	adap->udelay = ClkCycle;//1000/(hd->clkRate);
	adap->timeout = 2;
	bit_dbg("emitting start condition\n");
	i2c_start(adap);
	nak_ok = pmsg->flags & I2C_M_IGNORE_NAK;
	/*send address byte*/
	addr = adap->addr;
	if (pmsg->flags & I2C_M_RD){
		addr |= 1;
		if(pmsg->devtype == I2C_READ_BK1080)
		addr &= ~(0x01);
	}
	ret = i2c_outb(adap, addr);
	if( ret==0 && !nak_ok) { /*expected ack,but got none*/
		ret = -EREMOTEIO;
		goto bailout;
	}
	if((pmsg->devtype == I2C_READ_BK1080) || (pmsg->devtype == I2C_READ_LZ300)){
		i2c_outb(adap, pmsg->regaddr);
	}
	/*data phase*/
	if (pmsg->flags & I2C_M_RD) {
		/* read bytes into buffer*/
		ret = readbytes(adap, pmsg);
		if (ret >= 1)
			bit_dbg("read %d byte(s)\n", ret);
		if (ret < pmsg->len) {
			if (ret >= 0)
				ret = -EREMOTEIO;
			goto bailout; 
		}
	} else {
		/* write bytes from buffer */
		ret = sendbytes(adap, pmsg);
		if (ret >= 1)
			bit_dbg("write %dbyte(s)\n",ret);
		if (ret < pmsg->len) {
			if (ret >= 0)
				ret = -EREMOTEIO;
			goto bailout;
		}
	}

bailout:
	bit_dbg("emitting stop condition\n");
	i2c_stop(adap);
	gpio_HalI2cBusUninit();
	gpHalGpioSetPadGrp((0 << 16) | (13 << 8));
	return ret;
}
