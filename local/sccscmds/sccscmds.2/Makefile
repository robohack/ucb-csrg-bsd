#	@(#)Makefile	4.6	(Berkeley)	89/05/10
#
all:
	cd util; make $(MFLAGS)
	cd com; make $(MFLAGS)
	cd cmd; make $(MFLAGS)

install:
	cd cmd; make $(MFLAGS) DESTDIR=$(DESTDIR) install
	cd doc; make ${MFLAGS} DESTDIR=${DESTDIR} install
	install -c -m 644 sccs.hf ${DESTDIR}/usr/local/lib/sccs.hf

clean cleandir:
	rm -f lib*.a errs
	cd util; make $(MFLAGS) clean
	cd com; make $(MFLAGS) clean
	cd cmd; make $(MFLAGS) clean

depend:
