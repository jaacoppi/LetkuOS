#include "scrn.h"
#include "multiboot.h"
#include "letkuos-common.h"

/* http://www.gnu.org/software/grub/manual/multiboot/multiboot.html */
/* The Multiboot information. Not complete, only the parts we care about 
*/
struct multiboot_info bootinfo;

//////////////////////////////////////////////////////
// main - this is where the assembly routine brings us
//////////////////////////////////////////////////////
int main(struct multiboot_info *boot_info, int magic) {

init_video();

writeline(CODENAME);
writeline(" (build ");
writeline(REVID);
writeline(")\n");
writeline(COPYRIGHT);
writeline("\n\nHELLO WORLD!");
while(1) { __asm__ __volatile__ ("hlt"::); }
}



