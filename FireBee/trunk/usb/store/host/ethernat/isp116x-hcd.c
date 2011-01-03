/*
 * Modified for Atari-EtherNat by David Gálvez 2010
 *
 * ISP116x HCD (Host Controller Driver) for u-boot.
 *
 * Copyright (C) 2006-2007 Rodolfo Giometti <giometti@linux.it>
 * Copyright (C) 2006-2007 Eurotech S.p.A. <info@eurotech.it>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 *
 * Derived in part from the SL811 HCD driver "u-boot/drivers/usb/sl811_usb.c"
 * (original copyright message follows):
 *
 *    (C) Copyright 2004
 *    Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 *    This code is based on linux driver for sl811hs chip, source at
 *    drivers/usb/host/sl811.c:
 *
 *    SL811 Host Controller Interface driver for USB.
 *
 *    Copyright (c) 2003/06, Courage Co., Ltd.
 *
 *    Based on:
 *         1.uhci.c by Linus Torvalds, Johannes Erdfelt, Randy Dunlap,
 *           Georg Acher, Deti Fliegl, Thomas Sailer, Roman Weissgaerber,
 *           Adam Richter, Gregory P. Smith;
 *         2.Original SL811 driver (hc_sl811.o) by Pei Liu <pbl@cypress.com>
 *         3.Rewrited as sl811.o by Yin Aihua <yinah:couragetech.com.cn>
 *
 *    [[GNU/GPL disclaimer]]
 *
 * and in part from AU1x00 OHCI HCD driver "u-boot/cpu/mips/au1x00_usb_ohci.c"
 * (original copyright message follows):
 *
 *    URB OHCI HCD (Host Controller Driver) for USB on the AU1x00.
 *
 *    (C) Copyright 2003
 *    Gary Jennejohn, DENX Software Engineering <garyj@denx.de>
 *
 *    [[GNU/GPL disclaimer]]
 *
 *    Note: Part of this code has been derived from linux
 */
#include "../../config.h"
#include "../../asm-m68k/io.h"
#include "../../usb.h"
#include "../../debug.h"

void udelay(unsigned long usec);

//extern void boot_printf(const char *fmt, ...);


/*
 * ISP116x chips require certain delays between accesses to its
 * registers. The following timing options exist.
 *
 * 1. Configure your memory controller (the best)
 * 2. Use ndelay (easiest, poorest). For that, enable the following macro.
 *
 * Value is in microseconds.
 */
#ifdef ISP116X_HCD_USE_UDELAY
#define UDELAY		1
#endif

/*
 * On some (slowly?) machines an extra delay after data packing into
 * controller's FIFOs is required, * otherwise you may get the following
 * error:
 *
 *   uboot> usb start
 *   (Re)start USB...
 *   USB:   scanning bus for devices... isp116x: isp116x_submit_job: CTL:TIMEOUT
 *   isp116x: isp116x_submit_job: ****** FIFO not ready! ******
 *
 *         USB device not responding, giving up (status=4)
 *         isp116x: isp116x_submit_job: ****** FIFO not empty! ******
 *         isp116x: isp116x_submit_job: ****** FIFO not empty! ******
 *         isp116x: isp116x_submit_job: ****** FIFO not empty! ******
 *         3 USB Device(s) found
 *                scanning bus for storage devices... 0 Storage Device(s) found
 *
 * Value is in milliseconds.
 */
#ifdef ISP116X_HCD_USE_EXTRA_DELAY
#define EXTRA_DELAY	2
#endif

/*
 * Enable the following defines if you wish enable extra debugging messages.
 * Normal debug messages controlled from config.h.
 */

//#define TRACE			/* enable tracing code */
//#define VERBOSE		/* verbose debugging messages */

#include "isp116x.h"

#define DRIVER_VERSION	"08 Jan 2007"
static const char hcd_name[] = "isp116x-hcd";

struct isp116x isp116x_dev;
struct isp116x_platform_data isp116x_board;
static int got_rhsc;		/* root hub status change */
struct usb_device *devgone;	/* device which was disconnected */
static int rh_devnum;		/* address of Root Hub endpoint */

/* ------------------------------------------------------------------------- */

#define ALIGN(x,a)	(((x)+(a)-1UL)&~((a)-1UL))
#define min1_t(type,x,y)	\
	({ type __x = (x); type __y = (y); __x < __y ? __x : __y; })

/* Galvez: added to avoid shadow warnings */
#define min2_t(type,x,y)	\
	({ type __a = (x); type __b = (y); __a < __b ? __a : __b; })

/* ------------------------------------------------------------------------- */


static int isp116x_reset(struct isp116x *isp116x);

/* --- Debugging functions ------------------------------------------------- */

#define isp116x_show_reg(d, r) {				\
	if ((r) < 0x20) {					\
		DEBUG_HOST("%-12s[%02x]: %08x", #r,			\
			r, isp116x_read_reg32(d, r));		\
	} else {						\
		DEBUG_HOST("%-12s[%02x]:     %04x", #r,		\
			r, isp116x_read_reg16(d, r));		\
	}							\
}

#define isp116x_show_regs(d) {					\
	isp116x_show_reg(d, HCREVISION);			\
	isp116x_show_reg(d, HCCONTROL);				\
	isp116x_show_reg(d, HCCMDSTAT);				\
	isp116x_show_reg(d, HCINTSTAT);				\
	isp116x_show_reg(d, HCINTENB);				\
	isp116x_show_reg(d, HCFMINTVL);				\
	isp116x_show_reg(d, HCFMREM);				\
	isp116x_show_reg(d, HCFMNUM);				\
	isp116x_show_reg(d, HCLSTHRESH);			\
	isp116x_show_reg(d, HCRHDESCA);				\
	isp116x_show_reg(d, HCRHDESCB);				\
	isp116x_show_reg(d, HCRHSTATUS);			\
	isp116x_show_reg(d, HCRHPORT1);				\
	isp116x_show_reg(d, HCRHPORT2);				\
	isp116x_show_reg(d, HCHWCFG);				\
	isp116x_show_reg(d, HCDMACFG);				\
	isp116x_show_reg(d, HCXFERCTR);				\
	isp116x_show_reg(d, HCuPINT);				\
	isp116x_show_reg(d, HCuPINTENB);			\
	isp116x_show_reg(d, HCCHIPID);				\
	isp116x_show_reg(d, HCSCRATCH);				\
	isp116x_show_reg(d, HCITLBUFLEN);			\
	isp116x_show_reg(d, HCATLBUFLEN);			\
	isp116x_show_reg(d, HCBUFSTAT);				\
	isp116x_show_reg(d, HCRDITL0LEN);			\
	isp116x_show_reg(d, HCRDITL1LEN);			\
}

#if defined(TRACE)

static int isp116x_get_current_frame_number(struct usb_device *usb_dev)
{
	struct isp116x *isp116x = &isp116x_dev;

	return isp116x_read_reg32(isp116x, HCFMNUM);
}

