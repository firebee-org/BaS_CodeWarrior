/*
 * BaS
 *
 */


#include "MCF5475.h"
#include "startcf.h"

#include "MCD_dma.h"
#define MBAR_BASE_ADRS 0xff000000
#define MMAP_DMA       0x00008000
#define MMAP_SRAM      0x00010000

extern unsigned long far __SDRAM_SIZE;

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
  		move.l		#1330000,d0
  		move.l		d0,MCF_SLT0_STCNT
 warte_d6:
 		move.b		MCF_SLT0_SSR,d0
		btst.b		#0,d0
		beq			warte_d6
		move.l		(sp)+,d0
	}
}

void warte_1ms(void)
{
	asm
	{
warte_1ms:
		move.l		d0,-(sp) 		
  		move.l		#133000,d0
  		move.l		d0,MCF_SLT0_STCNT
 warte_d6:
 		move.b		MCF_SLT0_SSR,d0
		btst.b		#0,d0
		beq			warte_d6
		move.l		(sp)+,d0
	}
}

void warte_100us(void)
{
	asm
	{
 warte_100us:
		move.l		d0,-(sp) 		
  		move.l		#13300,d0
  		move.l		d0,MCF_SLT0_STCNT
 warte_d6:
 		move.b		MCF_SLT0_SSR,d0
		btst.b		#0,d0
		beq			warte_d6
		move.l		(sp)+,d0
	}
}

void warte_50us(void)
{
	asm
	{
warte_50us:
		move.l		d0,-(sp) 		
  		move.l		#6650,d0
  		move.l		d0,MCF_SLT0_STCNT
 warte_d6:
 		move.b		MCF_SLT0_SSR,d0
		btst.b		#0,d0
		beq			warte_d6
		move.l		(sp)+,d0
	}
}
void warte_10us(void)
{
	asm
	{
warte_10us:
		move.l		d0,-(sp) 		
  		move.l		#1330,d0
  		move.l		d0,MCF_SLT0_STCNT
 warte_d6:
 		move.b		MCF_SLT0_SSR,d0
		btst.b		#0,d0
		beq			warte_d6
		move.l		(sp)+,d0
	}
}

void warte_1us(void)
{
	asm
	{
warte_1us:
		move.l		d0,-(sp) 		
  		move.l		#133,d0
  		move.l		d0,MCF_SLT0_STCNT
 warte_d6:
 		move.b		MCF_SLT0_SSR,d0
		btst.b		#0,d0
		beq			warte_d6
		move.l		(sp)+,d0
	}
}

/********************************************************************/
#define ide_dma_tnr 7

void BaS(void)
{
/*********************************************/
// SD-Card abfragen
		int		az_sectors;
		int		sd_status,i;
		int     dma_init_status,dma_run_status,dma_status;


		az_sectors = sd_card_init();
		
		if(az_sectors>0)
		{
			sd_card_idle();
		}
/*********************************************/
// FireTOS laden?
	asm
	{
		move.b	DIP_SWITCH,d0				
		btst.b	#6,d0
		beq		firetos_kopieren
/**************************************/
// PIC initieren und daten anfordern
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
		move.b 	#0x01,(a3)						// RTC DATEN ANFORDERN
		move.l 	#'OK!',(a6)
		move.l	#0x0a0d,(a6)
/**************************************/
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
		move.l	(a1)+,(a0)+
		move.l	(a1)+,(a0)+
		move.l	(a1)+,(a0)+
		move.l	(a1)+,(a0)+
		move.l	(a1)+,(a0)+
		move.l	(a1)+,(a0)+
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
// rtc daten laden, mmu set, etc nur wenn switch 6 = off -> kein init wenn FireTOS
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
		
		move.b	#63,(a0)
		move.b	2(a0),d0
		add		#1,d0
		move.b	d0,2(a0)
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
		move.b	#0xff,(a0)					// alle pending interrupts l�schen
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
video_setup:
// nichts
/*******************************************************************/
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
   		move.l	#__SDRAM_SIZE,d0			// ende ttram		
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
		
	}
/*
//     init_dma:
	dma_init_status =
	MCD_initDma(
		(dmaRegs*) (MBAR_BASE_ADRS + MMAP_DMA),
		(void*) (MBAR_BASE_ADRS + MMAP_SRAM),
		(MCD_RELOC_TASKS)
		); 

	for (i=0; i<64; i++)
	{
		
		dma_run_status =	
		MCD_startDma(ide_dma_tnr,			//int  channel,    the channel on which to run the DMA 
    	(char*) (0x60004000),			//s8   *srcAddr,   the address to move data from, or physical buffer-descriptor address 
    	4,								//s16  srcIncr,    the amount to increment the source address per transfer 
    	(char*) (0x60000000),			//s8   *destAddr,  the address to move data to 
    	4,								//s16  destIncr,   the amount to increment the destination address per transfer 
    	4*389120,						//u32  dmaSize,    the number of bytes to transfer independent of the transfer size 
    	4,								//u32  xferSize,   the number bytes in of each data movement (1, 2, or 4)
    	0,								//u32  initiator,  what device initiates the DMA 
    	5,								//int  priority,   priority of the DMA 
    	MCD_SINGLE_DMA+
    	MCD_TT_FLAGS_CW+				//u32  flags,      flags describing the DMA 
    	MCD_TT_FLAGS_RL+
    	MCD_TT_FLAGS_SP,
    	MCD_NO_BYTE_SWAP+
    	MCD_NO_BIT_REV					//u32  funcDesc    a description of byte swapping, bit swapping, and CRC actions 
		);
		while (MCD_dmaStatus(ide_dma_tnr)!=6);
	}

	dma_status =
	MCD_dmaStatus(ide_dma_tnr);
	
	MCD_killDma(ide_dma_tnr);
*/				
	asm
	{
// test auf protect mode ---------------------
		move.b	DIP_SWITCH,d0
		btst	#7,d0 
		beq		no_protect					// nein->
		move.w	#0x0700,sr
no_protect:
	}
		MCF_PSC0_PSCTB_8BIT = 0x0a0d;
		MCF_PSC0_PSCTB_8BIT = 'BaS ';
		MCF_PSC0_PSCTB_8BIT = 'comp';
		MCF_PSC0_PSCTB_8BIT = 'lete';
		MCF_PSC0_PSCTB_8BIT = 'd';
		MCF_PSC0_PSCTB_8BIT = 0x0a0d;
		MCF_PSC0_PSCTB_8BIT = '----';
		MCF_PSC0_PSCTB_8BIT = '----';
		MCF_PSC0_PSCTB_8BIT = '----';
		MCF_PSC0_PSCTB_8BIT = '-';
		MCF_PSC0_PSCTB_8BIT = 0x0a0d;
		MCF_PSC0_PSCTB_8BIT = 0x0a0d;
	asm
	{
		nop
		bsr		warte_10ms
		jmp		0xe00030
	}
}
