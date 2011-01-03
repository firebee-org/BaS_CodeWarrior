/*
 * debug.h
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

#ifndef _DEBUG_H
#define _DEBUG_H

#include "config.h"
#include "host/aranym/nf_ops.h"

void debug_init ( char *file);
void debug_exit ( void );
void debug (char *FormatString, ...);

/* You should activate global debug in config.h, uncommenting the #define DEBUG line */
/* After global debug is enable you can activate them independly in each file */ 


#if DEBUG_GLOBAL
#if DEBUG_TO_FILE
#define DEBUG(fmt, args...)	debug_init("usb.log"); \
				debug( "%s: "fmt"\n\r" , __FUNCTION__, ##args); \
				debug_exit( )
#endif /* DEBUG_TO_FILE */

#if DEBUG_TO_ARANYM
#define	DEBUG(fmt, args...)	nf_debug(fmt )
#endif /* DEBUG_TO_ARANYM */

#if DEBUG_TO_CONSOLE
#define DEBUG(fmt, args...)	printf("%s: "fmt"\n\r" , __FUNCTION__, ##args)
#endif /* DEBUG_TO_CONSOLE */

/* This allows control debug messages independenly for different layers */
#if DEBUG_HOST_LAYER
#define DEBUG_HOST(fmt, args...)	DEBUG(fmt, ##args)
#else
#define DEBUG_HOST(fmt, args...)	{}
#endif
#if DEBUG_USB_LAYER
#define DEBUG_USB(fmt, args...)		DEBUG(fmt, ##args)
#else
#define DEBUG_USB(fmt, args...)		{}
#endif
#if DEBUG_HUB_LAYER
#define DEBUG_HUB(fmt, args...)		DEBUG(fmt, ##args)
#else
#define DEBUG_HUB(fmt, args...)		{}
#endif
#if DEBUG_STORAGE_LAYER
#define DEBUG_STORAGE(fmt, args...)	DEBUG(fmt, ##args)
#else
#define DEBUG_STORAGE(fmt, args...)	{}
#endif
#if DEBUG_XHDI_LAYER
#define DEBUG_XHDI(fmt, args...)	DEBUG(fmt, ##args)
#else
#define DEBUG_XHDI(fmt, args...)	{}
#endif

#else
#define DEBUG(fmt, args...) 	{}
#if DEBUG_HOST_LAYER
#define DEBUG_HOST(fmt, args...)	DEBUG(fmt, ##args)
#endif
#define DEBUG(fmt, args...) 	{}
#if DEBUG_USB_LAYER
#define DEBUG_USB(fmt, args...)		DEBUG(fmt, ##args)
#endif
#if DEBUG_HUB_LAYER
#define DEBUG_HUB(fmt, args...)		DEBUG(fmt, ##args)
#endif
#if DEBUG_STORAGE_LAYER
#define DEBUG_STORAGE(fmt, args...)	DEBUG(fmt, ##args)
#endif
#if DEBUG_XHDI_LAYER
#define DEBUG_XHDI(fmt, args...)	DEBUG(fmt, ##args)
#endif

#endif  /* DEBUG_GLOBAL */


#endif /* _DEBUG_H */
