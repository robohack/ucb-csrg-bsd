/*
 * Copyright (c) 1990 Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)kgdb_stub.c	7.6 (Berkeley) 3/12/91
 */
/*
 * "Stub" to allow remote cpu to debug over a serial line using gdb.
 */
#ifdef KGDB
#ifndef lint
static char rcsid[] = "$Header: kgdb_stub.c,v 1.9 91/03/08 07:03:13 van Locked $";
#endif

#include "param.h"
#include "systm.h"
#include "machine/trap.h"
#include "machine/cpu.h"
#include "machine/psl.h"
#include "machine/reg.h"
#include "frame.h"
#include "buf.h"
#include "cons.h"

#include "kgdb_proto.h"
#include "machine/remote-sl.h"

extern int kernacc();
extern void chgkprot();

#ifndef KGDBDEV
#define KGDBDEV -1
#endif
#ifndef KGDBRATE
#define KGDBRATE 9600
#endif

int kgdb_dev = KGDBDEV;		/* remote debugging device (-1 if none) */
int kgdb_rate = KGDBRATE;	/* remote debugging baud rate */
int kgdb_active = 0;            /* remote debugging active if != 0 */
int kgdb_debug_init = 0;	/* != 0 waits for remote at system init */
int kgdb_debug = 0;

#define GETC	((*kgdb_getc)(kgdb_dev))
#define PUTC(c)	((*kgdb_putc)(kgdb_dev, c))
#define PUTESC(c) { \
	if (c == FRAME_END) { \
		PUTC(FRAME_ESCAPE); \
		c = TRANS_FRAME_END; \
	} else if (c == FRAME_ESCAPE) { \
		PUTC(FRAME_ESCAPE); \
		c = TRANS_FRAME_ESCAPE; \
	} \
	PUTC(c); \
}

static int (*kgdb_getc)();
static int (*kgdb_putc)();

/*
 * Send a message.  The host gets one chance to read it.
 */
static void
kgdb_send(type, bp, len)
	register u_char type;
	register u_char *bp;
	register int len;
{
	register u_char csum;
	register u_char *ep = bp + len;

	csum = type;
	PUTESC(type)

	while (bp < ep) {
		type = *bp++;
		csum += type;
		PUTESC(type)
	}
	csum = -csum;
	PUTESC(csum)
	PUTC(FRAME_END);
}

static int
kgdb_recv(bp, lenp)
	u_char *bp;
	int *lenp;
{
	register u_char c, csum;
	register int escape, len;
	register int type;

	csum = len = escape = 0;
	type = -1;
	while (1) {
		c = GETC;
		switch (c) {

		case FRAME_ESCAPE:
			escape = 1;
			continue;

		case TRANS_FRAME_ESCAPE:
			if (escape)
				c = FRAME_ESCAPE;
			break;

		case TRANS_FRAME_END:
			if (escape)
				c = FRAME_END;
			break;

		case FRAME_END:
			if (type < 0 || --len < 0) {
				csum = len = escape = 0;
				type = -1;
				continue;
			}
			if (csum != 0) {
				return (0);
			}
			*lenp = len;
			return type;
		}
		csum += c;
		if (type < 0) {
			type = c;
			escape = 0;
			continue;
		}
		if (++len > SL_MAXMSG) {
			while (GETC != FRAME_END)
				;
			return (0);
		}
		*bp++ = c;
		escape = 0;
	}
}

/*
 * Translate a trap number into a unix compatible signal value.
 * (gdb only understands unix signal numbers).
 */
static int 
computeSignal(type)
	int type;
{
	int sigval;

	switch (type) {
	case T_BUSERR:
		sigval = SIGBUS;
		break;
	case T_ADDRERR:
		sigval = SIGBUS;
		break;
	case T_ILLINST:
		sigval = SIGILL;
		break;
	case T_ZERODIV:
		sigval = SIGFPE;
		break;
	case T_CHKINST:
		sigval = SIGFPE;
		break;
	case T_TRAPVINST:
		sigval = SIGFPE;
		break;
	case T_PRIVINST:
		sigval = SIGILL;
		break;
	case T_TRACE:
		sigval = SIGTRAP;
		break;
	case T_MMUFLT:
		sigval = SIGSEGV;
		break;
	case T_SSIR:
		sigval = SIGSEGV;
		break;
	case T_FMTERR:
		sigval = SIGILL;
		break;
	case T_FPERR:
		sigval = SIGFPE;
		break;
	case T_COPERR:
		sigval = SIGFPE;
		break;
	case T_ASTFLT:
		sigval = SIGINT;
		break;
	case T_TRAP15:
		sigval = SIGTRAP;
		break;
	default:
		sigval = SIGEMT;
		break;
	}
	return (sigval);
}

/*
 * Definitions exported from gdb.
 */
#define NUM_REGS 18
#define REGISTER_BYTES ((16+2)*4)
#define REGISTER_BYTE(N)  ((N)*4)

#define GDB_SR 16
#define GDB_PC 17

static inline void
kgdb_copy(register u_char *src, register u_char *dst, register u_int nbytes)
{
	register u_char *ep = src + nbytes;

	while (src < ep)
		*dst++ = *src++;
}

#define regs_to_gdb(fp, regs) \
	(kgdb_copy((u_char *)((fp)->f_regs), (u_char *)(regs), REGISTER_BYTES))

#define gdb_to_regs(fp, regs) \
	(kgdb_copy((u_char *)(regs), (u_char *)((fp)->f_regs), REGISTER_BYTES))

static u_long reg_cache[NUM_REGS];
static u_char inbuffer[SL_MAXMSG+1];
static u_char outbuffer[SL_MAXMSG];

