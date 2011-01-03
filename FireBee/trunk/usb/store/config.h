#ifndef _CONFIG_H
#define _CONFIG_H

//#define ARCH m68k
//#define COLDFIRE      /* Besides change one(first) .chip in detxbios.S 68060 or 5200 */
//#define CONFIG_USB_ISP116X_HCD
//#define SUPERVISOR
/* Change .chip in detxbios.S 68060 or 5200 */

/*----- USB -----*/
//#define CONFIG_LEGACY_USB_INIT_SEQ
#define CONFIG_USB_STORAGE
//#define CONFIG_USB_KEYBOARD
//#define CONFIG_USB_MOUSE
//#define CONFIG_USB_INTERRUPT_POLLING
#define CONFIG_USB_ARANYM_HCD
/*----- ISP116x-HCD ------*/
#define ISP116X_HCD_USE_UDELAY
#define ISP116X_HCD_USE_EXTRA_DELAY
//#define ISP116X_HCD_SEL15kRES
//#define ISP116X_HCD_OC_ENABLE
//#define ISP116X_HCD_REMOTE_WAKEUP_ENABLE
/*----- OHCI-HCI -----*/
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	1
#define CONFIG_USB_OHCI
//#define PCI_XBIOS	/* Defined in the makefile */


/*----- DEBUG -----*/
/* You should activate global debug, 
 * #define DEBUG_GLOBAL 1 to turn on 
 * #define DEBUG_GLOBAL 0 to turn off
 * After global debug is enable 
 * you can activate the debug independently 
 * in each layer where debug is possible
 */

#define DEBUG_GLOBAL 1
#if DEBUG_GLOBAL
/* Define only one of the three debug posibilities below */
#define DEBUG_TO_FILE 1
#define DEBUG_TO_ARANYM 0	/* NOTE: No arguments are passed to the printf function */
#define DEBUG_TO_CONSOLE 0

/* Define which local layer you want on */
#define DEBUG_HOST_LAYER	0
#define DEBUG_USB_LAYER		0
#define DEBUG_HUB_LAYER		0
#define DEBUG_STORAGE_LAYER	0
#define DEBUG_XHDI_LAYER	0
#define DEBUG_BIOS_LAYER	0	/* NOTE: Always to console */

#endif
#endif /* _CONFIG_H */
