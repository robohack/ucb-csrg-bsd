#!/bin/sh
#	@(#)plot.sh	4.2	(Berkeley)	11/11/83
#
PATH=/bin:/usr/bin:/usr/ucb
case $1 in
-T*)	t=$1
	shift ;;
*)	t=-T$TERM
esac
case $t in
-T450)			exec t450 $*;;
-T300)			exec t300 $*;;
-T300S|-T300s)		exec t300s $*;;
-Tver)			exec lpr -Pversatec -g $*;;
-Tvar)			exec lpr -Pvarian -g $*;;
-Ttek|-T4014|-T)	exec tek $* ;;
-Tbitgraph|-Tbg)	exec bgplot $*;;
-Tgigi|-Tvt125)		exec gigiplot $*;;
-Taed)			exec aedplot $*;;
-Thp7221|-Thp7|-Th7)	exec hp7221plot $*;;
-Thp|-T2648|-T2648a|-Thp2648|-Thp2648a|h8)
			exec hpplot $*;;
-Tcrt)			exec crtplot $*;;
-Tdumb|un|unknown)	exec dumbplot $*;;
*)  			exec crtplot $*;;
esac
