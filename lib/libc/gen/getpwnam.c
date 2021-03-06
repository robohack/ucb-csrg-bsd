#ifndef lint
static char sccsid[] = "@(#)getpwnam.c	5.1 (Berkeley) 6/5/85";
#endif not lint

#include <pwd.h>

struct passwd *
getpwnam(name)
char *name;
{
	register struct passwd *p;
	struct passwd *getpwent();

	setpwent();
	while( (p = getpwent()) && strcmp(name,p->pw_name) );
	endpwent();
	return(p);
}
