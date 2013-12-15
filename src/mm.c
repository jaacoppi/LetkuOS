/* mm.c
* LetkuOS memory manager
*/

#include "multiboot.h"
#include "letkuos-common.h"
#include "stdio.h"
#include "mm.h"

void init_mm();
void init_paging();
void *kmalloc(int memsize);
void kfree(void *ptr);
void mm_defrag();

struct memorymap mmap; // used by the physical memory manager to know the available memory size & location
int *physmemptr; // pointer to the singly linked list
struct memory_map *memmap; // a multiboot struct

extern int endofkernel; // from linker.ld script. This is where our kernel ends

/* HOW LetkuOS memory management works:
1. The kernel is loaded at 0x00100000
2. LetkuOS uses a next-fit singly linked list for physical memory allocation and deallocation
3. by default the allocater allocates 4kb memory units (size of pages, so we can easily use paging)
4. the singly linked list needs to store info of pagenr AVAILABLE MEMORY / 4096 pages
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

printf("the machine has (roughly) %d megs of (upper) RAM!\n", bootinfo->mem_upper/1024);

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
		printf("found a contiguous usable uppermem area, start: 0x%xh, length: %d bytes!\n",mmap.base, mmap.size);
		break;
		}
	// debug: print the memory map
//	printf ("size = %d, base_addr = 0x%x, length = %d, type = %d\n", (unsigned) memmap->size, base, len, (unsigned) memmap->type);
	}

if (mmap.size == 0)
	panic("No contiguous usable upper memory area found");


// 3. set aside a region of memory to be used for the singly linked list of available memory
unsigned long pagenr = mmap.size / 4096;
unsigned long manager_reserved = sizeof(struct physmem) * pagenr;
// for debug
printf("need to reserve for kernel: %x\n", &endofkernel);
printf("need for pages: %d\nneed to reserve for memory manager: 0x%xh bytes\n",pagenr, manager_reserved);
printf("need for kernel + memory manager: 0x%xh\n",(&endofkernel + manager_reserved));
// 4. make sure the reserved memory for memory manager resides in right after the memory reserved for kernel: endofkernel
// when a pointer points to endofkernel and all the following structs will be loaded AFTER it, we know the whole singly linked list will be between endofkernel and endofkenerl + manager_reserved

//point a pointer at endofkernel and init a physmem struct there: this is our first memory allocation
physmemptr = &endofkernel;

//printf("physmemptr before: 0x%xh\n",physmemptr);
unsigned int *kernel_and_mm = kmalloc(((int) &endofkernel + manager_reserved));
// if everything is okay, kernal_and_mm should be equal to physmemptr before, and physmemptr after should be physmemptr + sizeof(struct physmem)
//printf("physmemptr after: 0x%xh\n",physmemptr);
//printf("kernal_and_mm after: 0x%xh\n",kernal_and_mm);
//printf("sizeof: 0x%xh\n",sizeof(struct physmem));


init_paging();
return;
}



/////////////////////////////////////////////
// Initialize paging
/////////////////////////////////////////////
void init_paging()
{
}


/////////////////////////////////////////////////////////////////
// Allocate memsize worth of memory and return a pointer to its base
/////////////////////////////////////////////////////////////////
void *kmalloc(int memsize)
{
// everything below this should be done in kmalloc..
struct physmem *temp = (struct physmem *) physmemptr;

temp->used = 1;
temp->base = (int) &physmemptr;
temp->size = memsize;
temp->next = (unsigned int) physmemptr + sizeof(struct physmem);
physmemptr = temp->next;
printf("reserved 0x%xh bytes, next pointer shall be at: 0x%xh\n",temp->size, physmemptr);
return temp;

}

/////////////////////////////////////////////////////////////////
// Deallocate memory pointed to by base
/////////////////////////////////////////////////////////////////
void kfree(void *ptr)
{
// defrag the singly linked list
mm_defrag();
}

/////////////////////////////////////////////////////////////////
// Defrag the singly linked list by merging adjacent free areas
/////////////////////////////////////////////////////////////////
void mm_defrag()
{
// loop from beginning until end of memory
// keep a counter of previous block and it's free state
// when a free block is found, merge it with the previous block if that block is free
}
