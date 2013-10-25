/* irq.c - IRQ handling code */

/* code partly from: 
* bkerndev - Bran's Kernel Development Tutorial
*  By:   Brandon F. (friesenb@gmail.com)
*  Desc: Interrupt Request management
*
*  Notes: No warranty expressed or implied. Use at own risk. */

#include "stdio.h"
#include "letkuos-common.h"
#include "isr.h"
#include "portio.h"
#include "desctables.h"

/* These are own ISRs that point to our special IRQ handler
*  instead of the regular 'fault_handler' function */
extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();



/* This array is actually an array of function pointers. We use
*  this to handle custom IRQ handlers for a given IRQ */
/* by me: don't understand how this works, but in effect you can do 
stuff like clock_install() to set a function control over an IRQ */
void *irq_routines[16] =
{
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

/* This installs a custom IRQ handler for the given IRQ */
void irq_install_handler(int irq, void (*handler)(struct registers *r))
{
    irq_routines[irq] = handler;
}

/* This clears the handler for a given IRQ */
void irq_uninstall_handler(int irq)
{
    irq_routines[irq] = NULL;
}

/* Normally, IRQs 0 to 7 are mapped to entries 8 to 15. This
*  is a problem in protected mode, because IDT entry 8 is a
*  Double Fault! Without remapping, every time IRQ0 fires,
*  you get a Double Fault Exception, which is NOT actually
*  what's happening. We send commands to the Programmable
*  Interrupt Controller (PICs - also called the 8259's) in
*  order to make IRQ0 to 15 be remapped to IDT entries 32 to
*  47 */
void irq_remap(void)
{
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x0);
    outb(0xA1, 0x0);
}

/* We first remap the interrupt controllers, and then we install
*  the appropriate ISRs to the correct entries in the IDT. This
*  is just like installing the exception handlers */
void init_irq()
{
    irq_remap();
    idt_set_gate(32, (unsigned)irq0, 0x08, 0x8E);
    idt_set_gate(33, (unsigned)irq1, 0x08, 0x8E);
    idt_set_gate(34, (unsigned)irq2, 0x08, 0x8E);
    idt_set_gate(35, (unsigned)irq3, 0x08, 0x8E);
    idt_set_gate(36, (unsigned)irq4, 0x08, 0x8E);
    idt_set_gate(37, (unsigned)irq5, 0x08, 0x8E);
    idt_set_gate(38, (unsigned)irq6, 0x08, 0x8E);
    idt_set_gate(39, (unsigned)irq7, 0x08, 0x8E);

    idt_set_gate(40, (unsigned)irq8, 0x08, 0x8E);
    idt_set_gate(41, (unsigned)irq9, 0x08, 0x8E);
    idt_set_gate(42, (unsigned)irq10, 0x08, 0x8E);
    idt_set_gate(43, (unsigned)irq11, 0x08, 0x8E);
    idt_set_gate(44, (unsigned)irq12, 0x08, 0x8E);
    idt_set_gate(45, (unsigned)irq13, 0x08, 0x8E);
    idt_set_gate(46, (unsigned)irq14, 0x08, 0x8E);
    idt_set_gate(47, (unsigned)irq15, 0x08, 0x8E);

__asm__ __volatile__ ("sti");
}

/* Each of the IRQ ISRs point to this function, rather than
*  the 'fault_handler' in 'isrs.c'. The IRQ Controllers need
*  to be told when you are done servicing them, so you need
*  to send them an "End of Interrupt" command (0x20). There
*  are two 8259 chips: The first exists at 0x20, the second
*  exists at 0xA0. If the second controller (an IRQ from 8 to
*  15) gets an interrupt, you need to acknowledge the
*  interrupt at BOTH controllers, otherwise, you only send
*  an EOI command to the first controller. If you don't send
*  an EOI, you won't raise any more IRQs */
void irq_handler(struct registers *r)
{
    /* This is a blank function pointer */
    void (*handler)(struct registers *r);

    /* Find out if we have a custom handler to run for this
    *  IRQ, and then finally, run it */
    handler = irq_routines[r->int_no - 32];
    if (handler)
    {
        handler(r);
    }
/*
	else {
	printf("debug, unidentified IRQ %d\n", r->int_no - 32);
	}
*/
    /* If the IDT entry that was invoked was greater than 40
    *  (meaning IRQ8 - 15), then we need to send an EOI to
    *  the slave controller */
    if (r->int_no >= 40)
    {
        outb(0xA0, 0x20);
    }

    /* In either case, we need to send an EOI to the master
    *  interrupt controller too */
    outb(0x20, 0x20);

}

/* http://en.wikipedia.org/wiki/Interrupt_request
Basically, you'll want 0, 1, 6, 8 and 14

     * IRQ 0 - System timer. Reserved for the system. Cannot be changed 
	by a user.
     * IRQ 1 - Keyboard. Reserved for the system. Cannot be altered even
       if no keyboard is present or needed.
     * IRQ 2 - Cascaded signals from IRQs 8-15. A device configured to use
       IRQ 2 will actually be using IRQ 9
     * IRQ 3 - COM2 (Default) and COM4 (User) serial ports
     * IRQ 4 - COM1 (Default) and COM3 (User) serial ports
     * IRQ 5 - LPT2 Parallel Port 2 or sound card
     * IRQ 6 - Floppy disk controller
     * IRQ 7 - LPT1 Parallel Port 1 or sound card (8-bit Sound Blaster 
	and compatibles)

     * IRQ 8 - Real-time clock (RTC)
     * IRQ 9 - Free / Open interrupt / Available / SCSI. Any devices
       configured to use IRQ 2 will actually be using IRQ 9.
     * IRQ 10 - Free / Open interrupt / Available / SCSI
     * IRQ 11 - Free / Open interrupt / Available / SCSI
     * IRQ 12 - PS/2 connector Mouse. If no PS/2 connector mouse is 
	used, this can be used for other peripherals
     * IRQ 13 - ISA / Math co-processor
     * IRQ 14 - Primary IDE. If no Primary IDE this can be changed
     * IRQ 15 - Secondary IDE

*/
