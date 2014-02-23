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

static int pmm_getframe();
static void pmm_releaseframe();
static void vmmlist_delete(struct vmmlist *listitem);
static void vmmlist_use(struct vmmlist *linkedlistptr, int memsize);
static struct vmmlist* vmmlist_merge(int memsize);

// pmm_stackptr points to the page frame stack - deals with the physical memory manager
int *pmm_stackptr = &endofkernel; // physical memory manager starts right after the kernel
int pmm_maxstacksize;
int vmm_maxlistsize;
// vmm_list is an item in the singly linked list
struct vmmlist *vmmlist_ptr;
struct vmmlist *vmmlist_start;

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
int pagenr = mmap.size / PAGESIZE;
pmm_maxstacksize = pagenr * sizeof(int);
vmm_maxlistsize  = pagenr * sizeof(struct vmmlist);

// init the physical memory manager and stack of free pages
int i;
for (i = pagenr; i > 0; i--) // push the last of physical memory first in stack - allocate from bottom up
	{
	*pmm_stackptr = i;	// the value in stack refers to the page frame index used by paging
	pmm_stackptr++;		// raise the stack
	}

// init the virtual memory manager - store it right after PMM. Note that pmm_stackptr is set correctly in the previous loop 
// vmmlist_start now always points to the beginning of the vmmlist, just like the name says
vmmlist_start  = (struct vmmlist *) ((int) &endofkernel + (int) pmm_maxstacksize);

vmmlist_ptr = vmmlist_start;
for (i = 0; i <= pagenr; i++)
	{
	// a dirty hack here
	// by default, a list item holds PAGESIZE worth of memory
	// the first kmalloc will be the kernel + pmm + vmm, thus a merge and deletion of list items is required
	// vmmlist_delete() requires paging, but we can't enable paging before the first kmalloc()
	// thus, manually set the size of the first memory area so that we can safely use kmalloc before setting up paging
	if (i == 0)
		{
		vmmlist_ptr->size = ((int) &endofkernel + (int) pmm_maxstacksize + (int) vmm_maxlistsize);
//		printf("first malloc is of size 0x%xh\n",vmmlist_ptr->size);
		vmmlist_ptr->used = MM_FREE;
		vmmlist_ptr->base = 0;
		}
	else
		{
		vmmlist_ptr->size = PAGESIZE;
		vmmlist_ptr->used = MM_FREE;
		vmmlist_ptr->base = (i * PAGESIZE + 0x1000 + ((int) &endofkernel + (int) pmm_maxstacksize + (int) vmm_maxlistsize)) & 0xFFFFF000;
		}

	if (i == pagenr) // this is the last occurrence, mark as last in list
		{
		vmmlist_ptr->next = (struct vmmlist *) MMLIST_LAST;
		break;
		}
	else
		{
		// causes a gcc warning, but what can we do?
		vmmlist_ptr->next = (int) vmmlist_ptr + sizeof(struct vmmlist);
		}
//	printf("ptr here 0x%xh, iteration %d\n",vmmlist_ptr,i);
	vmmlist_ptr = vmmlist_ptr->next;
	}

// Part 3 of memory init: mark memory used by kernel, PMM and VMM as used
kmalloc((int) &endofkernel + (int) pmm_maxstacksize + (int) vmm_maxlistsize);
printf("kernel memory reserved!\n");
init_paging();
kmalloc(0x3000);
int *hep = kmalloc(0x1000);
kfree (hep);
hep = kmalloc(0x1000);
kfree (hep);
hep = kmalloc(0x1000);
kfree (hep);
hep = kmalloc(0x1000);
kfree (hep);
// Part 4 of memory init: Start paging. Must be after kmalloc so that paging gets setup after the kernel + pmm + vmm


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
static int pmm_getframe()
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


// vmmlist_insert and vmmlist_delete follow a flat list approach. This is suitable for virtual memory, not physical

/* singly linked list functions start here.
copied the ideas from http://www.thelearningpoint.net/computer-science/data-structures-singly-linked-list-with-c-program-source-code */

