/* string.h */
#ifndef _LETKUOS_STRING_H
#define _LETKUOS_STRING_H _LETKUOS_STRING_H_

short *memsetw(short *dest, short val, int count);
void *memset(void *dest, char val, int count);
char *strcpy(char *dest, const char *src);
void * memcpy(void *dest, const void *src, int count);

#endif
