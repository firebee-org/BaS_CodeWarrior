
	andi	#$00,CCR
	clr		a
	move	#$100000,a
	move	#>$123456,x0
	move	#>$123456,y0
	macr	x0,y0,a
	move	#$100001,a
	move	#>$123456,x0
	move	#>$123456,y0
	macr	x0,y0,a
	move	#$100000,a
	move	#$800000,a0
	move	#>$123456,x0
	move	#>$123456,y0
	macr	x0,y0,a

