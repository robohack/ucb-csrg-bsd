/*
 * Copyright (c) 1980, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)config.h	8.1 (Berkeley) 6/6/93
 */

/*
 * Config.
 */
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

#define	NODEV	((dev_t)-1)

struct file_list {
	struct	file_list *f_next;	
	char	*f_fn;			/* the name */
	u_char	f_type;			/* see below */
	u_char	f_flags;		/* see below */
	char	*f_special;		/* special make rule if present */
	char	*f_needs;
	/*
	 * Random values:
	 *	swap space parameters for swap areas
	 *	root device, etc. for system specifications
	 */
	union {
		struct {		/* when swap specification */
			dev_t	fuw_swapdev;
			int	fuw_swapsize;
			int	fuw_swapflag;
		} fuw;
		struct {		/* when system specification */
			dev_t	fus_rootdev;
			dev_t	fus_dumpdev;
		} fus;
		struct {		/* when component dev specification */
			dev_t	fup_compdev;
			int	fup_compinfo;
		} fup;
	} fun;
#define	f_swapdev	fun.fuw.fuw_swapdev
#define	f_swapsize	fun.fuw.fuw_swapsize
#define	f_swapflag	fun.fuw.fuw_swapflag
#define	f_rootdev	fun.fus.fus_rootdev
#define	f_dumpdev	fun.fus.fus_dumpdev
#define f_compdev	fun.fup.fup_compdev
#define f_compinfo	fun.fup.fup_compinfo
};

/*
 * Types.
 */
#define DRIVER		1
#define NORMAL		2
#define	INVISIBLE	3
#define	PROFILING	4
#define	SYSTEMSPEC	5
#define	SWAPSPEC	6
#define COMPDEVICE	7
#define COMPSPEC	8

/*
 * Attributes (flags).
 */
#define	CONFIGDEP	1

struct	idlst {
	char	*id;
	struct	idlst *id_next;
};

struct device {
	int	d_type;			/* CONTROLLER, DEVICE, bus adaptor */
	struct	device *d_conn;		/* what it is connected to */
	char	*d_name;		/* name of device (e.g. rk11) */
	struct	idlst *d_vec;		/* interrupt vectors */
	int	d_pri;			/* interrupt priority */
	int	d_addr;			/* address of csr */
	int	d_unit;			/* unit number */
	int	d_drive;		/* drive number */
	int	d_slave;		/* slave number */
#define QUES	-1	/* -1 means '?' */
#define	UNKNOWN -2	/* -2 means not set yet */
	int	d_dk;			/* if init 1 set to number for iostat */
	int	d_flags;		/* flags for device init */
	char	*d_port;		/* io port base manifest constant */
	int	d_portn;	/* io port base (if number not manifest) */
	char	*d_mask;		/* interrupt mask */
	int	d_maddr;		/* io memory base */
	int	d_msize;		/* io memory size */
	int	d_drq;			/* DMA request  */
	int	d_irq;			/* interrupt request  */
	struct	device *d_next;		/* Next one in list */
};
#define TO_NEXUS	(struct device *)-1
#define TO_VBA		(struct device *)-2

struct config {
	char	*c_dev;
	char	*s_sysname;
};

/*
 * Config has a global notion of which machine type is
 * being used.  It uses the name of the machine in choosing
 * files and directories.  Thus if the name of the machine is ``vax'',
 * it will build from ``Makefile.vax'' and use ``../vax/inline''
 * in the makerules, etc.
 */
int	machine;
char	*machinename;
#define	MACHINE_VAX	1
#define	MACHINE_TAHOE	2
#define MACHINE_HP300	3
#define	MACHINE_I386	4
#define MACHINE_MIPS	5
#define MACHINE_PMAX	6
#define MACHINE_LUNA68K	7
#define MACHINE_NEWS3400	8

/*
 * For each machine, a set of CPU's may be specified as supported.
 * These and the options (below) are put in the C flags in the makefile.
 */
struct cputype {
	char	*cpu_name;
	struct	cputype *cpu_next;
} *cputype;

/*
 * A set of options may also be specified which are like CPU types,
 * but which may also specify values for the options.
 * A separate set of options may be defined for make-style options.
 */
struct opt {
	char	*op_name;
	char	*op_value;
	struct	opt *op_next;
} *opt, *mkopt;

char	*ident;
char	*ns();
char	*tc();
char	*qu();
char	*get_word();
char	*get_quoted_word();
char	*path();
char	*raise();

int	do_trace;

#if MACHINE_VAX
int	seen_mba, seen_uba;
#endif
#if MACHINE_TAHOE
int	seen_vba;
#endif
#if MACHINE_I386
int	seen_isa;
#endif
int	seen_cd;

struct	device *connect();
struct	device *dtab;
dev_t	nametodev();
char	*devtoname();

char	errbuf[80];
int	yyline;

struct	file_list *ftab, *conf_list, **confp, *comp_list, **compp;

int	zone, hadtz;
int	dst;
int	hz;
int	profiling;
int	debugging;

int	maxusers;

#define eq(a,b)	(!strcmp(a,b))
