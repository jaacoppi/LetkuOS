#include "multiboot.h"
#include "desctables.h"
#include "letkuos-common.h"
#include "irq.h"
#include "stdio.h"
#include "keyboard.h"
#include "portio.h"
#include "ata.h"
#include "fs.h"
#include "string.h"

void los_reboot();
void panic(const char *fmt, ...);

/* http://www.gnu.org/software/grub/manual/multiboot/multiboot.html */
/* The Multiboot information. Not complete, only the parts we care about */
struct multiboot_info bootinfo;

//////////////////////////////////////////////////////
// main - this is where the assembly routine brings us
//////////////////////////////////////////////////////
int main(struct multiboot_info *boot_info, int magic) {

init_video(); /* init the video device first so printf and others work for panic() */

/* see if the multiboot magic number is set correctly. If not, we're not sure we can trust the boot_info */
if (magic == MULTIBOOT_MAGIC)
	{
	bootinfo = *boot_info;
	/* complain about missing rootdevice boot parm in fs.c */
	}
else
	{
	printf("The multiboot magic number given by bootloader is: %x\n",magic);
	panic("No valid multiboot magic number found.\n");
	}

printf("%s (build %s)\n",CODENAME, REVID);
printf("%s\n", COPYRIGHT);
init_gdt();
init_idt();
init_irq();
init_keyboard();
init_ata();
init_vfs();

printf("%s booted and ready to go!\n", CODENAME);

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

/* something has gone horribly wrong, or you don't have the means to recover from a nonfatal error */

// use panic("Some error somewhere %d\n",ERRORCODE);

/* TODO:
MAKE FORMATTED OUTPUT WORK!
fancy color background,
a #defined list of errors,
enable rebooting */
void panic(const char *fmt, ...)
{
/* clear interrupts so we won't in trouble */
__asm__ __volatile__ ("cli");

printf("KERNEL PANIC, SYSTEM HALTED\n");
printf("Error message: ");
printf(fmt);
while(1) { __asm__ __volatile__ ("hlt"::);}
}


