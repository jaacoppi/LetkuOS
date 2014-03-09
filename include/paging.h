/* paging.h
* x86 paged memory management functions
*/
#ifndef  _letkuos_paging_h
#define _letkuos_paging_h _letkuos_paging_h

void init_paging();
int paging_mapvirt2phys(int virtaddr, int physframe);
int paging_remap(int newvirt, int oldvirt);
int paging_protection(int virtaddr, char flags);
int paging_present(int virtaddr, char present);
int paging_findphysframe(int virtaddr);

#define PAGESIZE 	4096	// 4kb pages

// paging directories / tables in a nutshell:
// 1. a page directory (page_directory) is the topmost structure. It holds 1024 page directory entries (directory)
// 2. each directory entry points to a page table (page table) plus has some additional information
// 3. each page table contains 1024 page entries (pages)

// a page entry explained . from wiki.osdev.org and a JamesM tutorial
// the bits idea copied from a JamesM tutorial. Never encountered the : 1 for bits before. a struct page is eactly 32 bits long

/* page entry is 32 bits. It's filled with a 32 bit, 4 byte aligned address. This means the following:

present		: 1;	// Page present in memory (1), not present (0)
rw		: 1;	// Read-only if 0, readwrite if 1
user		: 1;	// Supervisor level only if 0, user if 1
writethru	: 1;	// ??
cachedisab	: 1; 	// ??
accessed	: 1;	// Has the page been accessed since last refresh?
dirty	: 1;	// Has the page been written to since last refresh?
reserved	: 1;	// always 0
global	: 1;	// has to do with TLB;
unused	: 3;	// Amalgamation of unused and reserved bits
address	: 20;	// Frame address (physical page address), shifted right 12 bits to make 4kb aligned addresses
*/

// bit mask definitions for a page entry
#define PAGE_PRES_RW_KRN	3 // bits present 1 , rw 1, user 0
#define PAGE_NOTPRES_RW_KRN	2 // bits present 0 , rw 1, user 0

/* a page directory entry. The flags we use (present, rw, user) are the same, so we don't need more defines
int present	: 1;	// Page present in memory (1), not present (0)
int rw		: 1;	// Read-only if 0, readwrite if 1
int user	: 1;	// Supervisor level only if 0, user if 1
int writethru	: 1;	// ??
int cachedisab	: 1; 	// ??
int accessed	: 1;	// Has the page been accessed since last refresh? (remember, not cleared / set by CPU
int pagesize	: 1;	// 0 for 4kb pages, 1 for 4mb
int reserved	: 1;	// ignored, don't touch
int global	: 1;	// has to do with TLB;
int unused	: 3;	// Amalgamation of unused and reserved bits
int pagetable_addr	: 20;	// physical address of a page table, fits addresses/indexes until 0x80000
*/

#endif
