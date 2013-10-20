/* syscalls.c
* Mainly from Bran's kernel development tutorial, will be replaced later 
* includes LetkuOS syscalls
*/


#include "letkuos-common.h"

/* changed the loop from for in bkerndev
to while, no reason but liking whiles */


/* these two used for reading and writing ports */
unsigned char inb (unsigned short _port)
{
    unsigned char rv;
    __asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

void outb (unsigned short _port, unsigned char _data)
{
    __asm__ __volatile__ ("outb %1, %0" : : "dN" (_port), "a" (_data));
}

unsigned int inw(unsigned short _port)
{
unsigned short rv;
__asm__ __volatile__ ("inw %1,%0": "=a" (rv) : "dN"(_port));
return rv;
}

