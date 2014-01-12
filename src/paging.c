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
// create your page directory and set all entries to not present
printf("we're starting paging..\n");
page_directory_t *kernel_pagedir = kmalloc(sizeof(page_directory_t));
printf("kernel pagedir resides in 0x%xh\n",kernel_pagedir);

int i;
for (i = 0; i < 1024; i++)
	kernel_pagedir->direntries[i]->present = 0;

// create your page table and identity map the first 4mb to it
page_table_t *kernel_pagetable = kmalloc(sizeof(page_table_t));
printf("kernel pagetable resides in 0x%xh\n", kernel_pagetable);
int address = 0;
for (i = 0; i < 1024; i++)
	{
	// set bits present, read/write, supervisor only
	kernel_pagetable->pages[i].present = 1;
	kernel_pagetable->pages[i].rw = 1;
	kernel_pagetable->pages[i].user = 0;
	kernel_pagetable->pages[i].address = address >> 12; // remember, we store the 20 high bits, not 32
	address = address + PAGESIZE; // next page table entry shall control the next 4kb
	}

// set the first entry in page directory to point at our page table
//kernel_pagedir->direntries[0]->pagetable_addr = kernel_pagetable;
kernel_pagedir->direntries[0]->pagetable_addr = (int) kernel_pagetable >> 12; 
kernel_pagedir->direntries[0]->present = 1;
kernel_pagedir->direntries[0]->rw = 1;

// debug stuff
printf("first entry in kernel pagetable now points to 0x%xh\n",kernel_pagetable->pages[0].address);
printf("first entry in kernel pagedir now points to 0x%xh\n",kernel_pagedir->direntries[0]->pagetable_addr);

// register the page fault handler


// actual initialization in two steps

// 1. copy the physical address of the page directory to CR3
__asm__ __volatile__("mov %0, %%cr3":: "b"(kernel_pagedir));
// 2. set the PG bit in CR0

unsigned long cr0;
__asm__ __volatile__ ("mov %%cr0, %0": "=b"(cr0)); // get the bit
cr0 |= 0x80000000; // Enable paging by setting bit 31
//panic("waiting for writing cr0..");
__asm__ volatile ("int $0x2");
//i = 5 / 0;

__asm__ __volatile__ ("mov %0, %%cr0":: "b"(cr0)); // set the bit
panic("done");
}

