.\" Copyright (c) 1983, 1993
.\"	The Regents of the University of California.  All rights reserved.
.\"
.\" %sccs.include.redist.roff%
.\"
.\"	@(#)2.1.t	8.2 (Berkeley) 5/16/94
.\"
.Sh 2 "Generic operations
.PP
Many system abstractions support the
operations
.Fn read ,
.Fn write ,
and
.Fn ioctl .
We describe the basics of these common primitives here.
Similarly, the mechanisms whereby normally synchronous operations
may occur in a non-blocking or asynchronous fashion are
common to all system-defined abstractions and are described here.
.Sh 3 "Read and write
.PP
The
.Fn read
and
.Fn write
system calls can be applied to communications channels,
files, terminals and devices.
They have the form:
.DS
.Fd read 3 "read input
cc = read(fd, buf, nbytes);
result int cc; int fd; result caddr_t buf; int nbytes;
.DE
.DS
.Fd write 3 "write output
cc = write(fd, buf, nbytes);
result int cc; int fd; caddr_t buf; int nbytes;
.DE
The
.Fn read
call transfers as much data as possible from the
object defined by \fIfd\fP to the buffer at address \fIbuf\fP of
size \fInbytes\fP.  The number of bytes transferred is
returned in \fIcc\fP, which is \-1 if a return occurred before
any data was transferred because of an error or use of non-blocking
operations.
.PP
The
.Fn write
call transfers data from the buffer to the
object defined by \fIfd\fP.  Depending on the type of \fIfd\fP,
it is possible that the
.Fn write
call will accept some portion
of the provided bytes; the user should resubmit the other bytes
in a later request in this case.
Error returns because of interrupted or otherwise incomplete operations
are possible.
.PP
Scattering of data on input or gathering of data for output
is also possible using an array of input/output vector descriptors.
The type for the descriptors is defined in \fI<sys/uio.h>\fP as:
.DS
.TS
l s s s
l l l l.
struct iovec {
	caddr_t	iov_msg;	/* base of a component */
	int	iov_len;	/* length of a component */
};
.TE
.DE
The calls using an array of \fIiovec\fP structures are:
.DS
.Fd readv 3 "read gathered input
cc = readv(fd, iov, iovlen);
result int cc; int fd; struct iovec *iov; int iovlen;
.DE
.DS
.Fd writev 3 "write scattered output
cc = writev(fd, iov, iovlen);
result int cc; int fd; struct iovec *iov; int iovlen;
.DE
Here \fIiovlen\fP is the count of elements in the \fIiov\fP array.
.Sh 3 "Input/output control
.LP
Control operations on an object are performed by the
.Fn ioctl
operation:
.DS
.Fd ioctl 3 "control device
ioctl(fd, request, buffer);
int fd, request; caddr_t buffer;
.DE
This operation causes the specified \fIrequest\fP to be performed
on the object \fIfd\fP.  The \fIrequest\fP parameter specifies
whether the argument buffer is to be read, written, read and written,
or is not needed, and also the size of the buffer, as well as the
request.
Different descriptor types and subtypes within descriptor types
may use distinct
.Fn ioctl
requests. For example,
operations on terminals control flushing of input and output
queues and setting of terminal parameters; operations on
disks cause formatting operations to occur; operations on tapes
control tape positioning.
The names for basic control operations are defined in \fI<sys/ioctl.h>\fP.
.Sh 3 "Non-blocking and asynchronous operations
.PP
A process that wishes to do non-blocking operations on one of
its descriptors sets the descriptor in non-blocking mode as
described in section
.Xr 1.5.4 .
Thereafter the
.Fn read
call will return a specific EWOULDBLOCK error indication
if there is no data to be
.Fn read .
The process may
.Fn select
the associated descriptor to determine when a read is possible.
.PP
Output attempted when a descriptor can accept less than is requested
will either accept some of the provided data, returning a shorter than normal
length, or return an error indicating that the operation would block.
More output can be performed as soon as a
.Fn select
call indicates
the object is writeable.
.PP
Operations other than data input or output
may be performed on a descriptor in a non-blocking fashion.
These operations will return with a characteristic error indicating
that they are in progress
if they cannot complete immediately.  The descriptor
may then be
.Fn select 'ed
for
.Fn write
to find out when the operation has been completed.
When
.Fn select
indicates the descriptor is writeable, the operation has completed.
Depending on the nature of the descriptor and the operation,
additional activity may be started or the new state may be tested.
