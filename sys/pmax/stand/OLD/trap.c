/*
 * Copyright (c) 1988 University of Utah.
 * Copyright (c) 1992 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * the Systems Programming Group of the University of Utah Computer
 * Science Department and Ralph Campbell.
 *
 * %sccs.include.redist.c%
 *
 * from: Utah $Hdr: trap.c 1.32 91/04/06$
 *
 *	@(#)trap.c	7.1 (Berkeley) 11/15/92
 */

#include <sys/param.h>

#include <pmax/include/machConst.h>
#include <pmax/include/trap.h>
#include <pmax/pmax/clockreg.h>
#include <pmax/dev/device.h>
#include <pmax/stand/samachdep.h>

extern void MachKernGenException();
extern void MachKernIntr();

void (*machExceptionTable[])() = {
	MachKernIntr,			/* external interrupt */
	MachKernGenException,		/* TLB modification */
	MachKernGenException,		/* TLB miss (load or instr. fetch) */
	MachKernGenException,		/* TLB miss (store) */
	MachKernGenException,		/* address error (load or I-fetch) */
	MachKernGenException,		/* address error (store) */
	MachKernGenException,		/* bus error (I-fetch) */
	MachKernGenException,		/* bus error (load or store) */
	MachKernGenException,		/* system call */
	MachKernGenException,		/* breakpoint */
	MachKernGenException,		/* reserved instruction */
	MachKernGenException,		/* coprocessor unusable */
	MachKernGenException,		/* arithmetic overflow */
	MachKernGenException,		/* reserved */
	MachKernGenException,		/* reserved */
	MachKernGenException,		/* reserved */
};

#ifdef DEBUG
char	*trap_type[] = {
	"external interrupt",
	"TLB modification",
	"TLB miss (load or instr. fetch)",
	"TLB miss (store)",
	"address error (load or I-fetch)",
	"address error (store)",
	"bus error (I-fetch)",
	"bus error (load or store)",
	"system call",
	"breakpoint",
	"reserved instruction",
	"coprocessor unusable",
	"arithmetic overflow",
	"reserved 13",
	"reserved 14",
	"reserved 15",
};

#define TRAPSIZE	10
struct trapdebug {		/* trap history buffer for debugging */
	u_int	status;
	u_int	cause;
	u_int	vadr;
	u_int	pc;
	u_int	ra;
	u_int	code;
} trapdebug[TRAPSIZE], *trp = trapdebug;
#endif

int	*onfault;

/*
 * Handle an exception.
 * Called from MachKernGenException() or MachUserGenException()
 * when a processor trap occurs.
 * In the case of a kernel trap, we return the pc where to resume if
 * onfault is set, otherwise, return old pc.
 */
int *
trap(statusReg, causeReg, vadr, pc, args)
	unsigned statusReg;	/* status register at time of the exception */
	unsigned causeReg;	/* cause register at time of exception */
	unsigned vadr;		/* address (if any) the fault occured on */
	unsigned pc;		/* program counter where to continue */
{
	register int type, i;
	int *f;

#ifdef DEBUG
	trp->status = statusReg;
	trp->cause = causeReg;
	trp->vadr = vadr;
	trp->pc = pc;
	trp->ra = ((int *)&args)[19];
	trp->code = 0;
	if (++trp == &trapdebug[TRAPSIZE])
		trp = trapdebug;
#endif

	type = (causeReg & MACH_CR_EXC_CODE) >> MACH_CR_EXC_CODE_SHIFT;

	/*
	 * Enable hardware interrupts if they were on before.
	 * We only respond to software interrupts when returning to user mode.
	 */
	if (statusReg & MACH_SR_INT_ENA_PREV)
		splx((statusReg & MACH_HARD_INT_MASK) | MACH_SR_INT_ENA_CUR);

	switch (type) {
	case T_BUS_ERR_LD_ST:	/* BERR asserted to cpu */
		if (f = onfault) {
			onfault = (int *)0;
			return (f);
		}
		/* FALLTHROUGH */

	default:
	err:
#ifdef DEBUG
		trapDump("trap");
#else
		printf("trap: PC %x ADR %x CR %x SR %x\n", pc, vadr,
			causeReg, statusReg);
#endif
		panic("trap");
	}
	/* NOTREACHED */
}

#ifdef DS5000
struct	intr_tab intr_tab[8];
#endif

int	temp;		/* XXX ULTRIX compiler bug with -O */

/*
 * Handle an interrupt.
 * Called from MachKernIntr()
 */
