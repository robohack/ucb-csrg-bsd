#	@(#)Makefile	6.3 (Berkeley) 6/5/93

DIR=	psd/11.adb
SRCS=	adb.ms
MACROS=	-ms

paper.ps: ${SRCS}
	${EQN} ${SRCS} | ${TBL} | ${ROFF} > ${.TARGET}

.include <bsd.doc.mk>
