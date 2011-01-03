	clr		a
	clr		b
	andi	#$00,ccr
	move 	#>$7F,a2
	move 	#>$7F,b2
	add		a,b

	clr		a
	clr		b
	andi	#$00,ccr
	move 	#>$80,a2
	move 	#>$7F,b2
	add		a,b

	clr		a
	clr		b
	andi	#$00,ccr
	move 	#>$80,a2
	move 	#>$80,b2
	add		a,b
