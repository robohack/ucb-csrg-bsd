#ifndef lint
static char sccsid[] = "@(#)rvmacs.c	4.1 (Berkeley) 1/22/85";
#endif

#include "../condevs.h"
#ifdef	RVMACS

/*
 * Racal-Vadic 'RV820' MACS system with 831 adaptor.
 * A typical 300 baud L-devices entry is
 *	ACU tty10 tty11,48 300 rvmacs
 * where tty10 is the communication line (D_Line),
 * tty11 is the dialer line (D_calldev),
 * the '4' is the dialer address + modem type (viz. dialer 0, Bell 103),
 * the '8' is the communication port,
 * We assume the dialer speed is 1200 baud.
 */

#define	STX	02	/* Access Adaptor */
#define	ETX	03	/* Transfer to Dialer */
#define	SI	017	/* Buffer Empty (end of phone number) */
#define	ABORT	01	/* Abort */

#define	pc(fd, x)	(c = x, write(fd, &c, 1))

rvmacsopn(ph, flds, dev)
char *ph, *flds[];
struct Devices *dev;
{
	register int va, i, child;
	register char *p;
	char *q;
	char c, acu[20], com[20];
	int baudrate;
	int timelim;
	int pid, status;
	int zero = 0;
	struct sgttyb sg;

	child = -1;
	sprintf(com, "/dev/%s", dev->D_line);
	sprintf(acu, "/dev/%s", dev->D_calldev);
	if ((p = index(acu, ',')) == NULL) {
		DEBUG(2, "No dialer/modem specification\n", 0);
		return CF_DIAL;
	}
	*p++ = '\0';
	if (setjmp(Sjbuf)) {
		logent("rvmacsopn", "TIMEOUT");
		goto failret;
	}
	DEBUG(4, "STARTING CALL\n", 0);
	getnextfd();
	signal(SIGALRM, alarmtr);
	timelim = 5 * strlen(ph);
	alarm(timelim < 45 ? 45 : timelim);

	if ((va = open(acu, 2)) < 0) {
		logent(acu, "CAN'T OPEN");
		alarm(0);
		return CF_DIAL;
	}

	/* rti!trt: avoid passing acu file descriptor to children */
	next_fd = -1;
	fioclex(va);

	if ((child = fork()) == 0) {
		/* create child to do dialing */
		sleep(2);
		fclose(stdin);
		fclose(stdout);
		sg.sg_flags = RAW|ANYP;
		sg.sg_ispeed = sg.sg_ospeed = B1200;
		ioctl(va, TIOCSETP, &sg);
		pc(va, ABORT);
		sleep(1);
		ioctl(va, TIOCFLUSH, &zero);
		pc(va, STX);	/* access adaptor */
		pc(va, *p++);	/* Send Dialer Address Digit */
		pc(va, *p);	/* Send Modem Address Digit */
		while (*ph && *ph != '<') {
			switch (*ph) {
			case '_':
			case '-':
			case '=':
				pc(va, '=');
				break;
			default:
				if (*ph >= '0' && *ph <= '9')
					pc(va, *ph);
				break;
			}
			ph++;
		}
		pc(va, '<');	/* Transfer Control to Modem (sigh) */
		pc(va, SI);	/* Send Buffer Empty */
		pc(va, ETX);	/* Initiate Call */
		sleep(1);

		if (read(va, &c, 1) != 1) {
			close(va);
			logent("ACU READ", _FAILED);
			exit(1);
		}
		if (c == 'B' || c == 'G') {
			char cc;
			pc(va, ABORT);
			read(va, &cc, 1);
		}
		DEBUG(4, "Dialer returned %c\n", c);
		close(va);
		exit(c != 'A');
	}
	/*
	 * open line - will return on carrier
	 */
	if ((i = open(com, 2)) < 0) {
		if (errno == EIO)
			logent("carrier", "LOST");
		else
			logent("dialup open", _FAILED);
		goto failret;
	}
	while ((pid = wait(&status)) != child && pid != -1)
		;
	alarm(0);
	if (status) {
		close(i);
		close(va);		/* XXX */
		return CF_DIAL;
	}
	fixline(i, dev->D_speed);
	return i;

failret:
	alarm(0);
	close(va);
	if (child != -1)
		kill(child, SIGKILL);
	return CF_DIAL;
}

rvmacscls(fd)
register int fd;
{
	if (fd > 0) {
		ioctl(fd, TIOCCDTR, STBNULL);
		sleep(1);
		ioctl(fd, TIOCNXCL, STBNULL);
		close(fd);
		delock(devSel);
	}
}
#endif
