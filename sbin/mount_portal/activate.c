/*
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 * All rights reserved.
 *
 * This code is derived from software donated to Berkeley by
 * Jan-Simon Pendry.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)activate.c	8.2 (Berkeley) 3/27/94
 *
 * $Id: activate.c,v 1.2 1992/05/27 07:09:27 jsp Exp jsp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/syslog.h>
#include <sys/uio.h>

#include "portald.h"

/*
 * Scan the providers list and call the
 * appropriate function.
 */
static int activate_argv(pcr, key, v, so, fdp)
struct portal_cred *pcr;
char *key;
char **v;
int so;
int *fdp;
{
	provider *pr;

	for (pr = providers; pr->pr_match; pr++)
		if (strcmp(v[0], pr->pr_match) == 0)
			return ((*pr->pr_func)(pcr, key, v, so, fdp));

	return (ENOENT);
}

static int get_request(so, pcr, key, klen)
int so;
struct portal_cred *pcr;
char *key;
int klen;
{
	struct iovec iov[2];
	struct msghdr msg;
	int n;

	iov[0].iov_base = (caddr_t) pcr;
	iov[0].iov_len = sizeof(*pcr);
	iov[1].iov_base = key;
	iov[1].iov_len = klen;

	bzero((char *) &msg, sizeof(msg));
	msg.msg_iov = iov;
	msg.msg_iovlen = 2;

	n = recvmsg(so, &msg, 0);
	if (n < 0)
		return (errno);

	if (n <= sizeof(*pcr))
		return (EINVAL);

	n -= sizeof(*pcr);
	key[n] = '\0';

	return (0);
}

static void send_reply(so, fd, error)
int so;
int fd;
int error;
{
	int n;
	struct iovec iov;
	struct msghdr msg;
	struct {
		struct cmsghdr cmsg;
		int fd;
	} ctl;

	/*
	 * Line up error code.  Don't worry about byte ordering
	 * because we must be sending to the local machine.
	 */
	iov.iov_base = (caddr_t) &error;
	iov.iov_len = sizeof(error);

	/*
	 * Build a msghdr
	 */
	bzero((char *) &msg, sizeof(msg));
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	/*
	 * If there is a file descriptor to send then
	 * construct a suitable rights control message.
	 */
	if (fd >= 0) {
		ctl.fd = fd;
		ctl.cmsg.cmsg_len = sizeof(ctl);
		ctl.cmsg.cmsg_level = SOL_SOCKET;
		ctl.cmsg.cmsg_type = SCM_RIGHTS;
		msg.msg_control = (caddr_t) &ctl;
		msg.msg_controllen = ctl.cmsg.cmsg_len;
	}

	/*
	 * Send to kernel...
	 */
	if ((n = sendmsg(so, &msg, MSG_EOR)) < 0)
		syslog(LOG_ERR, "send: %s", strerror(errno));
#ifdef DEBUG
	fprintf(stderr, "sent %d bytes\n", n);
#endif
	sleep(1);	/*XXX*/
#ifdef notdef
	if (shutdown(so, 2) < 0)
		syslog(LOG_ERR, "shutdown: %s", strerror(errno));
#endif
	/*
	 * Throw away the open file descriptor
	 */
	(void) close(fd);
}

void activate(q, so)
qelem *q;
int so;
{
	struct portal_cred pcred;
	char key[MAXPATHLEN+1];
	int error;
	char **v;
	int fd = -1;

	/*
	 * Read the key from the socket
	 */
	error = get_request(so, &pcred, key, sizeof(key));
	if (error) {
		syslog(LOG_ERR, "activate: recvmsg: %s", strerror(error));
		goto drop;
	}

#ifdef DEBUG
	fprintf(stderr, "lookup key %s\n", key);
#endif

	/*
	 * Find a match in the configuration file
	 */
	v = conf_match(q, key);

	/*
	 * If a match existed, then find an appropriate portal
	 * otherwise simply return ENOENT.
	 */
	if (v) {
		error = activate_argv(&pcred, key, v, so, &fd);
		if (error)
			fd = -1;
		else if (fd < 0)
			error = -1;
	} else {
		error = ENOENT;
	}

	if (error >= 0)
		send_reply(so, fd, error);

drop:;
	close(so);
}
