#	@(#)Makefile	8.2 (Berkeley) 9/6/93

PROG=	dbtest
OBJS=	dbtest.o #strerror.o
CFLAGS=	-O -DDEBUG -DSTATISTICS
LIB=	../PORT/bsd.44/libdb.a

dbtest: ${OBJS} ${LIB}
	${CC} -o $@ ${OBJS} ${LIB}
