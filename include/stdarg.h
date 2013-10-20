/* http://www.sivachandran.in/2006/07/possible-implementation-of-valist.html */
/*I found this in C/C++ Users Journal Jan 1990 */

#ifndef _letkuos_stdarg_h
#define _letkuos_stdarg_h _letkuos_stdarg_h


/* va_list implementation from TCC (Tiny C Compiler) stdarg.h. File didn't have copyright info. */
typedef char * va_list;

#define va_start(ap,last) ap = ((char *)&(last)) + ((sizeof(last)+3)&~3)
#define va_arg(ap,type) (ap += (sizeof(type)+3)&~3, *(type *)(ap - ((sizeof(type)+3)&~3)))
#define va_copy(dest, src) (dest) = (src)
#define va_end(ap)


#endif
