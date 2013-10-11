//
// isr.h -- Interface and structures for high level interrupt service routines.
// Part of this code is modified from Bran's kernel development tutorials.
// Rewritten for JamesM's kernel development tutorials.
//


//
// isr.h -- Interface and structures for high level interrupt service routines.
// Part of this code is modified from Bran's kernel development tutorials.
// Rewritten for JamesM's kernel development tutorials.
//

typedef struct registers
{
   int ds;                  // Data segment selector
   int edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha.
   int int_no, err_code;    // Interrupt number and error code (if applicable)
   int eip, cs, eflags, useresp, ss; // Pushed by the processor automatically.
} registers_t;


#define ERR_0 0
#define ERR_1 1
#define ERR_2 2
#define ERR_3 3
#define ERR_4 4
#define ERR_5 5
#define ERR_6 6
#define ERR_7 7
#define ERR_8 8
#define ERR_9 9
#define ERR_10 10
#define ERR_11 11
#define ERR_12 12
#define ERR_13 13
#define ERR_14 14
#define ERR_15 15
#define ERR_16 16
#define ERR_17 17
#define ERR_18 18
#define ERR_19 19
#define ERR_20 20
#define ERR_21 21
#define ERR_22 22
#define ERR_23 23
#define ERR_24 24
#define ERR_25 25
#define ERR_26 26
#define ERR_27 27
#define ERR_28 28
#define ERR_29 29
#define ERR_30 30
#define ERR_31 31


/*

0 Division By Zero Exception
1 Debug Exception
2 Non Maskable Interrupt Exception
3 Breakpoint Exception
4 Into Detected Overflow Exception
5 Out of Bounds Exception
6 Invalid Opcode Exception
7 No Coprocessor Exception
8 Double Fault Exception
9 Coprocessor Segment Overrun Exception
10Bad TSS Exception
11Segment Not Present Exception
12Stack Fault Exception
13General Protection Fault Exception
14Page Fault Exception
15Unknown Interrupt Exception
16Coprocessor Fault Exception
17Alignment Check Exception (486+)
18Machine Check Exception (Pentium/586+)        No
19 to 31        Reserved Exceptions
*/
