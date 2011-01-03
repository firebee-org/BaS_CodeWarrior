
	andi	#$00,CCR
	move	#>$123456,a1
	move	#>$789ABC,a0
	rnd		a
	move	#>$123456,a1
	move	#>$800000,a0
	rnd		a
	move	#>$123455,a1
	move	#>$800000,a0
	rnd		a
