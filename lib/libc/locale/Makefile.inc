#	@(#)Makefile.inc	5.2 (Berkeley) 3/2/93

# locale sources
.PATH: ${.CURDIR}/${MACHINE}/locale ${.CURDIR}/locale

SRCS+=	lconv.c localeconv.c setlocale.c
MAN3+=	setlocale.0
