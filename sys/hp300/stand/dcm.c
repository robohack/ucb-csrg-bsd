/*
 * Copyright (c) 1988 University of Utah.
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * the Systems Programming Group of the University of Utah Computer
 * Science Department.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)dcm.c	8.1 (Berkeley) 6/10/93
 */

#ifdef DCMCONSOLE
#include <sys/param.h>
#include <hp/dev/cons.h>
#include <hp/dev/device.h>
#include <hp300/dev/dcmreg.h>

struct dcmdevice *dcmcnaddr = NULL;

dcmprobe(cp)
	struct consdev *cp;
{
	extern struct hp_hw sc_table[];
	register struct hp_hw *hw;
	register struct dcmdevice *dcm;

	for (hw = sc_table; hw < &sc_table[MAXCTLRS]; hw++)
	        if (HW_ISDEV(hw, D_COMMDCM) && !badaddr((caddr_t)hw->hw_kva))
			break;
	if (!HW_ISDEV(hw, D_COMMDCM)) {
		cp->cn_pri = CN_DEAD;
		return;
	}
	dcmcnaddr = (struct dcmdevice *) hw->hw_kva;

	dcm = dcmcnaddr;
	switch (dcm->dcm_rsid) {
	case DCMID:
		cp->cn_pri = CN_NORMAL;
		break;
	case DCMID|DCMCON:
		cp->cn_pri = CN_REMOTE;
		break;
	default:
		cp->cn_pri = CN_DEAD;
		break;
	}
}

dcminit(cp)
	struct consdev *cp;
{
	register struct dcmdevice *dcm = dcmcnaddr;
	register int port = CONUNIT;

	dcm->dcm_ic = IC_ID;
	while (dcm->dcm_thead[port].ptr != dcm->dcm_ttail[port].ptr)
		;
	dcm->dcm_data[port].dcm_baud = BR_9600;
	dcm->dcm_data[port].dcm_conf = LC_8BITS | LC_1STOP;
	SEM_LOCK(dcm);
	dcm->dcm_cmdtab[port].dcm_data |= CT_CON;
	dcm->dcm_cr |= (1 << port);
	SEM_UNLOCK(dcm);
	DELAY(15000);
}

#ifndef SMALL
dcmgetchar()
{
	register struct dcmdevice *dcm = dcmcnaddr;
	register struct dcmrfifo *fifo;
	register struct dcmpreg *pp;
	register unsigned head;
	int c, stat, port;

	port = CONUNIT;
	pp = dcm_preg(dcm, port);
	head = pp->r_head & RX_MASK;
	if (head == (pp->r_tail & RX_MASK))
		return(0);
	fifo = &dcm->dcm_rfifos[3-port][head>>1];
	c = fifo->data_char;
	stat = fifo->data_stat;
	pp->r_head = (head + 2) & RX_MASK;
	SEM_LOCK(dcm);
	stat = dcm->dcm_iir;
	SEM_UNLOCK(dcm);
	return(c);
}
#else
dcmgetchar()
{
	return(0);
}
#endif

dcmputchar(c)
	register int c;
{
	register struct dcmdevice *dcm = dcmcnaddr;
	register struct dcmpreg *pp;
	register int timo;
	unsigned tail;
	int port, stat;

	port = CONUNIT;
	pp = dcm_preg(dcm, port);
	tail = pp->t_tail & TX_MASK;
	timo = 50000;
	while (tail != (pp->t_head & TX_MASK) && --timo)
		;
	dcm->dcm_tfifos[3-port][tail].data_char = c;
	pp->t_tail = tail = (tail + 1) & TX_MASK;
	SEM_LOCK(dcm);
	dcm->dcm_cmdtab[port].dcm_data |= CT_TX;
	dcm->dcm_cr |= (1 << port);
	SEM_UNLOCK(dcm);
	timo = 1000000;
	while (tail != (pp->t_head & TX_MASK) && --timo)
		;
	SEM_LOCK(dcm);
	stat = dcm->dcm_iir;
	SEM_UNLOCK(dcm);
}
#endif
