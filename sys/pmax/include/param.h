/*
 * Copyright (c) 1988 University of Utah.
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * the Systems Programming Group of the University of Utah Computer
 * Science Department and Ralph Campbell.
 *
 * %sccs.include.redist.c%
 *
 * from: Utah $Hdr: machparam.h 1.11 89/08/14$
 *
 *	@(#)param.h	8.5 (Berkeley) 6/2/95
 */

/*
 * Machine dependent constants for DEC Station 3100.
 */
#define	MACHINE	"mips"
#define NCPUS 1

/*
 * Round p (pointer or byte index) up to a correctly-aligned value for all
 * data types (int, long, ...).   The result is u_int and must be cast to
 * any desired pointer type.
 */
#define	ALIGNBYTES	7
#define	ALIGN(p)	(((u_int)(p) + ALIGNBYTES) &~ ALIGNBYTES)

#define	NBPG		4096		/* bytes/page */
#define	PGOFSET		(NBPG-1)	/* byte offset into page */
#define	PGSHIFT		12		/* LOG2(NBPG) */
#define	NPTEPG		(NBPG/4)

#define NBSEG		0x400000	/* bytes/segment */
#define	SEGOFSET	(NBSEG-1)	/* byte offset into segment */
#define	SEGSHIFT	22		/* LOG2(NBSEG) */

#define	KERNBASE	0x80000000	/* start of kernel virtual */
#define	BTOPKERNBASE	((u_long)KERNBASE >> PGSHIFT)

#define	DEV_BSIZE	512
#define	DEV_BSHIFT	9		/* log2(DEV_BSIZE) */
#define BLKDEV_IOSIZE	2048
#define	MAXPHYS		(64 * 1024)	/* max raw I/O transfer size */

#define	CLSIZE		1
#define	CLSIZELOG2	0

/* NOTE: SSIZE, SINCR and UPAGES must be multiples of CLSIZE */
#define	SSIZE		1		/* initial stack size/NBPG */
#define	SINCR		1		/* increment of stack/NBPG */

#define	UPAGES		2		/* pages of u-area */
#define	UADDR		0xffffd000	/* address of u */
#define	UVPN		(UADDR>>PGSHIFT)/* virtual page number of u */
#define	KERNELSTACK	(UADDR+UPAGES*NBPG)	/* top of kernel stack */

/*
 * Constants related to network buffer management.
 * MCLBYTES must be no larger than CLBYTES (the software page size), and,
 * on machines that exchange pages of input or output buffers with mbuf
 * clusters (MAPPED_MBUFS), MCLBYTES must also be an integral multiple
 * of the hardware page size.
 */
#define	MSIZE		128		/* size of an mbuf */
#define	MCLBYTES	1024
#define	MCLSHIFT	10
#define	MCLOFSET	(MCLBYTES - 1)
#ifndef NMBCLUSTERS
#ifdef GATEWAY
#define	NMBCLUSTERS	512		/* map size, max cluster allocation */
#else
#define	NMBCLUSTERS	256		/* map size, max cluster allocation */
#endif
#endif

/*
 * Size of kernel malloc arena in CLBYTES-sized logical pages
 */ 
#ifndef NKMEMCLUSTERS
#define	NKMEMCLUSTERS	(512*1024/CLBYTES)
#endif

/* pages ("clicks") (4096 bytes) to disk blocks */
#define	ctod(x)	((x)<<(PGSHIFT-DEV_BSHIFT))
#define	dtoc(x)	((x)>>(PGSHIFT-DEV_BSHIFT))
#define	dtob(x)	((x)<<DEV_BSHIFT)

/* pages to bytes */
#define	ctob(x)	((x)<<PGSHIFT)

/* bytes to pages */
#define	btoc(x)	(((unsigned)(x)+(NBPG-1))>>PGSHIFT)

#define	btodb(bytes)	 		/* calculates (bytes / DEV_BSIZE) */ \
	((bytes) >> DEV_BSHIFT)
#define	dbtob(db)			/* calculates (db * DEV_BSIZE) */ \
	((db) << DEV_BSHIFT)

/*
 * Map a ``block device block'' to a file system block.
 * This should be device dependent, and should use the bsize
 * field from the disk label.
 * For now though just use DEV_BSIZE.
 */
#define	bdbtofsb(bn)	((bn) / (BLKDEV_IOSIZE/DEV_BSIZE))

/*
 * Mach derived conversion macros
 */
#define pmax_round_page(x)	((((unsigned)(x)) + NBPG - 1) & ~(NBPG-1))
#define pmax_trunc_page(x)	((unsigned)(x) & ~(NBPG-1))
#define pmax_btop(x)		((unsigned)(x) >> PGSHIFT)
#define pmax_ptob(x)		((unsigned)(x) << PGSHIFT)

#ifndef LOCORE
#ifdef KERNEL
extern int (*Mach_splnet)(), (*Mach_splbio)(), (*Mach_splimp)(),
	   (*Mach_spltty)(), (*Mach_splclock)(), (*Mach_splstatclock)();
#define	splnet()	((*Mach_splnet)())
#define	splbio()	((*Mach_splbio)())
#define	splimp()	((*Mach_splimp)())
#define	spltty()	((*Mach_spltty)())
#define	splclock()	((*Mach_splclock)())
#define	splstatclock()	((*Mach_splstatclock)())
extern	int cpuspeed;
#define	DELAY(n)	{ register int N = cpuspeed * (n); while (--N > 0); }
#else /* !KERNEL */
#define	DELAY(n)	{ register int N = (n); while (--N > 0); }
#endif /* !KERNEL */

#ifndef _SIMPLELOCK_H_
#define _SIMPLELOCK_H_
/*
 * A simple spin lock.
 *
 * This structure only sets one bit of data, but is sized based on the
 * minimum word size that can be operated on by the hardware test-and-set
 * instruction. It is only needed for multiprocessors, as uniprocessors
 * will always run to completion or a sleep. It is an error to hold one
 * of these locks while a process is sleeping.
 */
struct simplelock {
	int	lock_data;
};

#if !defined(DEBUG) && NCPUS > 1
/*
 * The simple-lock routines are the primitives out of which the lock
 * package is built. The machine-dependent code must implement an
 * atomic test_and_set operation that indivisibly sets the simple lock
 * to non-zero and returns its old value. It also assumes that the
 * setting of the lock to zero below is indivisible. Simple locks may
 * only be used for exclusive locks.
 */
static __inline void
simple_lock_init(lkp)
	struct simplelock *lkp;
{

	lkp->lock_data = 0;
}

static __inline void
simple_lock(lkp)
	__volatile struct simplelock *lkp;
{

	while (test_and_set(&lkp->lock_data))
		continue;
}

static __inline int
simple_lock_try(lkp)
	__volatile struct simplelock *lkp;
{

	return (!test_and_set(&lkp->lock_data))
}

static __inline void
simple_unlock(lkp)
	__volatile struct simplelock *lkp;
{

	lkp->lock_data = 0;
}
#endif /* NCPUS > 1 */
#endif /* !_SIMPLELOCK_H_ */
#endif /* LOCORE */
