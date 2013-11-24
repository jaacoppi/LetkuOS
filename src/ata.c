/* ata.c
* physical hard drive (ata) driver
*/

#include "ata.h"
#include "irq.h"
#include "isr.h"
#include "stdio.h"
#include "letkuos-common.h"
#include "portio.h"
#include "string.h"

/* this driver is limited to 4 primary partitions. They're called hda-hdd, linux style */
struct hd hda, hdb, hdc, hdd;

extern struct multiboot_info bootinfo;
void check_ata_exists(int controlsel, int drivesel,  struct hd *harddrive);
void ata_handler(struct registers *r);
int ata_outb(int port, unsigned char outbyte);
int ata_inb(int port);
int ata_drsel(int controlsel, int drsel);

int ata_dataport, ata_mastersel; /* these control which controller/drive to inb and outb */



/* used info from:
http://www.osdever.net/./tutorials/lba.php
http://wiki.osdev.org/ATA_PIO_Mode

This is a n lba28 driver, doesn't work with hard drives without lba
*/

///////////////////////////////////////////////////////////
// init_ata - 	install IRQ handlers, check that drives exist
///////////////////////////////////////////////////////////
void init_ata()
{
/* Installs 'ata_handler' to IRQ14 and IRQ15, master and slave */
irq_install_handler(14, ata_handler);
irq_install_handler(15, ata_handler);

/* this driver is limited to master drive on 1st bus */
check_ata_exists(ATA_PRI_DATAPORT, MASTER_HD, &drive[0]);
check_ata_exists(ATA_PRI_DATAPORT, SLAVE_HD, &drive[1]);
check_ata_exists(ATA_SEC_DATAPORT, MASTER_HD, &drive[2]);
check_ata_exists(ATA_SEC_DATAPORT, SLAVE_HD, &drive[3]);

}

//////////////////////////////////////////////////////
// check_ata_exists - checks which hard drives exist
//////////////////////////////////////////////////////
void check_ata_exists(int controlsel, int drivesel, struct hd *harddrive) {

/* use ATA IDENTIFY to see if the drive exists */
/* see wiki.osdev.for details */

/* first, choose the controller and drive. These are given as parameters.  */
/* after ata_drsel, all outb and inb are done to this specific drive */
ata_drsel(controlsel, drivesel);

/* Then set the Sectorcount, LBAlo, LBAmid, and LBAhi IO ports to 0 (port 0x1F2 to 0x1F5). */
ata_outb(ata_dataport + ATA_SCOUNT,0);
ata_outb(ata_dataport + ATA_ADDR,0);
ata_outb(ata_dataport + ATA_ADDR8,0);
ata_outb(ata_dataport + ATA_ADDR16,0);

/* Then send the IDENTIFY command (0xEC) to the Command IO port (0x1F7). */
ata_outb(ata_dataport + ATA_CMDSTATUS, ATA_IDENTIFY);
/* Then read the Status port (0x1F7) again. If the value read is 0, the drive does not exist. */
int i = ata_inb(ata_dataport + ATA_CMDSTATUS);
if (i == 0) /* if the result is 0, this ATA drive does not exist */
	{
	harddrive->exists = false;
	return;
	}
else
	{ // the drive exists, do some more checking
	/* Because of some ATAPI drives that do not follow spec, at this point you need to check the LBAmid and LBAhi ports
	(0x1F4 and 0x1F5) to see if they are non-zero. If so, the drive is not ATA, and you should stop polling. */

	/* TODO: implement the above instructions from wiki.osdev.org */

	/* Otherwise, continue polling one of the Status ports until bit 3 (DRQ, value = 8) sets, or until bit 0 (ERR, value = 1) sets. */
	while (!(ata_inb(ata_dataport + ATA_CMDSTATUS)&ATA_STATUS_DRQ))
	while (inb(ata_dataport + ATA_CMDSTATUS)&ATA_STATUS_ERR);

	/* At that point, if ERR is clear, the data is ready to read from the Data port (0x1F0). Read 256 words, and store them. */

	/* read the Ata identify packet to the struct ata_id */
	/* TODO: is this a pointer mess or the proper way to do it? */
	struct ata_identify ata_id;
	struct ata_identify *ataptr;
	ataptr = &ata_id;
	unsigned char buffer[512] = "";
	in16s(ata_dataport, (512 / 2), buffer);
	memcpy ( (char *) ataptr,buffer,sizeof(buffer));

	/* ATA Identify is in space padded MSB format. "Generic 1234" comes out as "eGenir c2143". Null terminate them and change byte order */
	char *ptr;
	ptr  = ata_id.sernum;
	/* go to the end of sernum and null terminate all spaces. We can't start from the beginning, because there might be spaces in modelnum (Generic 1234)*/
	ptr = ptr + sizeof(ata_id.sernum);
	while (*ptr == ' ')
		{
		*ptr = '\0';
		ptr--;
		}

	ptr = ata_id.modelnum;
	ptr = ptr + sizeof(ata_id.modelnum);
	while (*ptr == ' ')
		{
		*ptr = '\0';
		ptr--;
		}

	msbtolsb(ata_id.sernum, sizeof(ata_id.sernum));
	msbtolsb(ata_id.modelnum, sizeof(ata_id.modelnum));
	msbtolsb((char *)ata_id.lba_sectors,sizeof(ata_id.lba_sectors));

	harddrive->exists = true;
	/* print some debug info */
	printf("ATA driver found: ");
	if (controlsel == ATA_PRI_DATAPORT)
		printf("1st");
	else
		printf("2nd");

	printf(" controller");
	if (drivesel == MASTER_HD)
		printf(" master bus");
	else
		printf(" slave bus");
	printf(" model %s, serial %s\n", ata_id.modelnum, ata_id.sernum);
	}


return;
}

