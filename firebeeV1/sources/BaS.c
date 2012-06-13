/*
 * BaS
 *
 */


#include "MCF5475.h"
#include "startcf.h"

extern unsigned long far __SP_AFTER_RESET[];
extern unsigned long far __Bas_base[];

	/* imported routines */
extern int mmu_init();
extern int mmutr_miss();
extern int vec_init();
extern int illegal_table_make();
extern int cf68k_initialize();

/********************************************************************/
	/* warte_routinen /*
********************************************************************/

void warte_10ms(void)
{
	asm
	{
warte_10ms:
		move.l		d0,-(sp) 		
  		move.l		MCF_SLT0_SCNT,d0
  		sub.l		#1320000,d0
 warte_d6:
		cmp.l		MCF_SLT0_SCNT,d0
		bcs			warte_d6
		move.l		(sp)+,d0
	}
}

void warte_1ms(void)
{
	asm
	{
warte_1ms:
		move.l		d0,-(sp) 		
  		move.l		MCF_SLT0_SCNT,d0
  		sub.l		#132000,d0
 warte_d6:
		cmp.l		MCF_SLT0_SCNT,d0
		bcs			warte_d6
		move.l		(sp)+,d0
	}
}

void warte_100us(void)
{
	asm
	{
 warte_100us:
		move.l		d0,-(sp) 		
  		move.l		MCF_SLT0_SCNT,d0
  		sub.l		#13200,d0
 warte_d6:
		cmp.l		MCF_SLT0_SCNT,d0
		bcs			warte_d6
		move.l		(sp)+,d0
	}
}

void warte_50us(void)
{
	asm
	{
warte_50us:
		move.l		d0,-(sp) 		
  		move.l		MCF_SLT0_SCNT,d0
  		sub.l		#6600,d0
 warte_d6:
		cmp.l		MCF_SLT0_SCNT,d0
		bcs			warte_d6
		move.l		(sp)+,d0
	}
}
void warte_10us(void)
{
	asm
	{
warte_10us:
		move.l		d0,-(sp) 		
  		move.l		MCF_SLT0_SCNT,d0
  		sub.l		#1320,d0
 warte_d6:
		cmp.l		MCF_SLT0_SCNT,d0
		bcs			warte_d6
		move.l		(sp)+,d0
	}
}

void warte_1us(void)
{
	asm
	{
warte_1us:
		move.l		d0,-(sp) 		
  		move.l		MCF_SLT0_SCNT,d0
  		sub.l		#132,d0
 warte_d6:
		cmp.l		MCF_SLT0_SCNT,d0
		bcs			warte_d6
		move.l		(sp)+,d0
	}
}

