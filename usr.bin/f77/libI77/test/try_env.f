C
C Copyright (c) 1980 The Regents of the University of California.
C All rights reserved.
C
C %sccs.include.proprietary.f%
C
C	@(#)try_env.f	5.2 (Berkeley) 4/12/91
C

c		test overriding file names from program with
c			environment variables
c		formatted, seq
	write(8,8000) 8
8000	format('sample line ',i5)
	open(9,file='subdir/frmt.d')
	write(9,8000) 9
c		unform, seq
	open(10,form='unformatted')
	write(10) 10
	open(11,file='unfr.dat',form='unformatted')
	write(11) 11
c		direct
	open(12,access='direct',form='formatted',recl=20)
	write(12,8000,rec=3) 12
	open(13,file='dirct',access='direct',form='unformatted',recl=8)
	write(13,rec=3) 127, 16*127
	end
