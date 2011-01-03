
	andi	#$00,CCR
	move	#>0.25,x0
	move	#>0.50,y0
	mpy		x0,y0,a
	move	#>-0.25,x0
	move	#>-0.55,y0
	mpy		x0,y0,a
	move	#>-0.20,x0
	move	#>+0.55,y0
	mpy		x0,y0,a
	move	#>-0.20,x0
	move	#>+0.55,y0
	mpy		-x0,y0,a

