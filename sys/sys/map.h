/*	map.h	3.1	11/15/19	*/

/*
 * Resource Allocation Maps
 */
struct map
{
	int	m_size;
	int	m_addr;
};

#ifdef KERNEL
struct	map swapmap[SMAPSIZ];	/* space for swap allocation */

struct	map kernelmap[NPROC];	/* space for kernel map for user page tables */
#endif
