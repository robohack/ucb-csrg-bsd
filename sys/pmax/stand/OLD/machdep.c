/*
 * Copyright (c) 1988 University of Utah.
 * Copyright (c) 1992 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * the Systems Programming Group of the University of Utah Computer
 * Science Department, The Mach Operating System project at
 * Carnegie-Mellon University and Ralph Campbell.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)machdep.c	7.1 (Berkeley) 11/15/92
 */

/* from: Utah $Hdr: machdep.c 1.63 91/04/24$ */

#include <sys/param.h>
#include <pmax/dev/device.h>
#include <pmax/pmax/clockreg.h>
#include <pmax/include/machConst.h>
#include <pmax/stand/dec_prom.h>
#include <pmax/stand/samachdep.h>

const	struct callback *callv;
int	errno;

/*
 * Do all the stuff that locore normally does before calling main().
 * Process arguments passed to us by the prom monitor.
 * Return the first page address following the system.
 */
mach_init(argc, argv, code, cv)
	int argc;
	char *argv[];
	int code;
	const struct callback *cv;
{
	extern char MachUTLBMiss[], MachUTLBMissEnd[];
	extern char MachException[], MachExceptionEnd[];
	extern char edata[], end[];

	bzero(edata, end - edata);	/* clear bss */

	/*
	 * Copy down exception vector code.
	 */
	bcopy(MachUTLBMiss, (char *)MACH_UTLB_MISS_EXC_VEC,
		MachUTLBMissEnd - MachUTLBMiss);
	bcopy(MachException, (char *)MACH_GEN_EXC_VEC,
		MachExceptionEnd - MachException);

	/* check what type of PROM monitor we have */
	if (code != DEC_PROM_MAGIC)
		callv = &callvec;
#ifdef DS5000
	else
		callv = cv;
	{
		register struct pmax_ctlr *cp;
		register struct scsi_device *dp;
		register struct driver *drp;
		register int i;
		extern void tc_find_all_options();

		/* DS5000 3max */
		/* disable all TURBOchannel interrupts */
		i = *(volatile int *)MACH_SYS_CSR_ADDR;
		*(volatile int *)MACH_SYS_CSR_ADDR = i & ~(MACH_CSR_MBZ | 0xFF);

		/*
		 * Probe the TURBOchannel to see what controllers are present.
		 */
		tc_find_all_options();

		/* clear any memory errors from probes */
		*(unsigned *)MACH_ERROR_ADDR = 0;
	}
#endif

	initcpu();

	main(argc, argv, code, cv);
}

initcpu()
{
	register volatile struct chiptime *c;
	int i;

	/* disable clock interrupts (until startrtclock()) */
	c = (volatile struct chiptime *)MACH_CLOCK_ADDR;
	c->regb = REGB_DATA_MODE | REGB_HOURS_FORMAT;
	i = c->regc;
	spl0();		/* safe to turn interrupts on now */
	return (i);
}

panic(msg)
	char *msg;
{
	printf("panic: %s\n", msg);
	prom_restart();
}

#ifdef DS5000
/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS 
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 * 
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they make and grant Carnegie the
 * rights to redistribute these changes.
 */

#include "../pmax/turbochannel.h"

#define C (char *)

#if NASC > 0
extern struct driver ascdriver;

struct pmax_ctlr asc_ctlr = {
/*	driver,		unit,	addr,		pri,	flags */
	&ascdriver,	0,	C 0xffffffff,	-1,	0x0
};
#endif

/*
 * Driver map: associates a device driver to an option type.
 * Drivers name are (arbitrarily) defined in each driver and
 * used in the various config tables.
 */
struct drivers_map {
	char	module_name[TC_ROM_LLEN];	/* from ROM, literally! */
	struct	pmax_ctlr *ctlr;		/* device driver info */
	int	type;				/* class of device */
} tc_drivers_map[] = {
#if NASC > 0
	{ "PMAZ-AA ",	&asc_ctlr, TC_SCSI },	/* SCSI */
#endif

	{ "", 0 }			/* list end */
};