// find the first free area of size memsize. Return a pointer to the item in the list (a pointer to a struct vmmlist)
// TODO: should list_insert take care of setting all the properties, or should malloc/realloc?
int vmmlist_insert(unsigned long memsize, struct vmmlist *linkedlistptr)
{

//TODO: adjust vmmlist_insert_known so that it's used to have linkedlistptr->base as a known virtual address
// actually should we be given a vmmlist as an arg but an address instead?

// if linkedlistptr is NULL, start from the first item in the list. This is the first item in the vmmlist manager, right after the kernel
// if it's non NULL, it means we're calling from krealloc() and already know where the free area is
if (linkedlistptr == NULL)
	linkedlistptr = vmmlist_start;


// iterate through the list until the following conditions are met:
	// 1. the area is FREE (it means it has been initialized
	// 2. for a free area, the memory area is big enough

while (true)
	{
	// for debug:

//	printf("now looping list at 0x%xh, size is 0x%xh, need 0x%xh\n",linkedlistptr, linkedlistptr->size,memsize);
	if (linkedlistptr->used == MM_FREE && linkedlistptr->size >= memsize)
			{
			vmmlist_use(linkedlistptr, memsize);
			return (unsigned long) linkedlistptr->base;
			}

	// if we're at the end of the list, there was no big enough memory areas
	if (linkedlistptr->next == (struct vmmlist *)MMLIST_LAST)
		{
// for debug
//		printf("no big enough free memory areas found, merging...\n");
		linkedlistptr = vmmlist_merge(memsize);
		if (linkedlistptr == 0)
			panic("can't merge a big enough contiguous virtual memory area!");
		else
			{
			vmmlist_use(linkedlistptr, memsize);
			return (unsigned long) linkedlistptr->base;
			}
		break;
		}
	// this gives the gcc warning about pointer from integer. It could be avoided, for example, by using relative addresses in ->next and then simply incrementing the linkedlistptr
	linkedlistptr = linkedlistptr->next;
	}

return 0; // return 0 on error, we should never reach this part
}


// delete the item in the list. Caution must be taken to not leave memory holes in either the free memory area or the allocator list
// note that we don't free any memory like kfree does
static void vmmlist_delete(struct vmmlist *listitem)
{
listitem->used = MM_FREE;
listitem->base = 0;
listitem->size = 0;
listitem->next = MM_UNINIT;
}

/////////////////////////////////////////////////////////////////
// Defrag the singly linked list by merging adjacent free areas
/////////////////////////////////////////////////////////////////
struct vmmlist* vmmlist_merge(int memsize)
{
struct vmmlist *curptr = vmmlist_start;

// find the first free block
while (curptr->used != MM_FREE)
	curptr = curptr->next;

struct vmmlist *lastptr = curptr;
curptr = curptr->next;

// at this point, lastptr is the first free block, we search for the adjacent block with curptr

// loop through the VMM linked list until you find memsize amount of adjacent memory
while (curptr->next != (struct vmmlist *) MMLIST_LAST)
	{
	if (curptr->base == (lastptr->base + lastptr->size)) // found the adjacent block
		{
		if (curptr->used == MM_FREE) // the adjacent block is free, we can merge
			{
			lastptr->size = lastptr->size + curptr->size;
			// for debug
			//printf("vmmlist merge 0x%xh with 0x%xh, size now 0x%xh \n",lastptr->base, curptr->base,lastptr->size);
			lastptr->next = curptr->next; // skip curptr list item completely
			vmmlist_delete(curptr);

			// we have now merged once. If this is enough, return. If not, continue
			if (lastptr->size >= memsize)
				return lastptr;
			else
				{
				curptr = lastptr->next;
				continue;
				}
			}
		else	// the adjacent block is not free, we'll continue looping from the next block
			{
			lastptr = lastptr->next;
			curptr = lastptr;
			curptr = curptr->next;
			continue;
			}
		}
	else // this is not the adjacent block, loop until you find it
		{
		curptr = curptr->next;
		}

	}

}


/////////////////////////////////////////////////////////////////
// Mark a certain area in VMM used - map virt to phys
/////////////////////////////////////////////////////////////////
void vmmlist_use(struct vmmlist *linkedlistptr, int memsize)
{
// mark the memory area used both in VMM and in PMM
linkedlistptr->used = MM_USED;
unsigned int i;
// map virtual to physical
for (i = 0; i <= memsize / PAGESIZE; i++)
	{
	// TODO: adjust the page tables so that the virtual address space is mapped to the physical memory
//	tables[index] = WHAT VIRTUAL ADDRES? linkedlistptr->base?
	pmm_getframe();
	}

// found a big enough free memory area, let's use it
//printf("found a free area: 0x%xh\n",linkedlistptr->base);
}

// =====================================================================
// kmalloc, krealloc and kfree are the final functions used by kernel
// =====================================================================

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
int retval =  vmmlist_insert_unknown(memsize);

//printf("kmalloc() returns address 0x%xh\n",retval);
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

// find out which linked list item holds this pointer, and free it

struct vmmlist *curptr = vmmlist_start;
while (curptr->next != MMLIST_LAST)
	{
	if (curptr->base == ptr)
		{
		curptr->used = MM_FREE; // mark the area free in VMM
		break;
		}
	curptr = curptr->next;
	}


