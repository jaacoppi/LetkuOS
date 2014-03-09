/* mm.c
* LetkuOS memory manager
*/

#include "multiboot.h"
#include "letkuos-common.h"
#include "stdio.h"
#include "string.h"
#include "mm.h"
#include "paging.h"

struct memorymap mmap; // used by the physical memory manager to know the available memory size & location
struct memory_map *memmap; // a multiboot struct
extern int endofkernel; // from linker.ld script. This is where our kernel ends

static void pmm_releaseframe();


// pmm_stackptr points to the page frame stack - deals with the physical memory manager
int *pmm_stackptr = &endofkernel; // physical memory manager starts right after the kernel
int pmm_maxstacksize;
int vmm_maxlistsize;
// vmm_list is an item in the singly linked list
struct vmmlist *vmmlist_ptr;
//struct vmmlist *vmmlist_start;
int *vmmlist_baseptr = 0; // baseptr holds the base (virtual) memory address of the next vmmlist_insert
int heapstart;
/* HOW LetkuOS memory management works:
1. The kernel is loaded at 0x00100000
2. LetkuOS uses a stack of free pages for  physical memory allocation and deallocation.
   This part is called PMM, Physical Memory Mamager.
*/

// multiboot flags explation from here: https://www.gnu.org/software/grub/manual/multiboot/multiboot.html

/////////////////////////////////////////////
// Initialize all memory management functions
/////////////////////////////////////////////
void init_memory(struct multiboot_info *bootinfo)
{
// make sure that multiboot struct is well formed and we can trust the info from GRUB
if (!(bootinfo->flags & BIT0))
	panic("multiboot info doesn't include mem_ fields!\n");

if (!(bootinfo->flags & BIT6))
	panic("multiboot info doesn't include mmap_ fields!\n");

// for debug only
// printf("the machine has 0x%xh of (upper) RAM, that's (roughly) %d megs!\n", bootinfo->mem_upper*1024,bootinfo->mem_upper/1024);


/* HOW STUFF WORKS:
with the code below, one could print the memory map from GRUB. When type is 1, the memory is available for use.

With all tests so far (bochs & qemu, different memory setups), the code prints out two type == 1 memory areas,
lower memory area and the upper memory area. The size of the upper memory area depends on memory available, and is always equal to bootinfo->mem_upper.

Thus, for our memory management we want to use the 2nd type == 1 memory area available, if the length is equal to bootinfo->mem_upper. 
This code will be in trouble if run with real hardware that happens to have non-contiguous area of upper memory */

// Part 1 of memory init: use GRUB to get the memory map, that is, the physical address of usable memory
// This code adapted from grub manual: https://www.gnu.org/software/grub/manual/multiboot/multiboot.html
mmap.size = 0; // safety check
for (memmap = (struct memory_map *) bootinfo->mmap_addr; (unsigned long) memmap < bootinfo->mmap_addr + bootinfo->mmap_length; memmap = (struct memory_map *) ((unsigned long) memmap + memmap->size + sizeof (memmap->size)))
	{
	int base = memmap->base_addr_high;
	base = (base << 24) + memmap->base_addr_low;
	unsigned int len = memmap->length_high;
	len = (len << 24) + memmap->length_low;

	// found a contiguous upper memory area
	// copy the usable parts to your own struct
	if (memmap->type == 1 && len == bootinfo->mem_upper*1024)
		{
		mmap.base = base;
		mmap.size = len;
		// for debug only
		// printf("found a contiguous usable uppermem area, start: 0x%xh, length: 0x%xh bytes!\n",mmap.base, mmap.size);
		break;
		}
	// debug: print the memory map
	// printf ("size = %d, base_addr = 0x%x, length = %d, type = %d\n", (unsigned) memmap->size, base, len, (unsigned) memmap->type);
	}

if (mmap.size == 0)
	panic("No contiguous usable upper memory area found");


// Part 2 of memory init: set aside a region of memory to be used by PMM and VMM

// if all memory is free, all pages are pushed to the stack. pmm_maxstacksize gives us the maximum stacksize


// the actual mallocs (kernel heap) will start after the area reserved for
// 1. kernel
// 2. physical memory manager
// 3. some arbitrary memory space for linked list items

#define MAXLISTITEMS	0x100
int pagenr = mmap.size / PAGESIZE;
pmm_maxstacksize = pagenr * sizeof(int);
vmm_maxlistsize  = MAXLISTITEMS * sizeof(struct vmmlist);


// init the physical memory manager and stack of free pages
int i;
for (i = pagenr; i > 0; i--) // push the last of physical memory first in stack - allocate from bottom up
	{
	*pmm_stackptr = i;	// the value in stack refers to the page frame index used by paging
	pmm_stackptr++;		// raise the stack
	}

// init the virtual memory manager - store it right after PMM. Note that pmm_stackptr is set correctly in the previous loop 
// vmmlist_start now always points to the beginning of the vmmlist, just like the name says
vmmlist_start = (struct vmmlist *) ((int) &endofkernel + (int) pmm_maxstacksize);

// actual memory allocation will start from heapstart
heapstart  = (int) &endofkernel + (int) pmm_maxstacksize + (int) vmm_maxlistsize;
//printf("heapstart set to 0x%xh\n",heapstart);

// Part 3 of memory init: mark memory used by kernel, PMM and VMM as used
// the tricky part here is that kmalloc requires paging and the linked list to work. We'll insert the first list item manually

vmmlist_start->next = MMLIST_LAST;
vmmlist_start->used = MM_USED;
vmmlist_start->size = heapstart;
// vmmlist_ptr and vmmlist_baseptr should point to the first available slot for a list item
// vmmlist_start points to the area reserved for kernel. vmmlist_additem should start from one index after it.
vmmlist_ptr = vmmlist_start;
vmmlist_ptr++;
vmmlist_baseptr = heapstart;

// allocate the physical memory as well
for (i = 0; i <= (int) vmmlist_baseptr; i = i + PAGESIZE) // <= or < ..
	pmm_getframe();


// Part 4 of memory init: Start paging.
init_paging();

return;
}


