/*
	NatFeat USB host chip emulator

	ARAnyM (C) 2010 David Gálvez

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*--- Include ---*/

#include <stdlib.h>
#include <string.h>

#include <mint/cookie.h>
#include <mint/osbind.h>

#include "nf_ops.h"
#include "usbhost_nfapi.h"
#include "../../config.h"
#include "../../asm-m68k/io.h"
#include "../../usb.h"
#include "../../debug.h"

/*--- Defines ---*/

#ifndef EINVFN
#define EINVFN	-32
#endif

#ifndef DEV_CONSOLE
#define DEV_CONSOLE	2
#endif

#define DRIVER_NAME	"ARAnyM USB host chip emulator"
#define VERSION	"v0.1"


/*--- Functions prototypes ---*/

static void press_any_key(void);


/*--- Local variables ---*/

static struct nf_ops *nfOps;
static unsigned long nfUsbHostId;


/*--- Functions ---*/


static void press_any_key(void)
{

	(void) Cconws("- Press any key to continue -\r\n");
	while (Bconstat(DEV_CONSOLE) == 0) { };
}


/* --- Transfer functions -------------------------------------------------- */

int submit_int_msg(struct usb_device *dev, unsigned long pipe, void *buffer,
		   int len, int interval)
{
	int r;

	r = nfOps->call(USBHOST(USBHOST_SUBMIT_INT_MSG), dev, pipe, buffer, len, interval);

	return 0;
}

int submit_control_msg(struct usb_device *dev, unsigned long pipe, void *buffer,
		       int len, struct devrequest *setup)
{
	int r;

	r = nfOps->call(USBHOST(USBHOST_SUBMIT_CONTROL_MSG), dev, pipe, buffer, len, setup);

	return r;
}

int submit_bulk_msg(struct usb_device *dev, unsigned long pipe, void *buffer,
		    int len)
{
	int r;

	r = nfOps->call(USBHOST(USBHOST_SUBMIT_BULK_MSG), dev, pipe, buffer, len);

	return 0;
}

/* --- Init functions ------------------------------------------------------ */

int usb_lowlevel_init(void)
{
	int r;

	(void) Cconws(
		"\033p " DRIVER_NAME " " VERSION " \033q\r\n"
		"Copyright (c) ARAnyM Development Team, " __DATE__ "\r\n"
	);

	nfOps = nf_init();
	if (!nfOps) {
		(void) Cconws("__NF cookie not present on this system\r\n");
		press_any_key();
		return 0;
	}
	
	nfUsbHostId=nfOps->get_id("USBHOST");
	if (nfUsbHostId == 0) {
		(void) Cconws("NF USBHOST functions not present on this system\r\n");
		press_any_key();
	}

	/* List present devices */

	r = nfOps->call(USBHOST(USBHOST_LOWLEVEL_INIT));

	if (!r) 
		(void) Cconws(" USB Init \r\n");
	else
		(void) Cconws(" Couldn't init aranym host chip emulator \r\n");

	return 0;

}

int usb_lowlevel_stop(void)
{
	int r;

	r = nfOps->call(USBHOST(USBHOST_LOWLEVEL_STOP));

	return 0;

}

