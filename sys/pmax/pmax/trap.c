/*
 * Copyright (c) 1988 University of Utah.
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * the Systems Programming Group of the University of Utah Computer
 * Science Department and Ralph Campbell.
 *
 * %sccs.include.redist.c%
 *
 * from: Utah $Hdr: trap.c 1.32 91/04/06$
 *
 *	@(#)trap.c	8.7 (Berkeley) 6/2/95
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/kernel.h>
#include <sys/signalvar.h>
#include <sys/syscall.h>
#include <sys/user.h>
#include <sys/buf.h>
#ifdef KTRACE
#include <sys/ktrace.h>
#endif
#include <net/netisr.h>

#include <machine/trap.h>
#include <machine/psl.h>
#include <machine/reg.h>
#include <machine/cpu.h>
#include <machine/pte.h>
#include <machine/mips_opcode.h>

#include <vm/vm.h>
#include <vm/vm_kern.h>
#include <vm/vm_page.h>

#include <pmax/pmax/clockreg.h>
#include <pmax/pmax/kn01.h>
#include <pmax/pmax/kn02.h>
#include <pmax/pmax/kmin.h>
#include <pmax/pmax/maxine.h>
#include <pmax/pmax/kn03.h>
#include <pmax/pmax/asic.h>
#include <pmax/pmax/turbochannel.h>

#include <pmax/stand/dec_prom.h>

#include <asc.h>
#include <sii.h>
#include <le.h>
#include <dc.h>

struct	proc *machFPCurProcPtr;		/* pointer to last proc to use FP */

extern void MachKernGenException();
extern void MachUserGenException();
extern void MachKernIntr();
extern void MachUserIntr();
extern void MachTLBModException();
extern void MachTLBMissException();
extern unsigned MachEmulateBranch();

void (*machExceptionTable[])() = {
/*
 * The kernel exception handlers.
 */
	MachKernIntr,			/* external interrupt */
	MachKernGenException,		/* TLB modification */
	MachTLBMissException,		/* TLB miss (load or instr. fetch) */
	MachTLBMissException,		/* TLB miss (store) */
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
/*
 * The user exception handlers.
 */
	MachUserIntr,
	MachUserGenException,
	MachUserGenException,
	MachUserGenException,
	MachUserGenException,
	MachUserGenException,
	MachUserGenException,
	MachUserGenException,
	MachUserGenException,
	MachUserGenException,
	MachUserGenException,
	MachUserGenException,
	MachUserGenException,
	MachUserGenException,
	MachUserGenException,
	MachUserGenException,
};

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

#ifdef DEBUG
#define TRAPSIZE	10
struct trapdebug {		/* trap history buffer for debugging */
	u_int	status;
	u_int	cause;
	u_int	vadr;
	u_int	pc;
	u_int	ra;
	u_int	code;
} trapdebug[TRAPSIZE], *trp = trapdebug;

u_int	intr_level;		/* number of nested interrupts */
#endif

static void pmax_errintr();
static void kn02_errintr(), kn02ba_errintr();
#ifdef DS5000_240
static void kn03_errintr();
#endif
static unsigned kn02ba_recover_erradr();
extern tc_option_t tc_slot_info[TC_MAX_LOGICAL_SLOTS];
extern u_long kmin_tc3_imask, xine_tc3_imask;
extern const struct callback *callv;
#ifdef DS5000_240
extern u_long kn03_tc3_imask;
#endif
int (*pmax_hardware_intr)() = (int (*)())0;
extern volatile struct chiptime *Mach_clock_addr;
extern long intrcnt[];

/*
 * Handle an exception.
 * Called from MachKernGenException() or MachUserGenException()
 * when a processor trap occurs.
 * In the case of a kernel trap, we return the pc where to resume if
 * ((struct pcb *)UADDR)->pcb_onfault is set, otherwise, return old pc.
 */
