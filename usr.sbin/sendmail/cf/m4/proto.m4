divert(-1)
#
# Copyright (c) 1983 Eric P. Allman
# Copyright (c) 1988 The Regents of the University of California.
# All rights reserved.
#
# %sccs.include.redist.sh%
#
divert(0)

VERSIONID(`@(#)proto.m4	6.29 (Berkeley) 3/29/93')

MAILER(local)dnl

ifdef(`_OLD_SENDMAIL_', `dnl',
`# level 4 config file format
V4')

##################
#   local info   #
##################

Cwlocalhost
ifdef(`USE_CW_FILE',
`# file containing names of hosts for which we receive email
CONCAT(`Fw', confCW_FILE)', `dnl')

ifdef(`UUCP_RELAY',
`# UUCP relay host
CONCAT(DY, UUCP_RELAY)

')dnl
ifdef(`BITNET_RELAY',
`#  BITNET relay host
CONCAT(DB, BITNET_RELAY)

')dnl
ifdef(`CSNET_RELAY',
`# CSNET relay host
CONCAT(DC, CSNET_RELAY)

')dnl
ifdef(`FAX_RELAY',
`# FAX relay host
CONCAT(DF, FAX_RELAY)

')dnl
ifdef(`SMART_HOST',
`# "Smart" UUCP relay host
CONCAT(DS, SMART_HOST)

')dnl
ifdef(`MAILER_TABLE',
`# Mailer table (overriding domains)
Kmailertable MAILER_TABLE

')dnl
# who I send unqualified names to (null means deliver locally)
CONCAT(DR, ifdef(`LOCAL_RELAY', LOCAL_RELAY))

# who gets all local email traffic ($R has precedence for unqualified names)
CONCAT(DH, ifdef(`MAIL_HUB', MAIL_HUB))

# my official hostname ($w or $w.$D)
CONCAT(Dj$w, ifdef(`NEED_DOMAIN', .$D))

# who I masquerade as (can be $j)
CONCAT(DM, ifdef(`MASQUERADE_NAME', MASQUERADE_NAME, $j))

# class L: names that should be delivered locally, even if we have a relay
# class E: names that should be exposed as from this host, even if we masquerade
CLroot
CEroot
undivert(5)dnl

# operators that cannot be in local usernames (i.e., network indicators)
CO @ % ifdef(`_NO_UUCP_', `', `!')

# a class with just dot (for identifying canonical names)
C..

ifdef(`_OLD_SENDMAIL_', `dnl',
`# dequoting map
Kdequote dequote')

######################
#   Special macros   #
######################

# SMTP initial login message
CONCAT(De, confSMTP_LOGIN_MSG)

# UNIX initial From header format
CONCAT(Dl, confFROM_LINE)

# my name for error messages
CONCAT(Dn, confMAILER_NAME)

# delimiter (operator) characters
CONCAT(Do, confOPERATORS)

# format of a total name
CONCAT(Dq, ifdef(`confFROM_HEADER', confFROM_HEADER,
	ifdef(`_OLD_SENDMAIL_', `$g$?x ($x)$.', `$?x$x <$g>$|$g$.')))
include(`../m4/version.m4')

###############
#   Options   #
###############

# preserve 8 bits on message body on input?
CONCAT(O8, confEIGHT_BIT_INPUT)

# wait (in minutes) for alias file rebuild
CONCAT(Oa, confALIAS_WAIT)

# location of alias file
CONCAT(OA, ifdef(`ALIAS_FILE', ALIAS_FILE, /etc/aliases))

# minimum number of free blocks on filesystem
CONCAT(Ob, confMIN_FREE_BLOCKS)

# substitution for space (blank) characters
CONCAT(OB, confBLANK_SUB)

# connect to "expensive" mailers on initial submission?
CONCAT(Oc, confCON_EXPENSIVE)

# checkpoint queue runs after every N successful deliveries
CONCAT(OC, confCHECKPOINT_INTERVAL)

# default delivery mode
CONCAT(Od, confDELIVERY_MODE)

# automatically rebuild the alias database?
CONCAT(OD, confAUTO_REBUILD)

# error message header/file */
ifdef(`confERROR_MESSAGE',
	concat(OE, confERROR_MESSAGE),
	#OE/etc/sendmail.oE)

# error mode
ifdef(`confERROR_MODE',
	concat(Oe, confERROR_MODE),
	#Oep)

# save Unix-style "From_" lines at top of header?
CONCAT(Of, confSAVE_FROM_LINES)

# temporary file mode
CONCAT(OF, confTEMP_FILE_MODE)

# match recipients against GECOS field?
CONCAT(OG, confMATCH_GECOS)

# default GID
CONCAT(Og, confDEF_GROUP_ID)

# maximum hop count
CONCAT(Oh, confMAX_HOP)

# location of help file
CONCAT(OH, ifdef(`HELP_FILE', HELP_FILE, /usr/lib/sendmail.hf))

# ignore dots as terminators in incoming messages?
CONCAT(Oi, confIGNORE_DOTS)

# Insist that the BIND name server be running to resolve names
ifdef(`confBIND_OPTS',
	CONCAT(OI, confBIND_OPTS),
	#OI)

# Forward file search path
ifdef(`confFORWARD_PATH',
	CONCAT(OJ, confFORWARD_PATH),
	#OJ/var/forward/$u:$z/.forward.$w:$z/.forward)

# open connection cache size
CONCAT(Ok, confMCI_CACHE_SIZE)

# open connection cache timeout
CONCAT(OK, confMCI_CACHE_TIMEOUT)

# log level
CONCAT(OL, confLOG_LEVEL)

# send to me too, even in an alias expansion?
CONCAT(Om, confME_TOO)

# verify RHS in newaliases?
CONCAT(On, confCHECK_ALIASES)

# default messages to old style headers if no special punctuation?
CONCAT(Oo, confOLD_STYLE_HEADERS)

# privacy flags
CONCAT(Op, confPRIVACY_FLAGS)

# who (if anyone) should get extra copies of error messages
ifdef(`confCOPY_ERRORS_TO',
	CONCAT(OP, confCOPY_ERRORS_TO),
	#OPPostmaster)

# slope of queue-only function
ifdef(`confQUEUE_FACTOR',
	CONCAT(Oq, confQUEUE_FACTOR),
	#Oq600000)

# queue directory
CONCAT(OQ, ifdef(`QUEUE_DIR', QUEUE_DIR, /var/spool/mqueue))

# read timeout -- now OK per RFC 1123 section 5.3.2
ifdef(`confREAD_TIMEOUT',
	CONCAT(Or, confREAD_TIMEOUT),
	#Ordatablock=10m)

# queue up everything before forking?
CONCAT(Os, confSAFE_QUEUE)

# status file
CONCAT(OS, ifdef(`STATUS_FILE', STATUS_FILE, /etc/sendmail.st))

# default message timeout interval
CONCAT(OT, confMESSAGE_TIMEOUT)

# time zone handling:
#  if undefined, use system default
#  if defined but null, use TZ envariable passed in
#  if defined and non-null, use that info
ifelse(confTIME_ZONE, `USE_SYSTEM', `#Ot',
	confTIME_ZONE, `USE_TZ', `',
	`CONCAT(Ot, confTIME_ZONE)')

# default UID
CONCAT(Ou, confDEF_USER_ID)

# list of locations of user database file (null means no lookup)
OU`'ifdef(`confUSERDB_SPEC', `confUSERDB_SPEC')

# load average at which we just queue messages
CONCAT(Ox, confQUEUE_LA)

# load average at which we refuse connections
CONCAT(OX, confREFUSE_LA)

# work recipient factor
ifdef(`confWORK_RECIPIENT_FACTOR',
	CONCAT(Oy, confWORK_RECIPIENT_FACTOR),
	#Oy30000)

# deliver each queued job in a separate process?
CONCAT(OY, confSEPARATE_PROC)

# work class factor
ifdef(`confWORK_CLASS_FACTOR',
	CONCAT(Oz, confWORK_CLASS_FACTOR),
	#Oz1800)

# work time factor
ifdef(`confWORK_TIME_FACTOR',
	CONCAT(OZ, confWORK_TIME_FACTOR),
	#OZ90000)

###########################
#   Message precedences   #
###########################

Pfirst-class=0
Pspecial-delivery=100
Plist=-30
Pbulk=-60
Pjunk=-100

#####################
#   Trusted users   #
#####################

Troot
Tdaemon
Tuucp

#########################
#   Format of headers   #
#########################

H?P?Return-Path: $g
HReceived: $?sfrom $s $.by $j ($v/$Z) id $i; $b
H?D?Resent-Date: $a
H?D?Date: $a
H?F?Resent-From: $q
H?F?From: $q
H?x?Full-Name: $x
HSubject:
# HPosted-Date: $a
# H?l?Received-Date: $b
H?M?Resent-Message-Id: <$t.$i@$j>
H?M?Message-Id: <$t.$i@$j>
undivert(6)dnl
#
######################################################################
######################################################################
#####
#####			REWRITING RULES
#####
######################################################################
######################################################################

undivert(9)dnl

###########################################
###  Rulset 3 -- Name Canonicalization  ###
###########################################
S3

# handle null input and list syntax (translate to <@> special case)
R$@			$@ <@>
R$*:;$*			$@ $1 :; <@>

# basic textual canonicalization -- note RFC733 heuristic here
R$*<$*>$*<$*>$*		<$2>$3$4$5			strip multiple <> <>
R$*<$*<$+>$*>$*		<$3>$5				2-level <> nesting
R$*<>$*			$@ <@>				MAIL FROM:<> case
R$*<$+>$*		$2				basic RFC821/822 parsing

# make sure <@a,@b,@c:user@d> syntax is easy to parse -- undone later
R@ $+ , $+		@ $1 : $2			change all "," to ":"

# localize and dispose of route-based addresses
R@ $+ : $+		$@ $>6 < @$1 > : $2		handle <route-addr>

# find focus for list syntax
R $+ : $* ; @ $+	$@ $>6 $1 : $2 ; < @ $3 >	list syntax
R $+ : $* ;		$@ $1 : $2;			list syntax

# find focus for @ syntax addresses
R$+ @ $+		$: $1 < @ $2 >			focus on domain
R$+ < $+ @ $+ >		$1 $2 < @ $3 >			move gaze right
R$+ < @ $+ >		$@ $>6 $1 < @ $2 >		already canonical

ifdef(`_NO_UUCP_', `dnl',
`# convert old-style addresses to a domain-based address
R$- ! $+		$@ $>6 $2 < @ $1 .UUCP >	resolve uucp names
R$+ . $- ! $+		$@ $>6 $3 < @ $1 . $2 >		domain uucps
R$+ ! $+		$@ $>6 $2 < @ $1 .UUCP >	uucp subdomains')

# if we have % signs, take the rightmost one
R$* % $*		$1 @ $2				First make them all @s.
R$* @ $* @ $*		$1 % $2 @ $3			Undo all but the last.
R$* @ $*		$@ $>6 $1 < @ $2 >		Insert < > and finish

# else we must be a local name


###############################################
###  Ruleset 6 -- bottom half of ruleset 3  ###
###############################################

#  At this point, everything should be in a "local_part<@domain>extra" format.
S6

# handle special cases for local names
R$* < @ $=w > $*		$: $1 < @ $j . > $3		no domain at all
R$* < @ $=w . UUCP > $*		$: $1 < @ $j . > $3		.UUCP domain
undivert(2)dnl

ifdef(`UUCP_RELAY',
`# pass UUCP addresses straight through
R$* < @ $+ . UUCP > $*		$@ $1 < @ $2 . UUCP > $3',
`# if really UUCP, handle it immediately
ifdef(`_CLASS_U_',
`R$* < @ $=U . UUCP > $*	$@ $1 < @ $2 . UUCP > $3', `dnl')
ifdef(`_CLASS_V_',
`R$* < @ $=V . UUCP > $*	$@ $1 < @ $2 . UUCP > $3', `dnl')
ifdef(`_CLASS_W_',
`R$* < @ $=W . UUCP > $*	$@ $1 < @ $2 . UUCP > $3', `dnl')
ifdef(`_CLASS_X_',
`R$* < @ $=X . UUCP > $*	$@ $1 < @ $2 . UUCP > $3', `dnl')
ifdef(`_CLASS_Y_',
`R$* < @ $=Y . UUCP > $*	$@ $1 < @ $2 . UUCP > $3', `dnl')

# try UUCP traffic as a local address
R$* < @ $+ . UUCP > $*		$: $1 < @ $[ $2 $] . UUCP > $3
ifdef(`_OLD_SENDMAIL_',
`R$* < @ $+ . $+ . UUCP > $*		$@ $1 < @ $2 . $3 . > $4',
`R$* < @ $+ . . UUCP > $*		$@ $1 < @ $2 . > $3')')

# pass to name server to make hostname canonical
R$* < @ $* $~. > $*		$: $1 < @ $[ $2 $3 $] > $4

# handle possible alternate names
R$* < @ $=w . $m . > $*		$: $1 < @ $j . > $3
R$* < @ $=w . $m > $*		$: $1 < @ $j . > $3
undivert(8)dnl

# if this is the local hostname, make sure we treat is as canonical
R$* < @ $j > $*			$: $1 < @ $j . > $2


##################################################
###  Ruleset 4 -- Final Output Post-rewriting  ###
##################################################
S4

R$*<@>			$@ $1				handle <> and list:;

# resolve numeric addresses to name if possible
R$* < @ [ $+ ] > $*	$: $1 < @ $[ [$2] $] > $3	lookup numeric internet addr

# strip trailing dot off possibly canonical name
R$* < @ $+ . > $*	$1 < @ $2 > $3

# externalize local domain info
R$* < $+ > $*		$1 $2 $3			defocus
R@ $+ : @ $+ : $+	@ $1 , @ $2 : $3		<route-addr> canonical
R@ $*			$@ @ $1				... and exit

ifdef(`_NO_UUCP_', `dnl',
`# UUCP must always be presented in old form
R$+ @ $- . UUCP		$2!$1				u@h.UUCP => h!u')

# delete duplicate local names
R$+ % $=w @ $=w		$1 @ $j				u%host@host => u@host



#############################################################
###   Ruleset 7 -- recanonicalize and call ruleset zero   ###
###		   (used for recursive calls)		  ###
#############################################################

S7
R$*			$: $>3 $1
R$*			$@ $>0 $1


######################################
###   Ruleset 0 -- Parse Address   ###
######################################

S0

R<@>			$#local $: <>			special case error msgs

ifdef(`_MAILER_smtp_',
`# handle numeric address spec
R$* < @ [ $+ ] > $*	$: $1 < @ $[ [$2] $] > $3	numeric internet addr
R$* < @ [ $+ ] > $*	$#smtp $@ [$2] $: $1 @ [$2] $3	numeric internet spec',
`dnl')

# now delete the local info -- note $=O to find characters that cause forwarding
R$* < @ > $*		$@ $>7 $1			user@ => user
R< @ $j . > : $*	$@ $>7 $1			@here:... -> ...
R$* $=O $* < @ $j . >	$@ $>7 $1 $2 $3			...@here -> ...
ifdef(`MAILER_TABLE',
`
# try mailer table lookup
R$* < @ $+ > $*		$: $1 < @ $(mailertable $2 $) > $3
R$* < @ $-:$+ > $*	$# $2 $@ $3 $: $1 < @ $3 > $4	found a match',
`dnl')

# short circuit local delivery so forwarded email works
ifdef(`_LOCAL_NOT_STICKY_',
`R$=L < @ $j . >		$#local $: @ $1			special local names
R$+ < @ $j . >		$#local $: $1			dispose directly',
`R$+ < @ $j . >		$: $1 < @ $j @ $H >		first try hub
ifdef(`_OLD_SENDMAIL_',
`R$+ < $+ @ $-:$+ >	$# $3 $@ $4 $: $1 < $2 >	yep ....
R$+ < $+ @ $+ >		$#relay $@ $3 $: $1 < $2 >	yep ....
R$+ < $+ @ >		$#local $: $1			nope, local address',
`R$+ < $+ @ $+ >		$#local $: $1			yep ....
R$+ < $+ @ >		$#local $: @ $1			nope, local address')')
undivert(3)dnl
undivert(4)dnl

# resolve remotely connected UUCP links (if any)
ifdef(`_CLASS_V_',
`R$* < @ $=V . UUCP > $*		$#smtp $@ $V $: <@ $V> : $1 @ $2.UUCP $3',
	`dnl')
ifdef(`_CLASS_W_',
`R$* < @ $=W . UUCP > $*		$#smtp $@ $W $: <@ $W> : $1 @ $2.UUCP $3',
	`dnl')
ifdef(`_CLASS_X_',
`R$* < @ $=X . UUCP > $*		$#smtp $@ $X $: <@ $X> : $1 @ $2.UUCP $3',
	`dnl')

# resolve fake top level domains by forwarding to other hosts
ifdef(`BITNET_RELAY',
`R$*<@$+.BITNET>$*	$#smtp $@ $B $: $1 <@$2.BITNET> $3	user@host.BITNET',
	`dnl')
ifdef(`CSNET_RELAY',
`R$*<@$+.CSNET>$*	$#smtp $@ $C $: $1 <@$2.CSNET> $3	user@host.CSNET',
	`dnl')
ifdef(`_MAILER_fax_',
`R$+ < @ $+ .FAX >	$#fax $@ $2 $: $1			user@host.FAX',
`ifdef(`FAX_RELAY',
`R$*<@$+.FAX>$*		$#smtp $@ $F $: $1 <@$2.FAX> $3		user@host.FAX',
	`dnl')')

ifdef(`UUCP_RELAY',
`# forward non-local UUCP traffic to our UUCP relay
R$*<@$*.UUCP>$*		$#smtp $@ $Y $: <@ $Y> : $1 @ $2.UUCP $3	uucp mail',
`ifdef(`_MAILER_uucp_',
`# forward other UUCP traffic straight to UUCP
R< @ $+ .UUCP > : $+	$#uucp $@ $1 $: $1:$2			@host.UUCP:...
R$+ < @ $+ .UUCP >	$#uucp $@ $2 $: $1			user@host.UUCP',
	`dnl')')

ifdef(`_LOCAL_RULES_',
`# figure out what should stay in our local mail system
undivert(1)',
`ifdef(`_MAILER_smtp_',
`# deal with other remote names
R$* < @ $* > $*		$#smtp $@ $2 $: $1 < @ $2 > $3		user@host.domain')')
ifdef(`SMART_HOST', `
# pass names that still have a host to a smarthost
R$* < @ $* > $*		$: < $S > $1 < @ $2 > $3	glue on smarthost name
R<$-:$+> $* < @$* > $*	$# $1 $@ $2 $: $3 < @ $4 > $5	if non-null, use it
R<$+> $* < @$* > $*	$#suucp $@ $1 $: $2 < @ $3 > $4	if non-null, use it
R<> $* < @ $* > $*	$1 < @ $2 > $3			else strip off gunk',
`ifdef(`_LOCAL_RULES_', `
# reject messages that have host names we do not understand
R$* < @ $* > $*		$#error $@ NOHOST $: Unrecognized host name $2',
`dnl')')
ifdef(`_MAILER_USENET_', `
# addresses sent to net.group.USENET will get forwarded to a newsgroup
R$+ . USENET		$# usenet $: $1')

ifdef(`_OLD_SENDMAIL_',
`# forward remaining names to local relay, if any
R$=L			$#local $: $1			special local names
R$+			$: $1 < @ $R >			append relay
R$+ < @ >		$: $1 < @ $H >			no relay, try hub
R$+ < @ >		$#local $: $1			no relay or hub: local
R$+ < @ $j  >		$#local $: $1			we are relay/hub: local
R$+ < @ $-:$+ >		$# $2 $@ $3 $: $1		deliver to relay/hub
R$+ < @ $+ >		$#relay $@ $2 $: $1		deliver to relay/hub',

`# if this is quoted, strip the quotes and try again
R$+			$: $(dequote $1 $)		strip quotes
R$* $=O $*		$@ $>7 $1 $2 $3			try again

# handle locally delivered names
R$=L			$#local $: @ $1			special local names
R$+			$#local $: $1			regular local names

###########################################################################
###   Ruleset 5 -- special rewriting after aliases have been expanded   ###
###		   (new sendmail only)					###
###########################################################################

S5

# see if we have a relay or a hub
R$+			$: $1 < @ $R >
R$+ < @ >		$: $1 < @ $H >			no relay, try hub
R$+ < @ $j >		$@ $1				we are relay/hub: local
R$+ < @ $-:$+ >		$# $2 $@ $3 $: $1		send to relay or hub
ifdef(`_MAILER_smtp_',
`R$+ < @ $+ >		$#relay $@ $2 $: $1		send to relay or hub')')
#
######################################################################
######################################################################
#####
`#####			MAILER DEFINITIONS'
#####
######################################################################
######################################################################
undivert(7)dnl
