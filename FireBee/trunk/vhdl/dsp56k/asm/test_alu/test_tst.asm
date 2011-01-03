	clr		b
	tst		b
; set only carry bit
	andi	#$00,ccr
	ori		#$01,ccr
	move	#>$80,b2
	tst	b
	move	#>$7F,b2
	tst	b
