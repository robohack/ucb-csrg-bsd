#!/bin/sh -
#
# Copyright (c) 1988, 1993
#	The Regents of the University of California.  All rights reserved.
#
# %sccs.include.redist.sh%
#
#	@(#)fasthalt.sh	8.1 (Berkeley) 6/5/93
#

cp /dev/null /fastboot
/sbin/halt $*
