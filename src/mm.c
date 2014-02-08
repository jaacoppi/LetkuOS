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
// consider using long instead of int to cover all the memory..
unsigned long *memptr; // points to the address kmalloc reserves - the base of physical memory given to programs
struct memory_map *memmap; // a multiboot struct

extern int endofkernel; // from linker.ld script. This is where our kernel ends

/* HOW LetkuOS memory management works:
1. The kernel is loaded at 0x00100000
2. LetkuOS uses a next-fit singly linked list for physical memory allocation and deallocation
3. by default the allocator allocates 4kb memory units (size of pages, so we can easily use paging)
4. the singly linked list needs to store info of pagenr AVAILABLE MEMORY / PAGESIZE pages
5. because of #4, the memory manager reserves sizeof(physmem) * pagenr
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

printf("the machine has 0x%xh of (upper) RAM, that's (roughly) %d megs!\n", bootinfo->mem_upper*1024,bootinfo->mem_upper/1024);

// this function reserves the memory used by kernel & memory manager

// 1. use GRUB to get the memory map, that is, the physical address of usable memory

/* HOW STUFF WORKS:
with the code below, one could print the memory map from GRUB. When type is 1, the memory is available for use.

with all tests so far (bochs & qemu, different memory setups), the code prints out two type == 1 memory areas,
lower memory area and the upper memory area. The size of the upper memory area depends on memory available, and is always equal to bootinfo->mem_upper.

Thus, for our memory management we want to use the 2nd type == 1 memory area available, if the length is equal to bootinfo->mem_upper. 
This code will be in trouble if run with real hardware that happens to have non-contiguous area of upper memory */


// code adapted from grub manual: https://www.gnu.org/software/grub/manual/multiboot/multiboot.html
mmap.size = 0; // safety check
for (memmap = (struct memory_map *) bootinfo->mmap_addr; (unsigned long) memmap < bootinfo->mmap_addr + bootinfo->mmap_length; memmap = (struct memory_map *) ((unsigned long) memmap + memmap->size + sizeof (memmap->size)))
	{
	int base = memmap->base_addr_high;
	base = (base << 24) + memmap->base_addr_low;
	unsigned int len = memmap->length_high;
	len = (len << 24) + memmap->length_low;

	// found a contiguous upper memory area
	// 2. copy the usable parts to your own struct
	if (memmap->type == 1 && len == bootinfo->mem_upper*1024)
		{
		mmap.base = base;
		mmap.size = len;
		printf("found a contiguous usable uppermem area, start: 0x%xh, length: 0x%xh bytes!\n",mmap.base, mmap.size);
		break;
		}
	// debug: print the memory map
//	printf ("size = %d, base_addr = 0x%x, length = %d, type = %d\n", (unsigned) memmap->size, base, len, (unsigned) memmap->type);
	}

if (mmap.size == 0)
	panic("No contiguous usable upper memory area found");


// 3. set aside a region of memory to be used for the singly linked list of available memory
unsigned long pagenr = mmap.size / PAGESIZE;
unsigned long manager_reserved = sizeof(struct physmem) * pagenr;
// for debug
printf("need for kernel + memory manager: 0x%xh\n",((int) &endofkernel + manager_reserved));
printf("need for pages: %d\n",pagenr);

// 4. make sure the reserved memory for memory manager resides in right after the memory reserved for kernel: endofkernel
// when a pointer points to endofkernel and all the following structs will be loaded AFTER it, we know the whole singly linked list will be between endofkernel and endofkenerl + manager_reserved

// to be on the safe side, clear all memory used by the memory manager
// I wonder if this is needed or just slows down init
//printf("debug: clearing out 0x%xh bytes of memory for memory manager\n", manager_reserved);
memset(&endofkernel, 0, manager_reserved);

struct physmem *temp = (struct physmem *) &endofkernel;
temp->used = MM_UNINIT;		// for kmalloc to work, the memory area must be set uninit (it is by default, though)
temp->next = MMLIST_LAST;	// for kmalloc to work, the linked list has to have an end


