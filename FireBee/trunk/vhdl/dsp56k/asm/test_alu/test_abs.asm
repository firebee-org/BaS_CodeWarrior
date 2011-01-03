
; clear CCR
	andi	#$00,CCR
	move	#>0.25,a
	abs		a
	move	#>-0.25,a
	abs		a
	move	#>0,a
	abs		a
	move	#>$80,a2
	abs		a

