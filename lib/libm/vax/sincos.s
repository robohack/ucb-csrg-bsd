# Copyright (c) 1985, 1993
#	The Regents of the University of California.  All rights reserved.
#
# %sccs.include.redist.sh%
#
#	@(#)sincos.s	8.1 (Berkeley) 6/4/93
#
	.data
	.align	2
_sccsid:
.asciz	"@(#)sincos.s	1.1 (Berkeley) 8/21/85; 8.1 (ucb.elefunt) 6/4/93"

#  This is the implementation of Peter Tang's double precision  
#  sine and cosine for the VAX using Bob Corbett's argument reduction.
#  
#  Notes:
#       under 1,024,000 random arguments testing on [0,2*pi] 
#       sin() observed maximum error = 0.814 ulps
#       cos() observed maximum error = 0.792 ulps
#
# double sin(arg)
# double arg;
# method: true range reduction to [-pi/4,pi/4], P. Tang  &  B. Corbett
# S. McDonald, April 4,  1985
#
	.globl	_sin
	.text
	.align	1

_sin:	.word	0xffc		# save r2-r11
	movq	4(ap),r0
	bicw3	$0x807f,r0,r2
	beql	1f		# if x is zero or reserved operand then return x
#
# Save the PSL's IV & FU bits on the stack.
#
	movpsl	r2
	bicw3	$0xff9f,r2,-(sp)
#
# Clear the IV & FU bits.
#
	bicpsw	$0x0060
#
#  Entered by  sine    ; save  0  in  r4 .
#
	jsb	libm$argred
	movl	$0,r4
	jsb	libm$sincos
	bispsw	(sp)+
1:	ret

#
# double cos(arg)
# double arg;
# method: true range reduction to [-pi/4,pi/4], P. Tang  &  B. Corbett
# S. McDonald, April 4,  1985
#
	.globl	_cos
	.text
	.align	1

_cos:	.word	0xffc		# save r2-r11
	movq	4(ap),r0
	bicw3	$0x7f,r0,r2
	cmpw	$0x8000,r2
	beql	1f		# if x is reserved operand then return x
#
# Save the PSL's IV & FU bits on the stack.
#
	movpsl	r2
	bicw3	$0xff9f,r2,-(sp)
#
# Clear the IV & FU bits.
#
	bicpsw	$0x0060
#
#  Entered by  cosine  ; save  1  in  r4 .
#
	jsb	libm$argred
	movl	$1,r4
	jsb	libm$sincos
	bispsw	(sp)+
1:	ret
