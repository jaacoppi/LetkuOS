#ifndef _letkuos_portio_h
#define _letkuos_portio_h _letkuos_portio_h

unsigned char inb (unsigned short _port);
void outb (unsigned short _port, unsigned char _data);
int in16s(unsigned short port, int count, unsigned char *buf);
void msbtolsb(char *ptr, int count);

#endif