static void dump_msg(struct usb_device *dev, unsigned long pipe, void *buffer,
		     int len, char *str)
{
#if defined(VERBOSE)
	int i;
#endif
	DEBUG_HOST("%s URB:[%4x] dev:%2ld,ep:%2ld-%c,type:%s,len:%d stat:%#lx",
	    str,
	    isp116x_get_current_frame_number(dev),
	    usb_pipedevice(pipe),
	    usb_pipeendpoint(pipe),
	    usb_pipeout(pipe) ? 'O' : 'I',
	    usb_pipetype(pipe) < 2 ?
	    (usb_pipeint(pipe) ?
	     "INTR" : "ISOC") :
	    (usb_pipecontrol(pipe) ? "CTRL" : "BULK"), len, dev->status);
#if defined(VERBOSE)
	debug_init("usb.log");
	if (len > 0 && buffer) {
		debug(__FILE__ ": data(%d):", len);
		for (i = 0; i < 16 && i < len; i++)
			debug(" %02x", ((__u8 *) buffer)[i]);
		debug("%s\r\n", i < len ? "..." : "");
	}
	debug_exit();
#endif
}

#define PTD_DIR_STR(ptd)  ({char __c;		\
	switch(PTD_GET_DIR(ptd)){		\
	case 0:  __c = 's'; break;		\
	case 1:  __c = 'o'; break;		\
	default: __c = 'i'; break;		\
	}; __c;})

/*
  Dump PTD info. The code documents the format
  perfectly, right :)
*/
static inline void dump_ptd(struct ptd *ptd)
{
#if defined(VERBOSE)
	int k;
#endif

	DEBUG_HOST("PTD(ext) : cc:%x %d%c%d %d,%d,%d t:%x %x%x%x",
	    PTD_GET_CC(ptd),
	    PTD_GET_FA(ptd), PTD_DIR_STR(ptd), PTD_GET_EP(ptd),
	    PTD_GET_COUNT(ptd), PTD_GET_LEN(ptd), PTD_GET_MPS(ptd),
	    PTD_GET_TOGGLE(ptd),
	    PTD_GET_ACTIVE(ptd), PTD_GET_SPD(ptd), PTD_GET_LAST(ptd));
#if defined(VERBOSE)
	debug_init("usb.log");
	debug("isp116x: %s: PTD(byte): ", __FUNCTION__);
	for (k = 0; k < sizeof(struct ptd); ++k) {
		debug("%02x ", ((u8 *) ptd)[k]);
	}
	debug("\n\r");
	debug_exit();
#endif
}

static inline void dump_ptd_data(struct ptd *ptd, u8 * buf, int type)
{
#if defined(VERBOSE)
	int k;
	
	debug_init("usb.log");
	if (type == 0 /* 0ut data */ ) {
		debug("isp116x: %s: out data: ", __FUNCTION__);
		for (k = 0; k < PTD_GET_LEN(ptd); ++k) {
			debug("%02x ", ((u8 *) buf)[k]);
		}
		debug("\n\r");
	}
	if (type == 1 /* 1n data */ ) {
		debug("isp116x: %s: in data: ", __FUNCTION__);
		for (k = 0; k < PTD_GET_COUNT(ptd); ++k) {
			debug("%02x ", ((u8 *) buf)[k]);
		}
		debug("\n\r");
	}
	
	debug_exit();
	
	if (PTD_GET_LAST(ptd)) {
		DEBUG_HOST("--- last PTD ---");
	}
#endif
}

#else

#define dump_msg(dev, pipe, buffer, len, str)			do { } while (0)
#define dump_pkt(dev, pipe, buffer, len, setup, str, small)	do {} while (0)

#define dump_ptd(ptd)			do {} while (0)
#define dump_ptd_data(ptd, buf, type)	do {} while (0)

#endif

/* --- Virtual Root Hub ---------------------------------------------------- */

/* Device descriptor */
static __u8 root_hub_dev_des[] = {
	0x12,			/*  __u8  bLength; */
	0x01,			/*  __u8  bDescriptorType; Device */
	0x10,			/*  __u16 bcdUSB; v1.1 */
	0x01,
	0x09,			/*  __u8  bDeviceClass; HUB_CLASSCODE */
	0x00,			/*  __u8  bDeviceSubClass; */
	0x00,			/*  __u8  bDeviceProtocol; */
	0x08,			/*  __u8  bMaxPacketSize0; 8 Bytes */
	0x00,			/*  __u16 idVendor; */
	0x00,
	0x00,			/*  __u16 idProduct; */
	0x00,
	0x00,			/*  __u16 bcdDevice; */
	0x00,
	0x00,			/*  __u8  iManufacturer; */
	0x01,			/*  __u8  iProduct; */
	0x00,			/*  __u8  iSerialNumber; */
	0x01			/*  __u8  bNumConfigurations; */
};

/* Configuration descriptor */
static __u8 root_hub_config_des[] = {
	0x09,			/*  __u8  bLength; */
	0x02,			/*  __u8  bDescriptorType; Configuration */
	0x19,			/*  __u16 wTotalLength; */
	0x00,
	0x01,			/*  __u8  bNumInterfaces; */
	0x01,			/*  __u8  bConfigurationValue; */
	0x00,			/*  __u8  iConfiguration; */
	0x40,			/*  __u8  bmAttributes;
				   Bit 7: Bus-powered, 6: Self-powered, 5 Remote-wakwup, 4..0: resvd */
	0x00,			/*  __u8  MaxPower; */

	/* interface */
	0x09,			/*  __u8  if_bLength; */
	0x04,			/*  __u8  if_bDescriptorType; Interface */
	0x00,			/*  __u8  if_bInterfaceNumber; */
	0x00,			/*  __u8  if_bAlternateSetting; */
	0x01,			/*  __u8  if_bNumEndpoints; */
	0x09,			/*  __u8  if_bInterfaceClass; HUB_CLASSCODE */
	0x00,			/*  __u8  if_bInterfaceSubClass; */
	0x00,			/*  __u8  if_bInterfaceProtocol; */
	0x00,			/*  __u8  if_iInterface; */

	/* endpoint */
	0x07,			/*  __u8  ep_bLength; */
	0x05,			/*  __u8  ep_bDescriptorType; Endpoint */
	0x81,			/*  __u8  ep_bEndpointAddress; IN Endpoint 1 */
	0x03,			/*  __u8  ep_bmAttributes; Interrupt */
	0x00,			/*  __u16 ep_wMaxPacketSize; ((MAX_ROOT_PORTS + 1) / 8 */
	0x02,
	0xff			/*  __u8  ep_bInterval; 255 ms */
};

static unsigned char root_hub_str_index0[] = {
	0x04,			/*  __u8  bLength; */
	0x03,			/*  __u8  bDescriptorType; String-descriptor */
	0x09,			/*  __u8  lang ID */
	0x04,			/*  __u8  lang ID */
};

