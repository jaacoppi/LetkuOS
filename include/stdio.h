/* stdio.h
* (loosely) standard c library
*/

#include "stdarg.h"
#include "scrn.h"
#ifndef _LETKUOS_stdio_h
#define _LETKUOS_stdio_h _LETKUOS_stdio_h

/* the OS glue writech resides in scrn.c */

/* TODO: make %s use helpstring as well, use a common printing code */

/* a home made printf implementation, probably quite buggt */

extern void printf(const char *fmt, ...);
#endif

