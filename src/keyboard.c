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
#include "errors.h"
#include "fat.h"

#define KEYB_CONTROL_CONFIG_BYTE_READ 0x20
#define KEYB_CONTROL_CONFIG_BYTE_WRITE 0x60
/* make sure we use scan code set 2 */
#define KEYB_SCANCODE_CMD 0xF0
#define KEYB_SCANCODE_SET2 0x4 /* set bit 2 to 1 if you want scancode set 2 */


void keyboard_handler(struct registers *r);
extern void los_reboot();
extern char keymap_fi[3][256];
int layer = 0; /* shift = layer1, alt = layer2 */
int keyb_keyup = 0; /* controls shift and alt */

/* init the keyboard */
void init_keyboard() {
/* TODO: see if you need to flush the input buffer with inb calls after outb to clear ACKs and other return values */
/* TODO: Basic Assurance Self Test and other inits to make sure the keyboard is actually connected */
unsigned char recbyte;
/* reset keyboard - test if it exists and works */
outb(KEYB_CONTROL,KEYB_SELFTEST);
recbyte = inb(KEYB_DATA);

if (recbyte != KEYB_SELFTESTOK) /* self test failed */
	{
	panic("Keyboard self test failed!\n");
	return;
	}

/* read the ps2 control byte */
outb(KEYB_CONTROL, KEYB_CONTROL_CONFIG_BYTE_READ);
recbyte = inb(KEYB_DATA);

/* write the control after xor'ing (=toggling) the translation bit */
recbyte = recbyte ^ BIT6;	/* XOR - change nothing but the translation bit */
outb(KEYB_CONTROL, KEYB_CONTROL_CONFIG_BYTE_WRITE);
outb(KEYB_DATA, recbyte);

/* use scancode set 2. This should be default, but just in case */
outb(KEYB_DATA, KEYB_SCANCODE_CMD);
outb(KEYB_DATA, 0x2); // docs say only BIT2 should be set for scancode set2. 0x2, however, is BIT1. Strange

/* configure the IRQ handler. This should be the last call so we can safely init the keyboard first without needing to worry about interrupts */
irq_install_handler(1, keyboard_handler);
}


/* Handles the keyboard interrupt */
void keyboard_handler(struct registers *r)
{
/* NOTE: keypress = make code, key release / keyup = break code */

unsigned char scancode;
/* Read from the keyboard's data buffer */
scancode = inb(KEYB_DATA);

/* this scancode means that the next one) is a key up code - do nothing this time */
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
	printf("layer!\n");
	layer = 1;
	return;
	}

if (scancode == KEYB_CTRL_ACK) /* this scancode is an ack from the previous command */
	return;

/* else, sent the keycode to the actual keyboard driver */
	scancode_handler(scancode);
}




void scancode_handler (unsigned char scancode)
{
/* for debug purposes: a keypress does something */
switch (keymap_fi[layer][scancode])
	{
	case KEY_F12: /* software reset */
		los_reboot();
		break;
	case '1':
		cls();
		break;
	case '2':
		fat_readdir(3); // rootdevice.rootcluster == 2
		break;

	case '3':
		debug_showfat(0);
		break;
	default:
		printf("scancode: 0x%xh, key: %c\n",scancode, keymap_fi[layer][scancode]);
	}
}

