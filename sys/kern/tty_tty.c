/*-
 * Copyright (c) 1982, 1986, 1991 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)tty_tty.c	7.18 (Berkeley) 5/14/92
 */

/*
 * Indirect driver for controlling tty.
 */
#include "param.h"
#include "systm.h"
#include "conf.h"
#include "ioctl.h"
#include "proc.h"
#include "tty.h"
#include "vnode.h"
#include "file.h"

#define cttyvp(p) ((p)->p_flag&SCTTY ? (p)->p_session->s_ttyvp : NULL)

/*ARGSUSED*/
cttyopen(dev, flag, mode, p)
	dev_t dev;
	int flag, mode;
	struct proc *p;
{
	USES_VOP_ACCESS;
	USES_VOP_LOCK;
	USES_VOP_OPEN;
	USES_VOP_UNLOCK;
	struct vnode *ttyvp = cttyvp(p);
	int error;

	if (ttyvp == NULL)
		return (ENXIO);
	VOP_LOCK(ttyvp);
	error = VOP_ACCESS(ttyvp,
	  (flag&FREAD ? VREAD : 0) | (flag&FWRITE ? VWRITE : 0), p->p_ucred, p);
	if (!error)
		error = VOP_OPEN(ttyvp, flag, NOCRED, p);
	VOP_UNLOCK(ttyvp);
	return (error);
}

/*ARGSUSED*/
cttyread(dev, uio, flag)
	dev_t dev;
	struct uio *uio;
	int flag;
{
	USES_VOP_LOCK;
	USES_VOP_READ;
	USES_VOP_UNLOCK;
	register struct vnode *ttyvp = cttyvp(uio->uio_procp);
	int error;

	if (ttyvp == NULL)
		return (EIO);
	VOP_LOCK(ttyvp);
	error = VOP_READ(ttyvp, uio, flag, NOCRED);
	VOP_UNLOCK(ttyvp);
	return (error);
}

/*ARGSUSED*/
cttywrite(dev, uio, flag)
	dev_t dev;
	struct uio *uio;
	int flag;
{
	USES_VOP_LOCK;
	USES_VOP_UNLOCK;
	USES_VOP_WRITE;
	register struct vnode *ttyvp = cttyvp(uio->uio_procp);
	int error;

	if (ttyvp == NULL)
		return (EIO);
	VOP_LOCK(ttyvp);
	error = VOP_WRITE(ttyvp, uio, flag, NOCRED);
	VOP_UNLOCK(ttyvp);
	return (error);
}

/*ARGSUSED*/
cttyioctl(dev, cmd, addr, flag, p)
	dev_t dev;
	int cmd;
	caddr_t addr;
	int flag;
	struct proc *p;
{
	USES_VOP_IOCTL;
	struct vnode *ttyvp = cttyvp(p);

	if (ttyvp == NULL)
		return (EIO);
	if (cmd == TIOCNOTTY) {
		if (!SESS_LEADER(p)) {
			p->p_flag &= ~SCTTY;
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
	USES_VOP_SELECT;
	struct vnode *ttyvp = cttyvp(p);

	if (ttyvp == NULL)
		return (1);	/* try operation to get EOF/failure */
	return (VOP_SELECT(ttyvp, flag, FREAD|FWRITE, NOCRED, p));
}
