/***************** NOT READY YET ***********************/
/**************** ONLY EXPERIMENTAL ********************/ 

/* TOS 4.04 Xbios dispatcher for the CT60/CTPCI boards
 * and USB-disk / Ram-Disk utility
 * Didier Mequignon 2005-2009, e-mail: aniplay@wanadoo.fr
 * 
 * Translation to C by David Galvez. 2010, e-mail: dgalvez75@gmail.com
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <mint/osbind.h>

#include "config.h"
#include "vars.h"
#include "xhdi.h"
#include "debug.h"

long *drvbits = (long *)_DRVBITS;

struct pun_info *old_pun_ptr = (struct pun_info *)0x512;
struct usb_pun_info pun_ptr_usb;

XBRA xbra_hdv_bpb;
XBRA xbra_hdv_rw;
XBRA xbra_hdv_mediach;

extern long usb_1st_disk_drive;
extern short max_logical_drive;

extern unsigned long usb_stor_read(int device, unsigned long blknr, unsigned long blkcnt, void *buffer);

void drive_full_usb(void)
{
	DEBUG_XHDI("\n");
}

long __CDECL hdv_bpb_usb(void)
{
	unsigned short dev;
	long r;

	__asm__ volatile	/* get arguments from the stack */
	(
		"move.w 12(%%sp),%0\n\t"

		:/*outputs*/ "=m" (dev)
		:/*inputs*/ 
	);

	__asm__ volatile	/* call old vector */
	(
		"movem.l %%d2-%%d7/%%a2-%%a6,-(%%sp)\n\t"	/* important to save register */
		"move.l %1,%%a0\n\t"
		"move.w #0,-(%%sp)\n\t"
		"move.w %2,-(%%sp)\n\t"
		"jsr (%%a0)\n\t"
		"addq.l #4,%%sp\n\t"
		"move.l %%d0,%0\n\t"
		"movem.l (%%sp)+,%%d2-%%d7/%%a2-%%a6\n\t"
		:/*outputs*/ "=r" (r)
		:/*inputs*/ "m" (xbra_hdv_bpb.xb_oldvec), "m" (dev)
		:"d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", "a2", "a3", "a4", "a5" /*clobbered regs*/
	);
	return r;
}

long __CDECL hdv_rw_usb(void)
{	
	void *buf;
	short count, recno, dev, mode;
	long lrecno;

	long r = 0;
	
	__asm__ volatile	/* get arguments from the stack */
	(
		"move.l 18(%%sp),%0\n\t"
		"move.w 14(%%sp),%1\n\t"
		"move.w 12(%%sp),%2\n\t"
		"move.w 10(%%sp),%3\n\t"
		"move.l 8(%%sp),%4\n\t"
		"move.w 4(%%sp),%5\n\t"
		:/*outputs*/ "=r" (lrecno), "=r" (dev) ,"=r" (recno), "=r" (count), "=r" (buf), "=r" (mode)
		:/*inputs*/ 
	);
	
	DEBUG_XHDI("lrecno %lx dev %d recno %x count %d mode %x\n",
			lrecno, dev, recno, count, mode);
	
	DEBUG_XHDI("\n");
	DEBUG_XHDI("rw\n");

	__asm__ volatile	/* call old vector */
	(
		"movem.l %%d2-%%d7/%%a2-%%a6,-(%%sp)\n\t"
		"move.l %1,%%a0\n\t"
		"move.l (%%a0),%%d0\n\t"
		"move.w #0,-(%%sp)\n\t"
		"move.l %2,-(%%sp)\n\t"
		"move.w %3,-(%%sp)\n\t"
		"move.w %4,-(%%sp)\n\t"
		"move.w %5,-(%%sp)\n\t"
		"move.l %6,-(%%sp)\n\t"
		"move.w %7,-(%%sp)\n\t"
		"move.l %%d0,%%a0\n\t"
		"jsr (%%a0)\n\t"
		"addq.l #4,%%sp\n\t"
		"move.l %%d0,%0\n\t"
		"movem.l (%%sp)+,%%d2-%%d7/%%a2-%%a6\n\t"
		:/*outputs*/ "=r" (r)
		:/*inputs*/ "r" (xbra_hdv_rw.xb_oldvec), "r" (lrecno), "r" (dev) ,"r" (recno), "r" (count), "r" (buf), "r" (mode)
		:"d0"/*, "d1", "d2", "d3", "d4", "d5", "d6", "d7", "a2", "a3", "a4", "a5" clobbered regs*/
	);

//	r = (*xbra_hdv_rw.xb_oldvec)(mode, buf, num, recno, dev, l);
	DEBUG_XHDI (" r %lx \n", r);

	return r;
}

