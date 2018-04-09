
/******************************************
// setze zugriffsgeschwindigkeit cf card
/********************************************************************/

#include "MCF5475.h"

#define		error_reg	(0x05)
#define		seccnt		(0x09)
#define		stasec		(0x0d)
#define		cmd_reg		(0x1d)
#define		status_reg	(0x1d)
#define		wati		(200)

extern void warte_10ms();
extern void warte_10us();
	
void ds_rx(void)
{
	asm
	{
// 1 sector lesen word
	ds_rx:
		move.l		#128,d0
		move.l		a1,a0
	ds_rx_loop:		
		move.l		(a2),(a0)+
		subq.l		#1,d0
		bne			ds_rx_loop
	}
	
};

void test_drive(void)
{
	asm
	{
		moveq.l		#11,d1					// default access (3) mode not wait (+8)
// reset
	ide_reset:
		clr.b		0x19(a2)
		move.b		#0x8,cmd_reg(a2)		// device reset
		move.l		#wati,d0				// max. 2s warten
	wait_ready:					
		tst.b		status_reg(a2)			// comando fertig?
		bpl			ide_ready				// nein
		subq.l		#1,d0
		bmi			dam_nok					// nicht da -> default werwenden
		bsr			warte_10ms
		bra			wait_ready

	ide_ready:
		clr.b		0x19(a2)
		move.b		#0xec,cmd_reg(a2)		// identify devcie 
		move.l		#wati,d0				// max. 2s warten
	wait_busy:					
		tst.b		status_reg(a2)			// laufwerk bereit?
		bpl			ide_busy
		subq.l		#1,d0
		bmi			dam_nok					// nicht da -> default werwenden
		bsr			warte_10ms
		bra			wait_busy
	ide_busy:
		clr.l		(a1)
		btst		#3,status_reg(a2)
		beq			non_data
		bsr			ds_rx
		tst.l		(a1)
		bne			dam_ok
	non_data:
		subq.l		#1,d3
		bmi			dam_nok
		bra			ide_ready
	dam_ok:
		tst.b		status_reg(a2)			// interrupt rückstellen

		lea			MCF_PSC0_PSCTB_8BIT,a2	// name ausgeben
		lea			54(a1),a0
		moveq.l		#40,d0
	name_loop:
		move.b  	(a0)+,(a2)
		subq.l		#1,d0
		bne			name_loop
		move.l  	#' OK!',(a2)
		move.l		#0x0a0d,(a2)

		moveq.l		#3,d1
		clr.l		d0
		move.w		2*68(a1),d0				// pio cycle time
		lsr.l		#5,d0					// :32(ns)
		subq.l		#1,d0
		bmi			siam_fertig
		move.w		d0,d1
	siam_fertig:
		cmp.w		#3,d1					// grösser als max?
		ble			dam_nok	
		moveq.l		#3,d1					// sonst default
	dam_nok:
	}
};


void set_ide_access_mode(void)
{
	
	asm
	{
		lea			0xf0040000,a3
		bset.b		#1,(a3)					// ide reset
		bsr			warte_10ms
		move.w 		#0x1033,(a3)			// 1.cf 2.ide, ide int activ, scsi int disable, speed = min
		bsr			warte_10ms
		lea			0xfff00000,a2
		lea			0xc00000,a1
	}
	test_drive();
	asm
	{
		move.b		d1,d2
	}
	asm
	{
		lea			0xfff00040,a2
		lea			0xc00000,a1
	}
	test_drive();
	asm
	{
		lsl.l		#4,d1
		or.l		d1,d2
		move.b		d2,1(a3)
	}
/********************************************************************/
};