/*
 * TURBOchannel autoconf procedure. Finds in one sweep what is
 * hanging on the bus and fills in the tc_slot_info array.
 * This is only the first part of the autoconf scheme, at this
 * time we are basically only looking for a graphics board and
 * serial port to use as system console (all workstations).
 *
 * XXX Someday make number of slots dynamic too.
 */

#define KN02_TC_NSLOTS	8

tc_option_t	tc_slot_info[KN02_TC_NSLOTS];

caddr_t	tc_slot_virt_addr[] = {
	(caddr_t)0xbe000000,	/* TURBOchannel, slot 0 */
	(caddr_t)0xbe400000,	/* TURBOchannel, slot 1 */
	(caddr_t)0xbe800000,	/* TURBOchannel, slot 2 */
	(caddr_t)0xbec00000,	/* TURBOchannel, slot 3 */
	(caddr_t)0xbf000000,	/* TURBOchannel, slot 4 */
	(caddr_t)0xbf400000,	/* TURBOchannel, slot 5 */
	(caddr_t)0xbf800000,	/* TURBOchannel, slot 6 */
/*	(caddr_t)0xbfc00000,	   TURBOchannel, slot 7 */
};

void
tc_find_all_options()
{
	register int i;
	caddr_t addr;

	/*
	 * Look for all controllers on the bus.
	 */
	i = sizeof(tc_slot_virt_addr) / sizeof(tc_slot_virt_addr[0]) - 1;
	while (i >= 0) {
		addr = tc_slot_virt_addr[i];
		if (tc_probe_slot(addr, &tc_slot_info[i])) {
			/*
			 * Found a slot, make a note of it 
			 */
			tc_slot_info[i].present = 1;
			tc_slot_info[i].module_address = addr;
		} else
			tc_slot_info[i].present = 0;

		i -= tc_slot_info[i].slot_size;
	}
}

/*
 * Probe a slot in the TURBOchannel. Return TRUE if a valid option
 * is present, FALSE otherwise. A side-effect is to fill the slot
 * descriptor with the size of the option, whether it is
 * recognized or not.
 */
int
tc_probe_slot(addr, slot)
	caddr_t addr;
	tc_option_t *slot;
{
	int i;
	static unsigned tc_offset_rom[] = {
		TC_OFF_PROTO_ROM, TC_OFF_ROM
	};
#define TC_N_OFFSETS	sizeof(tc_offset_rom)/sizeof(unsigned)

	slot->slot_size = 1;

	for (i = 0; i < TC_N_OFFSETS; i++) {
		if (badaddr(addr + tc_offset_rom[i], 4))
			continue;
		/* complain only on last chance */
		if (tc_identify_option((tc_rommap_t *)(addr + tc_offset_rom[i]),
		    slot, i == (TC_N_OFFSETS-1)))
			return (1);
	}
	return (0);
#undef TC_N_OFFSETS
}

/*
 * Identify an option on the TURBOchannel.  Looks at the mandatory
 * info in the option's ROM and checks it.
 */