// mark memory use by kernel and memory manager as used
// malloc reserves the area from memptr, so manually adjust it to zero first
memptr = 0;
kmalloc(( (int) &endofkernel + manager_reserved));

init_paging();
return;
}



/////////////////////////////////////////////////////////////////
// Allocate memsize worth of memory and return a pointer to its base
/////////////////////////////////////////////////////////////////
void *kmalloc(unsigned long memsize)
{
// kmalloc allocates memsize worth of memory and returns the pointer to the starting addres
// it goes through a singly linked list of used/unused memory areas until it finds one that is free and big enough
// if it doesn', it runs memdefrag to see if two adjacent free lists can be combined to be big enough
// if they can't, we're out of memory and thus we panic (or handle the error correctly..)

// sanity check: if asked for 0 memory, return NULL
if (memsize == 0)
	return NULL; // TODO: should NULL be != 0? random code could be executed if ever run from 0

// when allocating memory for page directories and page tables, the memory needs to be page aligned.
// here, we make all calls to kmalloc() return a 4kb aligned address. This is wasteful, but this is a simple kmalloc() after all
// TODO: make this test for PAGESIZE instead of assuming 4kb pages (0xFFFFF000)

// if memptr is not already 4kb aligned, align it.
int bit_test = (unsigned int long) memptr & 0x00000FFF;
if (bit_test != 0)
	{
	memptr  = (long unsigned int) memptr & 0xFFFFF000;
	memptr = (unsigned long) memptr + 0x1000;
	}

// the maximum number of linked list items is "available memory / PAGESIZE". This means we reserve 4kb pages at minimum
if (memsize < PAGESIZE)
	memsize = PAGESIZE;


// if there's no more memory, panic.
if ((unsigned long) (memptr + memsize) > mmap.size)
	panic("kmalloc: we're out of memory!\n"); // TODO: memdefrag and loop the linked list from the beginning


// find an unused item in the singly linked list and set the properties Note that ->next is set in list_insert
struct physmem *temp = (struct physmem *) list_insert_unknown(memsize);

temp->used = MM_USED;
temp->base = (unsigned long) memptr;
temp->size = memsize;


int retval = (unsigned long) memptr; // return the area in current memptr address

// advance memptr so next kmalloc() will not overwrite this one
// without the typecasts (unsigned long), the result below would be memptr + memsize*4. Relates to sizeof pointer, I'd guess.
// this causes a GCC warning "assignment makes pointer from integer without a cast"
// the reason for that is we're assigning an address to pointer. That's what we want to do with malloc
memptr = (unsigned long ) memptr + (unsigned long) memsize; //the next time an alloc happens, claim memory starting from this address

//printf("debug: reserved 0x%xh bytes, start 0x%xh, return: 0x%xh\n",temp->size, temp->base, retval);
return (unsigned long *) retval;
}

/////////////////////////////////////////////////////////////////
// Deallocate memory pointed to by ptr(->base)
/////////////////////////////////////////////////////////////////
void kfree(void *ptr)
{
// if pointer is NULL (or 0), do nothing
if (ptr == NULL || ptr == 0)
	return;

// kfree only marks the memory area free
struct physmem *temp = (struct physmem *) ptr;
temp->used = MM_FREE;
}


