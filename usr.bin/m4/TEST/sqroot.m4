#
# Copyright (c) 1989, 1993
#	The Regents of the University of California.  All rights reserved.
#
# This code is derived from software contributed to Berkeley by
# Ozan Yigit.
#
# %sccs.include.redist.sh%
#
#	@(#)sqroot.m4	8.1 (Berkeley) 6/6/93
#

define(square_root, 
	`ifelse(eval($1<0),1,negative-square-root,
			     `square_root_aux($1, 1, eval(($1+1)/2))')')
define(square_root_aux,
	`ifelse($3, $2, $3,
		$3, eval($1/$2), $3,
		`square_root_aux($1, $3, eval(($3+($1/$3))/2))')')