static unsigned char root_hub_str_index1[] = {
	0x22,			/*  __u8  bLength; */
	0x03,			/*  __u8  bDescriptorType; String-descriptor */
	'I',			/*  __u8  Unicode */
	0,			/*  __u8  Unicode */
	'S',			/*  __u8  Unicode */
	0,			/*  __u8  Unicode */
	'P',			/*  __u8  Unicode */
	0,			/*  __u8  Unicode */
	'1',			/*  __u8  Unicode */
	0,			/*  __u8  Unicode */
	'1',			/*  __u8  Unicode */
	0,			/*  __u8  Unicode */
	'6',			/*  __u8  Unicode */
	0,			/*  __u8  Unicode */
	'x',			/*  __u8  Unicode */
	0,			/*  __u8  Unicode */
	' ',			/*  __u8  Unicode */
	0,			/*  __u8  Unicode */
	'R',			/*  __u8  Unicode */
	0,			/*  __u8  Unicode */
	'o',			/*  __u8  Unicode */
	0,			/*  __u8  Unicode */
	'o',			/*  __u8  Unicode */
	0,			/*  __u8  Unicode */
	't',			/*  __u8  Unicode */
	0,			/*  __u8  Unicode */
	' ',			/*  __u8  Unicode */
	0,			/*  __u8  Unicode */
	'H',			/*  __u8  Unicode */
	0,			/*  __u8  Unicode */
	'u',			/*  __u8  Unicode */
	0,			/*  __u8  Unicode */
	'b',			/*  __u8  Unicode */
	0,			/*  __u8  Unicode */
};

/*
 * Hub class-specific descriptor is constructed dynamically
 */

/* --- Virtual root hub management functions ------------------------------- */

static int rh_check_port_status(struct isp116x *isp116x)
{
	u32 temp, ndp, i;
	int res;

	res = -1;
	temp = isp116x_read_reg32(isp116x, HCRHSTATUS);
	ndp = (temp & RH_A_NDP);
	for (i = 0; i < ndp; i++) {
		temp = isp116x_read_reg32(isp116x, HCRHPORT1 + i);
		/* check for a device disconnect */
		if (((temp & (RH_PS_PESC | RH_PS_CSC)) ==
		     (RH_PS_PESC | RH_PS_CSC)) && ((temp & RH_PS_CCS) == 0)) {
			res = i;
			break;
		}
	}
	return res;
}

/* --- HC management functions --------------------------------------------- */

/* Write len bytes to fifo, pad till 32-bit boundary
 */
static void write_ptddata_to_fifo(struct isp116x *isp116x, void *buf, int len)
{
	u8 *dp = (u8 *) buf;
	u16 *dp2 = (u16 *) buf;
	u16 w;
	int quot = len % 4;
	
/* For EtherNat, take the raw_write out in write functions, here we don't
 * like that EtherNat swap the bytes for us, so we swap them before we send
 * them, then the bytes will arrive to the USB device with the correct positions
 */
 	if ((unsigned long)dp2 & 1) {
	DEBUG_HOST("---not aligned ---");
		/* not aligned */
		for (; len > 1; len -= 2) {
			w = *dp++;
			w |= *dp++ << 8;
			isp116x_write_data16(isp116x, w);
		}
		if (len)
			isp116x_write_data16(isp116x, (u16) * dp);
	} else {
	DEBUG_HOST("---aligned ---");
		/* aligned */
		for (; len > 1; len -= 2)
			isp116x_write_data16(isp116x, *dp2++);
		if (len){DEBUG_HOST("write_data16\r\n"); /* GALVEZ: DEBUG */
			isp116x_raw_write_data16(isp116x, 0xff & *((u8 *) dp2));}
	}
	if (quot == 1 || quot == 2)
		isp116x_write_data16(isp116x, 0);
}

/* Read len bytes from fifo and then read till 32-bit boundary
 */
static void read_ptddata_from_fifo(struct isp116x *isp116x, void *buf, int len)
{
	u8 *dp = (u8 *) buf;
	u16 *dp2 = (u16 *) buf;
	u16 w;
	int quot = len % 4;

/* For EtherNAT, take the raw_read out from read functions, we want to swap the bytes
   to read correct values because EtherNat swapped the bytes by hardware before we read 
   them */

	if ((unsigned long)dp2 & 1) {
		/* not aligned */
		DEBUG_HOST("---not aligned ---");
		for (; len > 1; len -= 2) {
			w = isp116x_read_data16(isp116x);
			*dp++ = w & 0xff;
			*dp++ = (w >> 8) & 0xff;
		}
		if (len)
			*dp = 0xff & isp116x_read_data16(isp116x);
	} else {
		/* aligned */
		DEBUG_HOST("---aligned ---");
		for (; len > 1; len -= 2)
			*dp2++ = isp116x_read_data16(isp116x);
		if (len)
			*(u8 *) dp2 = 0xff & isp116x_raw_read_data16(isp116x);
	}
	if (quot == 1 || quot == 2)
		isp116x_read_data16(isp116x);
}

/* Write PTD's and data for scheduled transfers into the fifo ram.
 * Fifo must be empty and ready */
static void pack_fifo(struct isp116x *isp116x, struct usb_device *dev,
		      unsigned long pipe, struct ptd *ptd, int n, void *data,
		      int len)
{
	int buflen = n * sizeof(struct ptd) + len;
	int i, done;

	DEBUG_HOST("--- pack buffer %p - %d bytes (fifo %d) ---", data, len, buflen);
	
	isp116x_write_reg16(isp116x, HCuPINT, HCuPINT_AIIEOT);
	
	isp116x_write_reg16(isp116x, HCXFERCTR, buflen);
	isp116x_write_addr(isp116x, HCATLPORT | ISP116x_WRITE_OFFSET);

	done = 0;
	for (i = 0; i < n; i++) {
		DEBUG_HOST("i=%d - done=%d - len=%d", i, done, PTD_GET_LEN(&ptd[i]));

/* For EtherNAT, use raw_write to don't swap bytes */
		dump_ptd(&ptd[i]);
		isp116x_raw_write_data16(isp116x, ptd[i].count);
		isp116x_raw_write_data16(isp116x, ptd[i].mps);
		isp116x_raw_write_data16(isp116x, ptd[i].len);
		isp116x_raw_write_data16(isp116x, ptd[i].faddr);

		dump_ptd_data(&ptd[i], (__u8 *) data + done, 0);

		write_ptddata_to_fifo(isp116x,
				      (__u8 *) data + done,
				      PTD_GET_LEN(&ptd[i]));

		done += PTD_GET_LEN(&ptd[i]);
	}
}

/* Read the processed PTD's and data from fifo ram back to URBs' buffers.
 * Fifo must be full and done */
