/* desctables.c - functions to load GDT and IDT */
#include "desctables.h"
#include "letkuos-common.h"
#include "string.h"

/* Some code copied from:
James Malloy. http://www.jamesmolloy.co.uk/tutorial_html/4.-The%20GDT%20and%20IDT.html
Brandon Friesen. http://www.osdever.net/bkerndev/Docs/gdt.htm
*/

#define NUM_GDT 5
gdt_entry_t gdt_entries[NUM_GDT];
gdt_ptr_t gdt_ptr;
idt_entry_t idt_entries[NUM_IDT];
idt_ptr_t   idt_ptr;

extern int gdt_flush();

int init_gdt()
{
/* initialize the pointer */
gdt_ptr.limit = (sizeof(gdt_entry_t) * NUM_GDT) -1; /* the size of our gdt entry is 5 gdt entries */
// gdt_ptr.base = (u32int)&gdt_entries; /* store the gdt register from where ever gdt_entries begins */
gdt_ptr.base = (unsigned int)&gdt_entries; /* store the gdt register from where ever gdt_entries begins */

/* set the gates */
/* from intel manuals :
"The architecture also defines a set of special descriptors called gates (call gates, interrupt gates, trap gates, and
task gates). These provide protected gateways to system procedures and handlers that may operate at a different
privilege level than application programs and most procedures. For example, a CALL to a call gate can provide
access to a procedure in a code segment that is at the same or a numerically lower privilege level (more privileged)
than the current code segment. To access a procedure through a call gate, the calling procedure
supplies the selector for the call gate. The processor then performs an access rights check on thecall gate, comparing the CPL
with the privilege level of the call gate and the destination code segment pointed to by the call gate"
*/


/* TODO: understand
why the segment bases and limits are allowed to overlap - is it physical or virtual memory?
how binary and hexadecimal conversions, CF, 9A and so on work
*/

/* set the gates. Only the access byte changes. */
gdt_set_gate(NULL, NULL, NULL, NULL, NULL);                // NULL segment, required for some reason
gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // Ring 0 Code segment
gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // Ring 0 Data segment

// for some reason, these cause a triple fault with the mov ss, ax command in boot.asm
gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // Ring 3 (User mode) code segment
gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // Ring 3 (User mode) data segment


gdt_flush(); /* an asm routine to lgdt gdt_ptr and set up the segment registers */
return 1;
}

/* init one gdt_entry with parameters */
//static void gdt_set_gate(s32int num, u32int base, u32int limit, u8int access, u8int gran)
int gdt_set_gate(int num, unsigned long base, unsigned long limit, unsigned char access, unsigned char gran)
{
/* use bit manipulation to get the correct base address */
   gdt_entries[num].base_low    = (base & 0xFFFF);
   gdt_entries[num].base_middle = (base >> 16) & 0xFF;
   gdt_entries[num].base_high   = (base >> 24) & 0xFF;

/* use bit manipulation to get the correct limit (= size of GDT entry)*/
   gdt_entries[num].limit_low   = (limit & 0xFFFF);
   gdt_entries[num].granularity = (limit >> 16) & 0x0F;

/* use bit manipulation to get the correct granularity*/
   gdt_entries[num].granularity |= gran & 0xF0;

/* access byte is already masked at function call time */
   gdt_entries[num].access      = access;

return 1;
}

int init_idt()
{
   idt_ptr.limit = sizeof(idt_entry_t) * NUM_IDT -1;
   idt_ptr.base  = (int)&idt_entries;

   memset(&idt_entries, 0, sizeof(idt_entry_t)*NUM_IDT);

/* TODO: understand selector and flags */
idt_set_gate( 1, (int)isr1 , 0x08, 0x8E);
idt_set_gate( 2, (int)isr2 , 0x08, 0x8E);
idt_set_gate( 3, (int)isr3 , 0x08, 0x8E);
idt_set_gate( 4, (int)isr4 , 0x08, 0x8E);
idt_set_gate( 5, (int)isr5 , 0x08, 0x8E);
idt_set_gate( 6, (int)isr6 , 0x08, 0x8E);
idt_set_gate( 7, (int)isr7 , 0x08, 0x8E);
idt_set_gate( 8, (int)isr8 , 0x08, 0x8E);
idt_set_gate( 9, (int)isr9 , 0x08, 0x8E);
idt_set_gate( 10, (int)isr10 , 0x08, 0x8E);
idt_set_gate( 11, (int)isr11 , 0x08, 0x8E);
idt_set_gate( 12, (int)isr12 , 0x08, 0x8E);
idt_set_gate( 13, (int)isr13 , 0x08, 0x8E);
idt_set_gate( 14, (int)isr14 , 0x08, 0x8E);
idt_set_gate( 15, (int)isr15 , 0x08, 0x8E);
idt_set_gate( 16, (int)isr16 , 0x08, 0x8E);
idt_set_gate( 17, (int)isr17 , 0x08, 0x8E);
idt_set_gate( 18, (int)isr18 , 0x08, 0x8E);
idt_set_gate( 19, (int)isr19 , 0x08, 0x8E);
idt_set_gate( 20, (int)isr20 , 0x08, 0x8E);
idt_set_gate( 21, (int)isr21 , 0x08, 0x8E);
idt_set_gate( 22, (int)isr22 , 0x08, 0x8E);
idt_set_gate( 23, (int)isr23 , 0x08, 0x8E);
idt_set_gate( 24, (int)isr24 , 0x08, 0x8E);
idt_set_gate( 25, (int)isr25 , 0x08, 0x8E);
idt_set_gate( 26, (int)isr26 , 0x08, 0x8E);
idt_set_gate( 27, (int)isr27 , 0x08, 0x8E);
idt_set_gate( 28, (int)isr28 , 0x08, 0x8E);
idt_set_gate( 29, (int)isr29 , 0x08, 0x8E);
idt_set_gate( 30, (int)isr30 , 0x08, 0x8E);
idt_set_gate( 31, (int)isr31 , 0x08, 0x8E);


idt_flush((int)&idt_ptr);
return 1;

}



int idt_set_gate(unsigned char num, int base, unsigned short int sel, unsigned char flags)
{
   idt_entries[num].base_low = base & 0xFFFF;
   idt_entries[num].base_high = (base >> 16) & 0xFFFF;

   idt_entries[num].sel     = sel;
   idt_entries[num].reserved_0 = 0;
   // TODO: We must uncomment the OR below when we get to using user-mode.
   // It sets the interrupt gate's privilege level to 3.
   idt_entries[num].flags   = flags /* | 0x60 */;

return 1;
}
