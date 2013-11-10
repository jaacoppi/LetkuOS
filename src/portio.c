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

/* in16s used for reading x amount of words: use 512/2 as a count to read 512 bytes */
/*
can be used like this:
unsigned char buffer;
in16s(port, (512 / 2), buffer);
unsigned char *ptr = buffer;
return ptr;
*/
/* thanks for exclaimOS for the idea */
int in16s(unsigned short port, int count, unsigned char *buf) {
__asm__ volatile("rep insw" : "=c"(count), "=D"(buf) : "d"(port), "0"(count), "1"(buf) : "memory");
return 1;
}


unsigned int inw(unsigned short _port)
{
unsigned short rv;
__asm__ __volatile__ ("inw %1,%0": "=a" (rv) : "dN"(_port));
return rv;
}

