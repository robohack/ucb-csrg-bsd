#	@(#)Makefile	8.1 (Berkeley) 6/6/93

all explain clean cleandir depend lint tags:

install:
	install -c -o ${BINOWN} -g ${BINGRP} -m 444 \
	    ${.CURDIR}/explain.d ${DESTDIR}/usr/share/dict
	install -c -o ${BINOWN} -g ${BINGRP} -m ${BINMODE} \
	    ${.CURDIR}/explain.sh ${DESTDIR}${BINDIR}/explain

.include "../../Makefile.inc"
.include <bsd.prog.mk>