unsigned
trap(statusReg, causeReg, vadr, pc, args)
	unsigned statusReg;	/* status register at time of the exception */
	unsigned causeReg;	/* cause register at time of exception */
	unsigned vadr;		/* address (if any) the fault occured on */
	unsigned pc;		/* program counter where to continue */
{
	register int type, i;
	unsigned ucode = 0;
	register struct proc *p = curproc;
	u_quad_t sticks;
	vm_prot_t ftype;
	extern unsigned onfault_table[];

#ifdef DEBUG
	trp->status = statusReg;
	trp->cause = causeReg;
	trp->vadr = vadr;
	trp->pc = pc;
	trp->ra = !USERMODE(statusReg) ? ((int *)&args)[19] :
		p->p_md.md_regs[RA];
	trp->code = 0;
	if (++trp == &trapdebug[TRAPSIZE])
		trp = trapdebug;
#endif

	cnt.v_trap++;
	type = (causeReg & MACH_CR_EXC_CODE) >> MACH_CR_EXC_CODE_SHIFT;
	if (USERMODE(statusReg)) {
		type |= T_USER;
		sticks = p->p_sticks;
	}

	/*
	 * Enable hardware interrupts if they were on before.
	 * We only respond to software interrupts when returning to user mode.
	 */
	if (statusReg & MACH_SR_INT_ENA_PREV)
		splx((statusReg & MACH_HARD_INT_MASK) | MACH_SR_INT_ENA_CUR);

	switch (type) {
	case T_TLB_MOD:
		/* check for kernel address */
		if ((int)vadr < 0) {
			register pt_entry_t *pte;
			register unsigned entry;
			register vm_offset_t pa;

			pte = kvtopte(vadr);
			entry = pte->pt_entry;
#ifdef DIAGNOSTIC
			if (!(entry & PG_V) || (entry & PG_M))
				panic("trap: ktlbmod: invalid pte");
#endif
			if (entry & PG_RO) {
				/* write to read only page in the kernel */
				ftype = VM_PROT_WRITE;
				goto kernel_fault;
			}
			entry |= PG_M;
			pte->pt_entry = entry;
			vadr &= ~PGOFSET;
			MachTLBUpdate(vadr, entry);
			pa = entry & PG_FRAME;
#ifdef ATTR
			pmap_attributes[atop(pa)] |= PMAP_ATTR_MOD;
#else
			if (!IS_VM_PHYSADDR(pa))
				panic("trap: ktlbmod: unmanaged page");
			PHYS_TO_VM_PAGE(pa)->flags &= ~PG_CLEAN;
#endif
			return (pc);
		}
		/* FALLTHROUGH */

	case T_TLB_MOD+T_USER:
	    {
		register pt_entry_t *pte;
		register unsigned entry;
		register vm_offset_t pa;
		pmap_t pmap = &p->p_vmspace->vm_pmap;

		if (!(pte = pmap_segmap(pmap, vadr)))
			panic("trap: utlbmod: invalid segmap");
		pte += (vadr >> PGSHIFT) & (NPTEPG - 1);
		entry = pte->pt_entry;
#ifdef DIAGNOSTIC
		if (!(entry & PG_V) || (entry & PG_M))
			panic("trap: utlbmod: invalid pte");
#endif
		if (entry & PG_RO) {
			/* write to read only page */
			ftype = VM_PROT_WRITE;
			goto dofault;
		}
		entry |= PG_M;
		pte->pt_entry = entry;
		vadr = (vadr & ~PGOFSET) |
			(pmap->pm_tlbpid << VMMACH_TLB_PID_SHIFT);
		MachTLBUpdate(vadr, entry);
		pa = entry & PG_FRAME;
#ifdef ATTR
		pmap_attributes[atop(pa)] |= PMAP_ATTR_MOD;
#else
		if (!IS_VM_PHYSADDR(pa))
			panic("trap: utlbmod: unmanaged page");
		PHYS_TO_VM_PAGE(pa)->flags &= ~PG_CLEAN;
#endif
		if (!USERMODE(statusReg))
			return (pc);
		goto out;
	    }

	case T_TLB_LD_MISS:
	case T_TLB_ST_MISS:
		ftype = (type == T_TLB_ST_MISS) ? VM_PROT_WRITE : VM_PROT_READ;
		/* check for kernel address */
		if ((int)vadr < 0) {
			register vm_offset_t va;
			int rv;

		kernel_fault:
			va = trunc_page((vm_offset_t)vadr);
			rv = vm_fault(kernel_map, va, ftype, FALSE);
			if (rv == KERN_SUCCESS)
				return (pc);
			if (i = ((struct pcb *)UADDR)->pcb_onfault) {
				((struct pcb *)UADDR)->pcb_onfault = 0;
				return (onfault_table[i]);
			}
			goto err;
		}
		/*
		 * It is an error for the kernel to access user space except
		 * through the copyin/copyout routines.
		 */
		if ((i = ((struct pcb *)UADDR)->pcb_onfault) == 0)
			goto err;
		/* check for fuswintr() or suswintr() getting a page fault */
		if (i == 4)
			return (onfault_table[i]);
		goto dofault;

	case T_TLB_LD_MISS+T_USER:
		ftype = VM_PROT_READ;
		goto dofault;

	case T_TLB_ST_MISS+T_USER:
		ftype = VM_PROT_WRITE;
	dofault:
	    {
		register vm_offset_t va;
		register struct vmspace *vm;
		register vm_map_t map;
		int rv;

		vm = p->p_vmspace;
		map = &vm->vm_map;
		va = trunc_page((vm_offset_t)vadr);
		rv = vm_fault(map, va, ftype, FALSE);
		/*
		 * If this was a stack access we keep track of the maximum
		 * accessed stack size.  Also, if vm_fault gets a protection
		 * failure it is due to accessing the stack region outside
		 * the current limit and we need to reflect that as an access
		 * error.
		 */
		if ((caddr_t)va >= vm->vm_maxsaddr) {
			if (rv == KERN_SUCCESS) {
				unsigned nss;

				nss = clrnd(btoc(USRSTACK-(unsigned)va));
				if (nss > vm->vm_ssize)
					vm->vm_ssize = nss;
			} else if (rv == KERN_PROTECTION_FAILURE)
				rv = KERN_INVALID_ADDRESS;
		}
		if (rv == KERN_SUCCESS) {
			if (!USERMODE(statusReg))
				return (pc);
			goto out;
		}
		if (!USERMODE(statusReg)) {
			if (i = ((struct pcb *)UADDR)->pcb_onfault) {
				((struct pcb *)UADDR)->pcb_onfault = 0;
				return (onfault_table[i]);
			}
			goto err;
		}
		ucode = vadr;
		i = (rv == KERN_PROTECTION_FAILURE) ? SIGBUS : SIGSEGV;
		break;
	    }

	case T_ADDR_ERR_LD+T_USER:	/* misaligned or kseg access */
	case T_ADDR_ERR_ST+T_USER:	/* misaligned or kseg access */
	case T_BUS_ERR_IFETCH+T_USER:	/* BERR asserted to cpu */
	case T_BUS_ERR_LD_ST+T_USER:	/* BERR asserted to cpu */
		i = SIGSEGV;
		break;

	case T_SYSCALL+T_USER:
	    {
		register int *locr0 = p->p_md.md_regs;
		register struct sysent *callp;
		unsigned int code;
		int numsys;
		struct args {
			int i[8];
		} args;
		int rval[2];
		struct sysent *systab;
		extern int nsysent;
#ifdef ULTRIXCOMPAT
		extern struct sysent ultrixsysent[];
		extern int ultrixnsysent;
#endif

		cnt.v_syscall++;
		/* compute next PC after syscall instruction */
		if ((int)causeReg < 0)
			locr0[PC] = MachEmulateBranch(locr0, pc, 0, 0);
		else
			locr0[PC] += 4;
		systab = sysent;
		numsys = nsysent;
#ifdef ULTRIXCOMPAT
		if (p->p_md.md_flags & MDP_ULTRIX) {
			systab = ultrixsysent;
			numsys = ultrixnsysent;
		}
#endif
		code = locr0[V0];
		switch (code) {
		case SYS_syscall:
			/*
			 * Code is first argument, followed by actual args.
			 */
			code = locr0[A0];
			if (code >= numsys)
				callp = &systab[SYS_syscall]; /* (illegal) */
			else
				callp = &systab[code];
			i = callp->sy_argsize;
			args.i[0] = locr0[A1];
			args.i[1] = locr0[A2];
			args.i[2] = locr0[A3];
			if (i > 3 * sizeof(register_t)) {
				i = copyin((caddr_t)(locr0[SP] +
						4 * sizeof(register_t)),
					(caddr_t)&args.i[3],
					(u_int)(i - 3 * sizeof(register_t)));
				if (i) {
					locr0[V0] = i;
					locr0[A3] = 1;
#ifdef KTRACE
					if (KTRPOINT(p, KTR_SYSCALL))
						ktrsyscall(p->p_tracep, code,
							callp->sy_argsize,
							args.i);
#endif
					goto done;
				}
			}
			break;

		case SYS___syscall:
			/*
			 * Like syscall, but code is a quad, so as to maintain
			 * quad alignment for the rest of the arguments.
			 */
			code = locr0[A0 + _QUAD_LOWWORD];
			if (code >= numsys)
				callp = &systab[SYS_syscall]; /* (illegal) */
			else
				callp = &systab[code];
			i = callp->sy_argsize;
			args.i[0] = locr0[A2];
			args.i[1] = locr0[A3];
			if (i > 2 * sizeof(register_t)) {
				i = copyin((caddr_t)(locr0[SP] +
						4 * sizeof(register_t)),
					(caddr_t)&args.i[2],
					(u_int)(i - 2 * sizeof(register_t)));
				if (i) {
					locr0[V0] = i;
					locr0[A3] = 1;
#ifdef KTRACE
					if (KTRPOINT(p, KTR_SYSCALL))
						ktrsyscall(p->p_tracep, code,
							callp->sy_argsize,
							args.i);
#endif
					goto done;
				}
			}
			break;

		default:
			if (code >= numsys)
				callp = &systab[SYS_syscall]; /* (illegal) */
			else
				callp = &systab[code];
			i = callp->sy_argsize;
			args.i[0] = locr0[A0];
			args.i[1] = locr0[A1];
			args.i[2] = locr0[A2];
			args.i[3] = locr0[A3];
			if (i > 4 * sizeof(register_t)) {
				i = copyin((caddr_t)(locr0[SP] +
						4 * sizeof(register_t)),
					(caddr_t)&args.i[4],
					(u_int)(i - 4 * sizeof(register_t)));
				if (i) {
					locr0[V0] = i;
					locr0[A3] = 1;
#ifdef KTRACE
					if (KTRPOINT(p, KTR_SYSCALL))
						ktrsyscall(p->p_tracep, code,
							callp->sy_argsize,
							args.i);
#endif
					goto done;
				}
			}
		}
#ifdef KTRACE
		if (KTRPOINT(p, KTR_SYSCALL))
			ktrsyscall(p->p_tracep, code, callp->sy_argsize, args.i);
#endif
		rval[0] = 0;
		rval[1] = locr0[V1];
#ifdef DEBUG
		if (trp == trapdebug)
			trapdebug[TRAPSIZE - 1].code = code;
		else
			trp[-1].code = code;
#endif
		i = (*callp->sy_call)(p, &args, rval);
		/*
		 * Reinitialize proc pointer `p' as it may be different
		 * if this is a child returning from fork syscall.
		 */
		p = curproc;
		locr0 = p->p_md.md_regs;
#ifdef DEBUG
		{ int s;
		s = splhigh();
		trp->status = statusReg;
		trp->cause = causeReg;
		trp->vadr = locr0[SP];
		trp->pc = locr0[PC];
		trp->ra = locr0[RA];
		trp->code = -code;
		if (++trp == &trapdebug[TRAPSIZE])
			trp = trapdebug;
		splx(s);
		}
#endif
		switch (i) {
		case 0:
			locr0[V0] = rval[0];
			locr0[V1] = rval[1];
			locr0[A3] = 0;
			break;

		case ERESTART:
			locr0[PC] = pc;
			break;

		case EJUSTRETURN:
			break;	/* nothing to do */

		default:
			locr0[V0] = i;
			locr0[A3] = 1;
		}
	done:
#ifdef KTRACE
		if (KTRPOINT(p, KTR_SYSRET))
			ktrsysret(p->p_tracep, code, i, rval[0]);
#endif
		goto out;
	    }

	case T_BREAK+T_USER:
	    {
		register unsigned va, instr;

		/* compute address of break instruction */
		va = pc;
		if ((int)causeReg < 0)
			va += 4;

		/* read break instruction */
		instr = fuiword((caddr_t)va);
#if 0
		printf("trap: %s (%d) breakpoint %x at %x: (adr %x ins %x)\n",
			p->p_comm, p->p_pid, instr, pc,
			p->p_md.md_ss_addr, p->p_md.md_ss_instr); /* XXX */
#endif
#ifdef KADB
		if (instr == MACH_BREAK_BRKPT || instr == MACH_BREAK_SSTEP)
			goto err;
#endif
		if (p->p_md.md_ss_addr != va || instr != MACH_BREAK_SSTEP) {
			i = SIGTRAP;
			break;
		}

		/* restore original instruction and clear BP  */
		i = suiword((caddr_t)va, p->p_md.md_ss_instr);
		if (i < 0) {
			vm_offset_t sa, ea;
			int rv;

			sa = trunc_page((vm_offset_t)va);
			ea = round_page((vm_offset_t)va+sizeof(int)-1);
			rv = vm_map_protect(&p->p_vmspace->vm_map, sa, ea,
				VM_PROT_DEFAULT, FALSE);
			if (rv == KERN_SUCCESS) {
				i = suiword((caddr_t)va, p->p_md.md_ss_instr);
				(void) vm_map_protect(&p->p_vmspace->vm_map,
					sa, ea, VM_PROT_READ|VM_PROT_EXECUTE,
					FALSE);
			}
		}
		if (i < 0)
			printf("Warning: can't restore instruction at %x: %x\n",
				p->p_md.md_ss_addr, p->p_md.md_ss_instr);
		p->p_md.md_ss_addr = 0;
		i = SIGTRAP;
		break;
	    }

	case T_RES_INST+T_USER:
		i = SIGILL;
		break;

	case T_COP_UNUSABLE+T_USER:
		if ((causeReg & MACH_CR_COP_ERR) != 0x10000000) {
			i = SIGILL;	/* only FPU instructions allowed */
			break;
		}
		MachSwitchFPState(machFPCurProcPtr, p->p_md.md_regs);
		machFPCurProcPtr = p;
		p->p_md.md_regs[PS] |= MACH_SR_COP_1_BIT;
		p->p_md.md_flags |= MDP_FPUSED;
		goto out;

	case T_OVFLOW+T_USER:
		i = SIGFPE;
		break;

	case T_ADDR_ERR_LD:	/* misaligned access */
	case T_ADDR_ERR_ST:	/* misaligned access */
	case T_BUS_ERR_LD_ST:	/* BERR asserted to cpu */
		if (i = ((struct pcb *)UADDR)->pcb_onfault) {
			((struct pcb *)UADDR)->pcb_onfault = 0;
			return (onfault_table[i]);
		}
		/* FALLTHROUGH */

	default:
	err:
#ifdef KADB
	    {
		extern struct pcb kdbpcb;

		if (USERMODE(statusReg))
			kdbpcb = p->p_addr->u_pcb;
		else {
			kdbpcb.pcb_regs[ZERO] = 0;
			kdbpcb.pcb_regs[AST] = ((int *)&args)[2];
			kdbpcb.pcb_regs[V0] = ((int *)&args)[3];
			kdbpcb.pcb_regs[V1] = ((int *)&args)[4];
			kdbpcb.pcb_regs[A0] = ((int *)&args)[5];
			kdbpcb.pcb_regs[A1] = ((int *)&args)[6];
			kdbpcb.pcb_regs[A2] = ((int *)&args)[7];
			kdbpcb.pcb_regs[A3] = ((int *)&args)[8];
			kdbpcb.pcb_regs[T0] = ((int *)&args)[9];
			kdbpcb.pcb_regs[T1] = ((int *)&args)[10];
			kdbpcb.pcb_regs[T2] = ((int *)&args)[11];
			kdbpcb.pcb_regs[T3] = ((int *)&args)[12];
			kdbpcb.pcb_regs[T4] = ((int *)&args)[13];
			kdbpcb.pcb_regs[T5] = ((int *)&args)[14];
			kdbpcb.pcb_regs[T6] = ((int *)&args)[15];
			kdbpcb.pcb_regs[T7] = ((int *)&args)[16];
			kdbpcb.pcb_regs[T8] = ((int *)&args)[17];
			kdbpcb.pcb_regs[T9] = ((int *)&args)[18];
			kdbpcb.pcb_regs[RA] = ((int *)&args)[19];
			kdbpcb.pcb_regs[MULLO] = ((int *)&args)[21];
			kdbpcb.pcb_regs[MULHI] = ((int *)&args)[22];
			kdbpcb.pcb_regs[PC] = pc;
			kdbpcb.pcb_regs[SR] = statusReg;
			bzero((caddr_t)&kdbpcb.pcb_regs[F0], 33 * sizeof(int));
		}
		if (kdb(causeReg, vadr, p, !USERMODE(statusReg)))
			return (kdbpcb.pcb_regs[PC]);
	    }
#else
#ifdef DEBUG
		trapDump("trap");
#endif
#endif
		panic("trap");
	}
	trapsignal(p, i, ucode);
out:
	/*
	 * Note: we should only get here if returning to user mode.
	 */
	/* take pending signals */
	while ((i = CURSIG(p)) != 0)
		postsig(i);
	p->p_priority = p->p_usrpri;
	astpending = 0;
	if (want_resched) {
		int s;

		/*
		 * Since we are curproc, clock will normally just change
		 * our priority without moving us from one queue to another
		 * (since the running process is not on a queue.)
		 * If that happened after we put ourselves on the run queue
		 * but before we switched, we might not be on the queue
		 * indicated by our priority.
		 */
		s = splstatclock();
		setrunqueue(p);
		p->p_stats->p_ru.ru_nivcsw++;
		mi_switch();
		splx(s);
		while ((i = CURSIG(p)) != 0)
			postsig(i);
	}

	/*
	 * If profiling, charge system time to the trapped pc.
	 */
	if (p->p_flag & P_PROFIL) {
		extern int psratio;

		addupc_task(p, pc, (int)(p->p_sticks - sticks) * psratio);
	}

	curpriority = p->p_priority;
	return (pc);
}

/*
 * Handle an interrupt.
 * Called from MachKernIntr() or MachUserIntr()
 * Note: curproc might be NULL.
 */
interrupt(statusReg, causeReg, pc)
	unsigned statusReg;	/* status register at time of the exception */
	unsigned causeReg;	/* cause register at time of exception */
	unsigned pc;		/* program counter where to continue */
{
	register unsigned mask;
	struct clockframe cf;

#ifdef DEBUG
	trp->status = statusReg;
	trp->cause = causeReg;
	trp->vadr = 0;
	trp->pc = pc;
	trp->ra = 0;
	trp->code = 0;
	if (++trp == &trapdebug[TRAPSIZE])
		trp = trapdebug;

	intr_level++;
#endif

	cnt.v_intr++;
	mask = causeReg & statusReg;	/* pending interrupts & enable mask */
	if (pmax_hardware_intr)
		splx((*pmax_hardware_intr)(mask, pc, statusReg, causeReg));
	if (mask & MACH_INT_MASK_5) {
		intrcnt[7]++;
		if (!USERMODE(statusReg)) {
#ifdef DEBUG
			trapDump("fpintr");
#else
			printf("FPU interrupt: PC %x CR %x SR %x\n",
				pc, causeReg, statusReg);
#endif
		} else
			MachFPInterrupt(statusReg, causeReg, pc);
	}
	if (mask & MACH_SOFT_INT_MASK_0) {
		clearsoftclock();
		cnt.v_soft++;
		intrcnt[0]++;
		softclock();
	}
	/* process network interrupt if we trapped or will very soon */
	if ((mask & MACH_SOFT_INT_MASK_1) ||
	    netisr && (statusReg & MACH_SOFT_INT_MASK_1)) {
		clearsoftnet();
		cnt.v_soft++;
		intrcnt[1]++;
#ifdef INET
		if (netisr & (1 << NETISR_ARP)) {
			netisr &= ~(1 << NETISR_ARP);
			arpintr();
		}
		if (netisr & (1 << NETISR_IP)) {
			netisr &= ~(1 << NETISR_IP);
			ipintr();
		}
#endif
#ifdef NS
		if (netisr & (1 << NETISR_NS)) {
			netisr &= ~(1 << NETISR_NS);
			nsintr();
		}
#endif
#ifdef ISO
		if (netisr & (1 << NETISR_ISO)) {
			netisr &= ~(1 << NETISR_ISO);
			clnlintr();
		}
#endif
	}
#ifdef DEBUG
	intr_level--;
#endif
}

/*
 * Handle pmax (DECstation 2100/3100) interrupts.
 */
pmax_intr(mask, pc, statusReg, causeReg)
	unsigned mask;
	unsigned pc;
	unsigned statusReg;
	unsigned causeReg;
{
	register volatile struct chiptime *c = Mach_clock_addr;
	struct clockframe cf;
	int temp;

	/* handle clock interrupts ASAP */
	if (mask & MACH_INT_MASK_3) {
		intrcnt[6]++;
		temp = c->regc;	/* XXX clear interrupt bits */
		cf.pc = pc;
		cf.sr = statusReg;
		hardclock(&cf);
		/* keep clock interrupts enabled */
		causeReg &= ~MACH_INT_MASK_3;
	}
	/* Re-enable clock interrupts */
	splx(MACH_INT_MASK_3 | MACH_SR_INT_ENA_CUR);
#if NSII > 0
	if (mask & MACH_INT_MASK_0) {
		intrcnt[2]++;
		siiintr(0);
	}
#endif
#if NLE > 0
	if (mask & MACH_INT_MASK_1) {
		intrcnt[3]++;
		leintr(0);
	}
#endif
#if NDC > 0
	if (mask & MACH_INT_MASK_2) {
		intrcnt[4]++;
		dcintr(0);
	}
#endif
	if (mask & MACH_INT_MASK_4) {
		intrcnt[5]++;
		pmax_errintr();
	}
	return ((statusReg & ~causeReg & MACH_HARD_INT_MASK) |
		MACH_SR_INT_ENA_CUR);
}

/*
 * Handle hardware interrupts for the KN02. (DECstation 5000/200)
 * Returns spl value.
 */
kn02_intr(mask, pc, statusReg, causeReg)
	unsigned mask;
	unsigned pc;
	unsigned statusReg;
	unsigned causeReg;
{
	register unsigned i, m;
	register volatile struct chiptime *c = Mach_clock_addr;
	register unsigned csr;
	int temp;
	struct clockframe cf;
	static int warned = 0;

	/* handle clock interrupts ASAP */
	if (mask & MACH_INT_MASK_1) {
		csr = *(unsigned *)MACH_PHYS_TO_UNCACHED(KN02_SYS_CSR);
		if ((csr & KN02_CSR_PSWARN) && !warned) {
			warned = 1;
			printf("WARNING: power supply is overheating!\n");
		} else if (warned && !(csr & KN02_CSR_PSWARN)) {
			warned = 0;
			printf("WARNING: power supply is OK again\n");
		}
		intrcnt[6]++;

		temp = c->regc;	/* XXX clear interrupt bits */
		cf.pc = pc;
		cf.sr = statusReg;
		hardclock(&cf);

		/* keep clock interrupts enabled */
		causeReg &= ~MACH_INT_MASK_1;
	}
	/* Re-enable clock interrupts */
	splx(MACH_INT_MASK_1 | MACH_SR_INT_ENA_CUR);
	if (mask & MACH_INT_MASK_0) {
		static int map[8] = { 8, 8, 8, 8, 8, 4, 3, 2 };

		csr = *(unsigned *)MACH_PHYS_TO_UNCACHED(KN02_SYS_CSR);
		m = csr & (csr >> KN02_CSR_IOINTEN_SHIFT) & KN02_CSR_IOINT;
#if 0
		*(unsigned *)MACHPHYS_TO_UNCACHED(KN02_SYS_CSR) =
			(csr & ~(KN02_CSR_WRESERVED | 0xFF)) |
			(m << KN02_CSR_IOINTEN_SHIFT);
#endif
		for (i = 0; m; i++, m >>= 1) {
			if (!(m & 1))
				continue;
			intrcnt[map[i]]++;
			if (tc_slot_info[i].intr)
				(*tc_slot_info[i].intr)(tc_slot_info[i].unit);
			else
				printf("spurious interrupt %d\n", i);
		}
#if 0
		*(unsigned *)MACH_PHYS_TO_UNCACHED(KN02_SYS_CSR) =
			csr & ~(KN02_CSR_WRESERVED | 0xFF);
#endif
	}
	if (mask & MACH_INT_MASK_3) {
		intrcnt[5]++;
		kn02_errintr();
	}
	return ((statusReg & ~causeReg & MACH_HARD_INT_MASK) |
		MACH_SR_INT_ENA_CUR);
}

/*
 * 3min hardware interrupts. (DECstation 5000/1xx)
 */
kmin_intr(mask, pc, statusReg, causeReg)
	unsigned mask;
	unsigned pc;
	unsigned statusReg;
	unsigned causeReg;
{
	register u_int intr;
	register volatile struct chiptime *c = Mach_clock_addr;
	volatile u_int *imaskp =
		(volatile u_int *)MACH_PHYS_TO_UNCACHED(KMIN_REG_IMSK);
	volatile u_int *intrp =
		(volatile u_int *)MACH_PHYS_TO_UNCACHED(KMIN_REG_INTR);
	unsigned int old_mask;
	struct clockframe cf;
	int temp;
	static int user_warned = 0;

	old_mask = *imaskp & kmin_tc3_imask;
	*imaskp = old_mask;

	if (mask & MACH_INT_MASK_4)
		(*callv->halt)((int *)0, 0);
	if (mask & MACH_INT_MASK_3) {
		intr = *intrp;
		/* masked interrupts are still observable */
		intr &= old_mask;
	
		if (intr & KMIN_INTR_SCSI_PTR_LOAD) {
			*intrp &= ~KMIN_INTR_SCSI_PTR_LOAD;
#ifdef notdef
			asc_dma_intr();
#endif
		}
	
		if (intr & (KMIN_INTR_SCSI_OVRUN | KMIN_INTR_SCSI_READ_E))
			*intrp &= ~(KMIN_INTR_SCSI_OVRUN | KMIN_INTR_SCSI_READ_E);

		if (intr & KMIN_INTR_LANCE_READ_E)
			*intrp &= ~KMIN_INTR_LANCE_READ_E;

		if (intr & KMIN_INTR_TIMEOUT)
			kn02ba_errintr();
	
		if (intr & KMIN_INTR_CLOCK) {
			temp = c->regc;	/* XXX clear interrupt bits */
			cf.pc = pc;
			cf.sr = statusReg;
			hardclock(&cf);
		}
	
		if ((intr & KMIN_INTR_SCC_0) &&
			tc_slot_info[KMIN_SCC0_SLOT].intr)
			(*(tc_slot_info[KMIN_SCC0_SLOT].intr))
			(tc_slot_info[KMIN_SCC0_SLOT].unit);
	
		if ((intr & KMIN_INTR_SCC_1) &&
			tc_slot_info[KMIN_SCC1_SLOT].intr)
			(*(tc_slot_info[KMIN_SCC1_SLOT].intr))
			(tc_slot_info[KMIN_SCC1_SLOT].unit);
	
		if ((intr & KMIN_INTR_SCSI) &&
			tc_slot_info[KMIN_SCSI_SLOT].intr)
			(*(tc_slot_info[KMIN_SCSI_SLOT].intr))
			(tc_slot_info[KMIN_SCSI_SLOT].unit);
	
		if ((intr & KMIN_INTR_LANCE) &&
			tc_slot_info[KMIN_LANCE_SLOT].intr)
			(*(tc_slot_info[KMIN_LANCE_SLOT].intr))
			(tc_slot_info[KMIN_LANCE_SLOT].unit);
	
		if (user_warned && ((intr & KMIN_INTR_PSWARN) == 0)) {
			printf("%s\n", "Power supply ok now.");
			user_warned = 0;
		}
		if ((intr & KMIN_INTR_PSWARN) && (user_warned < 3)) {
			user_warned++;
			printf("%s\n", "Power supply overheating");
		}
	}
	if ((mask & MACH_INT_MASK_0) && tc_slot_info[0].intr)
		(*tc_slot_info[0].intr)(tc_slot_info[0].unit);
	if ((mask & MACH_INT_MASK_1) && tc_slot_info[1].intr)
		(*tc_slot_info[1].intr)(tc_slot_info[1].unit);
	if ((mask & MACH_INT_MASK_2) && tc_slot_info[2].intr)
		(*tc_slot_info[2].intr)(tc_slot_info[2].unit);
	return ((statusReg & ~causeReg & MACH_HARD_INT_MASK) |
		MACH_SR_INT_ENA_CUR);
}

/*
 * Maxine hardware interrupts. (Personal DECstation 5000/xx)
 */
xine_intr(mask, pc, statusReg, causeReg)
	unsigned mask;
	unsigned pc;
	unsigned statusReg;
	unsigned causeReg;
{
	register u_int intr;
	register volatile struct chiptime *c = Mach_clock_addr;
	volatile u_int *imaskp = (volatile u_int *)
		MACH_PHYS_TO_UNCACHED(XINE_REG_IMSK);
	volatile u_int *intrp = (volatile u_int *)
		MACH_PHYS_TO_UNCACHED(XINE_REG_INTR);
	u_int old_mask;
	struct clockframe cf;
	int temp;

	old_mask = *imaskp & xine_tc3_imask;
	*imaskp = old_mask;

	if (mask & MACH_INT_MASK_4)
		(*callv->halt)((int *)0, 0);

	/* handle clock interrupts ASAP */
	if (mask & MACH_INT_MASK_1) {
		temp = c->regc;	/* XXX clear interrupt bits */
		cf.pc = pc;
		cf.sr = statusReg;
		hardclock(&cf);
		causeReg &= ~MACH_INT_MASK_1;
		/* reenable clock interrupts */
		splx(MACH_INT_MASK_1 | MACH_SR_INT_ENA_CUR);
	}
	if (mask & MACH_INT_MASK_3) {
		intr = *intrp;
		/* masked interrupts are still observable */
		intr &= old_mask;

		if (intr & XINE_INTR_SCSI_PTR_LOAD) {
			*intrp &= ~XINE_INTR_SCSI_PTR_LOAD;
#ifdef notdef
			asc_dma_intr();
#endif
		}
	
		if (intr & (XINE_INTR_SCSI_OVRUN | XINE_INTR_SCSI_READ_E))
			*intrp &= ~(XINE_INTR_SCSI_OVRUN | XINE_INTR_SCSI_READ_E);

		if (intr & XINE_INTR_LANCE_READ_E)
			*intrp &= ~XINE_INTR_LANCE_READ_E;

		if ((intr & XINE_INTR_SCC_0) &&
			tc_slot_info[XINE_SCC0_SLOT].intr)
			(*(tc_slot_info[XINE_SCC0_SLOT].intr))
			(tc_slot_info[XINE_SCC0_SLOT].unit);
	
		if ((intr & XINE_INTR_DTOP_RX) &&
			tc_slot_info[XINE_DTOP_SLOT].intr)
			(*(tc_slot_info[XINE_DTOP_SLOT].intr))
			(tc_slot_info[XINE_DTOP_SLOT].unit);
	
		if ((intr & XINE_INTR_FLOPPY) &&
			tc_slot_info[XINE_FLOPPY_SLOT].intr)
			(*(tc_slot_info[XINE_FLOPPY_SLOT].intr))
			(tc_slot_info[XINE_FLOPPY_SLOT].unit);
	
		if ((intr & XINE_INTR_TC_0) &&
			tc_slot_info[0].intr)
			(*(tc_slot_info[0].intr))
			(tc_slot_info[0].unit);
	
		if ((intr & XINE_INTR_TC_1) &&
			tc_slot_info[1].intr)
			(*(tc_slot_info[1].intr))
			(tc_slot_info[1].unit);
	
		if ((intr & XINE_INTR_ISDN) &&
			tc_slot_info[XINE_ISDN_SLOT].intr)
			(*(tc_slot_info[XINE_ISDN_SLOT].intr))
			(tc_slot_info[XINE_ISDN_SLOT].unit);
	
		if ((intr & XINE_INTR_SCSI) &&
			tc_slot_info[XINE_SCSI_SLOT].intr)
			(*(tc_slot_info[XINE_SCSI_SLOT].intr))
			(tc_slot_info[XINE_SCSI_SLOT].unit);
	
		if ((intr & XINE_INTR_LANCE) &&
			tc_slot_info[XINE_LANCE_SLOT].intr)
			(*(tc_slot_info[XINE_LANCE_SLOT].intr))
			(tc_slot_info[XINE_LANCE_SLOT].unit);
	
	}
	if (mask & MACH_INT_MASK_2)
		kn02ba_errintr();
	return ((statusReg & ~causeReg & MACH_HARD_INT_MASK) |
		MACH_SR_INT_ENA_CUR);
}

