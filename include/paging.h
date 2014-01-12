/* paging.h
* x86 paged memory management functions
*/
#ifndef  _letkuos_paging_h
#define _letkuos_paging_h _letkuos_paging_h

void init_paging();

#define PAGESIZE 	4096	// 4kb pages

// paging directories / tables in a nutshell:
// 1. a page directory (page_directory) is the topmost structure. It holds 1024 page directory entries (directory)
// 2. each directory entry points to a page table (page table) plus has some additional information
// 3. each page table contains 1024 page entries (pages)

// a page entry. from wiki.osdev.org and a JamesM tutorial
// the bits idea copied from a JamesM tutorial. Never encountered the : 1 for bits before. a struct page is eactly 32 bits long

typedef struct page
{
	int present	: 1;	// Page present in memory (1), not present (0)
	int rw		: 1;	// Read-only if 0, readwrite if 1
	int user	: 1;	// Supervisor level only if 0, user if 1
	int writethru	: 1;	// ??
	int cachedisab	: 1; 	// ??
	int accessed	: 1;	// Has the page been accessed since last refresh?
	int dirty	: 1;	// Has the page been written to since last refresh?
	int reserved	: 1;	// always 0
	int global	: 1;	// has to do with TLB;
	int unused	: 3;	// Amalgamation of unused and reserved bits
	int address	: 20;	// Frame address (physical page address), shifted right 12 bits to make 4kb aligned addresses
} page_t;

// a page directory entry
typedef struct directory
{
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
} directory_t;

// a page table, contains 1024 page entries, thus controls 1024*4kb = 4mb worth of memory
typedef struct page_table
	{
	page_t pages[1024];
	} page_table_t;


// a page directory. Contains pointers to 1024 page tables.
// pointers, because page tables reside in memory somewhere
typedef struct page_directory
	{
	directory_t *direntries[1024];
	} page_directory_t;

#endif