// =====================================================================
// functions starting with pmm_()  are the Physical Memory Manager (PMM)
// =====================================================================

/* in the physical memory manager:
**
** a stack is used to keep track of free pages - thus, all addresses not in stack are used.
** the stack grows upwards
** we're dealing with page frame indexes, not with addresses, so it doesn't matter whether paging is enabled or not
*/

///////////////////////////////////////////////////////////////////////
// pmm_getframe: mark a physical page as used by popping from the stack
///////////////////////////////////////////////////////////////////////
int pmm_getframe()
{
// get the topmost item in the stack
int retval = *pmm_stackptr;
pmm_stackptr--;
return retval;
}

///////////////////////////////////////////////////////////////////////////
// pmm_releaseframe: mark a physical page as free by pushing it to the stack
///////////////////////////////////////////////////////////////////////////
void pmm_releaseframe(int index)
{
pmm_stackptr++;
*pmm_stackptr = index;
}



// ================================================================================
// functions starting with vmmlist_()  are part of the Virtual Memory Manager (VMM)
// ================================================================================


// vmmlist_additem and vmmlist_insert follow a flat list approach. This is suitable for virtual memory, not physical

/* singly linked list functions start here.
copied the ideas from http://www.thelearningpoint.net/computer-science/data-structures-singly-linked-list-with-c-program-source-code */

/////////////////////////////////////////////////////////////////
// add a new item in the singly linked list. adjust base and size
/////////////////////////////////////////////////////////////////
int vmmlist_additem(unsigned long memsize)
{
// no more space in the linked list
// TODO: get rid of this limitation
if (vmmlist_ptr == heapstart)
	panic("vmmlist_additem(): no space in linked list\n");

// find the last item in the list - this is where we will add our item

// the first item..
struct vmmlist *linkedlistptr = vmmlist_start;

// TODO: get this as a paramater from vmmlist_insert, it's done there already
// loop until the end of list - after the loop linkedlistptr points to the last item
while (true)
	{
	if (linkedlistptr->next == (struct vmmlist *) MMLIST_LAST)
		break;

	linkedlistptr = linkedlistptr->next;
	}

// 4-byte align baseptr so we can utilize it with paging and stuff
int bit_test = (int) vmmlist_baseptr & 0x00000FFF;
if (bit_test != 0)
	{
	vmmlist_baseptr = (int) vmmlist_baseptr & 0xFFFFF000;
	vmmlist_baseptr = (int) vmmlist_baseptr + PAGESIZE;
	}


// vmmlist_ptr holds the  address for the next memory item, make it the next item
vmmlist_ptr->base = (int) vmmlist_baseptr;
vmmlist_ptr->size = memsize;
vmmlist_ptr->used = MM_USED;

linkedlistptr->next = vmmlist_ptr;	// adjust the linked list
vmmlist_ptr->next = (struct vmmlist *) MMLIST_LAST;
vmmlist_baseptr = (int) vmmlist_baseptr + memsize; // the next allocation of virtual memory will be done here
vmmlist_ptr++; // next additem will be done here
return linkedlistptr->next;
}


