#ifndef lint
static char sccsid[] = "@(#)getpwinfo.c	5.2 (Berkeley) 1/22/85";
#endif

#include "uucp.h"
#include <pwd.h>

/*******
 *	guinfo(uid, name, path)	get passwd file info for uid
 *	int uid;
 *	char *path, *name;
 *
 *	return codes:  0  |  FAIL
 *
 *	modified 3/16/81 to use the "real" login name -- mcnc!dennis
 *						(Dennis Rockwell)
 */

guinfo(uid, name, path)
int uid;
register char *path, *name;
{
	register struct passwd *pwd;
	struct passwd *getpwuid(), *getpwnam();
	char *getlogin(), *l;

	if ((l = getlogin()) != NULL) {
		pwd = getpwnam(l);
		if (pwd->pw_uid == uid || *l == 'U')
			goto setup;
	}
	if ((pwd = getpwuid(uid)) == NULL) {
		/* can not find uid in passwd file */
		*path = '\0';
		return FAIL;
	}

    setup:
	strcpy(path, pwd->pw_dir);
	strcpy(name, pwd->pw_name);
	return SUCCESS;
}


/*
 *	get passwd file info for name
 *
 *	return codes:  SUCCESS  |  FAIL
 */

gninfo(name, uid, path)
char *path, *name;
int *uid;
{
	register struct passwd *pwd;
	struct passwd *getpwnam();

	if ((pwd = getpwnam(name)) == NULL) {
		/* can not find name in passwd file */
		*path = '\0';
		return FAIL;
	}

	strcpy(path, pwd->pw_dir);
	*uid = pwd->pw_uid;
	return SUCCESS;
}
