.\" Copyright (c) 1983 Regents of the University of California.
.\" All rights reserved.  The Berkeley software License Agreement
.\" specifies the terms and conditions for redistribution.
.\"
.\"	@(#)1.5.t	6.1 (Berkeley) 5/8/86
.\"
.sh Descriptors
.PP
.NH 3
The reference table
.PP
Each process has access to resources through
\fIdescriptors\fP.  Each descriptor is a handle allowing
the process to reference objects such as files, devices
and communications links.
.PP
Rather than allowing processes direct access to descriptors, the system
introduces a level of indirection, so that descriptors may be shared
between processes.  Each process has a \fIdescriptor reference table\fP,
containing pointers to the actual descriptors.  The descriptors
themselves thus have multiple references, and are reference counted by the
system.
.PP
Each process has a fixed size descriptor reference table, where
the size is returned by the \fIgetdtablesize\fP call:
.DS
nds = getdtablesize();
result int nds;
.DE
and guaranteed to be at least 20.  The entries in the descriptor reference
table are referred to by small integers; for example if there
are 20 slots they are numbered 0 to 19.
.NH 3
Descriptor properties
.PP
Each descriptor has a logical set of properties maintained
by the system and defined by its \fItype\fP.
Each type supports a set of operations;
some operations, such as reading and writing, are common to several
abstractions, while others are unique.
The generic operations applying to many of these types are described
in section 2.1.  Naming contexts, files and directories are described in
section 2.2.  Section 2.3 describes communications domains and sockets.
Terminals and (structured and unstructured) devices are described
in section 2.4.
.NH 3
Managing descriptor references
.PP
A duplicate of a descriptor reference may be made by doing
.DS
new = dup(old);
result int new; int old;
.DE
returning a copy of descriptor reference \fIold\fP indistinguishable from
the original.  The \fInew\fP chosen by the system will be the
smallest unused descriptor reference slot.
A copy of a descriptor reference may be made in a specific slot
by doing
.DS
dup2(old, new);
int old, new;
.DE
The \fIdup2\fP call causes the system to deallocate the descriptor reference
current occupying slot \fInew\fP, if any, replacing it with a reference
to the same descriptor as old.
This deallocation is also performed by:
.DS
close(old);
int old;
.DE
.NH 3
Multiplexing requests
.PP
The system provides a
standard way to do
synchronous and asynchronous multiplexing of operations.
.PP
Synchronous multiplexing is performed by using the \fIselect\fP call:
.DS
nds = select(nd, in, out, except, tvp);
result int nds; int nd; result *in, *out, *except;
struct timeval *tvp;
.DE
The \fIselect\fP call examines the descriptors
specified by the
sets \fIin\fP, \fIout\fP and \fIexcept\fP, replacing
the specified bit masks by the subsets that select for input,
output, and exceptional conditions respectively (\fInd\fP
indicates the size, in bytes, of the bit masks).
If any descriptors meet the following criteria,
then the number of such descriptors is returned in \fInds\fP and the
bit masks are updated.
.if n .ds bu *
.if t .ds bu \(bu
.IP \*(bu
A descriptor selects for input if an input oriented operation
such as \fIread\fP or \fIreceive\fP is possible, or if a
connection request may be accepted (see section 2.3.1.4).
.IP \*(bu
A descriptor selects for output if an output oriented operation
such as \fIwrite\fP or \fIsend\fP is possible, or if an operation
that was ``in progress'', such as connection establishment,
has completed (see section 2.1.3).
.IP \*(bu
A descriptor selects for an exceptional condition if a condition
that would cause a SIGURG signal to be generated exists (see section 1.3.2).
.LP
If none of the specified conditions is true, the operation blocks for
at most the amount of time specified by \fItvp\fP, or waits for one of the
conditions to arise if \fItvp\fP is given as 0.
.PP
Options affecting i/o on a descriptor
may be read and set by the call:
.DS
._d
dopt = fcntl(d, cmd, arg)
result int dopt; int d, cmd, arg;

/* interesting values for cmd */
#define	F_SETFL	3	/* set descriptor options */
#define	F_GETFL	4	/* get descriptor options */
#define	F_SETOWN	5	/* set descriptor owner (pid/pgrp) */
#define	F_GETOWN	6	/* get descriptor owner (pid/pgrp) */
.DE
The F_SETFL \fIcmd\fP may be used to set a descriptor in 
non-blocking i/o mode and/or enable signalling when i/o is
possible.  F_SETOWN may be used to specify a process or process
group to be signalled when using the latter mode of operation.
.PP
Operations on non-blocking descriptors will
either complete immediately,
note an error EWOULDBLOCK,
partially complete an input or output operation returning a partial count,
or return an error EINPROGRESS noting that the requested operation is
in progress.
A descriptor which has signalling enabled will cause the specified process
and/or process group
be signaled, with a SIGIO for input, output, or in-progress
operation complete, or
a SIGURG for exceptional conditions.
.PP
For example, when writing to a terminal
using non-blocking output,
the system will accept only as much data as there is buffer space for
and return; when making a connection on a \fIsocket\fP, the operation may
return indicating that the connection establishment is ``in progress''.
The \fIselect\fP facility can be used to determine when further
output is possible on the terminal, or when the connection establishment
attempt is complete.
.NH 3
Descriptor wrapping.\(dg
.PP
.FS
\(dg The facilities described in this section are not included
in 4.3BSD.
.FE
A user process may build descriptors of a specified type by
\fIwrapping\fP a communications channel with a system supplied protocol
translator:
.DS
new = wrap(old, proto)
result int new; int old; struct dprop *proto;
.DE
Operations on the descriptor \fIold\fP are then translated by the
system provided protocol translator into requests on the underyling
object \fIold\fP in a way defined by the protocol.
The protocols supported by the kernel may vary from system to system
and are described in the programmers manual.
.PP
Protocols may be based on communications multiplexing or a rights-passing
style of handling multiple requests made on the same object.  For instance,
a protocol for implementing a file abstraction may or may not include
locally generated ``read-ahead'' requests.  A protocol that provides for
read-ahead may provide higher performance but have a more difficult
implementation.
.PP
Another example is the terminal driving facilities.  Normally a terminal
is associated with a communications line and the terminal type
and standard terminal access protocol is wrapped around a synchronous
communications line and given to the user.  If a virtual terminal
is required, the terminal driver can be wrapped around a communications
link, the other end of which is held by a virtual terminal protocol
interpreter.
