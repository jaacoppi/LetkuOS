//
// isr.h -- Interface and structures for high level interrupt service routines.
// Part of this code is modified from Bran's kernel development tutorials.
// Rewritten for JamesM's kernel development tutorials.
//

#ifndef _letkuos_isrs_h
#define _letkuos_isrs_h  _letkuos_isrs_h

//
// isr.h -- Interface and structures for high level interrupt service routines.
// Part of this code is modified from Bran's kernel development tutorials.
// Rewritten for JamesM's kernel development tutorials.
//

typedef struct registers
{
	int gs, fs, es, ds;
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

extern char *exception_messages[];

#endif