// ask the page table which physical frames are now to be released and release them
int i = curptr->base;
int j,k;
int *pagetable;

for (i = curptr->base; (unsigned int) i < (curptr->base + curptr->size); i = i + PAGESIZE)
	{
	for (j = 0; j < 1024; j++) // page dir entries
		{
		pagetable = kernel_pagedir[j] & 0xFFFFF000;
		for (k = 0;k < 1024; k++) // page table entries
			{
			if ((pagetable[k] & 0xFFFFF000) == curptr->base)
				{
				pmm_releaseframe(j*1024 + k);	// release the physical frame
				}
			}
		}
	}

return;
}


void *krealloc(void *ptr, unsigned long memsize)
{
// TODO: remember to go through this function..
panic("krealloc() has not been revised - here be dragons");
/* straight from linux man:
The realloc() function changes the size of the memory block pointed to by ptr to size bytes.  The contents
will be unchanged in the range from the start of the region up to the minimum of the old and new sizes.  If
the new size is larger than the old size, the added memory will not be initialized.

If the area pointed to was moved, a free(ptr) is done.
*/

// If ptr is NULL, then the call is equivalent to malloc(size), for all values of size;
if (ptr == NULL)
	return kmalloc(memsize);

// if size is equal to zero,  and  ptr  is  not NULL,  then  the call is equivalent to free(ptr).
if (memsize == 0)
	{
	kfree(ptr);
	return 0;
	}

// reallocation


// read the current values of the list item
// ptr points to a base address of memory, so we need to loop through the list to find the item that has the same base
struct vmmlist *linkedlistptr = (struct vmmlist *) &endofkernel;
while (linkedlistptr->next != (struct vmmlist *) MMLIST_LAST)
	{
	if(linkedlistptr->base == (unsigned long) ptr)
		break;
	linkedlistptr = linkedlistptr->next;
	}

if(linkedlistptr->next == (struct vmmlist *) MMLIST_LAST && linkedlistptr->base != (unsigned long) ptr) // didn't find anything
	panic("krealloc couldn't find the proper list item!\n");

//at this point, linkedlistptr is the item we want to realloc
struct vmmlist *temp = (struct vmmlist *) linkedlistptr;

// case 0: do nothing
if (memsize == linkedlistptr->size)
	return ptr;

printf("ptr is 0x%xh, newsize is 0x%xh, oldsize is 0x%xh\n",linkedlistptr,memsize, temp->size);
// case 1: decrease memory size and add the extra memory to the linked list as free
if (memsize < temp->size)
	{
	temp->size = memsize;

	// make linkedlistptr point to the last item
	linkedlistptr = (struct vmmlist *) &endofkernel;
	while (linkedlistptr->next != (struct vmmlist *) MMLIST_LAST)
		linkedlistptr = linkedlistptr->next;

	struct vmmlist *linkedlistptr = (struct vmmlist *) &endofkernel;
	while (linkedlistptr->next != (struct vmmlist *) MMLIST_LAST)
		linkedlistptr = linkedlistptr->next;

	// add the new struct as the last item in the list
	struct vmmlist *newarea = (struct vmmlist *) vmmlist_insert_known(temp->size - memsize,linkedlistptr);
	newarea->base = temp->base + temp->size;
	newarea->used = MM_FREE;
	newarea->size = temp->size;

	return ptr;
	}

// case 2: increase memory size
if (memsize > temp->size)
	{
	// loop through the items to see if the item with base overlapping ptr + memsize is free or uninitialized - can we merge?
	// TODO..

	// can't merge, we need to relocate the memory.

	// kmalloc the first usable  memory area
	int *newptr = kmalloc(memsize);
//	struct vmmlist *newlistptr = (struct vmmlist *) &newptr;



	// copy the current used memory to the new area
	// ptr points to a base address of memory, so we need to loop through the list to find the item that has the same base
	struct vmmlist *oldptr = (struct vmmlist *) &endofkernel;
	while (oldptr->next != (struct vmmlist *) MMLIST_LAST)
		{
		if(oldptr->base == (unsigned long) ptr)
			break;
		oldptr = oldptr->next;
		}

	// note that newptr is a pointer to the actual memory, not to the list vmmlist like oldptr is
	memcpy((void *) newptr, (void *) oldptr->base,oldptr->size);
	printf("debug: krealloc old base 0x%xh and size 0x%xh\n",oldptr->base,oldptr->size);
	printf("debug: krealloc moved the stuff from 0x%xh to 0x%xh\n",oldptr->base,newptr);

	// kfree the old area
	kfree(ptr);

	// return the new pointer
	return newptr;
	}


return 0; // don't think we should ever reach this point
}

