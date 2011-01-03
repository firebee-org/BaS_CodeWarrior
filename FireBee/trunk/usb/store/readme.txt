This application allows you to mount an USB mass storage device though 
the Ethernat hardware or the Aranym USB Natfeat, read or copy files from/to this device.
It's derived from the work done by Didier Mequignon for FireTOS.

This is still quite experimental so i am not responsable for any data loss
or corruption, play with it at your own risk. Please backup your data ;-)

**** Binaries ****

stor_etn.tos ---> EtherNat
stor_ara.tos ---> Aranym
stor_ntu.tos ---> NetUSBee
stor_pci.tos ---> PCI-OHCI

Note that NetUSBee and PCI-OHCI don't work yet

**** How it works *****

Depending on your hardware run stor_etn.tos or stor_ara.tos application from your desktop,
if everything goes well you can use the "install partition" option in your desktop menu to 
access the new partitions.

It has been tested it under CT060 TOS and MiNT 1.16.3.
Teted in Aranym with MiNT 1.17.0 beta and TOS 4.04.

**** Limits/Problems *****

- The supported partitions are the supported partitions by the OS.
It has been tested with FAT16 in TOS4.04, with FAT16, FAT32 and ext2 with MiNT.

- There is still no handle for mounting/unmounting partitions. Neither detection for
devices already plugged, so when you mount one device you can't unmount it.
If you run the application several times with the same USB stick plugged, it
will mount the device again as a new different logical partition.

- It's VERY VERY slow, for now the transfer rate it's quite ridiculous. I hope
to solve this soon. Under Aranym it can be better.

- I don't think that it works together with umouse from Jan Thomas.

- This is only a start don't expect too much.

- Thing desktop crashes when inquiring to show info about the device.

For feedback, suggestions or tips mail me at dgalvez75@gmail.com

**** Histoy ****
* 5/10/2010 (alfa 05)
 - XHDI working (assembler version).
 - Under MiNT FAT32 and ext2 partitions can be accesed.
 - Introduced NetUSBee sources (not working yet)
 - Introduced OHCI-PCI sources (to be tested)
* 27/8/2010 (alfa 04)
 - Start XHDI translation to C.
 - Support for Aranym HCD.
* 19/5/2010 (alfa 03)
 - Killed a bug that could produce some corruption in the pun_info struct.
* 26/4/2010 (alfa 02) 
 - Under MiNT if the number of partitions was greater than 16, the driver wasn't installed.
 - Wait for a key pess before retuning the desktop under TOS when driver is loaded.
 - Resolved big bug that produced that data written/read above the first 16 MB to be corrupted.
* 1/3/2010 (alfa 01)
 - Initial release 

David Galvez 	05/10/2010
Version: alfa 05
 

