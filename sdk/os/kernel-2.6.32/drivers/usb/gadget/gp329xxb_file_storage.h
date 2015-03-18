#ifndef _GP_329XXB_FILE_STORAGE_H
#define _GP_329XXB_FILE_STORAGE_H

#define DRIVER_DESC		"USB Mass Storage Device"
#define DRIVER_NAME		"Generic Storage Device"
#define DRIVER_VERSION	"20 November 2013"
#define DRIVER_AUTHOR	"Eugene Hsu<eugenehsu@generalplus.com>"

#define TYPE_SD0  	0
#define TYPE_SD1	1
#define TYPE_NAND	2

#define DRIVER_VENDOR_ID	0x1b3f	// GENERALPLUS
#define DRIVER_PRODUCT_ID	0x6000	// PMP32900 Sample Code

/* Encapsulate the module parameter settings */

#define MAX_LUNS	8
/* SCSI device types */
#define TYPE_DISK	0x00
#define TYPE_CDROM	0x05

/* USB protocol value = the transport method */
#define USB_PR_CBI	0x00		// Control/Bulk/Interrupt
#define USB_PR_CB	0x01		// Control/Bulk w/o interrupt
#define USB_PR_BULK	0x50		// Bulk-only

/* USB subclass value = the protocol encapsulation */
#define USB_SC_RBC	0x01		// Reduced Block Commands (flash)
#define USB_SC_8020	0x02		// SFF-8020i, MMC-2, ATAPI (CD-ROM)
#define USB_SC_QIC	0x03		// QIC-157 (tape)
#define USB_SC_UFI	0x04		// UFI (floppy)
#define USB_SC_8070	0x05		// SFF-8070i (removable)
#define USB_SC_SCSI	0x06		// Transparent SCSI

#define USB_BULK_CB_WRAP_LEN	31
#define USB_BULK_CB_SIG			0x43425355	// Spells out USBC
#define USB_BULK_IN_FLAG		0x80

#define USB_BULK_CS_WRAP_LEN	13
#define USB_BULK_CS_SIG			0x53425355	// Spells out 'USBS'
#define USB_STATUS_PASS			0
#define USB_STATUS_FAIL			1
#define USB_STATUS_PHASE_ERROR	2

/* Bulk-only class specific requests */
#define USB_BULK_RESET_REQUEST			0xff
#define USB_BULK_GET_MAX_LUN_REQUEST	0xfe

#define CBI_INTERRUPT_DATA_LEN		2

/* CBI Accept Device-Specific Command request */
#define USB_CBI_ADSC_REQUEST		0x00


#define MAX_COMMAND_SIZE	16	// Length of a SCSI Command Data Block

/* SCSI commands that we recognize */
#define SC_FORMAT_UNIT					0x04
#define SC_INQUIRY						0x12
#define SC_MODE_SELECT_6				0x15
#define SC_MODE_SELECT_10				0x55
#define SC_MODE_SENSE_6					0x1a
#define SC_MODE_SENSE_10				0x5a
#define SC_PREVENT_ALLOW_MEDIUM_REMOVAL	0x1e
#define SC_READ_6						0x08
#define SC_READ_10						0x28
#define SC_READ_12						0xa8
#define SC_READ_CAPACITY				0x25
#define SC_READ_FORMAT_CAPACITIES		0x23
#define SC_READ_HEADER					0x44
#define SC_READ_TOC						0x43
#define SC_RELEASE						0x17
#define SC_REQUEST_SENSE				0x03
#define SC_RESERVE						0x16
#define SC_SEND_DIAGNOSTIC				0x1d
#define SC_START_STOP_UNIT				0x1b
#define SC_SYNCHRONIZE_CACHE			0x35
#define SC_TEST_UNIT_READY				0x00
#define SC_VERIFY						0x2f
#define SC_WRITE_6						0x0a
#define SC_WRITE_10						0x2a
#define SC_WRITE_12						0xaa
#define SC_TVID							0xc2
#define SC_READ_TVID					0xda
#define SC_WRITE_TVID					0xad
#define SC_GP_SETVID					0xf0
#define SC_GP_VENDOR_USER1				0xf5
#define SC_GP_VENDOR_USER2				0xf6

