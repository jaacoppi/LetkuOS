/* keyboard.h */
#ifndef _letkuos_keyboard_h
#define _letkuos_keyboard_h  _letkuos_keyboard_h

/* some register defines */

extern void init_keyboard();
void scancode_handler (unsigned char scancode);

#define KEYB_DATA       0x60
#define KEYB_CONTROL	0x64
#endif
