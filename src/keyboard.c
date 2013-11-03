/* keyboard.c - the name says it all */
/* parts borrowed from Bran's kernel development tutorial */
/* info from
http://www.osdever.net/documents/kbd.php?the_id=14
http://www.brokenthorn.com/Resources/OSDev19.html
and from Acess2 */

#include "isr.h"
#include "irq.h"
#include "portio.h"
#include "keyboard.h"
#include "stdio.h"
#include "letkuos-common.h"

/* TODO: keyboard init should be done before irq_install_handler: poll the keyboard, don't use interrupts */
int ps2_control_byte = 1; /* a lousy way to code reading ps2 control byte */
int keyb_keyup = 0;

#define KEYB_CONTROL_CONFIG_BYTE_READ 0x20
#define KEYB_CONTROL_CONFIG_BYTE_WRITE 0x60

void keyboard_handler(struct registers *r);
extern void los_reboot();
extern char keymap_fi[3][256];
int layer = 0; /* shift = layer1, alt = layer2 */

void init_keyboard() {
/* configure the IRQ handler */
irq_install_handler(1, keyboard_handler);

/* init the keyboard */
/* TODO: Basic Assurance Self Test and other inits to make sure the keyboard is actually connected */

/* TODO: keyboard init should be done before irq_install_handler: poll the keyboard, don't use interrupts */
/* make sure we use scan code set 2 */
#define KEYB_SCANCODE_CMD 0xF0
#define KEYB_SCANCODE_SET2 0x4 /* set bit 2 to 1 if you want scancode set 2 */

/* read the ps2 control byte */
outb(KEYB_CONTROL, KEYB_CONTROL_CONFIG_BYTE_READ);

/* use scancode set 2 */
outb(KEYB_DATA, KEYB_SCANCODE_CMD);
outb(KEYB_DATA, 1 & BIT2);

//i = inb(KEYB_DATA);
//printf("scancode here: 0x%xh\n",i);

//outb(KEYB_DATA, KEYB_SCANCODE_SET2);

}


/* Handles the keyboard interrupt */
void keyboard_handler(struct registers *r)
{
/*
keypress = make code
key release = break code

break code seems to be make code + 0x80h
most shift keys seem to be 
*/

unsigned char scancode;
/* Read from the keyboard's data buffer */
scancode = inb(KEYB_DATA);

/* the scancode (and the next one) is a key up code - do nothing */
if (scancode == KEYB_BREAK_CODE)
	{
	keyb_keyup = 1;
	return;
	}

if (keyb_keyup)
	{
	/* if it's the keyup for shift, stop capitalization */
	if (scancode == KEY_LS)
		layer = 0;

	keyb_keyup = 0;
	return;
	}

/* left shift is down - capitalize */
if (scancode == KEY_LS)
	{
	layer = 1;
	return;
	}

if (scancode == KEYB_CTRL_ACK) /* this scancode is an ack from the previous command */
	return;


/* used to disable scancode translation. TODO: poll in init before enabling keyboard IRQ */
if (ps2_control_byte)
	{
	/*
	send the CONTROL_CONFIG_BYTE as 0000 0101 - basically disables port clock and translation
	TODO: figure out what port clock does
	*/
	outb(KEYB_CONTROL, KEYB_CONTROL_CONFIG_BYTE_WRITE);
	outb(KEYB_DATA, 5); /* 0000 0101 */ 
	ps2_control_byte = 0;
	return;
	}

	/* hardware reset */
	if (scancode == KEY_F12)
		los_reboot();

	/* handle key release by skipping it for now */
//	if (scancode > 128)
//		return;

 	/* else, sent the keycode to the actual keyboard driver */
		scancode_handler(scancode);
}




void scancode_handler (unsigned char scancode)
{
printf("scancode: 0x%xh, key: %c\n",scancode, keymap_fi[layer][scancode]);
//printf("here: %d\n",scancode);
}

