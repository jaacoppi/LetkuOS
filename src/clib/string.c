/* string.c */
#include "string.h"

short *memsetw(short *dest, short val, int count)
{
    unsigned short *temp = (unsigned short *)dest;
    for( ; count != 0; count--) *temp++ = val;
    return dest;
}

void *memset(void *dest, char val, int count) {
    char *temp = (char *)dest;
while (count != 0) {
        *temp++= val;
        count--;
        }
    return dest;
}

/* from wikipedia */
char *strcpy(char *dest, const char *src)
{
   const char *p;
   char *q;

   for(p = src, q = dest; *p != '\0'; p++, q++)
     *q = *p;

   *q = '\0';

   return dest;
}

void * memcpy(void *dest, const void *src, int count)
{
    const char *sp = (const char *)src;
    char *dp = (char *)dest;
    for(; count != 0; count--) *dp++ = *sp++;
    return dest;
}

char *strstr(char *s1, const char *s2)
{
    int len = strlen(s2);
    while(*s1)
        if(!memcmp(s1++,s2,len))
            return s1 -1;
    return 0;
}

int memcmp(const void* s1, const void* s2,int len)
{
    const unsigned char *p1 = s1, *p2 = s2;
    while(len--)
        if( *p1 != *p2 )
            return *p1 - *p2;
        else
            *p1++,*p2++; // gives a "value computed not used" error from GCC. what can we do? 
    return 0;
}

int strlen(const char *str)
{
    int retval;
    for(retval = 0; *str != '\0'; str++)
        retval++;

    return retval;
}

// from wikipedia: (P.J. Plauger, The Standard C Library, 1992)
int strcmp (const char *s1, const char *s2)
{

    for(; *s1 == *s2; ++s1, ++s2)
        if(*s1 == 0)
            return 0;
    return *(unsigned char *)s1 < *(unsigned char *)s2 ? -1 : 1;	// no idea what this means..
}