static int unpack_fifo(struct isp116x *isp116x, struct usb_device *dev,
		       unsigned long pipe, struct ptd *ptd, int n, void *data,
		       int len)
{
	int buflen = n * sizeof(struct ptd) + len;
	int i, done, cc, ret;

	isp116x_write_reg16(isp116x, HCuPINT, HCuPINT_AIIEOT);
	isp116x_write_reg16(isp116x, HCXFERCTR, buflen);
	isp116x_write_addr(isp116x, HCATLPORT);

	ret = TD_CC_NOERROR;
	done = 0;
	for (i = 0; i < n; i++) {
		/* Galvez: DEBUG */
//		DEBUG_HOST("i=%d - done=%d - len=%d", i, done, PTD_GET_LEN(&ptd[i]));
		DEBUG_HOST("i=%d n=%d - done=%d - len= %d ptd_len=%d\n\r", i,n, done, len, PTD_GET_LEN(&ptd[i]));
		/*****************/
		
	/* For EtherNAT, use raw_read to don't swap bytes */
		ptd[i].count = isp116x_raw_read_data16(isp116x);
		ptd[i].mps = isp116x_raw_read_data16(isp116x);
		ptd[i].len = isp116x_raw_read_data16(isp116x);
		ptd[i].faddr = isp116x_raw_read_data16(isp116x);
		dump_ptd(&ptd[i]);
		
		/* when cc is 15 the data has not being touch by the HC
		 * so we have to read all to empty completly the buffer
		 */
//		if ( PTD_GET_COUNT(ptd) != 0 || PTD_GET_CC(ptd) == 15 )
			read_ptddata_from_fifo(isp116x,
					       (__u8 *) data + done,
					       PTD_GET_LEN(&ptd[i]));
		dump_ptd_data(&ptd[i], (__u8 *) data + done, 1);

		done += PTD_GET_LEN(&ptd[i]);
	
		cc = PTD_GET_CC(&ptd[i]);
	
		/* Data underrun means basically that we had more buffer space than
		 * the function had data. It is perfectly normal but upper levels have
		 * to know how much we actually transferred.
		 */
		if (cc == TD_NOTACCESSED ||
				(cc != TD_CC_NOERROR && (ret == TD_CC_NOERROR || ret == TD_DATAUNDERRUN)))
			ret = cc;
	}

	DEBUG_HOST("--- unpack buffer %p - %d bytes (fifo %d) count: %d---\n", data, len, buflen, PTD_GET_COUNT(ptd));

	return ret;
}

/* Interrupt handling
 */
static int isp116x_interrupt(struct isp116x *isp116x)
{
	u16 irqstat;
	u32 intstat;
	int ret = 0;

	isp116x_write_reg16(isp116x, HCuPINTENB, 0);
	irqstat = isp116x_read_reg16(isp116x, HCuPINT);
	isp116x_write_reg16(isp116x, HCuPINT, irqstat);
	DEBUG_HOST(">>>>>> irqstat %x <<<<<<", irqstat);
	
	if (irqstat & HCuPINT_ATL) {
		DEBUG_HOST(">>>>>> HCuPINT_ATL <<<<<<");
		udelay(500);
		ret = 1;
	}

	if (irqstat & HCuPINT_OPR) {
		intstat = isp116x_read_reg32(isp116x, HCINTSTAT);
		isp116x_write_reg32(isp116x, HCINTSTAT, intstat);
		DEBUG_HOST(">>>>>> HCuPINT_OPR %x <<<<<<", intstat);

		if (intstat & HCINT_UE) {
			ERR("unrecoverable error, controller disabled");

			/* FIXME: be optimistic, hope that bug won't repeat
			 * often. Make some non-interrupt context restart the
			 * controller. Count and limit the retries though;
			 * either hardware or software errors can go forever...
			 */
			isp116x_reset(isp116x);
			ret = -1;
			return -1;
		}

		if (intstat & HCINT_RHSC) {
			got_rhsc = 1;
			ret = 1;
			/* When root hub or any of its ports is going
			   to come out of suspend, it may take more
			   than 10ms for status bits to stabilize. */
			wait_ms(20);
		}

		if (intstat & HCINT_SO) {
			ERR("schedule overrun");
			ret = -1;
		}

		irqstat &= ~HCuPINT_OPR;
	}

	return ret;
}

/* With one PTD we can transfer almost 1K in one go;
 * HC does the splitting into endpoint digestible transactions
 */
struct ptd ptd[1];

static inline int max_transfer_len(struct usb_device *dev, unsigned long pipe)
{
	unsigned mpck = usb_maxpacket(dev, pipe);

	/* One PTD can transfer 1023 bytes but try to always
	 * transfer multiples of endpoint buffer size
	 */
	return 1023 / mpck * mpck;
}

/* Do an USB transfer
 */
