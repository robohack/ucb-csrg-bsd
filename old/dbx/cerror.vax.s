# Copyright (c) 1982 Regents of the University of California
#
# static char sccsid[] = "@(#)cerror.vax.s	1.3 (Berkeley) 3/1/85";
#
# static char rcsid[] = "$Header: cerror.s,v 1.5 84/12/26 10:38:33 linton Exp $";
#
# modified version of cerror
#
# The idea is that every time an error occurs in a system call
# I want a special function "syserr" called.  This function will
# either print a message and exit or do nothing depending on
# defaults and use of "onsyserr".
#

.globl	cerror
.comm	_errno,4

cerror:
	movl	r0,_errno
	calls	$0,_syserr	# new code
	mnegl	$1,r0
	ret

.globl	__mycerror		# clumsy way to get this loaded

__mycerror:
	.word	0
	ret