long __CDECL hdv_mediach_usb(void)
{
	DEBUG_XHDI("\n");
	DEBUG_XHDI("mediach\n");

	short dev, dev1, dev2;
	long r;

	__asm__ volatile	/* get arguments from the stack */
	(
		"move.w 2(%%sp),%0\n\t"
		"move.w 4(%%sp),%1\n\t"
		"move.w 6(%%sp),%2\n\t"
		:/*outputs*/ "=r" (dev), "=r" (dev1), "=r" (dev2)
		:/*inputs*/ 
	);

	DEBUG_XHDI (" dev(2) %x dev(4) %x dev(6) %x\n", dev, dev1, dev2);

	__asm__ volatile	/* call old vector */
	(
		"movem.l %%d2-%%d7/%%a2-%%a6,-(%%sp)\n\t"
		"move.l %1,%%a0\n\t"
		"move.l (%%a0),%%d0\n\t"
		"move.w #0,-(%%sp)\n\t"
		"move.w %2,-(%%sp)\n\t"
		"move.l %%d0,%%a0\n\t"
		"jsr (%%a0)\n\t"
		"addq.l #4,%%sp\n\t"
		"move.l %%d0,%0\n\t"
		"movem.l (%%sp)+,%%d2-%%d7/%%a2-%%a6\n\t"
		:/*outputs*/ "=r" (r)
		:/*inputs*/ "r" (xbra_hdv_mediach.xb_oldvec), "r" (dev)
		:"d0"/*, "d1", "d2", "d3", "d4", "d5", "d6", "d7", "a2", "a3", "a4", "a5" clobbered regs*/
	);
//	printf (" r %lx \n", r);
//	r = (*xbra_hdv_mediach.xb_oldvec)(d);
//	DEBUG_XHDI("Calling vector: %x\n", xbra_hdv_mediach.xb_oldvec);
//	r = (*xbra_hdv_mediach.xb_oldvec)(5);
	DEBUG_XHDI(" r %lx \n", r);

	return r;
}

void install_xbra(XBRA *xbra_hd, long id, long old_vec, long (*handle)())
{
	DEBUG_XHDI("\n");
#define XBRA_MAGIC	0x58425241L /* "XBRA" */
#define JMP_OPCODE	0x4EF9

	xbra_hd->xb_magic = XBRA_MAGIC;
	xbra_hd->xb_id = id;
	xbra_hd->xb_oldvec = *((Func *)old_vec);
	xbra_hd->jump = JMP_OPCODE;
	xbra_hd->handle = (Func) handle;	

	*((Func *)old_vec) = xbra_hd->handle;

	DEBUG_XHDI("id: %x xbra_hd->xb_oldvec %x old_vec %x old_vec (*) %x old_vec (&)  %x\n", 
			xbra_hd->xb_id, xbra_hd->xb_oldvec, old_vec, *((Func *)old_vec), &old_vec);

	__asm__ volatile	/* clean cache ??? */
	(
		"cpusha BC\n\t"
		:/*outputs*/
		:/*inputs*/
	);
}

void usb_drive_ok(void)
{
	DEBUG_XHDI("\n");

}


