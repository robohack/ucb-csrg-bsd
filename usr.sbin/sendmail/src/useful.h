/*
**  USEFUL.H -- Some useful stuff.
**
**	@(#)useful.h	3.4	9/24/81
*/

# ifndef makedev
# include <sys/types.h>
# endif

/* support for bool type */
typedef char	bool;
# define TRUE	1
# define FALSE	0

# ifndef NULL
# define NULL	0
# endif NULL

/* bit hacking */
# define bitset(bit, word)	((word) & (bit))

/* define the types of some common functions */
extern char	*strcpy();
extern char	*strcat();
extern char	*malloc();
extern char	*index(), *rindex();
extern int	errno;
extern char	*sprintf();
extern long	time();
extern char	*ctime();
# ifndef V6
extern char	*getenv();
# else V6
typedef char	*u_short;
typedef long	u_long;
typedef char	u_char;
typedef int	void;
# endif V6
