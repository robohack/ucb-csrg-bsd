/*
 * Copyright (c) University of British Columbia, 1984
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * the Laboratory for Computation Vision and the Computer Science Department
 * of the University of British Columbia.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)hd_output.c	7.4 (Berkeley) 8/30/90
 */

#include "param.h"
#include "systm.h"
#include "mbuf.h"
#include "domain.h"
#include "socket.h"
#include "protosw.h"
#include "errno.h"
#include "time.h"
#include "kernel.h"

#include "../net/if.h"

#include "hdlc.h"
#include "hd_var.h"
#include "x25.h"

/*
 *      HDLC OUTPUT INTERFACE
 *
 *      This routine is called when the X.25 packet layer output routine
 *      has a information frame (iframe)  to write.   It is  also called 
 *      by the input and control routines of the HDLC layer.
 */

hd_output (m, info)
register struct mbuf *m;
caddr_t	info;
{
	register struct hdcb *hdp = (struct hdcb *)info;
	struct x25config *xcp;

	if (m == NULL)
		panic ("hd_output");

	if (hdp->hd_state != ABM) {
		m_freem (m);
		return;
	}

	/*
	 * Make room for the hdlc header either by prepending
	 * another mbuf, or by adjusting the offset and length
	 * of the first mbuf in the mbuf chain.
	 */

	M_PREPEND(m, M_DONTWAIT, HDHEADERLN);
	if (m == NULL)
		return;

	hd_append (&hdp->hd_txq, m);
	hd_start (hdp);
}

hd_start (hdp)
register struct hdcb *hdp;
{
	register struct mbuf *m;

	/* 
	 * The iframe is only transmitted if all these conditions are FALSE.
	 * The iframe remains queued (hdp->hd_txq) however and will be
	 * transmitted as soon as these conditions are cleared.
	 */

	while (!(hdp->hd_condition & (TIMER_RECOVERY_CONDITION | REMOTE_RNR_CONDITION | REJ_CONDITION))) {
		if (hdp->hd_vs == (hdp->hd_lastrxnr + hdp->hd_xcp->xc_lwsize) % MODULUS) {

			/* We have now exceeded the  maximum  number  of 
			   outstanding iframes. Therefore,  we must wait 
			   until  at least  one is acknowledged if this 
			   condition  is not  turned off before we are
			   requested to write another iframe. */
			hdp->hd_window_condition++;
			break;
		}

		/* hd_remove top iframe from transmit queue. */
		if ((m = hd_remove (&hdp->hd_txq)) == NULL)
			break;

		hd_send_iframe (hdp, m, POLLOFF);
	}
}

/* 
 *  This procedure is passed a buffer descriptor for an iframe. It builds
 *  the rest of the control part of the frame and then writes it out.  It
 *  also  starts the  acknowledgement  timer and keeps  the iframe in the
 *  Retransmit queue (Retxq) just in case  we have to do this again.
 *
 *  Note: This routine is also called from hd_input.c when retransmission
 *       of old frames is required.
 */

hd_send_iframe (hdp, buf, poll_bit)
register struct hdcb *hdp;
register struct mbuf *buf;
int poll_bit;
{
	register struct Hdlc_iframe *iframe;
	struct mbuf *m;

	KILL_TIMER (hdp);

	if (buf == 0) {
		printf ("hd_send_iframe: zero arg\n");
#ifdef HDLCDEBUG
		hd_status (hdp);
		hd_dumptrace (hdp);
#endif
		hdp->hd_vs = (hdp->hd_vs + 7) % MODULUS;
		return;
	}
	iframe = mtod (buf, struct Hdlc_iframe *);

	iframe -> hdlc_0 = 0;
	iframe -> nr = hdp->hd_vr;
	iframe -> pf = poll_bit;
	iframe -> ns = hdp->hd_vs;
	iframe -> address = ADDRESS_B;
	hdp->hd_lasttxnr = hdp->hd_vr;
	hdp->hd_rrtimer = 0;

	if (hdp->hd_vs == hdp->hd_retxqi) {
		/* Check for retransmissions. */
		/* Put iframe only once in the Retransmission queue. */
		hdp->hd_retxq[hdp->hd_retxqi] = buf;
		hdp->hd_retxqi = (hdp->hd_retxqi + 1) % MODULUS;
		hdp->hd_iframes_out++;
	}

	hdp->hd_vs = (hdp->hd_vs + 1) % MODULUS;

	hd_trace (hdp, TX, (struct Hdlc_frame *)iframe);

	/* Write buffer on device. */
	m = hdp->hd_dontcopy ? buf : m_copy(buf, 0, (int)M_COPYALL);
	if (m == 0) {
		printf("hdlc: out of mbufs\n");
		return;
	}
	(*hdp->hd_output)(hdp, m);
	SET_TIMER (hdp);
}

hd_ifoutput(hdp, m)
register struct hdcb *hdp;
register struct mbuf *m;
{
	/*
	 * Queue message on interface, and start output if interface
	 * not yet active.
	 */
	register struct ifnet *ifp = hdp->hd_ifp;
	int s = splimp();
	if (IF_QFULL(&ifp->if_snd)) {
		IF_DROP(&ifp->if_snd);
		printf("%s%d: HDLC says OK to send but queue full, may hang\n",
			ifp->if_name, ifp->if_unit);
		m_freem(m);
	} else {
		IF_ENQUEUE(&ifp->if_snd, m);
		if ((ifp->if_flags & IFF_OACTIVE) == 0)
			(*ifp->if_start)(ifp);
	}
	splx(s);
}


/* 
 *  This routine gets control when the timer expires because we have not
 *  received an acknowledgement for a iframe.
 */

hd_resend_iframe (hdp)
register struct hdcb *hdp;
{

	if (hdp->hd_retxcnt++ < hd_n2) {
		if (!(hdp->hd_condition & TIMER_RECOVERY_CONDITION)) {
			hdp->hd_xx = hdp->hd_vs;
			hdp->hd_condition |= TIMER_RECOVERY_CONDITION;
		}

		hdp->hd_vs = hdp->hd_lastrxnr;
		hd_send_iframe (hdp, hdp->hd_retxq[hdp->hd_vs], POLLON);
	} else {
		/* At this point we have not received a RR even after N2
		   retries - attempt to reset link. */

		hd_initvars (hdp);
		hd_writeinternal (hdp, SABM, POLLOFF);
		hdp->hd_state = WAIT_UA;
		SET_TIMER (hdp);
		hd_message (hdp, "Timer recovery failed: link down");
		(void) pk_ctlinput (PRC_LINKDOWN, hdp->hd_xcp);
	}
}
