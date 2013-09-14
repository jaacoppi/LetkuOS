#include "../include/multiboot.h"

/* http://www.gnu.org/software/grub/manual/multiboot/multiboot.html */
/* The Multiboot information. Not complete, only the parts we care about 
*/
struct multiboot_info bootinfo;

//////////////////////////////////////////////////////
// main - this is where the assembly routine brings us
//////////////////////////////////////////////////////
int main(struct multiboot_info *boot_info, int magic) {

  *((unsigned char *) 0xB8000) = 'H';
    *((unsigned char *) 0xB8001) = 0x1F;
    *((unsigned char *) 0xB8002) = 'E';
    *((unsigned char *) 0xB8003) = 0x1F;
    *((unsigned char *) 0xB8004) = 'L';
    *((unsigned char *) 0xB8005) = 0x1F;
    *((unsigned char *) 0xB8006) = 'L';
    *((unsigned char *) 0xB8007) = 0x1F;
    *((unsigned char *) 0xB8008) = 'O';
    *((unsigned char *) 0xB8009) = 0x1F;

while(1) { __asm__ __volatile__ ("hlt"::); }
}



