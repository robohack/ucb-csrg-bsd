/* Copyright (c) 1979 Regents of the University of California */

static char sccsid[] = "@(#)LINO.c 1.1 10/30/80";

#include "h00vars.h"
#include "h01errs.h"

LINO()
{
	if (++_stcnt >= _stlim) {
		ERROR(ESTLIM, _stcnt);
		return;
	}
}
