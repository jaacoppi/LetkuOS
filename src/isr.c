/* isr.c -- High level interrupt service routines and interrupt request handlers. */

#include "isr.h"
#include "string.h"
#include "stdio.h"
// Part of this code is modified from Bran's kernel development tutorials.
// Rewritten for JamesM's kernel development tutorials.
//

char *exception_messages[] =
{
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",
    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "Reserved-20",
    "Reserved-21",
    "Reserved-22",
    "Reserved-23",
    "Reserved-24",
    "Reserved-25",
    "Reserved-26",
    "Reserved-27",
    "Reserved-28",
   "Reserved-29",
    "Reserved-30",
    "Reserved-31"
};



// This gets called from our ASM interrupt handler stub.
/* TODO: write a function to write numbers - printf? */
void isr_handler(registers_t regs)
{
writeline("received interrupt: ");

switch (regs.int_no)
        {
        default:
                printf("%s\n", exception_messages[regs.int_no]); /* TODO: decimal, not string */


        }

printf("PANIC: no interrupt handler code - system halted!\n");
while(1) { __asm__ __volatile__ ("hlt"::); }
}

