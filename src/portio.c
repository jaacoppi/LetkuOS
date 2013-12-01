/*
* portio.c
* read/write bytes, words or number of words to/from hardware registers.
* should be self explanatory, except for in16s, which could be named inwn for "in words (n)"
*/


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
unsigned char buffer[512];
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

/* msbtolsb takes a char string[6] = "214365" and changes the byte order to "123456" */
/* usefel when you use in16s or something similar to read MSB data to a struct or char array */
/*
char test[10] ="214365";
msbtolsb(test, sizeof(test));
printf("test: %s\n",test);
*/
void msbtolsb(char *ptr, int count)
{
int i;
for (i = 0; i < count / 2; i++)
        {
        char tmp = *ptr;
        ptr++;
        char tmp2 = *ptr;
        ptr--;
        *ptr = tmp2;
        ptr++;
        *ptr = tmp;
        ptr++;
        }
return;
}

