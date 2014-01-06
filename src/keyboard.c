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
#include "elf.h"
#include "mm.h"

#define KEYB_CONTROL_CONFIG_BYTE_READ 0x20
#define KEYB_CONTROL_CONFIG_BYTE_WRITE 0x60
/* make sure we use scan code set 2 */
#define KEYB_SCANCODE_CMD 0xF0
#define KEYB_SCANCODE_SET2 0x02 /* set bit 2 to 1 if you want scancode set 2 */

void keyb_outb(int port, int data);
unsigned int keyb_inb(int port);
void keyboard_handler(struct registers *r);
extern void los_reboot();
extern char keymap_fi[3][256];
int layer = 0; /* shift = layer1, alt = layer2 */
int keyb_keyup = 0; /* controls shift and alt */

/* init the keyboard */
void init_keyboard() {
// TODO: use a timer for timeouts to keyboard read and writes
/* TODO: see if you need to flush the input buffer with keyb_inb calls after outb to clear ACKs and other return values */

unsigned char recbyte;
/* reset keyboard - test 5 times if it exists and works.
Note that this is a quick & dirty hack, not the proper way to check whether or not a keyboard exists */
int i, j = 0;
for (i = 0; i < 5; i++)
	{
	keyb_outb(KEYB_CONTROL,KEYB_SELFTEST);
	recbyte = keyb_inb(KEYB_DATA);

	if (recbyte != KEYB_SELFTESTOK) /* self test failed */
		j++;
	}

if (j != 0)
	printf("WARN: keyboard self test failed %d times\n",j);

/* read the ps2 control byte */
keyb_outb(KEYB_CONTROL, KEYB_CONTROL_CONFIG_BYTE_READ);
recbyte = keyb_inb(KEYB_DATA);

/* write the control after xor'ing (=toggling) the translation bit */
recbyte = recbyte ^ BIT6;	/* XOR - change nothing but the translation bit */
keyb_outb(KEYB_CONTROL, KEYB_CONTROL_CONFIG_BYTE_WRITE);
keyb_outb(KEYB_DATA, recbyte);

/* use scancode set 2. This should be default, but just in case */
keyb_outb(KEYB_DATA, KEYB_SCANCODE_CMD);
keyb_outb(KEYB_DATA, KEYB_SCANCODE_SET2);

// Set scancode command sends an ACK when a scancode is set. Discard it by flushing the output buffer
// this seems to work best for both bochs and qemu, since bochs returns more bytes than qemu
while (inb(KEYB_CONTROL) & BIT0)
	keyb_inb(KEYB_DATA);

/* configure the IRQ handler. This should be the last call so we can safely init the keyboard first without needing to worry about interrupts */
irq_install_handler(1, keyboard_handler);
}


/* Handles the keyboard interrupt */
void keyboard_handler(struct registers *r)
{
/* NOTE: keypress = make code, key release / keyup = break code */

unsigned char scancode;
/* Read from the keyboard's data buffer. We don't need keyb_inb since this is an IRQ and the buffer has to have something because the IRQ fired */
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

if (scancode == KEYB_CTRL_ACK) /* this scancode is an ack from the previous command. We should never get this when doing normal user interaction, only in the init phase. This is here just in case, but slows down the keyboard routine. */
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
//		fat_readdir(4); // rootdevice.rootcluster == 2
		// the cluster that has the rest of /boot/grub is 0x010B
//		fat_readdir(0x13896); // \home
		fat_readdir(0x13896); // rootdevice.rootcluster == 2
		break;

	case '4':
//		TODO: be able to follow the clusterchain of prince.txt - 0x3897 - high byte problem?
		follow_clusterchain(0x13897); // prince.txt?
//		int i = get_cluster_value(0x13897); // prince.txt?
//		printf("cluster value is 0x%xh\n",i);

//		follow_clusterchain(0x7); // rootdevice.rootcluster == 2
		break;

	case '3':
		debug_showfat(625);
		extern struct fat32_BS rootdevice;
		printf("note that the fat is %d sectors and it starts from 0\n",rootdevice.sectors_per_fat);
		printf("after that, there's the copy of fat that's identical to the first one!\n");
		break;

	case '5':
		{
//		int i =fat_parse_path("\\boot\\grub\\menu.lst");
		int i =fat_parse_path("\\home\\prince.txt");
		if (i == -1)
			printf("Error, invalid directory or not an absolute path\n");
		break;
		}

	case '6':
		{
		parse_elf("\\boot\\grub\\menu.lst");
		break;
		}

	case '7':
		{
		int *fileptr = fat_loadfile("\\home\\prince.txt");
		printf("loaded a file at 0x%xh\n",fileptr);
		int i = 0;
		printf("text file starting here:\n");
		for (i = 1024; i < 1220; i++) // arbitrary location in file for testing purposes
			printf("%c",fileptr[i]);
		break;
		}
	default:
		printf("scancode: 0x%xh, key: %c\n",scancode, keymap_fi[layer][scancode]);
	}
}


////////////////////////////////////////////
// outb commands to the keyboard after checking for input buffer empty/full
////////////////////////////////////////////

void keyb_outb(int port, int data)
{
// TODO: use a timer for timeouts to keyboard read and writes

// KEYB_CONTROL 0x64 is the Status Register when reading from it.
// loop until bit 1 is clear (input buffer is empty, we can put our own data there
while (keyb_inb(KEYB_CONTROL) & BIT1);

// do a normal outb
outb(port,data);
}

////////////////////////////////////////////
// inb commands to the keyboard after checking for output buffer empty/full
////////////////////////////////////////////

unsigned int keyb_inb(int port)
{
// TODO: use a timer for timeouts to keyboard read and writes

// KEYB_CONTROL 0x64 is the Status Register when reading from it.
// loop until bit 0 is set (output buffer is full, there's something to read
while (!inb(KEYB_CONTROL) & BIT0);

int retval = inb(port);

if (retval == KEYB_CTRL_ACK) // don't care for ACKS when initializing keyboard. This means no error checking, which is bad.. TODO: fix it by checking for retval 0 in the init routine
	return 0;

// TODO: if you receive a 0xFE, the keyboard asks you to resend the previous command
if (retval == KEYB_RESEND)
	panic("keyboard sent a RESEND command, but it's not implemented yet!\n");

return retval;
}

