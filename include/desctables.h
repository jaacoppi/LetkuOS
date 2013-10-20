/* desctables.h - declarations for GDT and IDT */
#define NUM_IDT 47
#ifndef _letkuos_desctables_h
#define _letkuos_desctables_h _letkuos_desctables_h

int init_gdt();
/* Some code copied from:
James Malloy. http://www.jamesmolloy.co.uk/tutorial_html/4.-The%20GDT%20and%20IDT.html 
Brandon Friesen. http://www.osdever.net/bkerndev/Docs/gdt.htm 
*/

/* all structs need to be packed to prevent compiler optimizations that delete empty bits and thus screw this up */

/* this is the gdt_entry that will be sent to the cpu */

struct gdt_entry_struct
{
// gcc uses u16int and u8int
// tcc uses unsigned short int and unsigned char
//   u16int limit_low;           // The lower 16 bits of the limit.
	unsigned short limit_low;           // The lower 16 bits of the limit.
	unsigned short  base_low;            // The lower 16 bits of the base.
	unsigned char  base_middle;         // The next 8 bits of the base.
	unsigned char access;              // Access flags, determine what ring this segment can be used in.
	unsigned char granularity;
	unsigned char base_high;           // The last 8 bits of the base.
} __attribute__((packed));
typedef struct gdt_entry_struct gdt_entry_t;

/* this is the pointer to be used with lgdt that points to the gdt_entry */
struct gdt_ptr_struct
{
	unsigned short limit;
	unsigned int base;
//   u16int limit;               // The upper 16 bits of all selector limits.
//   u32int base;                // The address of the first gdt_entry_t struct.
}
 __attribute__((packed));
typedef struct gdt_ptr_struct gdt_ptr_t; 

/* A struct describing an interrupt gate. */
struct idt_entry_struct
{
/*
   u16int base_low;             // The lower 16 bits of the address to jump to when this interrupt fires.
   u16int sel;                 // Kernel segment selector.
   u8int  reserved_0;             // This must always be zero.
   u8int  flags;               // More flags. See documentation.
   u16int base_hight;             // The upper 16 bits of the address to jump to.
*/

	unsigned short int base_low;
	unsigned short int  sel;
	unsigned char reserved_0;
	unsigned char flags;
	unsigned short int base_high;

} __attribute__((packed));
typedef struct idt_entry_struct idt_entry_t;

// A struct describing a pointer to an array of interrupt handlers.
// This is in a format suitable for giving to 'lidt'.
struct idt_ptr_struct
{
//   u16int limit;
//   u32int base;                // The address of the first element in our idt_entry_t array.

	unsigned short int limit;
	int base;
} __attribute__((packed));
typedef struct idt_ptr_struct idt_ptr_t;

int init_idt();

/* idt flush from boot.asm */
extern void idt_flush();


/* handlers found in boot.asm */
extern void isr0 ();
extern void isr1 ();
extern void isr2 ();
extern void isr3 ();
extern void isr4 ();
extern void isr5 ();
extern void isr6 ();
extern void isr7 ();
extern void isr8 ();
extern void isr9 ();
extern void isr10 ();
extern void isr11 ();
extern void isr12 ();
extern void isr13 ();
extern void isr14 ();
extern void isr15 ();
extern void isr16 ();
extern void isr17 ();
extern void isr18 ();
extern void isr19 ();
extern void isr20 ();
extern void isr21 ();
extern void isr22 ();
extern void isr23 ();
extern void isr24 ();
extern void isr25 ();
extern void isr26 ();
extern void isr27 ();
extern void isr28 ();
extern void isr29 ();
extern void isr30 ();
extern void isr31 ();

int idt_set_gate(unsigned char num, int base, unsigned short int sel, unsigned char flags);
int gdt_set_gate(int num, unsigned long base, unsigned long limit, unsigned char access, unsigned char gran);


#endif