/////////////////////////////////////////////////////////////////
// use an existing list item, don't create a new one
/////////////////////////////////////////////////////////////////
int vmmlist_insert(unsigned long memsize)
{
struct vmmlist *linkedlistptr = vmmlist_start;

// TODO: also utilize the last item, correct the while loop
//printf("searching for existing list items..\n");

while (true)
	{
	// for debug:
//	printf("looping a listitem at 0x%xh, size of this is 0x%xh, next is 0x%xh\n",linkedlistptr, linkedlistptr->size,linkedlistptr->next);
	if (linkedlistptr->used == MM_FREE && linkedlistptr->size >= memsize)
			{
//			printf("using available free listitem 0x%xh\n",linkedlistptr);
			linkedlistptr->used = MM_USED;
			return (unsigned long) linkedlistptr->base;
			}

	// next item is the last in list, stop looping
	if (linkedlistptr->next == (struct vmmlist *) MMLIST_LAST)
		break;

	// this gives the gcc warning about pointer from integer. It could be avoided, for example, by using relative addresses in ->next and then simply incrementing the linkedlistptr
	linkedlistptr = linkedlistptr->next;
	}

// if we're here, we're at the end of the list and  there was no big enough free memory areas
// for debug
linkedlistptr = vmmlist_additem(memsize);
if (linkedlistptr == 0)
	panic("can't add a big enough contiguous virtual memory area!");
else
	{
	linkedlistptr->used = MM_USED;
	return (unsigned long) linkedlistptr->base;
	}

return 0; // return 0 on error, we should never reach this part
}


// =====================================================================
// kmalloc and kfree are the final functions used by kernel
// =====================================================================

// TODO: heap memory manager
// currently, we can only allocate 4kb chunks with the vm.
// Instead, we should be able to allocate smaller chunks
// so, the current vmm should be the heap manager that allocates variable size memory chunks, not 4kb ones
// also, this requires the singly linked list to be of variable size (no need to worry about deletion anymore)
// and the virtual memory manager allocates virtual memory (through page faults) whenever needed.
// thus, divide the current VMM into two parts, the page fault part and the current part (and rename it "heap")

/////////////////////////////////////////////////////////////////
// Allocate memsize worth of memory and return a pointer to its base
/////////////////////////////////////////////////////////////////
void *kmalloc(unsigned long memsize)
{
// kmalloc allocates memsize worth of memory and returns the pointer to the starting address
// it goes through a singly linked list of used/unused VIRTUAL memory areas until it finds one that is free and big enough
// if it doesn't, it runs list_defrag to see if two adjacent free lists can be combined to be big enough
// if they can't, we're out of memory and thus we panic (or handle the error correctly..)

// sanity check: if asked for 0 memory, return NULL
if (memsize == 0)
	return NULL; // TODO: should NULL be != 0? random code could be executed if ever run from 0

// the maximum number of linked list items is "available memory / PAGESIZE".
// This means we reserve 4kb pages at minimum. Yes, it wastes a lot of memory
if (memsize < PAGESIZE)
	memsize = PAGESIZE;

// if there's no more physical memory, panic.
// the stack of free physical page frames starts at endofkernel. Thus we can easily calculate how much free memory we have
if (memsize > ((unsigned long) (&endofkernel - pmm_stackptr) * PAGESIZE))
	panic("kmalloc: we're out of memory!\n"); 

// ask the virtual memory manager for a contiguous virtual memory address the size of memsize
int retval =  vmmlist_insert(memsize);

// find out which listitem holds this memory area
struct vmmlist *curptr = vmmlist_start;
while (curptr->base != retval)
	curptr = curptr->next;

int i;
// reserve physical memory, adjust paging and virt2phys mappings
for ( i = curptr->base; i < (curptr->base + curptr->size); i = i + PAGESIZE)
	{
	paging_mapvirt2phys(i, pmm_getframe()); // get a physical frame and map it to a virtual address
	paging_protection(i, 0); // set protection
	paging_present(i,1); // set page present
	}

printf("kmalloc() returns address 0x%xh\n",retval);
return (int *)retval;
}


//////////////////////////////////////
// Deallocate memory pointed to by ptr
//////////////////////////////////////
void kfree(void *ptr)
{
// if pointer is NULL (or 0), do nothing
if (ptr == NULL || ptr == 0)
	return;

// find out which linked list item holds this pointer

struct vmmlist *curptr = vmmlist_start;
while (curptr->next != MMLIST_LAST)
	{
	if (curptr->base == ptr) // found it
		break;

	curptr = curptr->next;
	}


// ask the page table which physical frames are now to be released and release them
int *pte_ptr;
int i = curptr->base;

int pageframe;

for (i = curptr->base; (unsigned int) i < (curptr->base + curptr->size); i = i + PAGESIZE)
	{
	pte_ptr = paging_findphysaddr(i); // get the page table address that holds this virtual address
	// get the physical frame (index, not address)
	// TODO: wonder what'll happen when we'll move from PAGESIZE'd kmallocs to variable sized..
	pageframe = (int) pte_ptr / PAGESIZE;
	// 1. release the physical frame
	pmm_releaseframe(pageframe);
	// 2. mark the area free in paging
	paging_present(pte_ptr,0);
	}

// 3. finally, set the area free in the linked list
printf("kfree() freed 0x%xh\n",curptr->base);
curptr->used = MM_FREE;
return;
}
