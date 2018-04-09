/*
 *    CF_Startup.c - Default init/startup/termination routines for
 *                     Embedded Metrowerks C++
 *
 *    Copyright © 1993-1998 Metrowerks, Inc. All Rights Reserved.
 *    Copyright © 2005 Freescale semiConductor Inc. All Rights Reserved.
 *
 *
 *    THEORY OF OPERATION
 *
 *    This version of thestartup code is intended for linker relocated
 *    executables.  The startup code will assign the stack pointer to
 *    __SP_INIT, assign the address of the data relative base address
 *    to a5, initialize the .bss/.sbss sections to zero, call any
 *    static C++ initializers and then call main.  Upon returning from
 *    main it will call C++ destructors and call exit to terminate.
 */

#ifdef __cplusplus
#pragma cplusplus off
#endif
#pragma PID off 
#pragma PIC off

#include "MCF5475.h"
#include "startcf.h"
#include "RuntimeConfig.h"

	/* imported data */

extern unsigned long far _SP_INIT, _SDA_BASE;
extern unsigned long far _START_BSS, _END_BSS;
extern unsigned long far _START_SBSS, _END_SBSS;
extern unsigned long far __DATA_RAM, __DATA_ROM, __DATA_END;

extern unsigned long far __SUP_SP,__BOOT_FLASH;
extern unsigned long far rt_mbar;
extern unsigned long far _S_romp;

	/* imported routines */


	/* exported routines */
extern void __initialize_hardware(void);
extern void BaS(void);


void _startup(void)
{
	asm
	{
		bra			warmstart
		jmp			__BOOT_FLASH + 8					// ist zugleich reset vector 
		/* disable interrupts */
	warmstart:	
	// disable interrupts 
		move.w     	#0x2700,sr
	// Initialize MBAR
		MOVE.L		#__MBAR,D0
		MOVEC		D0,MBAR
		move.l		d0,rt_mbar
	// mmu off   	
		move.l		#__MMUBAR+1,d0	
		movec		d0,MMUBAR				//mmubar setzen
		clr.l		d0
		move.l		d0,MCF_MMU_MMUCR		// mmu off
	/* Initialize RAMBARs: locate SRAM and validate it */ \
   		move.l 		#__RAMBAR0 + 0x7,d0		// supervisor only
   		movec      	d0,RAMBAR0
   		move.l 		#__RAMBAR1 + 0x1,d0		// on for all
   		movec      	d0,RAMBAR1
   	
	// STACKPOINTER AUF ENDE SRAM1
		lea 			__SUP_SP,a7
		
// hardware initieren
		jsr		  	  __initialize_hardware

	// STACKPOINTER AUF ENDE SRAM1
		lea 			__SUP_SP,a7
		
	/* setup A6 dummy stackframe */
    	movea.l       #0,a6
    	link          a6,#0

	/* setup A5 */
    	lea           _SDA_BASE,a5
    

/* copy all ROM sections to their RAM locations ... */
	/*
	 * _S_romp is a null terminated array of
	 * typedef struct RomInfo {
     *      unsigned long	Source;
     *      unsigned long	Target;
     *      unsigned long 	Size;
     *  } RomInfo;
     *
     * Watch out if you're rebasing using _PICPID_DELTA
     */
    	lea   		_S_romp, a0
    	move.l		(a0),a1
    	move.l		4(a0),a2
    	move.l		8(a0),d0
    	lsr.l		#2,d0
    crr_loop:
    	move.l		(a1)+,(a2)+
    	subq.l		#1,d0
    	bne			crr_loop

	/* call main(int, char **) */
		pea			__argv
		clr.l		-(sp)				/* clearing a long is ok since it's caller cleanup */
		jsr			 BaS
		addq.l		#8, sp
	
		unlk		a6
		
	/* should never reach here but just in case */
		jsr       warmstart
		illegal
		rts

	/* exit will never return */
	__argv:
    	dc.l          0
	};
}


