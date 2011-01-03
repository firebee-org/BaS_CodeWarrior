/* 
 * David Galvez. 2010, e-mail: dgalvez75@gmail.com
 * PCI code taken from FireTos by Didier Mequignon
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

#include "config.h"
#include "usb.h"
#include "super.h"
#include "debug.h"
#ifdef PCI_XBIOS
#include "host/ohci-pci/pci_ids.h"
#include "host/ohci-pci/pcixbios.h"
#endif
#include <mint/basepage.h>
#include <mint/cookie.h>

extern long install_usb_stor(int dev_num, unsigned long part_type, 
			     unsigned long part_offset, unsigned long part_size, 
			     char *vendor, char *revision, char *product);
extern int do_usb(int argc, char **argv);
extern unsigned long _PgmSize;
extern long __mint;
#ifdef PCI_XBIOS
extern struct pci_device_id usb_pci_table[]; /* ohci-hcd.c */
#endif

int usb_stor_curr_dev;
unsigned long usb_1st_disk_drive;
short max_logical_drive;

#ifdef PCI_XBIOS
short pci_init(void)
{
/* PCI devices detection */ 
	struct pci_device_id *board;
	long handle;
	short usb_found;

	short idx;
	long err;
	unsigned long class;
		
	usb_found = 0;
	idx = 0;
	do {
		handle = find_pci_device(0x0000FFFFL, idx++);
printf("idx %d PCI handle: %lx\n", idx -1, handle); /* Galvez: Debug */
		if(handle >= 0)	{
			unsigned long id = 0;
			err = read_config_longword(handle, PCIIDR, &id);

			if((err >= 0) && !usb_found) {
				if(read_config_longword(handle, PCIREV, &class) >= 0
					&& ((class >> 16) == PCI_CLASS_SERIAL_USB)) {	
					if((class >> 8) == PCI_CLASS_SERIAL_USB_UHCI)
						(void) Cconws("UHCI USB controller found\r\n");
					else if((class >> 8) == PCI_CLASS_SERIAL_USB_OHCI) {
						(void) Cconws("OHCI USB controller found\r\n");
#ifdef CONFIG_USB_OHCI
						board = usb_pci_table; /* compare table */
						while(board->vendor) {
							if((board->vendor == (id & 0xFFFF))
				 				&& (board->device == (id >> 16))) {
								if(usb_init(handle, board) >= 0)
									usb_found = 1;
								break;
							}
							board++;
						}
#endif /* CONFIG_USB_OHCI */
					}
					else if((class >> 8) == PCI_CLASS_SERIAL_USB_EHCI)
						(void) Cconws("EHCI USB controller found\r\n");
				}
			}
		}    
	}
	while(handle >= 0);
	return usb_found;
}
#endif /* PCI_XBIOS */

int main(int argc, char **argv)
{
#ifdef CONFIG_USB_STORAGE			
	long p = 0;
	int r;

	if (__mint)
		max_logical_drive = 24;
	else max_logical_drive = 16;

	if (argc == 1) {
		short usb_found = 0;
	
		usb_stor_curr_dev = -1;
		usb_1st_disk_drive = 0;

		usb_stop();
#ifdef PCI_XBIOS
		usb_found = pci_init();	
#else
		if (usb_init() >= 0)
			usb_found = 1;
#endif /* PCI_XBIOS */
		if (usb_found) {
			/* Scan and get info from all the storage devices found */
			usb_stor_curr_dev = usb_stor_scan();
			/* it doesn't really return current device *
			 * only 0 if it has found any store device *
			 * -1 otherwise				   */
			if (usb_stor_curr_dev != -1) {
				int dev_num = usb_stor_curr_dev;
				block_dev_desc_t *stor_dev;
	
				while ((stor_dev = usb_stor_get_dev(dev_num)) != NULL) {
					int part_num = 1;
					unsigned long part_type, part_offset, part_size;
					/* Now find partitions in this storage device */	
					while (!fat_register_device(stor_dev, part_num, &part_type, 
								    &part_offset, &part_size)) {
		 				if (!(Super(SUP_INQUIRE))) {
							p = SuperFromUser ();
						}

						/* install partition */
						r = install_usb_stor(dev_num, part_type, part_offset, 
								     part_size, stor_dev->vendor, 
								     stor_dev->revision, stor_dev->product);
						if (r == -1)
							printf("unable to install storage device\n");		

						if (p)
							SuperToUser(p);
						
						part_num++;
					}	
					dev_num++;
				}
			
			}
#if 0			
			long *drvbits;
			long value;

			p = SuperFromUser();
					
			drvbits =  0x000004c2;
						
			value = *drvbits;
			printf("\ndrvbits: %x \n", (unsigned)value);
			SuperToUser(p);	
#endif		

		}
		
		if (!__mint) {	
			printf(" Press any key");
			Bconin(DEV_CONSOLE);
		}	
		Ptermres( _PgmSize, 0);	
	}
#endif	/* CONFIG_USB_STORAGE */

	if (strncmp(argv[1], "tree", 4) == 0) {
		argc = 2;
		argv[1] = "start";
		do_usb(argc, argv);

		argv[1] = "inf";
		do_usb(argc, argv);

		argv[1] = "tree";
		do_usb(argc, argv);
	
		argv[1] = "storage";
		do_usb(argc, argv);

//		argc = 3;
//		argv[1] = "dev";
//		argv[2] = "0";
//		do_usb(argc, argv);

//		argc = 2;
		argv[1] = "stop";
		do_usb (argc, argv);

		printf(" Press any key\r\n");
		Bconin(DEV_CONSOLE);
	}	
	return 0;	
}