/* SCSI Sense Key/Additional Sense Code/ASC Qualifier values */
#define SS_NO_SENSE								0
#define SS_COMMUNICATION_FAILURE				0x040800
#define SS_INVALID_COMMAND						0x052000
#define SS_INVALID_FIELD_IN_CDB					0x052400
#define SS_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE	0x052100
#define SS_LOGICAL_UNIT_NOT_SUPPORTED			0x052500
#define SS_MEDIUM_NOT_PRESENT					0x023a00
#define SS_MEDIUM_REMOVAL_PREVENTED				0x055302
#define SS_NOT_READY_TO_READY_TRANSITION		0x062800
#define SS_RESET_OCCURRED						0x062900
#define SS_SAVING_PARAMETERS_NOT_SUPPORTED		0x053900
#define SS_UNRECOVERED_READ_ERROR				0x031100
#define SS_WRITE_ERROR							0x030c02
#define SS_WRITE_PROTECTED						0x072700

#define SK(x)		((u8) ((x) >> 16))	// Sense Key byte, etc.
#define ASC(x)		((u8) ((x) >> 8))
#define ASCQ(x)		((u8) (x))

#define BUFFER_SIZE_16K		(16 << 10)
#define BUFFER_SIZE_32K		(32 << 10)

/*
 * These definitions will permit the compiler to avoid generating code for
 * parts of the driver that aren't used in the non-TEST version.  Even gcc
 * can recognize when a test of a constant expression yields a dead code
 * path.
 */
#ifdef CONFIG_USB_FILE_STORAGE_TEST

#define transport_is_bbb()	(mod_data.transport_type == USB_PR_BULK)
#define transport_is_cbi()	(mod_data.transport_type == USB_PR_CBI)
#define protocol_is_scsi()	(mod_data.protocol_type == USB_SC_SCSI)

#else

#define transport_is_bbb()	1
#define transport_is_cbi()	0
#define protocol_is_scsi()	1

#endif /* CONFIG_USB_FILE_STORAGE_TEST */

/* Big enough to hold our biggest descriptor */
#define EP0_BUFSIZE	256
#define DELAYED_STATUS	(EP0_BUFSIZE + 999)	// An impossibly large value

/* Number of buffers we will use.  2 is enough for double-buffering */
#define NUM_BUFFERS	2

#define REGISTERED			0
#define IGNORE_BULK_OUT		1
#define SUSPENDED			2

#define backing_file_is_open(curlun)	((curlun)->filp != NULL)
#define dev_to_lun(dev) (container_of(dev, struct lun, dev))

/* The CBI specification limits the serial string to 12 uppercase hexadecimal
 * characters. */
#define USBSCSI_INQ_VENDOR_TXT_IDX   8
#define USBSCSI_INQ_VENDOR_TXT_LEN   8
#define USBSCSI_INQ_PRODUCT_TXT_IDX  16
#define USBSCSI_INQ_PRODUCT_TXT_LEN  16
#define USBSCSI_INQ_REVISION_TXT_IDX 32
#define USBSCSI_INQ_REVISION_TXT_LEN 4

#define DESC_STR_VENDOR_MAX_SIZE   64
#define DESC_STR_PRODUCT_MAX_SIZE  64
#define DESC_STR_SERIAL_MAX_SIZE   32

/*
 * DESCRIPTORS ... most are static, but strings and (full) configuration
 * descriptors are built on demand.  Also the (static) config and interface
 * descriptors are adjusted during fsg_bind().
 */
#define STRING_MANUFACTURER	1
#define STRING_PRODUCT		2
#define STRING_SERIAL		3
#define STRING_CONFIG		4
#define STRING_INTERFACE	5
#define SERIAL_NUMBER_FILE "/mnt/tmp/msdc.txt"
//#define SERIAL_NUMBER_FILE "/media/sdcarda1/msdc.txt"
/* There is only one configuration. */
#define	CONFIG_VALUE		1

