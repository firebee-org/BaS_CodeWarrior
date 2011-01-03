
	andi	#$00,CCR
	move	#>0.25,a
	move	#>$AAAAAA,a
	move	#>$BCDEFA,a0
	rep		#24
	lsl		a
