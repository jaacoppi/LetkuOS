/* mm.h
* memory management functions
*/
#ifndef  _letkuos_mm_h
#define _letkuos_mm_h _letkuos_mm_h

#define vmmlist_insert_known(memsize, ptr) vmmlist_insert(memsize,ptr)
#define vmmlist_insert_unknown(memsize) vmmlist_insert(memsize,NULL)

#include "multiboot.h"

void *kmalloc(unsigned long memsize);
int pmm_getframe();
// the usable memory area is divided into a singly linked list of struct physmems.
// a list item can control anything from a 4kb to memorymap.size (=all memory available) chunk of memory
// if all memory allocated is 4kb, the singly linked list is full and there can be no more items
struct vmmlist
        {
        short used; // 0 for free, 1 for used
        unsigned long base;	// virtual address base
        unsigned long size;	// size of memory allocated, in bytes
        struct vmmlist *next;	// the address of next item in this singly linked list. if NULL, this is the last imte
        };

void init_mm();
void *kmalloc(unsigned long memsize);
void *kmalloc_known(void *addr, unsigned long memsize, int protection);
void kfree(void *ptr);
void *krealloc(void *ptr, unsigned long memsize);
void mm_defrag();
int list_insert(unsigned long memsize, struct vmmlist *linkedlistptr);

extern unsigned int *kernel_pagedir; // used by vmm to find out virtual to physical mappings
// for the linked list heap manager
#define MMLIST_LAST	0x108
#define MM_FREE		0x123
#define MM_USED		0x321

// start of the heap manager
struct vmmlist *vmmlist_start;

void init_memory(struct multiboot_info *bootinfo);

// the available memory for use is stored in struct memorymap
struct memorymap
        {
        unsigned long base; // points to the address of physical memory base
        unsigned long size;      // size of memory, in bytes
        };

#endif