/* IRQs get handled here */
void ata_handler(struct registers *r) {
int helppri = inb(ATA_PRI_DATAPORT + ATA_CMDSTATUS);
int helpsec = inb(ATA_SEC_DATAPORT + ATA_CMDSTATUS);

/* The irqs from hard drives should never require any action. */
/* TODO: print ata cmdstatus to debug output*/ 

if (helppri & ATA_STATUS_ERR)
	printf("ATA error in primary bus!\n");
if (helpsec & ATA_STATUS_ERR)
	printf("ATA error in secondary bus!\n");
}


/* choose which driver to send future commands to: dataport = 1st / 2nd controller, drivesel = master/slave */
/* TODO: the relationship between ata_drsel and ata_readblock should be defined carefully */
int ata_drsel(int controlsel, int drivesel)
{
/* always refer to ata_dataport and mastersel */
ata_dataport = controlsel;
ata_mastersel = drivesel;
/* send the drive select command to choose proper master / slave */
ata_outb(ata_dataport + ATA_DRSEL, ata_mastersel);
/* wait until the drive is ready */
while (inb(controlsel + ATA_CMDSTATUS)&ATA_STATUS_BSY);

return 1;
}

/* we use this to do ata outb - poll under drive BSY is unset and then a normal outb */
int ata_outb(int port, unsigned char outbyte)
{
/* wait until the drive is ready - that is, until BSY is unset */
// ata_dataport is changed by ata_drsel
while (inb(ata_dataport + ATA_CMDSTATUS)&ATA_STATUS_BSY);


/* write outbyte to the chosen register. Controller is selected by ata_drsel */
outb(port, outbyte);

return 1;
}

/* we use this to do ata inb - poll under drive BSY is unset and then a normal inb */
int ata_inb(int port)
{
/* wait until the drive is ready - that is, until BSY is unset */
// ata_dataport is changed by ata_drsel
while (inb(ata_dataport + ATA_CMDSTATUS)&ATA_STATUS_BSY);


/* read outbyte to the chosen register. Controller is selected by ata_drsel */
int i = inb(port);

return i;
}

/* ata_readblock reads the block (sector) lba_address from hd pointed to by globals ata_dataport, ata_mastersel) */
unsigned char *ata_readblock(int lba_address)
{
/* TODO: should you check for ATA DRQ / BUSY with ata_outb? */
outb(ata_dataport + ATA_FEATURES, 0x00); // null byte
outb(ata_dataport + ATA_SCOUNT, 0x01);  // sector count to be read
outb(ata_dataport + ATA_ADDR, lba_address ); /* cylinder lowest byte */
outb(ata_dataport + ATA_ADDR8, lba_address >> 8 ); /* cylinder low byte */
outb(ata_dataport  + ATA_ADDR16, lba_address >> 16); /* cylinder high byte */

/* the last 4 bytes of lba_address depend on the drive (master/slave) */
if (ata_mastersel == MASTER_HD)
        outb(ata_dataport + ATA_DRSEL, 0xE0 | (ata_mastersel << 4) | (lba_address >> 24));
else /* SLAVE_HD */
        outb(ata_dataport + ATA_DRSEL, 0xF0 | (ata_mastersel << 4) | (lba_address >> 24));

outb(ata_dataport + ATA_CMDSTATUS,0x20); /* send a read command */

/* wait for the drive to have something to deliver */
while (!(inb(ata_dataport + ATA_CMDSTATUS) & ATA_STATUS_DRQ)) {};

// Read the whole sector from disk into buffer
unsigned char buffer[512];
in16s(ata_dataport, (512 / 2), buffer);
unsigned char *ptr = buffer;
return ptr;
}