interrupt(statusReg, causeReg, pc)
	unsigned statusReg;	/* status register at time of the exception */
	unsigned causeReg;	/* cause register at time of exception */
	unsigned pc;		/* program counter where to continue */
{
	register unsigned mask;

#ifdef DEBUG
	trp->status = statusReg;
	trp->cause = causeReg;
	trp->vadr = 0;
	trp->pc = pc;
	trp->ra = 0;
	trp->code = 0;
	if (++trp == &trapdebug[TRAPSIZE])
		trp = trapdebug;
#endif

	mask = causeReg & statusReg;	/* pending interrupts & enable mask */
#ifdef DS3100
	/* handle clock interrupts ASAP */
	if (mask & MACH_INT_MASK_3) {
		register volatile struct chiptime *c =
			(volatile struct chiptime *)MACH_CLOCK_ADDR;

		temp = c->regc;	/* XXX clear interrupt bits */
		causeReg &= ~MACH_INT_MASK_3;	/* reenable clock interrupts */
	}
	/*
	 * Enable hardware interrupts which were enabled but not pending.
	 * We only respond to software interrupts when returning to spl0.
	 */
	splx((statusReg & ~causeReg & MACH_HARD_INT_MASK) |
		MACH_SR_INT_ENA_CUR);
	if (mask & MACH_INT_MASK_0)
		siiintr(0);
	if (mask & MACH_INT_MASK_1)
		leintr(0);
	if (mask & MACH_INT_MASK_2)
		dcintr(0);
	if (mask & MACH_INT_MASK_4) {
		volatile u_short *sysCSRPtr = (u_short *)MACH_SYS_CSR_ADDR;
		u_short csr;

		csr = *sysCSRPtr;

		if (csr & MACH_CSR_MEM_ERR) {
			printf("Memory error at 0x%x\n",
				*(unsigned *)MACH_WRITE_ERROR_ADDR);
			panic("Mem error interrupt");
		}
		*sysCSRPtr = (csr & ~MACH_CSR_MBZ) | 0xff;
	}
#endif /* DS3100 */
#ifdef DS5000
	/* handle clock interrupts ASAP */
	if (mask & MACH_INT_MASK_1) {
		register volatile struct chiptime *c =
			(volatile struct chiptime *)MACH_CLOCK_ADDR;

		temp = c->regc;	/* XXX clear interrupt bits */
		causeReg &= ~MACH_INT_MASK_1;	/* reenable clock interrupts */
	}
	if (mask & MACH_INT_MASK_0) {
		register unsigned csr;
		register unsigned i, m;

		csr = *(unsigned *)MACH_SYS_CSR_ADDR;
		m = csr & (csr >> MACH_CSR_IOINTEN_SHIFT) & MACH_CSR_IOINT_MASK;
#if 0
		*(unsigned *)MACH_SYS_CSR_ADDR =
			(csr & ~(MACH_CSR_MBZ | 0xFF)) |
			(m << MACH_CSR_IOINTEN_SHIFT);
#endif
		/*
		 * Enable hardware interrupts which were enabled but not
		 * pending. We only respond to software interrupts when
		 * returning to spl0.
		 */
		splx((statusReg & ~causeReg & MACH_HARD_INT_MASK) |
			MACH_SR_INT_ENA_CUR);
		for (i = 0; m; i++, m >>= 1) {
			if (!(m & 1))
				continue;
			if (intr_tab[i].func)
				(*intr_tab[i].func)(intr_tab[i].unit);
			else
				printf("spurious interrupt %d\n", i);
		}
#if 0
		*(unsigned *)MACH_SYS_CSR_ADDR =
			csr & ~(MACH_CSR_MBZ | 0xFF);
#endif
	} else {
		/*
		 * Enable hardware interrupts which were enabled but not
		 * pending. We only respond to software interrupts when
		 * returning to spl0.
		 */
		splx((statusReg & ~causeReg & MACH_HARD_INT_MASK) |
			MACH_SR_INT_ENA_CUR);
	}
	if (mask & MACH_INT_MASK_3) {
		printf("erradr %x\n", *(unsigned *)MACH_ERROR_ADDR);
		*(unsigned *)MACH_ERROR_ADDR = 0;
		MachEmptyWriteBuffer();
	}
#endif /* DS5000 */
	if (mask & MACH_INT_MASK_5) {
#ifdef DEBUG
		trapDump("fpintr");
#else
		printf("FPU interrupt: PC %x CR %x SR %x\n",
			pc, causeReg, statusReg);
#endif
	}
}

#ifdef DEBUG
trapDump(msg)
	char *msg;
{
	register int i;
	int s;

	s = splhigh();
	printf("trapDump(%s)\n", msg);
	for (i = 0; i < TRAPSIZE; i++) {
		if (trp == trapdebug)
			trp = &trapdebug[TRAPSIZE - 1];
		else
			trp--;
		if (trp->cause == 0)
			break;
		printf("%s: ADR %x PC %x CR %x SR %x\n",
			trap_type[(trp->cause & MACH_CR_EXC_CODE) >>
				MACH_CR_EXC_CODE_SHIFT],
			trp->vadr, trp->pc, trp->cause, trp->status);
		printf("   RA %x code %d\n", trp-> ra, trp->code);
	}
	bzero(trapdebug, sizeof(trapdebug));
	trp = trapdebug;
	splx(s);
}
#endif