static int isp116x_submit_job(struct usb_device *dev, unsigned long pipe,
			      int dir, void *buffer, int len)
{
	struct isp116x *isp116x = &isp116x_dev;
	int type = usb_pipetype(pipe);
	int epnum = usb_pipeendpoint(pipe);
	int max = usb_maxpacket(dev, pipe);
	int dir_out = usb_pipeout(pipe);
	int speed_low = usb_pipeslow(pipe);
	int i, done = 0, stat, timeout, cc;

	/* 500 frames or 0.5s timeout when function is busy and NAKs transactions for a while */
	int retries = 500;


	DEBUG_HOST("------------------------------------------------");
	dump_msg(dev, pipe, buffer, len, "SUBMIT");
	DEBUG_HOST("------------------------------------------------");
	
	if (len >= 1024) {
		ERR("Too big job");
		dev->status = USB_ST_CRC_ERR;
		return -1;
	}

	if (isp116x->disabled) {
		ERR("EPIPE");
		dev->status = USB_ST_CRC_ERR;
		return -1;
	}

	/* device pulled? Shortcut the action. */
	if (devgone == dev) {
		ERR("ENODEV");
		dev->status = USB_ST_CRC_ERR;
		return USB_ST_CRC_ERR;
	}

	if (!max) {
		ERR("pipesize for pipe %lx is zero", pipe);
		dev->status = USB_ST_CRC_ERR;
		return -1;
	}

	if (type == PIPE_ISOCHRONOUS) {
		ERR("isochronous transfers not supported");
		dev->status = USB_ST_CRC_ERR;
		return -1;
	}
	
	/* FIFO not empty? */
	if (isp116x_read_reg16(isp116x, HCBUFSTAT) & HCBUFSTAT_ATL_FULL) {
		DEBUG_HOST("****** FIFO not empty! ******");
		printf("****** FIFO not empty! ******\n\r");
		dev->status = USB_ST_BUF_ERR;
		return -1;
	}

retry:
	isp116x_write_reg32(isp116x, HCINTSTAT, 0xff);
	/* Prepare the PTD data */
	ptd->count = PTD_CC_MSK | PTD_ACTIVE_MSK |
		PTD_TOGGLE(usb_gettoggle(dev, epnum, dir_out));
	ptd->mps = PTD_MPS(max) | PTD_SPD(speed_low) | PTD_EP(epnum) | PTD_LAST_MSK;
	ptd->len = PTD_LEN(len) | PTD_DIR(dir);
	ptd->faddr = PTD_FA(usb_pipedevice(pipe));

	
retry_same:

	/* FIFO not empty? */ /* GALVEZ: DEBUG */
	if (isp116x_read_reg16(isp116x, HCBUFSTAT) & HCBUFSTAT_ATL_FULL) {
		DEBUG_HOST("****** FIFO not empty! ******");
		printf("****** FIFO not empty! ******\n\r");
		dev->status = USB_ST_BUF_ERR;
		return -1;
	}

	/* Pack data into FIFO ram */
	pack_fifo(isp116x, dev, pipe, ptd, 1, buffer, len);

#ifdef EXTRA_DELAY
	wait_ms(EXTRA_DELAY);
#endif

	/* Start the data transfer */

	/* Allow more time for a BULK device to react - some are slow */
	if (usb_pipebulk(pipe))
		timeout = 5000; /* Galvez: default = 5000 */
	else
		timeout = 100;

	/* Wait for it to complete */
	for (;;) {
		/* Check whether the controller is done */
		stat = isp116x_interrupt(isp116x);

		if (stat < 0) {
			dev->status = USB_ST_CRC_ERR;
			break;
		}
		if (stat > 0)
			break;

		/* Check the timeout */
		if (--timeout)
			udelay(1);
		else {
			ERR("CTL:TIMEOUT ");
			printf("CTL:TIMEOUT ");
			stat = USB_ST_CRC_ERR;
			break;
		}
	}

	/* We got an Root Hub Status Change interrupt */
	if (got_rhsc) {
		isp116x_show_regs(isp116x);

		got_rhsc = 0;

		/* Abuse timeout */
		timeout = rh_check_port_status(isp116x);
		if (timeout >= 0) {
			/*
			 * FIXME! NOTE! AAAARGH!
			 * This is potentially dangerous because it assumes
			 * that only one device is ever plugged in!
			 */
			devgone = dev;
		}
	}


	/* Ok, now we can read transfer status */

	/* FIFO not ready? */
	if (!(isp116x_read_reg16(isp116x, HCBUFSTAT) & HCBUFSTAT_ATL_DONE)) {
		DEBUG_HOST("****** FIFO not ready! ******");
		printf("****** FIFO not ready! ******\n\r");
		dev->status = USB_ST_BUF_ERR;
		return -1;
	}
	
	
	/* Unpack data from FIFO ram */
	cc = unpack_fifo(isp116x, dev, pipe, ptd, 1, buffer, len);

	i = PTD_GET_COUNT(ptd);
	done += i;
	buffer += i;
	len -= i;
	
	
	/* There was some kind of real problem; Prepare the PTD again
	 * and retry from the failed transaction on
	 */
	if (cc && cc != TD_NOTACCESSED && cc != TD_DATAUNDERRUN) {
	DEBUG_HOST("PROBLEM cc: %d", cc);
		if (retries >= 100) {
			retries -= 100;
			/* The chip will have toggled the toggle bit for the failed
			 * transaction too. We have to toggle it back.
			 */
			usb_settoggle(dev, epnum, dir_out, !PTD_GET_TOGGLE(ptd));
			goto retry;
		}
	}
	/* "Normal" errors; TD_NOTACCESSED would mean in effect that the function have NAKed
	 * the transactions from the first on for the whole frame. It may be busy and we retry
	 * with the same PTD. PTD_ACTIVE (and not TD_NOTACCESSED) would mean that some of the
	 * PTD didn't make it because the function was busy or the frame ended before the PTD
	 * finished. We prepare the rest of the data and try again.
	 */
	else if ( cc == TD_NOTACCESSED ||  PTD_GET_ACTIVE(ptd) ||  ( cc != TD_DATAUNDERRUN && PTD_GET_COUNT(ptd) < PTD_GET_LEN(ptd))) {
		DEBUG_HOST("NORMAL ERROR");
		if (retries) {
			--retries;
			if (cc == TD_NOTACCESSED && PTD_GET_ACTIVE(ptd) && !PTD_GET_COUNT(ptd)) {
				goto retry_same;
			}
			usb_settoggle(dev, epnum, dir_out, PTD_GET_TOGGLE(ptd));
			goto retry;
		}
	}


	if (cc != TD_CC_NOERROR && cc != TD_DATAUNDERRUN) {
		DEBUG_HOST("****** completition code error %x ******", cc);
		switch (cc) {
		case TD_CC_BITSTUFFING:
			dev->status = USB_ST_BIT_ERR;
			break;
		case TD_CC_STALL:
			dev->status = USB_ST_STALLED;
			break;
		case TD_BUFFEROVERRUN:
		case TD_BUFFERUNDERRUN:
			dev->status = USB_ST_BUF_ERR;
			break;
		default:
			dev->status = USB_ST_CRC_ERR;
		}
		return -cc;
	}
	else usb_settoggle(dev, epnum, dir_out, PTD_GET_TOGGLE(ptd));

	dump_msg(dev, pipe, buffer, len, "SUBMIT(ret)");

	dev->status = 0;
	return done;
}

/* Adapted from au1x00_usb_ohci.c
 */
static int isp116x_submit_rh_msg(struct usb_device *dev, unsigned long pipe,
				 void *buffer, int transfer_len,
				 struct devrequest *cmd)
{
	struct isp116x *isp116x = &isp116x_dev;
	u32 tmp = 0;

	int leni = transfer_len;
	int len = 0;
	int stat = 0;
	u32 datab[4];
	u8 *data_buf = (u8 *) datab;
	u16 bmRType_bReq;
	u16 wValue;
	u16 wIndex;
	u16 wLength;

	if (usb_pipeint(pipe)) {
		INFO("Root-Hub submit IRQ: NOT implemented");
		return 0;
	}

	bmRType_bReq = cmd->requesttype | (cmd->request << 8);
	wValue = swap_16(cmd->value);
	wIndex = swap_16(cmd->index);
	wLength = swap_16(cmd->length);

	DEBUG_HOST("--- HUB ----------------------------------------");
	DEBUG_HOST("submit rh urb, req=%x val=%#x index=%#x len=%d",
	    bmRType_bReq, wValue, wIndex, wLength);
	dump_msg(dev, pipe, buffer, transfer_len, "RH");
	DEBUG_HOST("------------------------------------------------");

