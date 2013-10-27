#include "multiboot.h"
#include "desctables.h"
#include "letkuos-common.h"
#include "irq.h"
#include "stdio.h"
#include "keyboard.h"
#include "portio.h"

void los_reboot();
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
printf("%s\n", COPYRIGHT);

//__asm__ __volatile__ ("int $0x4");

while(1) { __asm__ __volatile__ ("hlt"::);}

}



/* reboot the CPU using the PS/2 controller */
void los_reboot()
{
/* if something goes wrong with the while loop we'll know.. */
printf("%s rebooting (hardware reset)", CODENAME);

/* clear interrupts so we won't in trouble */
__asm__ __volatile__ ("cli");

/* wait for the input buffer to clear - status register BIT1*/
/* TODO: a proper C loop */

while (true)
	{
	int i = inb(KEYB_CONTROL);
	if ((i & BIT1) == 0)
		break;
	}

/* send the reset signal */
outb(KEYB_CONTROL, 0xFE);
}