void *krealloc(void *ptr, unsigned long memsize)
{
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
struct physmem *linkedlistptr = (struct physmem *) &endofkernel;
while (linkedlistptr->next != MMLIST_LAST)
	{
	if(linkedlistptr->base == (unsigned long) ptr)
		break;
	linkedlistptr = linkedlistptr->next;
	}

if(linkedlistptr->next == MMLIST_LAST && linkedlistptr->base != (unsigned long) ptr) // didn't find anything
	panic("krealloc couldn't find the proper list item!\n");

//at this point, linkedlistptr is the item we want to realloc
struct physmem *temp = (struct physmem *) linkedlistptr;

// case 0: do nothing
if (memsize == linkedlistptr->size)
	return ptr;

printf("ptr is 0x%xh, newsize is 0x%xh, oldsize is 0x%xh\n",linkedlistptr,memsize, temp->size);
// case 1: decrease memory size and add the extra memory to the linked list as free
if (memsize < temp->size)
	{
	temp->size = memsize;

	// make linkedlistptr point to the last item
	linkedlistptr = (struct physmem *) &endofkernel;
	while (linkedlistptr->next != MMLIST_LAST)
		linkedlistptr = linkedlistptr->next;

	struct physmem *linkedlistptr = (struct physmem *) &endofkernel;
	while (linkedlistptr->next != MMLIST_LAST)
		linkedlistptr = linkedlistptr->next;

	// add the new struct as the last item in the list
	struct physmem *newarea = (struct physmem *) list_insert_known(temp->size - memsize,linkedlistptr);
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
//	struct physmem *newlistptr = (struct physmem *) &newptr;



	// copy the current used memory to the new area
	// ptr points to a base address of memory, so we need to loop through the list to find the item that has the same base
	struct physmem *oldptr = (struct physmem *) &endofkernel;
	while (oldptr->next != MMLIST_LAST)
		{
		if(oldptr->base == (unsigned long) ptr)
			break;
		oldptr = oldptr->next;
		}

	// note that newptr is a pointer to the actual memory, not to the list physmem like oldptr is
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

/////////////////////////////////////////////////////////////////
// Defrag the singly linked list by merging adjacent free areas
/////////////////////////////////////////////////////////////////
void mm_defrag()
{
// loop from beginning until end of memory
// keep a counter of previous block and it's free state
// when a free block is found, merge it with the previous block if that block is free

// figure out how this works with list_delete
}



/* singly linked list functions start here.
copied the ideas from http://www.thelearningpoint.net/computer-science/data-structures-singly-linked-list-with-c-program-source-code */

// find the first free area of size memsize. Return a pointer to the item in the list (a pointer to a struct physmem)
// TODO: should list_insert take care of setting all the properties, or should malloc/realloc?
int list_insert(unsigned long memsize, struct physmem *linkedlistptr)
{

// if linkedlistptr is NULL, start from the first item in the list. This is the first item in the physmem manager, right after the kernel
// if it's non NULL, it means we're calling from krealloc() and already know where the free area is
if (linkedlistptr == NULL)
	linkedlistptr = (struct physmem *) &endofkernel;

// iterate through the list until the following conditions are met:
	// 1. the area is FREE (it means it has been initialized
	// 2. for a free area, the memory area is big enough

while (true)
	{
	// for debug:
	// printf("looping the linked list at  0x%xh..\n",linkedlistptr);
	if (linkedlistptr->used == MM_FREE && linkedlistptr->size > memsize)
			{
			// debug: the freeing code does not exist when writing this, panic to debug
			panic("first free area in the linked list found!\n");

			// found a big enough free memory area, let's use it
			printf("found a free area: 0x%xh\n",linkedlistptr);
			return (unsigned long) linkedlistptr;
			}

	// if we're at the end of the list, break. otherwise, get the next item
	if (linkedlistptr->next == MMLIST_LAST)
		break;

	// this gives the gcc warning about pointer from integer. It could be avoided, for example, by using relative addresses in ->next and then simply incrementing the linkedlistptr
	linkedlistptr = linkedlistptr->next;
	}

// we ran until the end of the list. we need to insert a new item
// at this point, linkedlistptr points to the current last item

// loop through the memory manager area until you find an uninitialized memory manager area and utilize it
struct physmem *newitem = (struct physmem *) &endofkernel;
while (true)
	{
	if (newitem->used == MM_UNINIT)
		{
		linkedlistptr->next = (unsigned long) newitem;
		newitem->next = MMLIST_LAST;
//		printf("inserting the new listitem in 0x%xh - next is  0x%xh\n",newitem, newitem->next);
		break;
		}


	// loop the next item, note the gcc warning
	newitem = (unsigned long) newitem + sizeof (struct physmem);
	}

return (unsigned long) newitem;
}


// delete the item in the list. Caution must be taken to not leave memory holes in either the free memory area or the allocator list
// 1. Mark memory as free.
// 2. remove the item from the list
int list_delete(int listnode)
{
return listnode; // currently here just to remove compiler warnings. should return 1 on success. Could be this function will never be used anyway.
}
