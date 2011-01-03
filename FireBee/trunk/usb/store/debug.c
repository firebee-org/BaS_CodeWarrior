/* 
 * debug.c
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

#include <stdio.h>
#include <stdarg.h>
#include <string.h>


#if  DEBUG_TO_FILE
static FILE *debug_handle = NULL;


void
debug_init(char *file)
{
	char filename[20] = "";

	strcpy(filename, file);
		
	debug_handle = fopen(filename, "a");
		
	if (debug_handle != NULL)
		setvbuf (debug_handle, NULL, _IONBF, 0);
}


void debug_exit(void)
{
	if (debug_handle != NULL && debug_handle != stdout)
		fclose(debug_handle);
	debug_handle = NULL;
}


void debug(char *FormatString, ...)
{
	va_list arg_ptr;

	va_start(arg_ptr, FormatString);
	vfprintf(debug_handle, FormatString, arg_ptr);
	va_end(arg_ptr);
	fflush(debug_handle);
	
}
#endif


