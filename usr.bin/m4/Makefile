# @(#)Makefile	5.2 (Berkeley) 5/31/87

CFLAGS=-O
m4:	m4.o m4y.o
	$(CC) -o m4 m4.o m4y.o 

install:
	install -s m4 $(DESTDIR)/usr/bin

clean:
	rm -f *.o

depend:
