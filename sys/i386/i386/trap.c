/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * the University of Utah, and William Jolitz.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)trap.c	7.3 (Berkeley) 5/10/91
 */

/*
 * 386 Trap and System call handleing
 */

#include "machine/cpu.h"
#include "machine/psl.h"
#include "machine/reg.h"

#include "param.h"
#include "systm.h"
#include "proc.h"
#include "user.h"
#include "seg.h"
#include "acct.h"
#include "kernel.h"
#ifdef KTRACE
#include "ktrace.h"
#endif

#include "vm/vm_param.h"
#include "vm/pmap.h"
#include "vm/vm_map.h"
#include "sys/vmmeter.h"

#include "machine/trap.h"
#include "machine/dbg.h"


struct	sysent sysent[];
int	nsysent;
/*
 * trap(frame):
 *	Exception, fault, and trap interface to BSD kernel. This
 * common code is called from assembly language IDT gate entry
 * routines that prepare a suitable stack frame, and restore this
 * frame after the exception has been processed. Note that the
 * effect is as if the arguments were passed call by reference.
 */
extern caddr_t edata;
unsigned rcr2(), Sysbase;
extern short cpl;
int um;
/*ARGSUSED*/
trap(frame)
	struct trapframe frame;
/*#define type frame.tf_trapno
#define code frame.tf_err
#define pc frame.tf_eip*/
{
	register int i;
	register struct proc *p = curproc;
	struct timeval syst;
	int ucode, type, code, eva;

#ifdef DEBUG
/*dprintf(DALLTRAPS, "\n%d(%s). trap",p->p_pid, p->p_comm);*/
dprintf(DALLTRAPS, " pc:%x cs:%x ds:%x eflags:%x isp %x\n",
		frame.tf_eip, frame.tf_cs, frame.tf_ds, frame.tf_eflags,
		frame.tf_isp);
dprintf(DALLTRAPS, "edi %x esi %x ebp %x ebx %x esp %x\n",
		frame.tf_edi, frame.tf_esi, frame.tf_ebp,
		frame.tf_ebx, frame.tf_esp);
dprintf(DALLTRAPS, "edx %x ecx %x eax %x\n",
		frame.tf_edx, frame.tf_ecx, frame.tf_eax);
/*
dprintf(DALLTRAPS, "sig %x %x %x \n",
		p->p_sigignore, p->p_sigcatch, p->p_sigmask); */
dprintf(DALLTRAPS, " ec %x type %x cpl %x ",
		frame.tf_err&0xffff, frame.tf_trapno, cpl);
printf("trap cr2 %x ", rcr2());
#endif

/*if(um && frame.tf_trapno == 0xc && (rcr2()&0xfffff000) == 0){
	if (ISPL(locr0[tCS]) != SEL_UPL) {
		if(curpcb->pcb_onfault) goto anyways;
		locr0[tEFLAGS] |= PSL_T;
		*(int *)PTmap |= 1; load_cr3(rcr3());
		return;
	}
} else if (um) {
printf("p %x ", *(int *) PTmap);
*(int *)PTmap &= 0xfffffffe; load_cr3(rcr3());
printf("p %x ", *(int *) PTmap);
}
anyways:

if(pc == 0) um++;*/

	frame.tf_eflags &= ~PSL_NT;	/* clear nested trap XXX */
	type = frame.tf_trapno;
	if(curpcb->pcb_onfault && frame.tf_trapno != 0xc)
		{ frame.tf_eip = (int)curpcb->pcb_onfault; return;}

	syst = p->p_stime;
	if (ISPL(frame.tf_cs) == SEL_UPL) {
		type |= T_USER;
		p->p_regs = (int *)&frame;
		curpcb->pcb_flags |= FM_TRAP;	/* used by sendsig */
	}
if((caddr_t)p < edata) printf("trap with curproc garbage ");
if((caddr_t)p->p_regs < edata) printf("trap with pregs garbage ");

	ucode=0;
	eva = rcr2();
	code = frame.tf_err;
	switch (type) {

	default:
bit_sucker:
#ifdef KDB
		if (kdb_trap(&psl))
			return;
#endif

		printf("trap type %d code = %x eip = %x cs = %x eflags = %x ",
			frame.tf_trapno, frame.tf_err, frame.tf_eip,
			frame.tf_cs, frame.tf_eflags);
		printf("cr2 %x cpl %x\n", eva, cpl);
		type &= ~T_USER;
		panic("trap");
		/*NOTREACHED*/

	case T_SEGNPFLT|T_USER:
	case T_PROTFLT|T_USER:		/* protection fault */
copyfault:
		ucode = code + BUS_SEGM_FAULT ;
		i = SIGBUS;
		break;

	case T_PRIVINFLT|T_USER:	/* privileged instruction fault */
	case T_RESADFLT|T_USER:		/* reserved addressing fault */
	case T_RESOPFLT|T_USER:		/* reserved operand fault */
	case T_FPOPFLT|T_USER:		/* coprocessor operand fault */
		ucode = type &~ T_USER;
		i = SIGILL;
		break;

	case T_ASTFLT|T_USER:		/* Allow process switch */
	case T_ASTFLT:
		astoff();
		if ((p->p_flag & SOWEUPC) && p->p_stats->p_prof.pr_scale) {
			addupc(frame.tf_eip, &p->p_stats->p_prof, 1);
			p->p_flag &= ~SOWEUPC;
		}
		goto out;

	case T_DNA|T_USER:
#ifdef	NPX
		/* if a transparent fault (due to context switch "late") */
		if (npxdna()) return;
#endif
		ucode = FPE_FPU_NP_TRAP;
		i = SIGFPE;
		break;

	case T_BOUND|T_USER:
		ucode = FPE_SUBRNG_TRAP;
		i = SIGFPE;
		break;

	case T_OFLOW|T_USER:
		ucode = FPE_INTOVF_TRAP;
		i = SIGFPE;
		break;

	case T_DIVIDE|T_USER:
		ucode = FPE_INTDIV_TRAP;
		i = SIGFPE;
		break;

	case T_ARITHTRAP|T_USER:
		ucode = code;
		i = SIGFPE;
		break;

	case T_PAGEFLT:			/* allow page faults in kernel mode */
		if (code & PGEX_P) goto bit_sucker;
		/* fall into */
	case T_PAGEFLT|T_USER:		/* page fault */
	    {
		register vm_offset_t va;
		register struct vmspace *vm = p->p_vmspace;
		register vm_map_t map;
		int rv;
		vm_prot_t ftype;
		extern vm_map_t kernel_map;
		unsigned nss,v;

		va = trunc_page((vm_offset_t)eva);
		/*
		 * It is only a kernel address space fault iff:
		 * 	1. (type & T_USER) == 0  and
		 * 	2. pcb_onfault not set or
		 *	3. pcb_onfault set but supervisor space fault
		 * The last can occur during an exec() copyin where the
		 * argument space is lazy-allocated.
		 */
		/*if (type == T_PAGEFLT && !curpcb->pcb_onfault)*/
		if (type == T_PAGEFLT && va >= 0xfe000000)
			map = kernel_map;
		else
			map = &vm->vm_map;
		if (code & PGEX_W)
			ftype = VM_PROT_READ | VM_PROT_WRITE;
		else
			ftype = VM_PROT_READ;

#ifdef DEBUG
		if (map == kernel_map && va == 0) {
			printf("trap: bad kernel access at %x\n", va);
			goto bit_sucker;
		}
#endif
		/*
		 * XXX: rude hack to make stack limits "work"
		 */
		nss = 0;
		if ((caddr_t)va >= vm->vm_maxsaddr && map != kernel_map) {
			nss = clrnd(btoc(USRSTACK-(unsigned)va));
			if (nss > btoc(p->p_rlimit[RLIMIT_STACK].rlim_cur)) {
pg("stk fuck");
				rv = KERN_FAILURE;
				goto nogo;
			}
		}
		/* check if page table is mapped, if not, fault it first */
#define pde_v(v) (PTD[((v)>>PD_SHIFT)&1023].pd_v)
		if (!pde_v(va)) {
			v = trunc_page(vtopte(va));
/*pg("pt fault");*/
			rv = vm_fault(map, v, ftype, FALSE);
			if (rv != KERN_SUCCESS) goto nogo;
			/* check if page table fault, increment wiring */
			vm_map_pageable(map, v, round_page(v+1), FALSE);
		} else v=0;
		rv = vm_fault(map, va, ftype, FALSE);
		if (rv == KERN_SUCCESS) {
			/*
			 * XXX: continuation of rude stack hack
			 */
			if (nss > vm->vm_ssize)
				vm->vm_ssize = nss;
			va = trunc_page(vtopte(va));
			/* for page table, increment wiring
			   as long as not a page table fault as well */
			if (!v && type != T_PAGEFLT)
			  vm_map_pageable(map, va, round_page(va+1), FALSE);
			if (type == T_PAGEFLT)
				return;
			goto out;
		}
nogo:
/*pg("nogo");*/
		if (type == T_PAGEFLT) {
			if (curpcb->pcb_onfault)
				goto copyfault;
			printf("vm_fault(%x, %x, %x, 0) -> %x\n",
			       map, va, ftype, rv);
			printf("  type %x, code %x\n",
			       type, code);
			goto bit_sucker;
		}
		i = (rv == KERN_PROTECTION_FAILURE) ? SIGBUS : SIGSEGV;
		break;
	    }

	case T_TRCTRAP:	 /* trace trap -- someone single stepping lcall's */
		frame.tf_eflags &= ~PSL_T;
if (um) {*(int *)PTmap &= 0xfffffffe; load_cr3(rcr3()); }

			/* Q: how do we turn it on again? */
		return;
	
	case T_BPTFLT|T_USER:		/* bpt instruction fault */
	case T_TRCTRAP|T_USER:		/* trace trap */
		frame.tf_eflags &= ~PSL_T;
		i = SIGTRAP;
		break;

#include "isa.h"
#if	NISA > 0
	case T_NMI:
	case T_NMI|T_USER:
		/* machine/parity/power fail/"kitchen sink" faults */
		if(isa_nmi(code) == 0) return;
		else goto bit_sucker;
#endif
	}
/*if(u.u_procp && (u.u_procp->p_pid == 1 || u.u_procp->p_pid == 3)) {
	if( *(u_char *) 0xf7c != 0xc7) {
		printf("%x!", *(u_char *) 0xf7c);
		*(u_char *) 0xf7c = 0xc7;
	}
}*/
	trapsignal(p, i, ucode);
	if ((type & T_USER) == 0)
		return;
out:
	while (i = CURSIG(p))
		psig(i);
	p->p_pri = p->p_usrpri;
	if (want_resched) {
		/*
		 * Since we are curproc, clock will normally just change
		 * our priority without moving us from one queue to another
		 * (since the running process is not on a queue.)
		 * If that happened after we setrq ourselves but before we
		 * swtch()'ed, we might not be on the queue indicated by
		 * our priority.
		 */
		(void) splclock();
		setrq(p);
		p->p_stats->p_ru.ru_nivcsw++;
		swtch();
		while (i = CURSIG(p))
			psig(i);
	}
	if (p->p_stats->p_prof.pr_scale) {
		int ticks;
		struct timeval *tv = &p->p_stime;

		ticks = ((tv->tv_sec - syst.tv_sec) * 1000 +
			(tv->tv_usec - syst.tv_usec) / 1000) / (tick / 1000);
		if (ticks) {
#ifdef PROFTIMER
			extern int profscale;
			addupc(frame.tf_eip, &p->p_stats->p_prof,
			    ticks * profscale);
#else
			addupc(frame.tf_eip, &p->p_stats->p_prof, ticks);
#endif
		}
	}
	curpri = p->p_pri;
	curpcb->pcb_flags &= ~FM_TRAP;	/* used by sendsig */
	spl0(); /*XXX*/
}

/*
 * syscall(frame):
 *	System call request from POSIX system call gate interface to kernel.
 * Like trap(), argument is call by reference.
 */
/*ARGSUSED*/
syscall(frame)
	volatile struct syscframe frame;
{
	register int *locr0 = ((int *)&frame);
	register caddr_t params;
	register int i;
	register struct sysent *callp;
	register struct proc *p = curproc;
	struct timeval syst;
	int error, opc;
	int args[8], rval[2];
int code;

#ifdef lint
	r0 = 0; r0 = r0; r1 = 0; r1 = r1;
#endif
	syst = p->p_stime;
	if (ISPL(frame.sf_cs) != SEL_UPL)
		panic("syscall");

	code = frame.sf_eax;
	p->p_regs = (int *)&frame;
	params = (caddr_t)frame.sf_esp + sizeof (int) ;

	/*
	 * Reconstruct pc, assuming lcall $X,y is 7 bytes, as it is always.
	 */
	opc = frame.sf_eip - 7;
	callp = (code >= nsysent) ? &sysent[63] : &sysent[code];
	if (callp == sysent) {
		i = fuword(params);
		params += sizeof (int);
		callp = (code >= nsysent) ? &sysent[63] : &sysent[code];
	}
dprintf(DALLSYSC,"%d. call %d ", p->p_pid, code);
	if ((i = callp->sy_narg * sizeof (int)) &&
	    (error = copyin(params, (caddr_t)args, (u_int)i))) {
		frame.sf_eax = error;
		frame.sf_eflags |= PSL_C;	/* carry bit */
#ifdef KTRACE
		if (KTRPOINT(p, KTR_SYSCALL))
			ktrsyscall(p->p_tracep, code, callp->sy_narg, &args);
#endif
		goto done;
	}
#ifdef KTRACE
	if (KTRPOINT(p, KTR_SYSCALL))
		ktrsyscall(p->p_tracep, code, callp->sy_narg, &args);
#endif
	rval[0] = 0;
	rval[1] = frame.sf_edx;
	error = (*callp->sy_call)(p, args, rval);
	if (error == ERESTART)
		frame.sf_eip = opc;
	else if (error != EJUSTRETURN) {
		if (error) {
			frame.sf_eax = error;
			frame.sf_eflags |= PSL_C;	/* carry bit */
		} else {
			frame.sf_eax = rval[0];
			frame.sf_edx = rval[1];
			frame.sf_eflags &= ~PSL_C;	/* carry bit */
		}
	}
	/* else if (error == EJUSTRETURN) */
		/* nothing to do */
done:
	/*
	 * Reinitialize proc pointer `p' as it may be different
	 * if this is a child returning from fork syscall.
	 */
	p = curproc;
	while (i = CURSIG(p))
		psig(i);
	p->p_pri = p->p_usrpri;
	if (want_resched) {
		/*
		 * Since we are curproc, clock will normally just change
		 * our priority without moving us from one queue to another
		 * (since the running process is not on a queue.)
		 * If that happened after we setrq ourselves but before we
		 * swtch()'ed, we might not be on the queue indicated by
		 * our priority.
		 */
		(void) splclock();
		setrq(p);
		p->p_stats->p_ru.ru_nivcsw++;
		swtch();
		while (i = CURSIG(p))
			psig(i);
	}
	if (p->p_stats->p_prof.pr_scale) {
		int ticks;
		struct timeval *tv = &p->p_stime;

		ticks = ((tv->tv_sec - syst.tv_sec) * 1000 +
			(tv->tv_usec - syst.tv_usec) / 1000) / (tick / 1000);
		if (ticks) {
#ifdef PROFTIMER
			extern int profscale;
			addupc(frame.sf_eip, &p->p_stats->p_prof,
			    ticks * profscale);
#else
			addupc(frame.sf_eip, &p->p_stats->p_prof, ticks);
#endif
		}
	}
	curpri = p->p_pri;
#ifdef KTRACE
	if (KTRPOINT(p, KTR_SYSRET))
		ktrsysret(p->p_tracep, code, error, rval[0]);
#endif
}
