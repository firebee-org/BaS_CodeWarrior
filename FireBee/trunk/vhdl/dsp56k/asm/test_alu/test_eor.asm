	move	#>$000FFF,y0
	move	#>$FF00FF,b
	andi	#$00,ccr
	eor		y0,b
	move	#>$FFFFFF,y0
	move	#>$FFFFFF,b
	andi	#$00,ccr
	eor		y0,b
