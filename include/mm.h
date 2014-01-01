/* mm.h
* memory management functions
*/
#ifndef  _letkuos_mm_h
#define _letkuos_mm_h _letkuos_mm_h

#define list_insert_known(memsize, ptr) list_insert(memsize,ptr)
#define list_insert_unknown(memsize) list_insert(memsize,NULL)

#include "multiboot.h"

// the usable memory area is divided into a singly linked list of struct physmems.
// a list item can control anything from a 4kb to memorymap.size (=all memory available) chunk of memory
// if all memory allocated is 4kb, the singly linked list is full and there can be no more items
struct physmem
        {
        short used; // 0 for free, 1 for used
        unsigned long base;	// address of physical memory base
        unsigned long size;	// size of memory allocated, in bytes
        unsigned long next;	// the address of next item in this singly linked list. if NULL, this is the last imte
        };

void init_mm();
void init_paging();
void *kmalloc(unsigned long memsize);
void kfree(void *ptr);
void *krealloc(void *ptr, unsigned long memsize);
void mm_defrag();
int list_insert(unsigned long memsize, struct physmem *linkedlistptr);

// for the linked list
#define MMLIST_LAST	0x108
// for the singly linked next fit memory manager
#define MM_UNINIT	0x0
#define MM_FREE		0x123
#define MM_USED		0x321


void init_memory(struct multiboot_info *bootinfo);

// the available memory for use is stored in struct memorymap
struct memorymap
        {
        unsigned long base; // points to the address of physical memory base
        unsigned long size;      // size of memory, in bytes
        };

#endif