	switch (bmRType_bReq) {
	case RH_GET_STATUS:
		DEBUG_HOST("RH_GET_STATUS");

		*(__u16 *) data_buf = swap_16(1);
		len = 2;
		break;

	case RH_GET_STATUS | RH_INTERFACE:
		DEBUG_HOST("RH_GET_STATUS | RH_INTERFACE");

		*(__u16 *) data_buf = swap_16(0);
		len = 2;
		break;

	case RH_GET_STATUS | RH_ENDPOINT:
		DEBUG_HOST("RH_GET_STATUS | RH_ENDPOINT");

		*(__u16 *) data_buf = swap_16(0);
		len = 2;
		break;

	case RH_GET_STATUS | RH_CLASS:
		DEBUG_HOST("RH_GET_STATUS | RH_CLASS");

		tmp = isp116x_read_reg32(isp116x, HCRHSTATUS);

		*(__u32 *) data_buf = swap_32(tmp & ~(RH_HS_CRWE | RH_HS_DRWE));
		len = 4;
		break;

	case RH_GET_STATUS | RH_OTHER | RH_CLASS:
		DEBUG_HOST("RH_GET_STATUS | RH_OTHER | RH_CLASS");

		tmp = isp116x_read_reg32(isp116x, HCRHPORT1 + wIndex - 1);
		*(__u32 *) data_buf = swap_32(tmp);
		isp116x_show_regs(isp116x);
		len = 4;
		break;

	case RH_CLEAR_FEATURE | RH_ENDPOINT:
		DEBUG_HOST("RH_CLEAR_FEATURE | RH_ENDPOINT");

		switch (wValue) {
		case RH_ENDPOINT_STALL:
			DEBUG_HOST("C_HUB_ENDPOINT_STALL");
			len = 0;
			break;
		}
		break;

	case RH_CLEAR_FEATURE | RH_CLASS:
		DEBUG_HOST("RH_CLEAR_FEATURE | RH_CLASS");

		switch (wValue) {
		case RH_C_HUB_LOCAL_POWER:
			DEBUG_HOST("C_HUB_LOCAL_POWER");
			len = 0;
			break;

		case RH_C_HUB_OVER_CURRENT:
			DEBUG_HOST("C_HUB_OVER_CURRENT");
			isp116x_write_reg32(isp116x, HCRHSTATUS, RH_HS_OCIC);
			len = 0;
			break;
		}
		break;

	case RH_CLEAR_FEATURE | RH_OTHER | RH_CLASS:
		DEBUG_HOST("RH_CLEAR_FEATURE | RH_OTHER | RH_CLASS");

		switch (wValue) {
		case RH_PORT_ENABLE:
			isp116x_write_reg32(isp116x, HCRHPORT1 + wIndex - 1,
					    RH_PS_CCS);
			len = 0;
			break;

		case RH_PORT_SUSPEND:
			isp116x_write_reg32(isp116x, HCRHPORT1 + wIndex - 1,
					    RH_PS_POCI);
			len = 0;
			break;

		case RH_PORT_POWER:
			isp116x_write_reg32(isp116x, HCRHPORT1 + wIndex - 1,
					    RH_PS_LSDA);
			len = 0;
			break;

		case RH_C_PORT_CONNECTION:
			isp116x_write_reg32(isp116x, HCRHPORT1 + wIndex - 1,
					    RH_PS_CSC);
			len = 0;
			break;

		case RH_C_PORT_ENABLE:
			isp116x_write_reg32(isp116x, HCRHPORT1 + wIndex - 1,
					    RH_PS_PESC);
			len = 0;
			break;

		case RH_C_PORT_SUSPEND:
			isp116x_write_reg32(isp116x, HCRHPORT1 + wIndex - 1,
					    RH_PS_PSSC);
			len = 0;
			break;

		case RH_C_PORT_OVER_CURRENT:
			isp116x_write_reg32(isp116x, HCRHPORT1 + wIndex - 1,
					    RH_PS_POCI);
			len = 0;
			break;

		case RH_C_PORT_RESET:
			isp116x_write_reg32(isp116x, HCRHPORT1 + wIndex - 1,
					    RH_PS_PRSC);
			len = 0;
			break;

		default:
			ERR("invalid wValue");
			stat = USB_ST_STALLED;
		}

		isp116x_show_regs(isp116x);

		break;

	case RH_SET_FEATURE | RH_OTHER | RH_CLASS:
		DEBUG_HOST("RH_SET_FEATURE | RH_OTHER | RH_CLASS");

		switch (wValue) {
		case RH_PORT_SUSPEND:
			isp116x_write_reg32(isp116x, HCRHPORT1 + wIndex - 1,
					    RH_PS_PSS);
			len = 0;
			break;

		case RH_PORT_RESET:
			/* Spin until any current reset finishes */
			while (1) {
				tmp =
				    isp116x_read_reg32(isp116x,
						       HCRHPORT1 + wIndex - 1);
				if (!(tmp & RH_PS_PRS))
					break;
				wait_ms(1);
			}
			isp116x_write_reg32(isp116x, HCRHPORT1 + wIndex - 1,
					    RH_PS_PRS);
			wait_ms(10);

			len = 0;
			break;

		case RH_PORT_POWER:
			isp116x_write_reg32(isp116x, HCRHPORT1 + wIndex - 1,
					    RH_PS_PPS);
			len = 0;
			break;

		case RH_PORT_ENABLE:
			isp116x_write_reg32(isp116x, HCRHPORT1 + wIndex - 1,
					    RH_PS_PES);
			len = 0;
			break;

		default:
			ERR("invalid wValue");
			stat = USB_ST_STALLED;
		}

		isp116x_show_regs(isp116x);

		break;

	case RH_SET_ADDRESS:
		DEBUG_HOST("RH_SET_ADDRESS");

		rh_devnum = wValue;
		len = 0;
		break;

	case RH_GET_DESCRIPTOR:
		DEBUG_HOST("RH_GET_DESCRIPTOR: %x, %d", wValue, wLength);

		switch (wValue) {
		case (USB_DT_DEVICE << 8):	/* device descriptor */
			len = min1_t(unsigned int,
				    leni, min2_t(unsigned int,
						sizeof(root_hub_dev_des),
						wLength));
			data_buf = root_hub_dev_des;
			break;

		case (USB_DT_CONFIG << 8):	/* configuration descriptor */
			len = min1_t(unsigned int,
				    leni, min2_t(unsigned int,
						sizeof(root_hub_config_des),
						wLength));
			data_buf = root_hub_config_des;
			break;

		case ((USB_DT_STRING << 8) | 0x00):	/* string 0 descriptors */
			len = min1_t(unsigned int,
				    leni, min2_t(unsigned int,
						sizeof(root_hub_str_index0),
						wLength));
			data_buf = root_hub_str_index0;
			break;

		case ((USB_DT_STRING << 8) | 0x01):	/* string 1 descriptors */
			len = min1_t(unsigned int,
				    leni, min2_t(unsigned int,
						sizeof(root_hub_str_index1),
						wLength));
			data_buf = root_hub_str_index1;
			break;

		default:
			ERR("invalid wValue");
			stat = USB_ST_STALLED;
		}

		break;

	case RH_GET_DESCRIPTOR | RH_CLASS:
		DEBUG_HOST("RH_GET_DESCRIPTOR | RH_CLASS");

		tmp = isp116x_read_reg32(isp116x, HCRHDESCA);

		data_buf[0] = 0x09;	/* min length; */
		data_buf[1] = 0x29;
		data_buf[2] = tmp & RH_A_NDP;
		data_buf[3] = 0;
		if (tmp & RH_A_PSM)	/* per-port power switching? */
			data_buf[3] |= 0x01;
		if (tmp & RH_A_NOCP)	/* no overcurrent reporting? */
			data_buf[3] |= 0x10;
		else if (tmp & RH_A_OCPM)	/* per-port overcurrent rep? */
			data_buf[3] |= 0x08;

		/* Corresponds to data_buf[4-7] */
		datab[1] = 0;
		data_buf[5] = (tmp & RH_A_POTPGT) >> 24;

		tmp = isp116x_read_reg32(isp116x, HCRHDESCB);

		data_buf[7] = tmp & RH_B_DR;
		if (data_buf[2] < 7)
			data_buf[8] = 0xff;
		else {
			data_buf[0] += 2;
			data_buf[8] = (tmp & RH_B_DR) >> 8;
			data_buf[10] = data_buf[9] = 0xff;
		}

		len = min1_t(unsigned int, leni,
			    min2_t(unsigned int, data_buf[0], wLength));
		break;

	case RH_GET_CONFIGURATION:
		DEBUG_HOST("RH_GET_CONFIGURATION");

		*(__u8 *) data_buf = 0x01;
		len = 1;
		break;

	case RH_SET_CONFIGURATION:
		DEBUG_HOST("RH_SET_CONFIGURATION");

		isp116x_write_reg32(isp116x, HCRHSTATUS, RH_HS_LPSC);
		len = 0;
		break;

	default:
		ERR("*** *** *** unsupported root hub command *** *** ***");
		stat = USB_ST_STALLED;
	}