int
tc_identify_option(addr, slot, complain)
	tc_rommap_t *addr;
	tc_option_t *slot;
	int complain;
{
	register int i;
	unsigned char width;
	char firmwr[TC_ROM_LLEN+1];
	char vendor[TC_ROM_LLEN+1];
	char module[TC_ROM_LLEN+1];
	char host_type[TC_ROM_SLEN+1];

	/*
	 * We do not really use the 'width' info, but take advantage
	 * of the restriction that the spec impose on the portion
	 * of the ROM that maps between +0x3e0 and +0x470, which
	 * is the only piece we need to look at.
	 */
	width = addr->rom_width.value;
	switch (width) {
	case 1:
	case 2:
	case 4:
		break;

	default:
		if (complain)
			printf("Invalid ROM width (0x%x) at x%x\n",
			       width, addr);
		return (0);
	}

	if (addr->rom_stride.value != 4) {
		if (complain)
			printf("Invalid ROM stride (0x%x) at x%x\n",
			       addr->rom_stride.value, addr);
		return (0);
	}

	if (addr->test_data[0] != 0x55 ||
	    addr->test_data[4] != 0x00 ||
	    addr->test_data[8] != 0xaa ||
	    addr->test_data[12] != 0xff) {
		if (complain)
			printf("Test pattern failed, option at x%x\n",
			       addr);
		return (0);
	}

	for (i = 0; i < TC_ROM_LLEN; i++) {
		firmwr[i] = addr->firmware_rev[i].value;
		vendor[i] = addr->vendor_name[i].value;
		module[i] = addr->module_name[i].value;
		if (i >= TC_ROM_SLEN)
			continue;
		host_type[i] = addr->host_firmware_type[i].value;
	}

#if 0
	firmwr[TC_ROM_LLEN] = '\0';
	vendor[TC_ROM_LLEN] = '\0';
	module[TC_ROM_LLEN] = '\0';
	host_type[TC_ROM_SLEN] = '\0';
	printf("%s %s '%s' at x%x\n %s %s %s '%s'\n %s %d %s %d %s\n",
	       "Found a", vendor, module, addr,
	       "Firmware rev.", firmwr,
	       "diagnostics for a", host_type,
	       "ROM size is", addr->rom_size.value << 3,
	       "Kbytes, uses", addr->slot_size.value, "TC slot(s)");
#endif

	bcopy(module, slot->module_name, TC_ROM_LLEN);
	bcopy(vendor, slot->module_id, TC_ROM_LLEN);
	bcopy(firmwr, &slot->module_id[TC_ROM_LLEN], TC_ROM_LLEN);
	slot->slot_size = addr->slot_size.value;
	slot->rom_width = width;

	return (1);
}

struct pmax_ctlr *
tc_ctlr(ctlr, type)
	int ctlr;
	int type;
{
	register tc_option_t *sl = &tc_slot_info[ctlr];
	register struct drivers_map *map;
	register struct pmax_ctlr *cp;
	register struct driver *drp;

	/* check for valid controller */
	if (ctlr >= KN02_TC_NSLOTS || !sl->present)
		return ((struct pmax_ctlr *)0);

	/*
	 * Look for mapping between the module name and
	 * the device driver name.
	 */
	for (map = tc_drivers_map; map->ctlr; map++) {
		if (map->type == type &&
		    !bcmp(sl->module_name, map->module_name, TC_ROM_LLEN))
			goto fnd_map;
	}
	return ((struct pmax_ctlr *)0);

fnd_map:
	cp = map->ctlr;
	cp->pmax_addr = sl->module_address;
	cp->pmax_pri = ctlr;
	drp = cp->pmax_driver;
	if (!(*drp->d_init)(cp))
		return ((struct pmax_ctlr *)0);
	if (drp->d_intr) {
		if (intr_tab[ctlr].func)
			printf("%s: slot %d already in use\n",
				drp->d_name, ctlr);
		intr_tab[ctlr].func = drp->d_intr;
		intr_tab[ctlr].unit = cp->pmax_unit;
		tc_enable_interrupt(ctlr, 1);
	}
	cp->pmax_alive = 1;
	return (cp);
}

/*
 * Enable/Disable interrupts for a TURBOchannel slot.
 */
tc_enable_interrupt(slotno, on)
	register int slotno;
	int on;
{
	register volatile int *p_csr = (volatile int *)MACH_SYS_CSR_ADDR;
	int csr;
	int s;

	slotno = 1 << (slotno + MACH_CSR_IOINTEN_SHIFT);
	s = Mach_spl0();
	csr = *p_csr & ~(MACH_CSR_MBZ | 0xFF);
	if (on)
		*p_csr = csr | slotno;
	else
		*p_csr = csr & ~slotno;
	splx(s);
}

#endif /* DS5000 */
