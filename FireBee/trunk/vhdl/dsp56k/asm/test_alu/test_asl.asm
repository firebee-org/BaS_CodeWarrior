;	move	#>0,y0
;	move	#>1,y1
	clr		b
	move	#>$A5,b0
	move	#>$A5,b1
	move	#>$A5,b2
	andi	#$00,ccr
	asl		b
