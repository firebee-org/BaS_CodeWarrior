	move	#$ABCDEF,a
	move	#$123456,b
	tfr		a,b	b,a
	move	#$555555,x0
	move	#$AAAAAA,y1
	tfr		x0,a	a,x0
	tfr		y1,b	b,y0
