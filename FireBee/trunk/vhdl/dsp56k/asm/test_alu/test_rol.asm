
	andi	#$00,CCR
	move	#>$AAAAAA,a
	move	#>$BCDEFA,a0
	rep		#24
	rol		a
