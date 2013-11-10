/* string.c */

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

