/* paging.c
x86 Paged memory
*/

#include "letkuos-common.h"
#include "paging.h"
#include "mm.h"
#include "stdio.h"

/////////////////////////////////////////////
// Initialize paging
/////////////////////////////////////////////
void init_paging()
{
printf("Starting paging..\n");
int i; // for loops
int address = 0; // for identity mapping page entries / frames / whatever they're called. Should learn the terms.

// create your page directory and set all entries to not present

unsigned int *kernel_pagedir = kmalloc (PAGESIZE);
printf("created a kernel pagedir at 0x%xh\n",kernel_pagedir);

for (i = 0; i < 1024; i++)
	kernel_pagedir[i] = 0 |PAGE_NOTPRES_RW_KRN; // supervisor, read/write, not present

// create your page table and identity map the first 4mb to it
unsigned int *kernel_pagetable = kmalloc(PAGESIZE);

printf("created a kernel pagetable at 0x%xh\n", kernel_pagetable);

for (i = 0; i < 1024; i++)
	{
	// set bits present, read/write, supervisor only
	kernel_pagetable[i] = address |PAGE_PRES_RW_KRN;
	address = address + PAGESIZE;
	}


// set the first entry in page directory to point at our page table
// the kernel now has 4mb of identity mapped read/write memory for the kernel available from 0 to 4mb
kernel_pagedir[0] = kernel_pagetable;
kernel_pagedir[0] = kernel_pagedir[0]|PAGE_PRES_RW_KRN; // supervisor, read/write, present

// debug stuff
// printf("first entry in kernel pagedir now points to 0x%xh\n",kernel_pagedir[0]);

// actual initialization of paging in two steps
// 1. copy the physical address of the page directory to CR3
__asm__ __volatile__("mov %0, %%cr3":: "r"(kernel_pagedir));

// 2. read CR0 and set the PG bit in it
unsigned long cr0;
__asm__ __volatile__ ("mov %%cr0, %0": "=r"(cr0)); // get the dword
 cr0 |= 0x80000000; // Enable paging by setting bit 31
__asm__ __volatile__ ("mov %0, %%cr0":: "r"(cr0)); // write the dword back

printf("paging enabled!\n");

/* for testing a page fault
unsigned int *ptr = 0xA0000000;
unsigned int test = *ptr; // a read operation
*ptr = 'a'; // a write operation
*/
}