/*
 * This function does all command procesing for interfacing to 
 * a remote gdb.
 */
int 
kgdb_trap(int type, struct frame *frame)
{
	register u_long len;
	u_char *addr;
	register u_char *cp;
	register u_char out, in;
	register int outlen;
	int inlen;
	u_long gdb_regs[NUM_REGS];

	if (kgdb_dev < 0) {
		/* not debugging */
		return (0);
	}
	if (kgdb_active == 0) {
		if (type != T_TRAP15) {
			/* No debugger active -- let trap handle this. */
			return (0);
		}
		kgdb_getc = constab[major(kgdb_dev)].cn_getc;
		kgdb_putc = constab[major(kgdb_dev)].cn_putc;
		if (kgdb_getc == 0 || kgdb_putc == 0)
			return (0);
		/*
		 * If the packet that woke us up isn't a signal packet,
		 * ignore it since there is no active debugger.  Also,
		 * we check that it's not an ack to be sure that the 
		 * remote side doesn't send back a response after the
		 * local gdb has exited.  Otherwise, the local host
		 * could trap into gdb if it's running a gdb kernel too.
		 */
#ifdef notdef
		in = GETC;
		if (KGDB_CMD(in) != KGDB_SIGNAL || (in & KGDB_ACK) != 0)
			return (0);
#endif
		while (GETC != FRAME_END)
			;

		kgdb_active = 1;
	}
	/*
	 * Stick frame regs into our reg cache then tell remote host
	 * that an exception has occured.
	 */
	regs_to_gdb(frame, gdb_regs);
	outbuffer[0] = computeSignal(type);
	kgdb_send(KGDB_SIGNAL, outbuffer, 1);

	while (1) {
		in = kgdb_recv(inbuffer, &inlen);
		if (in == 0 || (in & KGDB_ACK))
			/* Ignore inbound acks and error conditions. */
			continue;

		out = in | KGDB_ACK;
		switch (KGDB_CMD(in)) {

		case KGDB_SIGNAL:
			/*
			 * if this command came from a running gdb,
			 * answer it -- the other guy has no way of
			 * knowing if we're in or out of this loop
			 * when he issues a "remote-signal".  (Note
			 * that without the length check, we could
			 * loop here forever if the ourput line is
			 * looped back or the remote host is echoing.)
			 */
			if (inlen == 0) {
				outbuffer[0] = computeSignal(type);
				kgdb_send(KGDB_SIGNAL, outbuffer, 1);
			}
			continue;

		case KGDB_REG_R:
		case KGDB_REG_R | KGDB_DELTA:
			cp = outbuffer;
			outlen = 0;
			for (len = inbuffer[0]; len < NUM_REGS; ++len) {
				if (reg_cache[len] != gdb_regs[len] ||
				    (in & KGDB_DELTA) == 0) {
					if (outlen + 5 > SL_MAXMSG) {
						out |= KGDB_MORE;
						break;
					}
					cp[outlen] = len;
					kgdb_copy((u_char *)&gdb_regs[len],
						  &cp[outlen + 1], 4);
					reg_cache[len] = gdb_regs[len];
					outlen += 5;
				}
			}
			break;
			
		case KGDB_REG_W:
		case KGDB_REG_W | KGDB_DELTA:
			cp = inbuffer;
			for (len = 0; len < inlen; len += 5) {
				register int j = cp[len];

				kgdb_copy(&cp[len + 1],
					  (u_char *)&gdb_regs[j], 4);
				reg_cache[j] = gdb_regs[j];
			}
			gdb_to_regs(frame, gdb_regs);
			outlen = 0;
			break;
				
		case KGDB_MEM_R:
			len = inbuffer[0];
			kgdb_copy(&inbuffer[1], (u_char *)&addr, 4);
			if (len + 1 > SL_MAXMSG) {
				outlen = 1;
				outbuffer[0] = E2BIG;
			} else if (!kgdb_acc(addr, len, B_READ)) {
				outlen = 1;
				outbuffer[0] = EFAULT;
			} else {
				outlen = len + 1;
				outbuffer[0] = 0;
				kgdb_copy(addr, &outbuffer[1], len);
			}
			break;

		case KGDB_MEM_W:
			len = inlen - 4;
			kgdb_copy(inbuffer, (u_char *)&addr, 4);
			outlen = 1;
			if (!kgdb_acc(addr, len, B_READ))
				outbuffer[0] = EFAULT;
			else {
				outbuffer[0] = 0;
				if (!kgdb_acc(addr, len, B_WRITE))
					chgkprot(addr, len, B_WRITE);
				kgdb_copy(&inbuffer[4], addr, len);
			}
			break;

		case KGDB_KILL:
			kgdb_active = 0;
			/* fall through */
		case KGDB_CONT:
			kgdb_send(out, 0, 0);
			frame->f_sr &=~ PSL_T;
			return (1);

		case KGDB_STEP:
			kgdb_send(out, 0, 0);
			frame->f_sr |= PSL_T;
			return (1);

		default:
			/* Unknown command.  Ack with a null message. */
			outlen = 0;
			break;
		}
		/* Send the reply */
		kgdb_send(out, outbuffer, outlen);
	}
}

/*
 * XXX do kernacc call if safe, otherwise attempt
 * to simulate by simple bounds-checking.
 */
kgdb_acc(addr, len, rw)
	caddr_t addr;
{
	extern char proc0paddr[], u[];		/* XXX! */
	extern char *kernel_map;		/* XXX! */

	if (kernel_map != NULL)
		return (kernacc(addr, len, rw));
	if (addr < proc0paddr + UPAGES * NBPG  ||
	    u <= addr && addr < u + UPAGES * NBPG)
		return (1);
	return (0);
}
#endif
