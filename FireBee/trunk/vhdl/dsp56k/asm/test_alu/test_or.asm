	move	#>$000FFF,y0
	move	#>$FF00FF,b
	andi	#$00,ccr
	or		y0,b
	move	#>$000000,y0
	move	#>$000000,b
	andi	#$00,ccr
	or		y0,b
