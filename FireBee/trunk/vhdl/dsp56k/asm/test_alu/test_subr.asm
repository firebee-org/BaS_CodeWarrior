	move	#>0,a0
	move	#>1,a1
	clr		b
	move	#>1,b0
; set only carry bit
	andi	#$00,ccr
	ori		#$01,ccr
	subr	a,b
	move	#>$800000,a1
	move	#>$80,b2
	subr	a,b
	clr		b
	move	#>$80,b2
	move	#>$1,a1
	subr	a,b