	len = min1_t(int, len, leni);
	if (buffer != data_buf)
		memcpy(buffer, data_buf, len);

	dev->act_len = len;
	dev->status = stat;
	DEBUG_HOST("dev act_len %d, status %ld", dev->act_len, dev->status);

	dump_msg(dev, pipe, buffer, transfer_len, "RH(ret)");

	return stat;
}

/* --- Transfer functions -------------------------------------------------- */

int submit_int_msg(struct usb_device *dev, unsigned long pipe, void *buffer,
		   int len, int interval)
{
	DEBUG_HOST("dev=%p pipe=%#lx buf=%p size=%d int=%d",
	    dev, pipe, buffer, len, interval);

	return -1;
}

int submit_control_msg(struct usb_device *dev, unsigned long pipe, void *buffer,
		       int len, struct devrequest *setup)
{
	int devnum = usb_pipedevice(pipe);
	int epnum = usb_pipeendpoint(pipe);
	int max = max_transfer_len(dev, pipe);
	int dir_in = usb_pipein(pipe);
	int done, ret;
	
	
	/* Control message is for the HUB? */
	if (devnum == rh_devnum)
		return isp116x_submit_rh_msg(dev, pipe, buffer, len, setup);

	/* Ok, no HUB message so send the message to the device */

	/* Setup phase */
	DEBUG_HOST("--- SETUP PHASE --------------------------------");
	usb_settoggle(dev, epnum, 1, 0);
	
	
	ret = isp116x_submit_job(dev, pipe,
				PTD_DIR_SETUP,
				 setup, sizeof(struct devrequest));
	if (ret < 0) {
		DEBUG_HOST("control setup phase error (ret = %d", ret);
		return -1;
	}

	/* Data phase */
	DEBUG_HOST("--- DATA PHASE ---------------------------------");
	done = 0;
	usb_settoggle(dev, epnum, !dir_in, 1);
	while (done < len) {
		ret = isp116x_submit_job(dev, pipe,
					 dir_in ? PTD_DIR_IN : PTD_DIR_OUT,
					 (__u8 *) buffer + done,
					 max > len - done ? len - done : max);
		if (ret < 0) {
			DEBUG_HOST("control data phase error (ret = %d)", ret);
			return -1;
		}
		done += ret;

		if (dir_in && ret < max)	/* short packet */
			break;
	}

	/* Status phase */
	DEBUG_HOST("--- STATUS PHASE -------------------------------");
	usb_settoggle(dev, epnum, !dir_in, 1);
	ret = isp116x_submit_job(dev, pipe,
				 !dir_in ? PTD_DIR_IN : PTD_DIR_OUT, NULL, 0);
	if (ret < 0) {
		DEBUG_HOST("control status phase error (ret = %d", ret);
		return -1;
	}

	dev->act_len = done;

	dump_msg(dev, pipe, buffer, len, "DEV(ret)");

	return done;
}

int submit_bulk_msg(struct usb_device *dev, unsigned long pipe, void *buffer,
		    int len)
{
	int dir_out = usb_pipeout(pipe);
	int max = max_transfer_len(dev, pipe);
	int done, ret;
	
	DEBUG_HOST("--- BULK ---------------------------------------");
	DEBUG_HOST("dev=%ld pipe=%ld buf=%p size=%d dir_out=%d",
	    usb_pipedevice(pipe), usb_pipeendpoint(pipe), buffer, len, dir_out);

	done = 0;
	while (done < len) {

		ret = isp116x_submit_job(dev, pipe,
					 !dir_out ? PTD_DIR_IN : PTD_DIR_OUT,
					 (__u8 *) buffer + done,
					 max > len - done ? len - done : max);

		if (ret < 0) {
			DEBUG_HOST("error on bulk message (ret = %d)", ret);
			return -1;
		}

		done += ret;

		if (!dir_out && ret < max)	/* short packet */
			break;
	}

	dev->act_len = done;

	return 0;
}

/* --- Basic functions ----------------------------------------------------- */


#if 0
/* GALVEZ: Test function */
static int GALVEZ_test_function( struct isp116x *isp116x )
{
	short rwc;

	rwc = isp116x_read_reg16(isp116x, HCCONTROL) & HCCONTROL_RWC;
	if (rwc) {
		INFO ("remote wake-up supported \n\r");
	}
	return 0;

}
#endif

static int isp116x_sw_reset(struct isp116x *isp116x)
{
	int retries = 15;
	int ret = 0;

	DEBUG_HOST("");

	isp116x->disabled = 1;

	isp116x_write_reg16(isp116x, HCSWRES, HCSWRES_MAGIC);
	isp116x_write_reg32(isp116x, HCCMDSTAT, HCCMDSTAT_HCR);
	
	while (--retries) {
		/* It usually resets within 1 ms */
		/* GALVEZ: not enough for TOS, try 7 ms */
		wait_ms(7);
		if (!(isp116x_read_reg32(isp116x, HCCMDSTAT) & HCCMDSTAT_HCR))
			break;
	}
	
	if (!retries) {
		ERR("software reset timeout");
		ret = -1;
	}

#if 0
	/* GALVEZ: DEBUG SOFTWARE RESET */
		
	retries = 5000;
	
	while (--retries){
		if ((isp116x_read_reg32(isp116x, HCCMDSTAT) & HCCMDSTAT_HCR)) {
			INFO ("HCR: 1 retries: %d\n\r",retries);
		}
	}
#endif /* END DEBUG */

	return ret;
}

