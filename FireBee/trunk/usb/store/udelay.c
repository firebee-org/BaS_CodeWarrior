/* 
 * David Galvez. 2010, e-mail: dgalvez75@gmail.com
 * Modified from MiNTlib
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
#include <errno.h>
#include <time.h>
#include <mint/mintbind.h>

#define USEC_PER_TICK (1000000L / ((unsigned long)CLOCKS_PER_SEC))
#define	USEC_TO_CLOCK_TICKS(us)	((us) / USEC_PER_TICK )

/*
 * Galvez: We should use usleep POSIX function in MiNTlib, but it gives problems related with
 * Fselect system call, until we trace where the problems come from we are using this function 
 */

void udelay(unsigned long usec)
{
	long stop;
	
	stop = _clock() + USEC_TO_CLOCK_TICKS(usec);
	while (_clock() < stop);
}
