
; clear CCR
	andi	#$00,CCR
	move	#>0.25,a
	clr		a
	move	#>-0.25,a
	andi	#$00,CCR
	ori		#$01,CCR
	clr		a
