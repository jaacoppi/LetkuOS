#include "multiboot.h"
#include "desctables.h"
#include "letkuos-common.h"
#include "irq.h"
#include "stdio.h"
#include "keyboard.h"

/* http://www.gnu.org/software/grub/manual/multiboot/multiboot.html */
/* The Multiboot information. Not complete, only the parts we care about 
*/
struct multiboot_info bootinfo;

//////////////////////////////////////////////////////
// main - this is where the assembly routine brings us
//////////////////////////////////////////////////////
int main(struct multiboot_info *boot_info, int magic) {

init_video();
init_gdt();
init_idt();
init_irq();
init_keyboard();
printf("%s (build %s)\n",CODENAME, REVID);
printf("%s\n\n", COPYRIGHT);

//__asm__ __volatile__ ("int $0x4");

while(1) { __asm__ __volatile__ ("hlt"::);}

}



