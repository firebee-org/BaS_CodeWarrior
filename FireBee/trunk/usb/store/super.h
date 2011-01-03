/*
 * super.h 
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

#ifndef _SUPER_H
#define _SUPER_H

static inline
long SuperFromUser()
{
	register long retvalue __asm__("d0");

	__asm__ volatile
	(
		"clr.l -(%%sp)\n\t"
		"move.w #0x20,-(%%sp)\n\t"
		"trap #1\n\t"
		"addq.l #6,%%sp"
		: "=r"(retvalue)				/* outputs */
	:							/* inputs  */
	: "d1", "d2", "a0", "a1", "a2"	/* clobbered regs */
	);

	return retvalue;
}

static inline
void SuperToUser(long ssp)
{
	register long spbackup;

	__asm__ volatile
	(
		"move.l sp,%0\n\t"
		"move.l %1,-(%%sp)\n\t"
		"move.w #0x20,-(%%sp)\n\t"
		"trap #1\n\t"
		"move.l %0,sp"
	: "=&r"(spbackup)						/* outputs */
	: "g"(ssp)							/* inputs  */
	: "d0", "d1", "d2", "a0", "a1", "a2"	/* clobbered regs */
	);
}
#endif /* _SUPER_H */
