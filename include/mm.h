/* mm.h
* memory management functions
*/
#ifndef  _letkuos_mm_h
#define _letkuos_mm_h _letkuos_mm_h

void init_memory(struct multiboot_info *bootinfo);

struct memorymap
        {
        unsigned long base; // points to the address of physical memory base
        unsigned long size;      // size of memory, in bytes
        };

struct physmem
        {
        short used; // 0 for free, 1 for used
        unsigned long base; // points to the address of physical memory base
        unsigned long size;      // size of memory
        unsigned long next;      // the address of next item in this singly linked list
        };

#endif
