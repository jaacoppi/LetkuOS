/* isr.c -- High level interrupt service routines and interrupt request handlers. */

#include "isr.h"
#include "string.h"
#include "stdio.h"
#include "letkuos-common.h"

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

int page_fault_handler(int errcode);



// This gets called from our ASM interrupt handler stub.
void isr_handler(registers_t regs)
{

switch (regs.int_no)
        {
	case 14:	// page fault
		{
		page_fault_handler(regs.err_code);
		break;
		}
        default:
		{
		printf("\nReceived an exception #%d (%s), with error code %d\n", regs.int_no, exception_messages[regs.int_no], regs.err_code);
//printf("gs: 0x%xh, fs 0x%xh, es 0x%xh, ds 0x%xh\n", regs.gs, regs.fs, regs.es, regs.ds);
//printf("edi 0x%xh, esi 0x%xh, ebp 0x%xh, esp 0x%xh, edx 0x%xh, ecx 0x%xh, eax 0x%xh\n",regs.edi,regs.esi,regs.ebp,regs.esp,regs.ebx,regs.edx,regs.ecx,regs.eax);
//printf("eip 0x%xh, cs 0x%xh, eflasgs 0x%xh, usersep 0x%xh, ss 0x%xh\n",regs.eip,regs.cs,regs.eflags,regs.useresp,regs.ss);

		panic("No interrupt handler code - system halted!\n");
		break;
		}
        }
}


/* Page fault handler */

/* theory and some practise from JamesM, http://jamesmolloy.co.uk/tutorial_html/6.-Paging.html

A cpu generates a page fault in the following situations:
1. Reading from or writing to an area of memory that is not mapped (page entry/table's 'present' flag is not set)
2. The process is in user-mode and tries to write to a read-only page.
3. The process is in user-mode and tries to access a kernel-only page.
4. The page table entry is corrupted - the reserved bits have been overwritten.

Error code meaning:
Bit 0 - If set, the fault was _not_ because the page wasn't present. If unset, the page wasn't present.
Bit 1 - If set, the operation that caused the fault was a write, else it was a read.
Bit 2 - If set, the processor was running in user-mode when it was interrupted. Else, it was running in kernel-mode.
Bit 3 - If set, the fault was caused by reserved bits being overwritten.
Bit 4 - If set, the fault occurred during an instruction fetch.
*/
int page_fault_handler(int errcode)
{
// get the details from the errorcode

int present   = (errcode & 0x1); // Page not present
int write = errcode & 0x2;           // Write operation?
int user = errcode & 0x4;           // Processor was in user-mode?
int reserved = errcode & 0x8;     // Overwritten CPU-reserved bits of page entry?
int id = errcode & 0x10;          // Caused by an instruction fetch?

// currently just print and panic since we don't know how to handle it

// The faulting address is stored in the CR2 register.
unsigned int faulting_address;
__asm__ __volatile__("mov %%cr2, %0" : "=r" (faulting_address));

printf("\nA page fault occurred at 0x%xh. Here's the details:\n",faulting_address);
if (present)
	printf("1. The page was present\n");
else
	printf("1. The page wasn't present\n");

if (write)
	printf("2. It was a write operation\n");
else
	printf("2. It was a read operation\n");

if (user)
	printf("3. The cpu was in user mode\n");
else
	printf("3. The cpu was in kernel mode\n");

if (reserved)
	printf("Also, tried to overwrite reserved bits\n");

if (id)
	printf("Also, happened during an instruction fetch\n");

panic("Don't know how to handle a page fault!");
}
