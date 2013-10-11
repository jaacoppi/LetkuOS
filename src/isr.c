/* isr.c -- High level interrupt service routines and interrupt request handlers. */
#include "isr.h"
#include "string.h"
#include "scrn.h" /* for writeline and writech */
// Part of this code is modified from Bran's kernel development tutorials.
// Rewritten for JamesM's kernel development tutorials.
//


// This gets called from our ASM interrupt handler stub.
/* TODO: write a function to write numbers - printf? */
void isr_handler(registers_t regs)
{
writeline("received interrupt: ");
while(1) {  __asm__ __volatile__ ("hlt"::); }

switch (regs.int_no)
        {
        case ERR_0:
        /* .. enter the rest here */
        default:
                writeline(regs.int_no); /* TODO: decimal, not string */


        }

   writech('\n');
}


