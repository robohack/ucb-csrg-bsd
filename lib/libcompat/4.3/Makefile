#
# Copyright (c) 1988 Regents of the University of California.
# All rights reserved.
#
# Redistribution and use in source and binary forms are permitted
# provided that the above copyright notice and this paragraph are
# duplicated in all such forms and that any documentation,
# advertising materials, and other materials related to such
# distribution and use acknowledge that the software was developed
# by the University of California, Berkeley.  The name of the
# University may not be used to endorse or promote products derived
# from this software without specific prior written permission.
# THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
# WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
#
#	@(#)Makefile	5.7 (Berkeley) 3/22/90
#

DEFS=	-DLIBC_SCCS
CFLAGS=	-O ${DEFS}
SRCS=	ecvt.c gcvt.c lsearch.c rexec.c ruserpass.c sibuf.c sobuf.c strout.c
OBJS=	ecvt.o gcvt.o lsearch.o rexec.o ruserpass.o sibuf.o sobuf.o strout.o
MAN=	ecvt.0 lsearch.0 rexec.0
TAGSFILE=tags
MDIR=	${DESTDIR}/usr/man/cat3

.c.o:
	@${CC} -p ${CFLAGS} -c $*.c
	@-ld -X -o profiled/$*.o -r $*.o
	${CC} ${CFLAGS} -c $*.c
	@-ld -x -r $*.o
	@mv a.out $*.o

all: ${OBJS}

tags:
	rm -f ${TAGSFILE}
	cwd=`pwd`; \
	for i in ${SRCS}; do \
		ctags -a -f ${TAGSFILE} $$cwd/$$i; \
	done

clean:
	rm -f *.o profiled/* a.out core ${TAGSFILE}

cleandir: clean
	rm -f ${MAN} tags .depend

depend:
	mkdep ${CFLAGS} ${SRCS}

install: ${MAN}
	cd ../library; rm -f ${OBJS}
	cd ../profiled; rm -f ${OBJS}
	ln ${OBJS} ../library
	cd profiled; ln ${OBJS} ../../profiled
	install -c -o bin -g bin -m 444 ${MAN} ${MDIR}
	rm -f ${MDIR}/fcvt.0; ln ${MDIR}/ecvt.0 ${MDIR}/fcvt.0
	rm -f ${MDIR}/gcvt.0; ln ${MDIR}/ecvt.0 ${MDIR}/gcvt.0