#define FS_FUNCTION_PRE_EP_ENTRIES	2

#define LDBG(lun,fmt,args...) \
	dev_dbg(&(lun)->dev , fmt , ## args)
#define MDBG(fmt,args...) \
	pr_debug(DRIVER_NAME ": " fmt , ## args)

//#ifndef DEBUG
#undef VERBOSE_DEBUG
#undef DUMP_MSGS
//#endif /* !DEBUG */

#ifdef VERBOSE_DEBUG
#define VLDBG	LDBG
#else
#define VLDBG(lun,fmt,args...) \
	do { } while (0)
#endif /* VERBOSE_DEBUG */

#define LERROR(lun,fmt,args...) \
	dev_err(&(lun)->dev , fmt , ## args)
#define LWARN(lun,fmt,args...) \
	dev_warn(&(lun)->dev , fmt , ## args)
#define LINFO(lun,fmt,args...) \
	dev_info(&(lun)->dev , fmt , ## args)

#define MINFO(fmt,args...) \
	pr_info(DRIVER_NAME ": " fmt , ## args)

#define DBG(d, fmt, args...) \
	dev_dbg(&(d)->gadget->dev , fmt , ## args)
#define VDBG(d, fmt, args...) \
	dev_vdbg(&(d)->gadget->dev , fmt , ## args)
#define ERROR(d, fmt, args...) \
	dev_err(&(d)->gadget->dev , fmt , ## args)
#define WARNING(d, fmt, args...) \
	dev_warn(&(d)->gadget->dev , fmt , ## args)
#define INFO(d, fmt, args...) \
	dev_info(&(d)->gadget->dev , fmt , ## args)

/* Command Block Wrapper */
struct bulk_cb_wrap {
	__le32	Signature;		// Contains 'USBC'
	u32	Tag;			// Unique per command id
	__le32	DataTransferLength;	// Size of the data
	u8	Flags;			// Direction in bit 7
	u8	Lun;			// LUN (normally 0)
	u8	Length;			// Of the CDB, <= MAX_COMMAND_SIZE
	u8	CDB[16];		// Command Data Block
};

/* Command Status Wrapper */
struct bulk_cs_wrap {
	__le32	Signature;		// Should = 'USBS'
	u32	Tag;			// Same as original command
	__le32	Residue;		// Amount not transferred
	u8	Status;			// See below
};

/* CBI Interrupt data structure */
struct interrupt_data {
	u8	bType;
	u8	bValue;
};

struct lun {
	struct file	*filp;
	loff_t		file_length;
	loff_t		num_sectors;

	unsigned int	ro : 1;
	unsigned int	prevent_medium_removal : 1;
	unsigned int	registered : 1;
	unsigned int	info_valid : 1;

	u32		sense_data;
	u32		sense_data_info;
	u32		unit_attention_data;

	struct device	dev;
	struct block_device	*bdev;
	unsigned char	 type;
};

enum fsg_buffer_state {
	BUF_STATE_EMPTY = 0,
	BUF_STATE_FULL,
	BUF_STATE_BUSY
};

struct fsg_buffhd {
	void				*buf;
	enum fsg_buffer_state		state;
	struct fsg_buffhd		*next;

	/* The NetChip 2280 is faster, and handles some protocol faults
	 * better, if we don't submit any short bulk-out read requests.
	 * So we will record the intended request length here. */
	unsigned int			bulk_out_intended_length;

	struct usb_request		*inreq;
	int				inreq_busy;
	struct usb_request		*outreq;
	int				outreq_busy;
};

enum fsg_state {
	FSG_STATE_COMMAND_PHASE = -10,		// This one isn't used anywhere
	FSG_STATE_DATA_PHASE,
	FSG_STATE_STATUS_PHASE,

	FSG_STATE_IDLE = 0,
	FSG_STATE_ABORT_BULK_OUT,
	FSG_STATE_RESET,
	FSG_STATE_INTERFACE_CHANGE,
	FSG_STATE_CONFIG_CHANGE,
	FSG_STATE_DISCONNECT,
	FSG_STATE_EXIT,
	FSG_STATE_TERMINATED
};

enum data_direction {
	DATA_DIR_UNKNOWN = 0,
	DATA_DIR_FROM_HOST,
	DATA_DIR_TO_HOST,
	DATA_DIR_NONE
};

struct fsg_dev {
	/* lock protects: state, all the req_busy's, and cbbuf_cmnd */
	spinlock_t		lock;
	struct usb_gadget	*gadget;

	/* filesem protects: backing files in use */
	struct rw_semaphore	filesem;

	/* reference counting: wait until all LUNs are released */
	struct kref		ref;

	struct usb_ep		*ep0;		// Handy copy of gadget->ep0
	struct usb_request	*ep0req;	// For control responses
	unsigned int		ep0_req_tag;
	const char		*ep0req_name;

	struct usb_request	*intreq;	// For interrupt responses
	int			intreq_busy;
	struct fsg_buffhd	*intr_buffhd;

 	unsigned int		bulk_out_maxpacket;
	enum fsg_state		state;		// For exception handling
	unsigned int		exception_req_tag;

	u8			config, new_config;

	unsigned int		running : 1;
	unsigned int		bulk_in_enabled : 1;
	unsigned int		bulk_out_enabled : 1;
	unsigned int		intr_in_enabled : 1;
	unsigned int		phase_error : 1;
	unsigned int		short_packet_received : 1;
	unsigned int		bad_lun_okay : 1;

	unsigned long		atomic_bitflags;
	struct usb_ep		*bulk_in;
	struct usb_ep		*bulk_out;
	struct usb_ep		*intr_in;

	struct fsg_buffhd	*next_buffhd_to_fill;
	struct fsg_buffhd	*next_buffhd_to_drain;
	struct fsg_buffhd	buffhds[NUM_BUFFERS];

	int			thread_wakeup_needed;
	struct completion	thread_notifier;
	struct task_struct	*thread_task;

	int			cmnd_size;
	u8			cmnd[MAX_COMMAND_SIZE];
	enum data_direction	data_dir;
	u32			data_size;
	u32			data_size_from_cmnd;
	u32			tag;
	unsigned int		lun;
	u32			residue;
	u32			usb_amount_left;

	/* The CB protocol offers no way for a host to know when a command
	 * has completed.  As a result the next command may arrive early,
	 * and we will still have to handle it.  For that reason we need
	 * a buffer to store new commands when using CB (or CBI, which
	 * does not oblige a host to wait for command completion either). */
	int			cbbuf_cmnd_size;
	u8			cbbuf_cmnd[MAX_COMMAND_SIZE];

	unsigned int		nluns;
	struct lun		*luns;
	struct lun		*curlun;
};

/** Parameter table for block device sector read/write **/
struct para_tab{
	struct lun	*curlun;
	u32 start;
	u32 offset;
	struct fsg_buffhd	*bh;
	u32			data_left;
	u32			data_length;
};

enum sync_state {
	SYNC_IDLE = 0,
	SYNC_RUNNING,
	SYNC_READY,
	SYNC_STOP
};

typedef struct sync_work_s{
	struct workqueue_struct 		*workqueue;
	struct work_struct				work;
	enum sync_state					state;
	u8								response;
}sync_queue_t;


extern void start_transfer(struct fsg_dev *fsg, struct usb_ep *ep,
										struct usb_request *req, int *pbusy,
										enum fsg_buffer_state *state);
extern int sleep_thread(struct fsg_dev *fsg);
extern u16 get_be16(u8 *buf);
extern u32 get_be32(u8 *buf);
extern void put_be16(u8 *buf, u16 val);
extern void put_be32(u8 *buf, u32 val);
extern int gp_do_read(struct fsg_dev *fsg);
extern int gp_do_write(struct fsg_dev *fsg);
extern int initSyncWork(void);
extern void delSyncwork(u8);
extern void addSyncwork(u8);
extern int exitSyncWork(void);

#endif