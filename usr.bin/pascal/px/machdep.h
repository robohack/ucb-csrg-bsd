/* Copyright (c) 1979 Regents of the University of California */

/* static 	char sccsid[] = "@(#)machdep.h 1.1 1/7/81"; */

#define pushaddr push4
#define popaddr (char *)pop4
#define popargs(num) popsp(num * sizeof(int))
