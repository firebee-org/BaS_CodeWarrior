	move	#>0,a0
	move	#>1,a1
	clr		b
	move	#>1,b0
; set only carry bit
	andi	#$00,ccr
	ori		#$01,ccr
	subl	a,b
	move	#>$800000,a1
	move	#>$80,b2
	subl	a,b
	clr		b
	move	#>$80,b2
	move	#>$1,a1
	subl	a,b
