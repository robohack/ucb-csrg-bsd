#	@(#)Makefile	8.1 (Berkeley) 6/6/93

PROG=	diff3
BINDIR=	/usr/libexec

beforeinstall:
	install -c -o ${BINOWN} -g ${BINGRP} -m ${BINMODE} \
	    ${.CURDIR}/diff3.sh ${DESTDIR}/usr/bin/${PROG}

.include "../../Makefile.inc"
.include <bsd.prog.mk>
