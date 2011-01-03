
	move	#>$10,x0
	move	#>$11,x1
	move	#11,a1
	move	#-3,a2
	jclr	#0,a,blubb
	bset	#0,x:(r0)+
	move	#>$26,y0
	move	#>$27,y1
	move	x,L:(r0)+
	move	y,L:(r0)+
	move	x,L:$0A
	move	y,L:$1F
	move	y,L:$00A0
	move	x,L:$004F
	move	L:-(r0),x
	move	L:-(r0),y
	move	L:$0A,x
	move	L:$1F,y
blubb

