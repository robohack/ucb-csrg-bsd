#ifndef lint
static	char *sccsid = "@(#)string.c	3.4 84/01/09";
#endif

#include "string.h"

char *malloc();
char *sprintf();

char *
str_cpy(s)
register char *s;
{
	char *str;
	register char *p;

	str = p = str_alloc(strlen(s) + 1);
	if (p == 0)
		return 0;
	while (*p++ = *s++)
		;
	return str;
}

char *
str_itoa(i)
int i;
{
	char buf[30];

	(void) sprintf(buf, "%d", i);
	return str_cpy(buf);
}

char *
str_cat(s1, s2)
char *s1, *s2;
{
	char *str;
	register char *p, *q;

	str = p = str_alloc(strlen(s1) + strlen(s2) + 1);
	if (p == 0)
		return 0;
	for (q = s1; *p++ = *q++;)
		;
	for (q = s2, p--; *p++ = *q++;)
		;
	return str;
}

/*
 * match s against p.
 * s can be a prefix of p with at least min characters.
 */
str_match(s, p, min)
register char *s, *p;
register min;
{
	for (; *s && *p && *s == *p; s++, p++, min--)
		;
	return *s == *p || *s == 0 && min <= 0;
}

#ifdef STR_DEBUG
struct string str_head = {
	&str_head, &str_head
};

char *
str_alloc(l)
int l;
{
	register struct string *s;

	s = (struct string *) malloc((unsigned)l + str_offset);
	if (s == 0)
		return 0;
	s->s_forw = str_head.s_forw;
	s->s_back = &str_head;
	str_head.s_forw = s;
	s->s_forw->s_back = s;
	return s->s_data;
}

str_free(str)
char *str;
{
	register struct string *s;

	for (s = str_head.s_forw; s != &str_head && s->s_data != str;
	     s = s->s_forw)
		;
	if (s == &str_head)
		abort();
	s->s_back->s_forw = s->s_forw;
	s->s_forw->s_back = s->s_back;
	free((char *)s);
}
#endif
