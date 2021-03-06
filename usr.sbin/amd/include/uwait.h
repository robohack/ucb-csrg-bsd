/*
 * Copyright (c) 1989 Jan-Simon Pendry
 * Copyright (c) 1989 Imperial College of Science, Technology & Medicine
 * Copyright (c) 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Jan-Simon Pendry at Imperial College, London.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)uwait.h	8.1 (Berkeley) 6/6/93
 *
 * $Id: uwait.h,v 5.2.2.1 1992/02/09 15:10:01 jsp beta $
 *
 */

#if defined(mc68k) || defined(mc68000) || defined(mc68020) || defined(sparc) || defined(hp9000s300) || defined(hp9000s800)
#define BITS_BIGENDIAN
#endif
#if defined(vax) || defined(i386)
#define BITS_LITTLENDIAN
#endif
#if !defined BITS_BIGENDIAN && !defined BITS_LITTLENDIAN
 #error Do not know my byte ordering
#endif

/*
 * Structure of the information in the first word returned by both
 * wait and wait3.  If w_stopval==WSTOPPED, then the second structure
 * describes the information returned, else the first.  See WUNTRACED below.
 */
union wait	{
	int	w_status;		/* used in syscall */
	/*
	 * Terminated process status.
	 */
	struct {
#ifdef BITS_LITTLENDIAN
		unsigned short	w_Termsig:7;	/* termination signal */
		unsigned short	w_Coredump:1;	/* core dump indicator */
		unsigned short	w_Retcode:8;	/* exit code if w_termsig==0 */
#endif
#ifdef BITS_BIGENDIAN
		unsigned short	w_Fill1:16;	/* high 16 bits unused */
		unsigned short	w_Retcode:8;	/* exit code if w_termsig==0 */
		unsigned short	w_Coredump:1;	/* core dump indicator */
		unsigned short	w_Termsig:7;	/* termination signal */
#endif
	} w_U;
};
#define	w_termsig	w_U.w_Termsig
#define w_coredump	w_U.w_Coredump
#define w_retcode	w_U.w_Retcode

#define WIFSIGNALED(x)	((x).w_termsig != 0)
#define WIFEXITED(x)	((x).w_termsig == 0)
