/* keyboard.c - the name says it all */
/* parts borrowed from Bran's kernel development tutorial */
#include "isr.h"
#include "irq.h"
#include "portio.h"
#include "keyboard.h"
#include "stdio.h"

void keyboard_handler(struct registers *r);
extern void los_reboot();

unsigned char keymap_us[128] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',     /* 9 */
  '9', '0', '-', '=', '\b',     /* Backspace */
  '\t',                 /* Tab */
  'q', 'w', 'e', 'r',   /* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', /* Enter key */
    0,                  /* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',     /* 39 */
 '\'', '`',   0,                /* Left shift */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',                    /* 49 */
  'm', ',', '.', '/',   0,                              /* Right shift 
*/
  '*',
    0,  /* Alt */
  ' ',  /* Space bar */
    0,  /* Caps lock */
    0,  /* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,  /* < ... F10 */
    0,  /* 69 - Num lock*/
    0,  /* Scroll Lock */
    0,  /* Home key */
    0,  /* Up Arrow */
    0,  /* Page Up */
  '-',
    0,  /* Left Arrow */
    0,
    0,  /* Right Arrow */
  '+',
    0,  /* 79 - End key*/
    0,  /* Down Arrow */
    0,  /* Page Down */
    0,  /* Insert Key */
    0,  /* Delete Key */
    0,   0,   0,
    0,  /* F11 Key */
    0,  /* F12 Key */
    0,  /* All other keys are undefined */
};


void init_keyboard() {
irq_install_handler(1, keyboard_handler);
}


/* Handles the keyboard interrupt */
void keyboard_handler(struct registers *r)
{
    unsigned char scancode;
    /* Read from the keyboard's data buffer */
    scancode = inb(KEYB_DATA);

	/* hardware reset */
	if (scancode == 88) /* F12 */
		los_reboot();

	/* handle key release by skipping it for now */
	if (scancode > 128)
		return;

 	/* else, sent the keycode to the actual keyboard driver */
		scancode_handler(scancode);
}




void scancode_handler (unsigned char scancode)
{
printf("%c",keymap_us[scancode]);
}

