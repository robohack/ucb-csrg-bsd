/*-
 * Copyright (c) 1982, 1986, 1991, 1993, 1995
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)tty_tty.c	8.4 (Berkeley) 5/14/95
 */

/*
 * Indirect driver for controlling tty.
 */
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/conf.h>
#include <sys/ioctl.h>
#include <sys/proc.h>
#include <sys/tty.h>
#include <sys/vnode.h>
#include <sys/file.h>

#define cttyvp(p) ((p)->p_flag & P_CONTROLT ? (p)->p_session->s_ttyvp : NULL)

/*ARGSUSED*/
cttyopen(dev, flag, mode, p)
	dev_t dev;
	int flag, mode;
	struct proc *p;
{
	struct vnode *ttyvp = cttyvp(p);
	int error;

	if (ttyvp == NULL)
		return (ENXIO);
	vn_lock(ttyvp, LK_EXCLUSIVE | LK_RETRY, p);
#ifdef PARANOID
	/*
	 * Since group is tty and mode is 620 on most terminal lines
	 * and since sessions protect terminals from processes outside
	 * your session, this check is probably no longer necessary.
	 * Since it inhibits setuid root programs that later switch 
	 * to another user from accessing /dev/tty, we have decided
	 * to delete this test. (mckusick 5/93)
	 */
	error = VOP_ACCESS(ttyvp,
	  (flag&FREAD ? VREAD : 0) | (flag&FWRITE ? VWRITE : 0), p->p_ucred, p);
	if (!error)
#endif /* PARANOID */
		error = VOP_OPEN(ttyvp, flag, NOCRED, p);
	VOP_UNLOCK(ttyvp, 0, p);
	return (error);
}

/*ARGSUSED*/
cttyread(dev, uio, flag)
	dev_t dev;
	struct uio *uio;
	int flag;
{
	struct proc *p = uio->uio_procp;
	register struct vnode *ttyvp = cttyvp(p);
	int error;

	if (ttyvp == NULL)
		return (EIO);
	vn_lock(ttyvp, LK_EXCLUSIVE | LK_RETRY, p);
	error = VOP_READ(ttyvp, uio, flag, NOCRED);
	VOP_UNLOCK(ttyvp, 0, p);
	return (error);
}

/*ARGSUSED*/
cttywrite(dev, uio, flag)
	dev_t dev;
	struct uio *uio;
	int flag;
{
	struct proc *p = uio->uio_procp;
	struct vnode *ttyvp = cttyvp(uio->uio_procp);
	int error;

	if (ttyvp == NULL)
		return (EIO);
	vn_lock(ttyvp, LK_EXCLUSIVE | LK_RETRY, p);
	error = VOP_WRITE(ttyvp, uio, flag, NOCRED);
	VOP_UNLOCK(ttyvp, 0, p);
	return (error);
}

/*ARGSUSED*/
cttyioctl(dev, cmd, addr, flag, p)
	dev_t dev;
	u_long cmd;
	caddr_t addr;
	int flag;
	struct proc *p;
{
	struct vnode *ttyvp = cttyvp(p);

	if (ttyvp == NULL)
		return (EIO);
	if (cmd == TIOCNOTTY) {
		if (!SESS_LEADER(p)) {
			p->p_flag &= ~P_CONTROLT;
			return (0);
		} else
			return (EINVAL);
	}
	return (VOP_IOCTL(ttyvp, cmd, addr, flag, NOCRED, p));
}

/*ARGSUSED*/
cttyselect(dev, flag, p)
	dev_t dev;
	int flag;
	struct proc *p;
{
	struct vnode *ttyvp = cttyvp(p);

	if (ttyvp == NULL)
		return (1);	/* try operation to get EOF/failure */
	return (VOP_SELECT(ttyvp, flag, FREAD|FWRITE, NOCRED, p));
}