static int isp116x_reset(struct isp116x *isp116x)
{
	unsigned long t;
	u16 clkrdy = 0;
	int ret, timeout = 15;/* ms
				* Galvez: 15 ms sometimes isn't enough,
				* for EtherNat under TOS ??????? increased to 150 ms 
				*/ 

	DEBUG_HOST("");

	ret = isp116x_sw_reset(isp116x);

	if (ret)
		return ret;

	for (t = 0; t < timeout; t++) {
		clkrdy = isp116x_read_reg16(isp116x, HCuPINT) & HCuPINT_CLKRDY;
		if (clkrdy)
			break;
		wait_ms(1);
	}
	if (!clkrdy) {
		ERR("clock not ready after %dms", timeout);
		/* After sw_reset the clock won't report to be ready, if
		   H_WAKEUP pin is high. */
		ERR("please make sure that the H_WAKEUP pin is pulled low!");
		ret = -1;
	}
	return ret;
}

static void isp116x_stop(struct isp116x *isp116x)
{
	u32 val;
	
	DEBUG_HOST("");
	
	isp116x_write_reg16(isp116x, HCuPINTENB, 0);

	/* Switch off ports' power, some devices don't come up
	   after next 'start' without this */
	val = isp116x_read_reg32(isp116x, HCRHDESCA);
	val &= ~(RH_A_NPS | RH_A_PSM);
	isp116x_write_reg32(isp116x, HCRHDESCA, val);
	isp116x_write_reg32(isp116x, HCRHSTATUS, RH_HS_LPS);
	
	isp116x_sw_reset(isp116x);
	
#if 0	
	/* EtherNAT control register, disamble interrupt for USB */
	u8 *cpld_cr, value;
	p = SuperFromUser();
	cpld_cr = ETHERNAT_CPLD_CR;
	value = *cpld_cr;
	INFO ("ETHERNAT_CPLD_CR %x", value);
	value &= 0xFB;
	INFO ("ETHERNAT_CPLD_CR %x", value);
	*cpld_cr = value;
	SuperToUser(p);
#endif
}

/*
 *  Configure the chip. The chip must be successfully reset by now.
 */
static int isp116x_start(struct isp116x *isp116x)
{
	struct isp116x_platform_data *board = isp116x->board;
	u32 val;
	
	DEBUG_HOST("");

	/* Clear interrupt status and disable all interrupt sources */
	isp116x_write_reg16(isp116x, HCuPINT, 0xff);
	isp116x_write_reg16(isp116x, HCuPINTENB, 0);

	isp116x_write_reg16(isp116x, HCITLBUFLEN, ISP116x_ITL_BUFSIZE);
	isp116x_write_reg16(isp116x, HCATLBUFLEN, ISP116x_ATL_BUFSIZE);

	/* Hardware configuration */
	val = HCHWCFG_DBWIDTH(1);

	if (board->sel15Kres)
		val |= HCHWCFG_15KRSEL;
	/* Remote wakeup won't work without working clock */
	if (board->remote_wakeup_enable)
		val |= HCHWCFG_CLKNOTSTOP;
	if (board->oc_enable)
		val |= HCHWCFG_ANALOG_OC;
	isp116x_write_reg16(isp116x, HCHWCFG, val);

#if 0
	/* EtherNAT control register, enable interrupt for USB */
	u8 *cpld_cr, value;
	p = SuperFromUser();
	cpld_cr = ETHERNAT_CPLD_CR;
	INFO ("%s: ETHERNAT_CPLD_CR reg: %x",__FUNCTION__, *cpld_cr);
	value = *cpld_cr;
	value |= 0x04;
	Bconin(DEV_CONSOLE);
	*cpld_cr = value;
	Bconin(DEV_CONSOLE);
	INFO ("%s: ETHERNAT_CPLD_CR reg:%x", __FUNCTION__, *cpld_cr);
	SuperToUser(p);
#endif	
	/* --- Root hub configuration */
	val = (25 << 24) & RH_A_POTPGT;
	/* AN10003_1.pdf recommends RH_A_NPS (no power switching) to
	   be always set. Yet, instead, we request individual port
	   power switching. */
	val |= RH_A_PSM;
	/* Report overcurrent per port */
	val |= RH_A_OCPM;
	isp116x_write_reg32(isp116x, HCRHDESCA, val);
	isp116x->rhdesca = isp116x_read_reg32(isp116x, HCRHDESCA);

	val = RH_B_PPCM;
	isp116x_write_reg32(isp116x, HCRHDESCB, val);
	isp116x->rhdescb = isp116x_read_reg32(isp116x, HCRHDESCB);

	val = 0;
	if (board->remote_wakeup_enable)
		val |= RH_HS_DRWE;
	isp116x_write_reg32(isp116x, HCRHSTATUS, val);
	isp116x->rhstatus = isp116x_read_reg32(isp116x, HCRHSTATUS);

	isp116x_write_reg32(isp116x, HCFMINTVL, 0x27782edf);

	/* Go operational */
	val = HCCONTROL_USB_OPER;
	if (board->remote_wakeup_enable)
		val |= HCCONTROL_RWE;
	isp116x_write_reg32(isp116x, HCCONTROL, val);

	/* Disable ports to avoid race in device enumeration */
	isp116x_write_reg32(isp116x, HCRHPORT1, RH_PS_CCS);
	isp116x_write_reg32(isp116x, HCRHPORT2, RH_PS_CCS);

	isp116x_show_regs(isp116x);

	isp116x->disabled = 0;

	return 0;
}

/* --- Init functions ------------------------------------------------------ */



int isp116x_check_id(struct isp116x *isp116x)
{
	u16 val;

	val = isp116x_read_reg16(isp116x, HCCHIPID);
	DEBUG_HOST("chip ID: %04x", val);

	if ((val & HCCHIPID_MASK) != HCCHIPID_MAGIC) {
		printf("invalid chip ID %04x", val);
		return -1;
	}

	return 0;
}


int usb_lowlevel_init(void)
{
//	u16 val;

	struct isp116x *isp116x = &isp116x_dev;

	DEBUG_HOST("");

	got_rhsc = rh_devnum = 0;

	/* Init device registers addr */
	isp116x->addr_reg = (u16 *) ISP116X_HCD_ADDR;
	isp116x->data_reg = (u16 *) ISP116X_HCD_DATA;

	/* Setup specific board settings */
#ifdef ISP116X_HCD_SEL15kRES
	isp116x_board.sel15Kres = 1;
#endif
#ifdef ISP116X_HCD_OC_ENABLE
	isp116x_board.oc_enable = 1;
#endif
#ifdef ISP116X_HCD_REMOTE_WAKEUP_ENABLE
	isp116x_board.remote_wakeup_enable = 1;
#endif
	isp116x->board = &isp116x_board;

	/* Try to get ISP116x silicon chip ID */
	if (isp116x_check_id(isp116x) < 0)
		return (-1);
		
	isp116x->disabled = 1;
	isp116x->sleeping = 0;

	isp116x_reset(isp116x);
	isp116x_start(isp116x);

	return 0;
}

int usb_lowlevel_stop(void)
{
	struct isp116x *isp116x = &isp116x_dev;

	DEBUG_HOST("");

	if (!isp116x->disabled)
		isp116x_stop(isp116x);

	return 0;
}
