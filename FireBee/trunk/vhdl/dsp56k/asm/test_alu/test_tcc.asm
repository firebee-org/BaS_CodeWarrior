	move	#20,r1
	move	#$ABCDEF,x0
	move	#$123456,b
	andi	#$00,ccr
	tcs		x0,a	r1,r3
	tcc		x0,b	r1,r2	
	; set Zero Flag
	ori		#$04,ccr
	teq		x0,a	r1,r3
	tne		x0,b	r1,r2	
