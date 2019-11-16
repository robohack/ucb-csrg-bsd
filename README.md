# UCB CSRG's BSD SCCS to Git conversion by git-sccsimport

This is a conversion to Git of the original SCCS source code history
files from the University of California at Berkeley Computer Systems
Research Group for the Berkeley Software Distribution (i.e. from
CD-ROM-#4 of Marshall Kirk McKusick's [CSRG Archive
CD-ROMs](https://www.mckusick.com/csrg/index.html)).

_"Thanks to the efforts of the volunteers of the ``Unix Heritage Society''
(TUHS) and the willingness of Caldera to release 32/V under an open
source license, it is now possible to make the full source archives of
the University of California at Berkeley's Computer Systems Research
Group (CSRG) available."_

## Statistics

There are 12,625 SCCS files in the collection, of which 9 are imposters
(files with names beginning with "s.", but which are not SCCS files),
and another three which are marked as ".bad", all of which have repaired
version also available.

These remaining 12,613 valid (after a few manual fixups) SCCS files
contain a total of 105,024 deltas.  The import program is able to
automatically merge these into 53,987 Git commits.

## The Conversion Procedure

The conversion was performed with
[git-sccsimport](https://github.com/robohack/git-sccsimport)
using the following command (run in the root of the `disk4` filesystem):

	git-sccsimport --tz=-0800 --authormap=author.map --expand-kw --git-dir=/work/csrg-bsd --init --dirs .

The conversion takes about an hour on a NetBSD Xen VM running on a Dell
PE2950 with 2x4-core X5460 3.16GHz CPUs, with 16GB memory assigned to
the domU, and with the source files on an NFS mount, but probably in the
VM file cache.  (This is when using the Heirloom SCCS programs to access
the SCCS files.)


# Caveats

This conversion is not yet ideal, and it is also incomplete:

* Symbolic links are not preserved in this conversion.

* Source files without a corresponding SCCS file have not be included!
  (E.g. **many** files under `local`.)

* Branch deltas are interleaved as regular commits to the `master`
  branch in strict chronological order.

* The tags created by my `git-sccsimport` are probably not very useful
  for this repository (they mark were various files had their SID
  release level incremented).

* Jonathan Gray's [conversion](https://github.com/jonathangray/csrg) has
  108,615 commits -- I'm not yet sure why.  See also his
  [notes](https://github.com/jonathangray/csrg-git-patches).

Perhaps a future conversion will fix some or all of these issues.


# So, What's This Good For?

An SCCS to Git conversion of historical files adds a dimension to how
one can view the source as it changed over time and as a whole body.

This conversion in particular attempts to combine related commits across
the whole source body, and although this is currently only done by
simple mechanical means, it none the less reduces the total number of
commits by half.

	Greg A. Woods


# Original README from disk #4 (also in the file README)

This disk contains a snapshot of the /usr/src tree as it existed
on June 23, 1995, approximately one hour after the final 4.4BSD-Lite
Release 2 distribution from the Computer Systems Research group
was cut.  The project began using a source code control system (SCCS)
in 1980 (three years after Bill Joy began his initial Berkeley UNIX
development).  Initially only the kernel was put under SCCS, but
by 1983 everything had been put under SCCS.  The source code for
SCCS commands can be found in local/sccscmds.  Once built they can
be used to review the SCCS logs.

The list of contributors to the Berkeley Software Distribution
(BSD) runs to hundreds of names.  For a complete list, see Contrib.

Three other disks are included in this series; two disks with the
various BSD distributions and one disk with historic AT&T and other
distributions.  Please note that all four disks contain AT&T
proprietary (both copyright and trade secret) files.  Therefore,
these disks may not be redistributed without permission or license
from AT&T or its successors.

	Marshall Kirk McKusick
	Keith Bostic