long install_usb_partition(unsigned char drive, int dev_num, unsigned long part_type, 
			 unsigned long part_offset, unsigned long part_size)
{
	DEBUG_XHDI("\n");	

	int *dskbufp = (int *)DSKBUFP; 
	unsigned short status_register;

	long old_hdv_bpb;
	long old_hdv_rw;
	long old_hdv_mediach;

	if (drive < 16) {
		(pun_ptr_usb.puns)++;
		pun_ptr_usb.pun[drive] = dev_num | PUN_USB;
		pun_ptr_usb.pstart[drive] = part_offset;
		pun_ptr_usb.ptype[drive] = part_type;
		pun_ptr_usb.psize[drive] = part_size;
		/* flags B15:swap, B7:change, B0:bootable */
#define BOOTABLE	0x0001
#define CHANGE		0x0080
#define SWAP		0x8000
		pun_ptr_usb.flags[drive] = CHANGE;
	}

	if (drive < 16) {
		old_pun_ptr->puns++;
		old_pun_ptr->pun[drive] = dev_num | PUN_USB;
	}

	if (usb_stor_read (dev_num, part_offset, 1, (void *)(*dskbufp)) == 0)
		return -1;

	if (usb_1st_disk_drive)
		usb_drive_ok();

	usb_1st_disk_drive = drive;

	long r;
	r = (long)Getbpb (2);
	DEBUG_XHDI("Before int. Getbpb return: %x \n", r);


	__asm__ volatile	/* mask interrupts */
	(
		"move.w %%sr,%%d0\n\t"
		"move.w %%d0,%0\n\t"
		"or.l #0x700,%%d0\n\t"
		"move.w %%d0,%%sr\n\t"
		:/*outputs*/ "=r" (status_register)
		:/*inputs*/
		:"d0" /*clobbered regs*/
	);

#define _USB		0x5F555342  /* _USB   */
	long id = _USB;
	old_hdv_bpb = (long)HDV_BPB;

	DEBUG_XHDI("id: %x old_hdv_bpb %x old_hdv_bpb (*) %x hdv_bpb_usb %x\n", 
			id, old_hdv_bpb, *((Func *)old_hdv_bpb), hdv_bpb_usb);
	install_xbra(&xbra_hdv_bpb, id, old_hdv_bpb, hdv_bpb_usb);
	DEBUG_XHDI("id: %x old_hdv_bpb %x old_hdv_bpb (*) %x hdv_bpb_usb %x\n", 
			xbra_hdv_bpb.xb_id, xbra_hdv_bpb.xb_oldvec, *((Func *)xbra_hdv_bpb.xb_oldvec), xbra_hdv_bpb.handle);
#if 0	
	old_hdv_rw = (long)HDV_RW;
	DEBUG_XHDI("id: %x old_hdv_rw %x old_hdv_rw (*) %x hdv_rw_usb %x\n", 
			id, old_hdv_rw, *((Func *)old_hdv_rw), hdv_rw_usb);
	install_xbra(&xbra_hdv_rw, id, old_hdv_rw, hdv_rw_usb);
	DEBUG_XHDI("id: %x old_hdv_rw %x old_hdv_rw (*) %x hdv_rw_usb %x\n", 
			xbra_hdv_rw.xb_id, xbra_hdv_rw.xb_oldvec, *((Func *)xbra_hdv_rw.xb_oldvec), xbra_hdv_rw.handle);

	old_hdv_mediach = (long)HDV_MEDIACH;
	DEBUG_XHDI("id: %x old_hdv_mediach %x old_hdv_mediach (*) %x hdv_mediach_usb %x\n", 
			id, old_hdv_mediach, *((Func *)old_hdv_mediach), hdv_mediach_usb);
	install_xbra(&xbra_hdv_mediach, id, old_hdv_mediach, hdv_mediach_usb);
	DEBUG_XHDI("id: %x old_hdv_mediach %x old_hdv_mediach (*) %x hdv_mediach_usb %x\n", 
			xbra_hdv_mediach.xb_id, xbra_hdv_mediach.xb_oldvec, *((Func *)xbra_hdv_mediach.xb_oldvec), xbra_hdv_mediach.handle);
#endif

	r = (long)Getbpb (4);

	DEBUG_XHDI("Before int. Getbpb return: %x \n", r);

//	Bconin(DEV_CONSOLE);
	__asm__ volatile	/* restore interrupts */
	(
		"move.w %%sr,%%d0\n\t"
		"and.w %0,%%d0\n\t"
		"move.w %%d0,%%sr\n\t"
		:/*outputs*/
		:/*inputs*/ "r" (status_register)
		:"d0"
	);
	DEBUG_XHDI("after restore interrups\n");
//	Bconin(DEV_CONSOLE);
	return 0;
} 	

unsigned char search_empty_drive(int dev_num, unsigned long part_type, 
	      unsigned long part_offset, unsigned long part_size)
{
	DEBUG_XHDI("\n");	
	
	unsigned char drive = 2;	

	DEBUG_XHDI("drvbits: %x\n", *drvbits);
	while (drive < MAX_LOGICAL_DRIVE) {
		if (!(*drvbits & (0x00000001 << drive))) {
			DEBUG_XHDI("drive: %d\n", drive);
			if (install_usb_partition(drive, dev_num, part_type, part_offset, part_size) == -1) {
				DEBUG_XHDI("Couldn't install USB partition\n");
				return -1;
			}
			else return drive;
		}
		drive++;
	}
	printf("all drives already used!\n\r");

	return -1;
}	

unsigned char add_partition(int dev_num, unsigned long part_type, 
			    unsigned long part_offset, unsigned long part_size)
{
	DEBUG_XHDI("\n");

	unsigned char i;
	unsigned char drive;

	pun_ptr_usb.puns = 0x0000;
	pun_ptr_usb.version_num = 0x0300;
	pun_ptr_usb.max_sect_siz = 0x4000;
	
	for (i=0; i<16; i++)
		pun_ptr_usb.pun[i] = 0xff;

	if ((drive = search_empty_drive(dev_num, part_type, part_offset, part_size)) == -1)
		return -1;
	return drive;
}

int install_usb_stor(int dev_num, unsigned long part_type, 
		      unsigned long part_offset, unsigned long part_size, 
		      char *vendor, char *revision, char *product )
{
	DEBUG_XHDI("\n");
	unsigned char part_num;

	if (dev_num <= PUN_DEV) {	/* Max. of 32 USB storage devices */
		switch (part_type) {	/* Although real limit is 16 of pinfo struct */
			case GEM:
				break;
			case BGM:
				break;
			case RAW:
				break;
			case FAT16_32MB:
				break;
			case FAT16:
				break;
			case FAT16_WIN95:
				break;
			case FAT32:
				break;
			case FAT32_II:
				break;
			default:
				printf("Invalid partition type (0x%08lx)\r\n", part_type);
				return -1;
		}
		if ((part_num = add_partition(dev_num, part_type, part_offset, part_size)) == -1)
			return -1;
		else return 0;
	}
	printf("Maxim number(%d) of USB storage device reached \n\r", dev_num);
	return -1;
}


