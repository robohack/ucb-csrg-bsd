#	@(#)Makefile	5.2 (Berkeley) 6/8/90

all learnlib clean cleandir depend lint tags:

install:
	install -c -o ${BINOWN} -g ${BINOWN} -m 444 Linfo Xinfo \
	    ${DESTDIR}${LLIB}/learn
	cp -pr C editor eqn files macros morefiles ${DESTDIR}${LLIB}/learn

.include "../Makefile.inc"
.include <bsd.prog.mk>
