#
#	@(#)vprm.sh	1.1	(Berkeley)	8/9/82
#
set remote = ucbernie
set execdir = /usr/ucb
if ($remote != `hostname`) then
	set cmd = "/usr/ucb/rsh $remote -n"
else
	set cmd = ""
endif
$cmd $execdir/vprm $argv
