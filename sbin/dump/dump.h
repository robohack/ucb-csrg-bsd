/*-
 * Copyright (c) 1980 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)dump.h	5.19 (Berkeley) 6/18/92
 */

#define MAXINOPB	(MAXBSIZE / sizeof(struct dinode))
#define MAXNINDIR	(MAXBSIZE / sizeof(daddr_t))

/*
 * Dump maps used to describe what is to be dumped.
 */
int	mapsize;	/* size of the state maps */
char	*usedinomap;	/* map of allocated inodes */
char	*dumpdirmap;	/* map of directories to be dumped */
char	*dumpinomap;	/* map of files to be dumped */
/*
 * Map manipulation macros.
 */
#define	SETINO(ino, map) \
	map[(u_int)((ino) - 1) / NBBY] |=  1 << ((u_int)((ino) - 1) % NBBY)
#define	CLRINO(ino, map) \
	map[(u_int)((ino) - 1) / NBBY] &=  ~(1 << ((u_int)((ino) - 1) % NBBY))
#define	TSTINO(ino, map) \
	(map[(u_int)((ino) - 1) / NBBY] &  (1 << ((u_int)((ino) - 1) % NBBY)))

/*
 *	All calculations done in 0.1" units!
 */
char	*disk;		/* name of the disk file */
char	*tape;		/* name of the tape file */
char	*dumpdates;	/* name of the file containing dump date information*/
char	*temp;		/* name of the file for doing rewrite of dumpdates */
char	lastlevel;	/* dump level of previous dump */
char	level;		/* dump level of this dump */
int	uflag;		/* update flag */
int	diskfd;		/* disk file descriptor */
int	tapefd;		/* tape file descriptor */
int	pipeout;	/* true => output to standard output */
ino_t	curino;		/* current inumber; used globally */
int	newtape;	/* new tape flag */
int	density;	/* density in 0.1" units */
long	tapesize;	/* estimated tape size, blocks */
long	tsize;		/* tape size in 0.1" units */
long	asize;		/* number of 0.1" units written on current tape */
int	etapes;		/* estimated number of tapes */

int	notify;		/* notify operator flag */
int	blockswritten;	/* number of blocks written on current tape */
int	tapeno;		/* current tape number */
time_t	tstart_writing;	/* when started writing the first tape block */
struct	fs *sblock;	/* the file system super block */
char	sblock_buf[MAXBSIZE];
long	dev_bsize;	/* block size of underlying disk device */
int	dev_bshift;	/* log2(dev_bsize) */
int	tp_bshift;	/* log2(TP_BSIZE) */

/* operator interface functions */
void	broadcast __P((char *message));
void	lastdump __P((int arg));	/* int should be char */
time_t	unctime __P((char *str));
#if __STDC__
void	msg(const char *fmt, ...);
void	msgtail(const char *fmt, ...);
void	quit(const char *fmt, ...);
#else
void	msg();
void	msgtail();
void	quit();
#endif
int	query __P((char *question));
void	set_operators __P(());
void	timeest __P(());

/* mapping rouintes */
struct	dinode;
long	blockest __P((struct dinode *dp));
int	mapfiles __P((ino_t maxino, long *tapesize));
int	mapdirs __P((ino_t maxino, long *tapesize));

/* file dumping routines */
void	dumpino __P((struct dinode *dp, ino_t ino));
void	blksout __P((daddr_t *blkp, int frags, ino_t ino));
void	dumpmap __P((char *map, int type, ino_t ino));
void	writeheader __P((ino_t ino));
void	bread __P((daddr_t blkno, char *buf, int size));	

/* tape writing routines */
int	alloctape __P(());
void	writerec __P((char *dp));
void	dumpblock __P((daddr_t blkno, int size));
void	flushtape __P(());
void	trewind __P(());
void	close_rewind __P(());
void	startnewtape __P(());

void	dumpabort __P(());
void	Exit __P((int status));
void	getfstab __P(());

char	*rawname __P((char *cp));
struct	dinode *getino __P((ino_t inum));

void	interrupt __P(());	/* in case operator bangs on console */

/*
 *	Exit status codes
 */
#define	X_FINOK		0	/* normal exit */
#define	X_REWRITE	2	/* restart writing from the check point */
#define	X_ABORT		3	/* abort dump; don't attempt checkpointing */

#define	OPGRENT	"operator"		/* group entry to notify */
#define DIALUP	"ttyd"			/* prefix for dialups */

struct	fstab *fstabsearch __P((char *key));	/* search fs_file and fs_spec */

#ifndef NAME_MAX
#define NAME_MAX 255
#endif

/*
 *	The contents of the file _PATH_DUMPDATES is maintained both on
 *	a linked list, and then (eventually) arrayified.
 */
struct dumpdates {
	char	dd_name[NAME_MAX+3];
	char	dd_level;
	time_t	dd_ddate;
};
struct dumptime {
	struct	dumpdates dt_value;
	struct	dumptime *dt_next;
};
struct	dumptime *dthead;	/* head of the list version */
int	nddates;		/* number of records (might be zero) */
int	ddates_in;		/* we have read the increment file */
struct	dumpdates **ddatev;	/* the arrayfied version */
void	initdumptimes __P(());
void	getdumptime __P(());
void	putdumptime __P(());
#define	ITITERATE(i, ddp) \
	for (ddp = ddatev[i = 0]; i < nddates; ddp = ddatev[++i])

/*
 *	We catch these interrupts
 */
void	sighup __P(());
void	sigtrap __P(());
void	sigfpe __P(());
void	sigbus __P(());
void	sigsegv __P(());
void	sigalrm __P(());
void	sigterm __P(());

/*
 * Compatibility with old systems.
 */
#ifndef __STDC__
#include <sys/file.h>
#define _PATH_FSTAB	"/etc/fstab"
extern char *index(), *strdup();
extern char *ctime();
extern int errno;
#endif

#ifdef sunos
extern char *calloc();
extern char *malloc();
extern long atol();
extern char *strcpy();
extern char *strncpy();
extern char *strcat();
extern time_t time();
extern void endgrent();
extern void exit();
extern off_t lseek();
extern char *strerror();
#endif