#ifdef DS5000_240
/*
 * 3Max+ hardware interrupts. (DECstation 5000/240) UNTESTED!!
 */
kn03_intr(mask, pc, statusReg, causeReg)
	unsigned mask;
	unsigned pc;
	unsigned statusReg;
	unsigned causeReg;
{
	register u_int intr;
	register volatile struct chiptime *c = Mach_clock_addr;
	volatile u_int *imaskp = (volatile u_int *)
		MACH_PHYS_TO_UNCACHED(KN03_REG_IMSK);
	volatile u_int *intrp = (volatile u_int *)
		MACH_PHYS_TO_UNCACHED(KN03_REG_INTR);
	u_int old_mask;
	struct clockframe cf;
	int temp;
	static int user_warned = 0;

	old_mask = *imaskp & kn03_tc3_imask;
	*imaskp = old_mask;

	if (mask & MACH_INT_MASK_4)
		(*callv->halt)((int *)0, 0);

	/* handle clock interrupts ASAP */
	if (mask & MACH_INT_MASK_1) {
		temp = c->regc;	/* XXX clear interrupt bits */
		cf.pc = pc;
		cf.sr = statusReg;
		hardclock(&cf);
		causeReg &= ~MACH_INT_MASK_1;
		/* reenable clock interrupts */
		splx(MACH_INT_MASK_1 | MACH_SR_INT_ENA_CUR);
	}
	if (mask & MACH_INT_MASK_0) {
		intr = *intrp;
		/* masked interrupts are still observable */
		intr &= old_mask;

		if (intr & KN03_INTR_SCSI_PTR_LOAD) {
			*intrp &= ~KN03_INTR_SCSI_PTR_LOAD;
#ifdef notdef
			asc_dma_intr();
#endif
		}
	
		if (intr & (KN03_INTR_SCSI_OVRUN | KN03_INTR_SCSI_READ_E))
			*intrp &= ~(KN03_INTR_SCSI_OVRUN | KN03_INTR_SCSI_READ_E);

		if (intr & KN03_INTR_LANCE_READ_E)
			*intrp &= ~KN03_INTR_LANCE_READ_E;

		if ((intr & KN03_INTR_SCC_0) &&
			tc_slot_info[KN03_SCC0_SLOT].intr)
			(*(tc_slot_info[KN03_SCC0_SLOT].intr))
			(tc_slot_info[KN03_SCC0_SLOT].unit);
	
		if ((intr & KN03_INTR_SCC_1) &&
			tc_slot_info[KN03_SCC1_SLOT].intr)
			(*(tc_slot_info[KN03_SCC1_SLOT].intr))
			(tc_slot_info[KN03_SCC1_SLOT].unit);
	
		if ((intr & KN03_INTR_TC_0) &&
			tc_slot_info[0].intr)
			(*(tc_slot_info[0].intr))
			(tc_slot_info[0].unit);
	
		if ((intr & KN03_INTR_TC_1) &&
			tc_slot_info[1].intr)
			(*(tc_slot_info[1].intr))
			(tc_slot_info[1].unit);
	
		if ((intr & KN03_INTR_TC_2) &&
			tc_slot_info[2].intr)
			(*(tc_slot_info[2].intr))
			(tc_slot_info[2].unit);
	
		if ((intr & KN03_INTR_SCSI) &&
			tc_slot_info[KN03_SCSI_SLOT].intr)
			(*(tc_slot_info[KN03_SCSI_SLOT].intr))
			(tc_slot_info[KN03_SCSI_SLOT].unit);
	
		if ((intr & KN03_INTR_LANCE) &&
			tc_slot_info[KN03_LANCE_SLOT].intr)
			(*(tc_slot_info[KN03_LANCE_SLOT].intr))
			(tc_slot_info[KN03_LANCE_SLOT].unit);
	
		if (user_warned && ((intr & KN03_INTR_PSWARN) == 0)) {
			printf("%s\n", "Power supply ok now.");
			user_warned = 0;
		}
		if ((intr & KN03_INTR_PSWARN) && (user_warned < 3)) {
			user_warned++;
			printf("%s\n", "Power supply overheating");
		}
	}
	if (mask & MACH_INT_MASK_3)
		kn03_errintr();
	return ((statusReg & ~causeReg & MACH_HARD_INT_MASK) |
		MACH_SR_INT_ENA_CUR);
}
#endif /* DS5000_240 */

/*
 * This is called from MachUserIntr() if astpending is set.
 * This is very similar to the tail of trap().
 */
