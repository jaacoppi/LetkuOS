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
