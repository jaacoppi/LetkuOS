/* keyboard.h */
#include "letkuos-common.h"

#ifndef _letkuos_keyboard_h
#define _letkuos_keyboard_h  _letkuos_keyboard_h

/* some register defines */

extern void init_keyboard();
void scancode_handler (unsigned char scancode);

#define KEYB_DATA       0x60
#define KEYB_CONTROL	0x64

#define KEYB_CTRL_ACK	0xFA /* keyboard acks commands with this */

#define KEYB_BREAK_CODE	0xF0	/* the next scancode received is a key up */
/* These are only valid for scancode set 2 */
/* SPECIAL KEYS */
#define KEY_F1	0x05
#define KEY_F2	0x06
#define KEY_F3	0x04
#define KEY_F4	0x0C
#define KEY_F5	0x03
#define KEY_F6	0x0B
#define KEY_F7	0x83
#define KEY_F8	0x0A
#define KEY_F9	0x01
#define KEY_F10	0x09
#define KEY_F11	0x78
#define KEY_F12	0x07
#define KEY_TAB	0x0D
#define KEY_LALT 0x11
#define KEY_LS	0x12 /* Left shift */
#define KEY_LCTRL 0x14
#define KEY_SPACE	0x29
#define KEY_CAPS	0x58
#define KEY_RS		0x59
#define KEY_ENT		0x5A
#define KEY_BSPACE	0x66
#define KEY_NUM1	0x69
#define KEY_NUM2	0x72
#define KEY_NUM3	0x7A
#define KEY_NUM4	0x6B
#define KEY_NUM5	0x73
#define KEY_NUM6	0x74
#define KEY_NUM7	0x6C
#define KEY_NUM8	0x75
#define KEY_NUM9	0x7D
#define KEY_NUM0	0x70
#define KEY_NUMDOT	0x71
#define KEY_ESC		0x76
#define KEY_NUMLOCK	0x77
#define KEY_SCROLLLOCK	0x7E

#endif