softintr(statusReg, pc)
	unsigned statusReg;	/* status register at time of the exception */
	unsigned pc;		/* program counter where to continue */
{
	register struct proc *p = curproc;
	int sig;

	cnt.v_soft++;
	/* take pending signals */
	while ((sig = CURSIG(p)) != 0)
		postsig(sig);
	p->p_priority = p->p_usrpri;
	astpending = 0;
	if (p->p_flag & P_OWEUPC) {
		p->p_flag &= ~P_OWEUPC;
		ADDUPROF(p);
	}
	if (want_resched) {
		int s;

		/*
		 * Since we are curproc, clock will normally just change
		 * our priority without moving us from one queue to another
		 * (since the running process is not on a queue.)
		 * If that happened after we put ourselves on the run queue
		 * but before we switched, we might not be on the queue
		 * indicated by our priority.
		 */
		s = splstatclock();
		setrunqueue(p);
		p->p_stats->p_ru.ru_nivcsw++;
		mi_switch();
		splx(s);
		while ((sig = CURSIG(p)) != 0)
			postsig(sig);
	}
	curpriority = p->p_priority;
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

/*
 *----------------------------------------------------------------------
 *
 * MemErrorInterrupts --
 *   pmax_errintr - for the DS2100/DS3100
 *   kn02_errintr - for the DS5000/200
 *   kn02ba_errintr - for the DS5000/1xx and DS5000/xx
 *
 *	Handler an interrupt for the control register.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */
static void
pmax_errintr()
{
	volatile u_short *sysCSRPtr =
		(u_short *)MACH_PHYS_TO_UNCACHED(KN01_SYS_CSR);
	u_short csr;

	csr = *sysCSRPtr;

	if (csr & KN01_CSR_MERR) {
		printf("Memory error at 0x%x\n",
			*(unsigned *)MACH_PHYS_TO_UNCACHED(KN01_SYS_ERRADR));
		panic("Mem error interrupt");
	}
	*sysCSRPtr = (csr & ~KN01_CSR_MBZ) | 0xff;
}

static void
kn02_errintr()
{
	u_int erradr, chksyn, physadr;
	int i;

	erradr = *(u_int *)MACH_PHYS_TO_UNCACHED(KN02_SYS_ERRADR);
	chksyn = *(u_int *)MACH_PHYS_TO_UNCACHED(KN02_SYS_CHKSYN);
	*(u_int *)MACH_PHYS_TO_UNCACHED(KN02_SYS_ERRADR) = 0;
	MachEmptyWriteBuffer();

	if (!(erradr & KN02_ERR_VALID))
		return;
	/* extract the physical word address and compensate for pipelining */
	physadr = erradr & KN02_ERR_ADDRESS;
	if (!(erradr & KN02_ERR_WRITE))
		physadr = (physadr & ~0xfff) | ((physadr & 0xfff) - 5);
	physadr <<= 2;
	printf("%s memory %s %s error at 0x%x\n",
		(erradr & KN02_ERR_CPU) ? "CPU" : "DMA",
		(erradr & KN02_ERR_WRITE) ? "write" : "read",
		(erradr & KN02_ERR_ECCERR) ? "ECC" : "timeout",
		physadr);
	if (erradr & KN02_ERR_ECCERR) {
		*(u_int *)MACH_PHYS_TO_UNCACHED(KN02_SYS_CHKSYN) = 0;
		MachEmptyWriteBuffer();
		printf("ECC 0x%x\n", chksyn);

		/* check for a corrected, single bit, read error */
		if (!(erradr & KN02_ERR_WRITE)) {
			if (physadr & 0x4) {
				/* check high word */
				if (chksyn & KN02_ECC_SNGHI)
					return;
			} else {
				/* check low word */
				if (chksyn & KN02_ECC_SNGLO)
					return;
			}
		}
	}
	panic("Mem error interrupt");
}

#ifdef DS5000_240
static void
kn03_errintr()
{

	printf("erradr %x\n", *(unsigned *)MACH_PHYS_TO_UNCACHED(KN03_SYS_ERRADR));
	*(unsigned *)MACH_PHYS_TO_UNCACHED(KN03_SYS_ERRADR) = 0;
	MachEmptyWriteBuffer();
}
#endif /* DS5000_240 */

static void
kn02ba_errintr()
{
	register int mer, adr, siz;
	static int errintr_cnt = 0;

	siz = *(volatile int *)MACH_PHYS_TO_UNCACHED(KMIN_REG_MSR);
	mer = *(volatile int *)MACH_PHYS_TO_UNCACHED(KMIN_REG_MER);
	adr = *(volatile int *)MACH_PHYS_TO_UNCACHED(KMIN_REG_AER);

	/* clear interrupt bit */
	*(unsigned int *)MACH_PHYS_TO_UNCACHED(KMIN_REG_TIMEOUT) = 0;

	errintr_cnt++;
	printf("(%d)%s%x [%x %x %x]\n", errintr_cnt,
	       "Bad memory chip at phys ",
	       kn02ba_recover_erradr(adr, mer),
	       mer, siz, adr);
}

static unsigned
kn02ba_recover_erradr(phys, mer)
	register unsigned phys, mer;
{
	/* phys holds bits 28:2, mer knows which byte */
	switch (mer & KMIN_MER_LASTBYTE) {
	case KMIN_LASTB31:
		mer = 3; break;
	case KMIN_LASTB23:
		mer = 2; break;
	case KMIN_LASTB15:
		mer = 1; break;
	case KMIN_LASTB07:
		mer = 0; break;
	}
	return ((phys & KMIN_AER_ADDR_MASK) | mer);
}

/*
 * Return the resulting PC as if the branch was executed.
 */
unsigned
MachEmulateBranch(regsPtr, instPC, fpcCSR, allowNonBranch)
	unsigned *regsPtr;
	unsigned instPC;
	unsigned fpcCSR;
	int allowNonBranch;
{
	InstFmt inst;
	unsigned retAddr;
	int condition;
	extern unsigned GetBranchDest();


	inst = *(InstFmt *)instPC;
#if 0
	printf("regsPtr=%x PC=%x Inst=%x fpcCsr=%x\n", regsPtr, instPC,
		inst.word, fpcCSR); /* XXX */
#endif
	switch ((int)inst.JType.op) {
	case OP_SPECIAL:
		switch ((int)inst.RType.func) {
		case OP_JR:
		case OP_JALR:
			retAddr = regsPtr[inst.RType.rs];
			break;

		default:
			if (!allowNonBranch)
				panic("MachEmulateBranch: Non-branch");
			retAddr = instPC + 4;
			break;
		}
		break;

	case OP_BCOND:
		switch ((int)inst.IType.rt) {
		case OP_BLTZ:
		case OP_BLTZAL:
			if ((int)(regsPtr[inst.RType.rs]) < 0)
				retAddr = GetBranchDest((InstFmt *)instPC);
			else
				retAddr = instPC + 8;
			break;

		case OP_BGEZAL:
		case OP_BGEZ:
			if ((int)(regsPtr[inst.RType.rs]) >= 0)
				retAddr = GetBranchDest((InstFmt *)instPC);
			else
				retAddr = instPC + 8;
			break;

		default:
			panic("MachEmulateBranch: Bad branch cond");
		}
		break;

	case OP_J:
	case OP_JAL:
		retAddr = (inst.JType.target << 2) | 
			((unsigned)instPC & 0xF0000000);
		break;

	case OP_BEQ:
		if (regsPtr[inst.RType.rs] == regsPtr[inst.RType.rt])
			retAddr = GetBranchDest((InstFmt *)instPC);
		else
			retAddr = instPC + 8;
		break;

	case OP_BNE:
		if (regsPtr[inst.RType.rs] != regsPtr[inst.RType.rt])
			retAddr = GetBranchDest((InstFmt *)instPC);
		else
			retAddr = instPC + 8;
		break;

	case OP_BLEZ:
		if ((int)(regsPtr[inst.RType.rs]) <= 0)
			retAddr = GetBranchDest((InstFmt *)instPC);
		else
			retAddr = instPC + 8;
		break;

	case OP_BGTZ:
		if ((int)(regsPtr[inst.RType.rs]) > 0)
			retAddr = GetBranchDest((InstFmt *)instPC);
		else
			retAddr = instPC + 8;
		break;

	case OP_COP1:
		switch (inst.RType.rs) {
		case OP_BCx:
		case OP_BCy:
			if ((inst.RType.rt & COPz_BC_TF_MASK) == COPz_BC_TRUE)
				condition = fpcCSR & MACH_FPC_COND_BIT;
			else
				condition = !(fpcCSR & MACH_FPC_COND_BIT);
			if (condition)
				retAddr = GetBranchDest((InstFmt *)instPC);
			else
				retAddr = instPC + 8;
			break;

		default:
			if (!allowNonBranch)
				panic("MachEmulateBranch: Bad coproc branch instruction");
			retAddr = instPC + 4;
		}
		break;

	default:
		if (!allowNonBranch)
			panic("MachEmulateBranch: Non-branch instruction");
		retAddr = instPC + 4;
	}
#if 0
	printf("Target addr=%x\n", retAddr); /* XXX */
#endif
	return (retAddr);
}

unsigned
GetBranchDest(InstPtr)
	InstFmt *InstPtr;
{
	return ((unsigned)InstPtr + 4 + ((short)InstPtr->IType.imm << 2));
}

/*
 * This routine is called by procxmt() to single step one instruction.
 * We do this by storing a break instruction after the current instruction,
 * resuming execution, and then restoring the old instruction.
 */
cpu_singlestep(p)
	register struct proc *p;
{
	register unsigned va;
	register int *locr0 = p->p_md.md_regs;
	int i;

	/* compute next address after current location */
	va = MachEmulateBranch(locr0, locr0[PC], locr0[FSR], 1);
	if (p->p_md.md_ss_addr || p->p_md.md_ss_addr == va ||
	    !useracc((caddr_t)va, 4, B_READ)) {
		printf("SS %s (%d): breakpoint already set at %x (va %x)\n",
			p->p_comm, p->p_pid, p->p_md.md_ss_addr, va); /* XXX */
		return (EFAULT);
	}
	p->p_md.md_ss_addr = va;
	p->p_md.md_ss_instr = fuiword((caddr_t)va);
	i = suiword((caddr_t)va, MACH_BREAK_SSTEP);
	if (i < 0) {
		vm_offset_t sa, ea;
		int rv;

		sa = trunc_page((vm_offset_t)va);
		ea = round_page((vm_offset_t)va+sizeof(int)-1);
		rv = vm_map_protect(&p->p_vmspace->vm_map, sa, ea,
			VM_PROT_DEFAULT, FALSE);
		if (rv == KERN_SUCCESS) {
			i = suiword((caddr_t)va, MACH_BREAK_SSTEP);
			(void) vm_map_protect(&p->p_vmspace->vm_map,
				sa, ea, VM_PROT_READ|VM_PROT_EXECUTE, FALSE);
		}
	}
	if (i < 0)
		return (EFAULT);
#if 0
	printf("SS %s (%d): breakpoint set at %x: %x (pc %x) br %x\n",
		p->p_comm, p->p_pid, p->p_md.md_ss_addr,
		p->p_md.md_ss_instr, locr0[PC], fuword((caddr_t)va)); /* XXX */
#endif
	return (0);
}

#ifdef DEBUG
kdbpeek(addr)
{
	if (addr & 3) {
		printf("kdbpeek: unaligned address %x\n", addr);
		return (-1);
	}
	return (*(int *)addr);
}

#define MIPS_JR_RA	0x03e00008	/* instruction code for jr ra */

/*
 * Print a stack backtrace.
 */
void
stacktrace(a0, a1, a2, a3)
	int a0, a1, a2, a3;
{
	unsigned pc, sp, fp, ra, va, subr;
	unsigned instr, mask;
	InstFmt i;
	int more, stksize;
	int regs[3];
	extern setsoftclock();
	extern char start[], edata[];

	cpu_getregs(regs);

	/* get initial values from the exception frame */
	sp = regs[0];
	pc = regs[1];
	ra = 0;
	fp = regs[2];

loop:
	/* check for current PC in the kernel interrupt handler code */
	if (pc >= (unsigned)MachKernIntr && pc < (unsigned)MachUserIntr) {
		/* NOTE: the offsets depend on the code in locore.s */
		printf("interrupt\n");
		a0 = kdbpeek(sp + 36);
		a1 = kdbpeek(sp + 40);
		a2 = kdbpeek(sp + 44);
		a3 = kdbpeek(sp + 48);
		pc = kdbpeek(sp + 20);
		ra = kdbpeek(sp + 92);
		sp = kdbpeek(sp + 100);
		fp = kdbpeek(sp + 104);
	}

	/* check for current PC in the exception handler code */
	if (pc >= 0x80000000 && pc < (unsigned)setsoftclock) {
		ra = 0;
		subr = 0;
		goto done;
	}

	/* check for bad PC */
	if (pc & 3 || pc < 0x80000000 || pc >= (unsigned)edata) {
		printf("PC 0x%x: not in kernel\n", pc);
		ra = 0;
		subr = 0;
		goto done;
	}

	/*
	 * Find the beginning of the current subroutine by scanning backwards
	 * from the current PC for the end of the previous subroutine.
	 */
	va = pc - sizeof(int);
	while ((instr = kdbpeek(va)) != MIPS_JR_RA)
		va -= sizeof(int);
	va += 2 * sizeof(int);	/* skip back over branch & delay slot */
	/* skip over nulls which might separate .o files */
	while ((instr = kdbpeek(va)) == 0)
		va += sizeof(int);
	subr = va;

	/* scan forwards to find stack size and any saved registers */
	stksize = 0;
	more = 3;
	mask = 0;
	for (; more; va += sizeof(int), more = (more == 3) ? 3 : more - 1) {
		/* stop if hit our current position */
		if (va >= pc)
			break;
		instr = kdbpeek(va);
		i.word = instr;
		switch (i.JType.op) {
		case OP_SPECIAL:
			switch (i.RType.func) {
			case OP_JR:
			case OP_JALR:
				more = 2; /* stop after next instruction */
				break;

			case OP_SYSCALL:
			case OP_BREAK:
				more = 1; /* stop now */
			};
			break;

		case OP_BCOND:
		case OP_J:
		case OP_JAL:
		case OP_BEQ:
		case OP_BNE:
		case OP_BLEZ:
		case OP_BGTZ:
			more = 2; /* stop after next instruction */
			break;

		case OP_COP0:
		case OP_COP1:
		case OP_COP2:
		case OP_COP3:
			switch (i.RType.rs) {
			case OP_BCx:
			case OP_BCy:
				more = 2; /* stop after next instruction */
			};
			break;

		case OP_SW:
			/* look for saved registers on the stack */
			if (i.IType.rs != 29)
				break;
			/* only restore the first one */
			if (mask & (1 << i.IType.rt))
				break;
			mask |= 1 << i.IType.rt;
			switch (i.IType.rt) {
			case 4: /* a0 */
				a0 = kdbpeek(sp + (short)i.IType.imm);
				break;

			case 5: /* a1 */
				a1 = kdbpeek(sp + (short)i.IType.imm);
				break;

			case 6: /* a2 */
				a2 = kdbpeek(sp + (short)i.IType.imm);
				break;

			case 7: /* a3 */
				a3 = kdbpeek(sp + (short)i.IType.imm);
				break;

			case 30: /* fp */
				fp = kdbpeek(sp + (short)i.IType.imm);
				break;

			case 31: /* ra */
				ra = kdbpeek(sp + (short)i.IType.imm);
			}
			break;

		case OP_ADDI:
		case OP_ADDIU:
			/* look for stack pointer adjustment */
			if (i.IType.rs != 29 || i.IType.rt != 29)
				break;
			stksize = (short)i.IType.imm;
		}
	}

done:
	printf("%x+%x (%x,%x,%x,%x) ra %x sz %d\n",
		subr, pc - subr, a0, a1, a2, a3, ra, stksize);

	if (ra) {
		if (pc == ra && stksize == 0)
			printf("stacktrace: loop!\n");
		else {
			pc = ra;
			sp -= stksize;
			goto loop;
		}
	}
}
#endif /* DEBUG */
