/* errors.h
* error handling stuff
*/
#ifndef _letkuos_errors_h
#define _letkuos_errors_h _letkuos_errors_h
extern void panic(const char *fmt, ...);

#define ERR_KEYB_INIT		"Keyboard initialization error"
//#define ERR_ATA_INIT	"ATA driver initialization error"
#define ERR_IRQ_NOHANDLER	"Unknown IRQ or exception!"
#define ERR_ATA_NODRIVES	"No ATA drives found"
#define ERR_ATA_NOMBR		"No valid MBR found on any drive"
#define ERR_ATA_NOPARTTABLES	"No valid partition tables found on any drive"

#endif
