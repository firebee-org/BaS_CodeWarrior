	move	#>$000FFF,y0
	move	#>$7F00FF,b
	andi	#$00,ccr
	not 	b
	move	#>$000000,y0
	move	#>$FFFFFF,b
	andi	#$00,ccr
	not		b