/********************************************************************/
void BaS(void)
{
		int		az_sectors;
		int		sd_status,i;

		az_sectors = sd_card_init();
		
		if(az_sectors>0)
		{
			sd_card_idle();
		}
	
	asm
{
		move.b	DIP_SWITCH,d0				// dip schalter adresse
		btst.b	#6,d0
		beq		firetos_kopieren
		lea		MCF_PSC0_PSCTB_8BIT,a6
		lea		MCF_PSC3_PSCTB_8BIT,a3
		lea		MCF_PSC3_PSCRB_8BIT,a4
		lea		MCF_PSC3_PSCRFCNT,a5
		move.l 	#'ACPF',(a3)					// SEND SYNC MARKE, MCF BEREIT
		bsr 	warte_10ms
		move.l 	#'PIC ',(a6)
		move.b 	(a4),d0
		move.b	d0,(a6)
		move.b 	(a4),d1
		move.b	d1,(a6)
		move.b 	(a4),d2
		move.b	d2,(a6)
		move.l	#0x0a0d,(a6)
		move.b 	#0x01,(a3)						// RTC DATEN ANFORDERN
// TOS kopieren
		lea		0x00e00000,a0
		lea		0xe0600000,a1					// default tos
		lea		0xe0700000,a2					// 1MB
		move.b	DIP_SWITCH,d0					// dip schalter adresse
		btst.b	#6,d0
		bne		cptos_loop
firetos_kopieren:
		lea		0x00e00000,a0
		lea		0xe0400000,a1
		lea		0xe0500000,a2					// 1MB
cptos_loop:
		move.l	(a1)+,(a0)+
		cmp.l	a2,a1
		blt		cptos_loop
/***************************************************************/
/*   div inits       
/***************************************************************/
div_inits:
		move.b	DIP_SWITCH,d0				// dip schalter adresse
		btst.b	#6,d0
		beq		video_setup
// rtc daten, mmu set, etc nur wenn switch 6 = off
		lea		0xffff8961,a0
		clr.l	d1	
		moveq	#64,d2
		move.b	(a4),d0
		cmp.b	#0x81,d0
		bne		not_rtc	
loop_sr:
		move.b	(a4),d0
		move.b  d1,(a0)
		move.b	d0,2(a0)
		addq.l	#1,d1
		cmp.b	d1,d2
		bne		loop_sr
/*
		// Set the NVRAM checksum as invalid
		move.b	#63,(a0)
		move.b	2(a0),d0
		add		#1,d0
		move.b	d0,2(a0)
*/
not_rtc:
		bsr		mmu_init
		bsr		vec_init
		bsr		illegal_table_make
		
// interrupts
		clr.l	0xf0010004					// disable all interrupts
		lea		MCF_EPORT_EPPAR,a0
		move.w	#0xaaa8,(a0)				// falling edge all,

// timer 0 on mit int -> video change -------------------------------------------
		move.l	#MCF_GPT_GMS_ICT(1)|MCF_GPT_GMS_IEN|MCF_GPT_GMS_TMS(1),d0	//caputre mit int on rising edge
		move.l	d0,MCF_GPT0_GMS
		moveq.l	#0x3f,d0					// max prority interrutp
 		move.b	d0,MCF_INTC_ICR62			// setzen
// -------------------------------------------------
		move.b	#0xfe,d0
		move.b	d0,0xf0010004				// enable int 1-7
		nop
		lea		MCF_EPORT_EPIER,a0
		move.b	#0xfe,(a0)					// int 1-7 on
		nop
		lea		MCF_EPORT_EPFR,a0
		move.b	#0xff,(a0)					// alle pending interrupts löschen
		nop
		lea		MCF_INTC_IMRL,a0
		move.l	#0xFFFFFF00,(a0)			// int 1-7 on
		lea		MCF_INTC_IMRH,a0
		move.l	#0xBFFFFFFE,(a0)			// psc3 and timer 0 int on

		move.l	#MCF_MMU_MMUCR_EN,d0
  		move.l	d0,MCF_MMU_MMUCR			// mmu on
		nop
		nop
/********************************************************************/
/* IDE reset
/********************************************************************/
		lea		0xffff8802,a0
		move.b	#14,-2(a0)
		move.b	#0x80,(a0)
		bsr		warte_1ms
		clr.b	(a0)
/********************************************************************/
/* video setup
/********************************************************************/
video_setup:
		lea		0xf0000410,a0
// 25MHz
 		move.l	#0x032002ba,(a0)+			// horizontal 640x480
		move.l	#0x020c020a,(a0)+			// vertikal 640x480
		move.l	#0x0190015d,(a0)+			// horizontal 320x240
		move.l	#0x020C020A,(a0)+			// vertikal 320x240 */
/*
//  32MHz
		move.l	#0x037002ba,(a0)+			// horizontal 640x480
		move.l	#0x020d020a,(a0)+			// vertikal 640x480
		move.l	#0x02A001e0,(a0)+			// horizontal 320x240
		move.l	#0x05a00160,(a0)+			// vertikal 320x240 
*/
		lea		-0x20(a0),a0
		move.l	#0x01070002,(a0)			// fifo on, refresh on, ddrcs und cke on, video dac on,
/********************************************************************/
/* memory setup
/********************************************************************/
 		lea		0x400,a0
		lea		0x800,a1
mem_clr_loop:
		clr.l	(a0)+
		clr.l	(a0)+
		clr.l	(a0)+
		clr.l	(a0)+
		cmp.l	a0,a1
		bgt		mem_clr_loop

		moveq	#0x48,d0
		move.b	d0,0xffff8007
// stram
		move.l	#0xe00000,d0				// ende stram
		move.l	d0,0x42e
		move.l	#0x752019f3,d0				// memvalid
		move.l	d0,0x420
		move.l	#0x237698aa,d0				// memval2
		move.l	d0,0x43a
		move.l	#0x5555aaaa,d0				// memval3
		move.l	d0,0x51a
// ttram
   		move.l	#__Bas_base,d0				// ende ttram		
  		move.l	d0,0x5a4
  		move.l	#0x1357bd13,d0				// ramvalid
   		move.l	d0,0x5a8
   		
// init acia
		moveq	#3,d0
		move.b	d0,0xfffffc00
		nop
		move.b	d0,0xfffffc04
		nop
		moveq	#0x96,d0
		move.b	d0,0xfffffc00
		moveq	#-1,d0
		nop
		move.b	d0,0xfffffa0f
		nop
		move.b	d0,0xfffffa11
		nop
		
// test auf protect mode ---------------------
		move.b	DIP_SWITCH,d0
		btst	#7,d0 
		beq		no_protect					// nein->
		move.w	#0x0700,sr
no_protect:
		jmp		0xe00030

}
}
