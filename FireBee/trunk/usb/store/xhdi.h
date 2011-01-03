/* 
 * David Galvez. 2010, e-mail: dgalvez75@gmail.com
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

#ifndef _XHDI_H
#define _XHDI_H

typedef long __CDECL (*Func)();

//#define old_pun_ptr 	0x516

#undef PUN_PTR
//#define pun_ptr pun_ptr_usb

#define MAX_LOGICAL_DRIVE	max_logical_drive

/* AHDI */

#define PUN_DEV           0x1F /* device number of HD */
#define PUN_UNIT          0x07 /* Unit number */
#define PUN_SCSI          0x08 /* 1=SCSI 0=ACSI */
#define PUN_IDE           0x10 /* Falcon IDE */
#define PUN_USB           0x20 /* USB */
#define PUN_REMOVABLE     0x40 /* Removable media */
#define PUN_VALID         0x80 /* zero if valid */

#define PINFO_PUNS     0  // 2 bytes
#define PINFO_PUN      2  // 16 bytes
#define PINFO_PSTART  18  // 16 x 4 bytes
#define PINFO_COOKIE  82  // 4 bytes
#define PINFO_COOKPTR 86  // 4 bytes
#define PINFO_VERNUM  90  // 2 bytes
#define PINFO_MAXSIZE  92  // 2 bytes
#define PINFO_PTYPE   94  // 16 x 4 bytes
#define PINFO_PSIZE  158  // 16 x 4 bytes
#define PINFO_FLAGS  222  // 16 x 2 bytes, internal use: B15:swap, B7:change, B0:bootable
#define PINFO_BPB    256  // 16 x 32 bytes
#define PINFO_SIZE   768

struct pun_info
{	
	short		puns;			/* Number of HD's */
	char		pun [16];		/* AND with masks below: */
	long		pstart [16];
	long		cookie;			/* 'AHDI' if following valid */
	long		*cook_ptr;		/* Points to 'cookie' */
	unsigned short	version_num;		/* AHDI version */
	unsigned short	max_sect_siz;		/* Max logical sec size */
	long		reserved[16];		/* Reserved */	
};

struct usb_pun_info
{	
	short		puns;			/* Number of HD's */
	char		pun [16];		/* AND with masks below: */
	long		pstart [16];
	long		cookie;			/* 'AHDI' if following valid */
	long		*cook_ptr;		/* Points to 'cookie' */
	unsigned short	version_num;		/* AHDI version */
	unsigned short	max_sect_siz;		/* Max logical sec size */
	long		reserved[16];		/* Reserved */	
	long		ptype[16];
	long		psize[16];
	unsigned short	flags[16];
};
	
/* PARTITIONS TYPES */
#define GEM		0x47454D   // GEM up to 16 MB
#define BGM		0x42474D   // BGM over 16 MB
#define RAW		0x524157   // RAW
#define FAT16_32MB	0x4	   // DOS FAT16 up to 32 MB
#define FAT16		0x6	   // DOS FAT16 over 32 MB
#define FAT16_WIN95     0xE        // WIN95 FAT16
#define FAT32           0xB        // FAT32
#define FAT32_II	0xC	   // FAT32

typedef struct xbra XBRA;
struct xbra {
	long  xb_magic;		/* "XBRA" = 0x58425241         */
	long  xb_id;      	/* ID of four ASCII characters */
	Func  xb_oldvec;	/* Old value of the vectors    */
	short jump;
  	Func  handle;
};


#endif /* _XHDI_H */
