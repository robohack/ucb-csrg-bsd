/*
 * Copyright (c) 1988 University of Utah.
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * the Systems Programming Group of the University of Utah Computer
 * Science Department.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)cons.c	8.1 (Berkeley) 6/11/93
 */


#include <sys/param.h>
#include <sys/proc.h>
#include <sys/user.h>
#include <sys/systm.h>
#include <sys/buf.h>
#include <sys/ioctl.h>
#include <sys/tty.h>
#include <sys/file.h>
#include <sys/conf.h>

#include <i386/i386/cons.h>

/* XXX - all this could be autoconfig()ed */
int pccnprobe(), pccninit(), pccngetc(), pccnputc();
#include "com.h"
#if NCOM > 0
int comcnprobe(), comcninit(), comcngetc(), comcnputc();
#endif

struct	consdev constab[] = {
	{ pccnprobe,	pccninit,	pccngetc,	pccnputc },
#if NCOM > 0
	{ comcnprobe,	comcninit,	comcngetc,	comcnputc },
#endif
	{ 0 },
};
/* end XXX */

struct	tty *constty = 0;	/* virtual console output device */
struct	consdev *cn_tab;	/* physical console device info */
struct	tty *cn_tty;		/* XXX: console tty struct for tprintf */

cninit()
{
	register struct consdev *cp;

	/*
	 * Collect information about all possible consoles
	 * and find the one with highest priority
	 */
	for (cp = constab; cp->cn_probe; cp++) {
		(*cp->cn_probe)(cp);
		if (cp->cn_pri > CN_DEAD &&
		    (cn_tab == NULL || cp->cn_pri > cn_tab->cn_pri))
			cn_tab = cp;
	}
	/*
	 * No console, we can handle it
	 */
	if ((cp = cn_tab) == NULL)
		return;
	/*
	 * Turn on console
	 */
	cn_tty = cp->cn_tp;
	(*cp->cn_init)(cp);
}

cnopen(dev, flag, mode, p)
	dev_t dev;
	int flag, mode;
	struct proc *p;
{
	if (cn_tab == NULL)
		return (0);
	dev = cn_tab->cn_dev;
	return ((*cdevsw[major(dev)].d_open)(dev, flag, mode, p));
}
 
cnclose(dev, flag, mode, p)
	dev_t dev;
	int flag, mode;
	struct proc *p;
{
	if (cn_tab == NULL)
		return (0);
	dev = cn_tab->cn_dev;
	return ((*cdevsw[major(dev)].d_close)(dev, flag, mode, p));
}
 
cnread(dev, uio, flag)
	dev_t dev;
	struct uio *uio;
{
	if (cn_tab == NULL)
		return (0);
	dev = cn_tab->cn_dev;
	return ((*cdevsw[major(dev)].d_read)(dev, uio, flag));
}
 
cnwrite(dev, uio, flag)
	dev_t dev;
	struct uio *uio;
{
	if (cn_tab == NULL)
		return (0);
	dev = cn_tab->cn_dev;
	return ((*cdevsw[major(dev)].d_write)(dev, uio, flag));
}
 
cnioctl(dev, cmd, data, flag, p)
	dev_t dev;
	caddr_t data;
	struct proc *p;
{
	int error;

	if (cn_tab == NULL)
		return (0);
	/*
	 * Superuser can always use this to wrest control of console
	 * output from the "virtual" console.
	 */
	if (cmd == TIOCCONS && constty) {
		error = suser(p->p_ucred, (u_short *) NULL);
		if (error)
			return (error);
		constty = NULL;
		return (0);
	}
	dev = cn_tab->cn_dev;
	return ((*cdevsw[major(dev)].d_ioctl)(dev, cmd, data, flag, p));
}

/*ARGSUSED*/
cnselect(dev, rw, p)
	dev_t dev;
	int rw;
	struct proc *p;
{
	if (cn_tab == NULL)
		return (1);
	return (ttselect(cn_tab->cn_dev, rw, p));
}

cngetc()
{
	if (cn_tab == NULL)
		return (0);
	return ((*cn_tab->cn_getc)(cn_tab->cn_dev));
}

cnputc(c)
	register int c;
{
	if (cn_tab == NULL)
		return;
	if (c) {
		(*cn_tab->cn_putc)(cn_tab->cn_dev, c);
		if (c == '\n')
			(*cn_tab->cn_putc)(cn_tab->cn_dev, '\r');
	}
}
