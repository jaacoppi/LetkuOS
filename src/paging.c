/* paging.c
x86 Paged memory
*/

#include "letkuos-common.h"
#include "paging.h"
#include "mm.h"
#include "stdio.h"

unsigned int *kernel_pagedir;
int paging_findpte(int virtaddr);

int paging_enabled = 0;

/////////////////////////////////////////////
// Initialize paging
/////////////////////////////////////////////
void init_paging()
{
int i; // for loops
int address = 0; // for identity mapping page entries / frames / whatever they're called. Should learn the terms.

// create your page directory and set all entries to not present
kernel_pagedir = kmalloc (PAGESIZE);
printf("created a kernel pagedir at 0x%xh\n",kernel_pagedir);
for (i = 0; i < 1024; i++)
	kernel_pagedir[i] = 0 |PAGE_NOTPRES_RW_KRN; // supervisor, read/write, not present

// create your kernel page table and identity map the first 4mb to it
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
kernel_pagedir[0] = (int) kernel_pagetable;
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
// a dirty hack with findpte()
paging_enabled = 1;

/* for testing a page fault
unsigned int *ptr = 0xA0000000;
unsigned int test = *ptr; // a read operation
*ptr = 'a'; // a write operation
*/
}



//////////////////////////////////////////////////////////////////////////////////
// Map a virtual address space to a new virtual address space
//////////////////////////////////////////////////////////////////////////////////
int paging_remap(int newvirt, int oldvirt)
{
// this function copies address spaces, not singleaddresses. Thus, utilize vmmlist

// find out which listitem holds this memory area
struct vmmlist *curptr = vmmlist_start;
while (curptr->base != oldvirt)
	curptr = curptr->next;

// get the correct pte
unsigned int *newpte, *oldpte;

int i, j = 0;
for (i = curptr->base; i < (curptr->base + curptr->size); i = i + PAGESIZE)
        {
	oldpte = paging_findpte(i); // we're looping with curptr, so this is okay
	// the new virtual address might not be initialized..
	newpte = paging_findpte(newvirt + j * PAGESIZE);
	j++;
	printf("remapped 0x%xh to 0x%xh\n",oldpte,newpte);
	// copy the contents of oldvirt PTE to newvirt PTE
	memcpy(newpte, oldpte, 32); // hard coded 32 bits, is this wise?
        }

// adjust the vmm
curptr->base = newvirt;

// remove oldvirt from use
paging_present(oldvirt, 0);
printf("debug: exiting remap()\n");
}

//////////////////////////////////////////////////////////////////////////////////
// Map a virtual address to a physical address
// with this function, we can have a contiguous virtual address space even though
// the physical memory is fragmented - noncontiguous
//////////////////////////////////////////////////////////////////////////////////
int paging_mapvirt2phys(int virtaddr, int physframe)
{
// get the correct pte
int *pte = paging_findpte(virtaddr);

// change the physical address, preserve the flags
//printf("pte before mapping.. 0x%xh\n",pte);
int flags = (int) pte & 0x00000FFF;
int physaddr = physframe * PAGESIZE; // TODO: could we have this as a parameter?
pte = physaddr;
pte = (int) pte | flags;
//printf("pte after mapping.. 0x%xh\n",pte);

return 1;
}
int paging_protection(int virtaddr, char flags)
{
return 1;
}


//////////////////////////////////////////////////////////////////////////////////
// set/unset present bit of a virtual address
//////////////////////////////////////////////////////////////////////////////////
int paging_present(int virtaddr, char present)
{
int *pte = paging_findpte(virtaddr);

if (present == 1)
	pte = (int )pte | 0x00000001;
if (present == 0)
	pte = (int) pte | 0x00000000;

return 1;
}


//////////////////////////////////////////////////////////////////////////////////
// Given a virtual address, return a physical address
//////////////////////////////////////////////////////////////////////////////////
int paging_findphysaddr(int virtaddr)
{
int *pte = paging_findpte(virtaddr);
int *physaddr = (int) pte & 0xFFFFF000;
return (int) physaddr;
}


//////////////////////////////////////////////////////////////////////////////////
// Given a virtual address, return a page table entry
// this function used as a helper for other in paging.c
//////////////////////////////////////////////////////////////////////////////////
int paging_findpte(int virtaddr)
{
// pde and pte are the indices to page directory and page table
int pde = virtaddr / ( PAGESIZE * 1024);
int pte = (virtaddr - (PAGESIZE*1024*pde)) / PAGESIZE;
printf("findpte(): for addr 0x%xh, pde is #%d, pte is #%d\n",virtaddr,pde,pte);

// TODO: use CR3 instead of this once we hit userland..
// remember, in pde and pte the physical address is 20 bits, rest are flags
unsigned int *pde_addr = kernel_pagedir[pde] & 0xFFFFF000;


// programs can access a memory area where no virtual mappings exist - no pde or pte
// this also affects creating new page tables: when kmalloc() creates a table at a certain virtual address, it doesn't exist yet
// therefore, we have a chicken vs. egg situation.

// findpte can access a nonexisting memory area (no page table). There be dragons
// if paging is not enabled, we're still in the process of creating the very first pde and this wouldn't work. It's a dirty hack, yes.
if (pde_addr == 0 && paging_enabled)
	{
	// create a pde and fill in all the entries
	unsigned int *pagetable = kmalloc(PAGESIZE);
	int i;
	for (i = 0; i < 1024; i++)
		{
		// set bits present, read/write, supervisor only
		pagetable[i] = 0x40000 |PAGE_PRES_RW_KRN;
		}
	kernel_pagedir[pde] = (int) pagetable|PAGE_PRES_RW_KRN;
	printf("new pagedir is at 0x%xh\n",kernel_pagedir[pde]);
	}
pde_addr = kernel_pagedir[pde] & 0xFFFFF000;
printf("findpte returns 0x%xh - 0x%xh\n", pde_addr, (int) pde_addr[pte]);
return (int) pde_addr[pte]; // should we return pte or pde for remap?
}
