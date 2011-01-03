	move	#>0,y0
	move	#>1,y1
	clr		b
	move	#>1,b0
; set only carry bit
	andi	#$00,ccr
	ori		#$01,ccr
	add		y,b
	move	#>$800000,y1
	move	#>$80,b2
	add		y,b
