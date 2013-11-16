#ifndef _letkuos_ata_h
#define _letkuos_ata_h _letkuos_ata_h

/* ATA identify struct and other info can be found from the specification
http://www.t13.org/documents/UploadedDocuments/docs2006/D1699r3f-ATA8-ACS.pdf
(table 21, section 7.16)
*/

/* simple ATA PIO driver */
/* used info from http://wiki.osdev.org/ATA_PIO_Mode */

void init_ata();
unsigned char *ata_readblock(int lba_address);
int ata_drsel(int controlsel, int drsel);
extern struct hd hda, hdb, hdc, hdd; /* these are our hard drives */

/* from exclaim-0.2.1, thanks for explaing bits and hexes */
#define ATA_CHECK_ERROR         0
#define ATA_CHECK_BUSY          1
#define ATA_CHECK_DATA          2

/* status byte mask */
#define ATA_STATUS_ERR          0x01
#define ATA_STATUS_DRQ          0x08
#define ATA_STATUS_SRV          0x10
#define ATA_STATUS_DF           0x20
#define ATA_STATUS_RDY          0x40
#define ATA_STATUS_BSY          0x80

/* commands */
#define ATA_IDENTIFY		0xEC	/* identify command to.. idenfity the drive */
#define ATA_READSECTORS		0x20	/* do exactly what the name says */

/* Dataports in absolute values */
#define ATA_PRI_DATAPORT        0x1F0
#define ATA_SEC_DATAPORT        0x170

/* other ports as offsets from dataport - ATA_FEATURES for Primary is 0x1F0 + 0x001 == 0x1F1 */
/* .. and for Secondary it's 0x170 + 0x001 */
#define ATA_FEATURES            0x001
#define ATA_SCOUNT              0x002
#define ATA_ADDR                0x003	/* ADDR = Lba low byte */
#define ATA_ADDR8               0x004	/* LBA middle byte */
#define ATA_ADDR16              0x005	/* LBA high byte */
#define ATA_DRSEL               0x006	/* LBA bits 24-27 map to bits 3-0 here*/
#define ATA_CMDSTATUS           0x007
#define ATA_DCREG               0x206

/* sent to ATA_DRSEL to choose master or slave drive on 1st or 2nd controller*/
#define MASTER_HD 		0x0A0
#define SLAVE_HD 		0x0B0

/* we can skip  the x words we don't care about with ignore[x] */
/* todo: replace uin16 and so on */
struct ata_identify {
	unsigned short ignore1[10]; /* words 0-9 */
	char sernum[20]; /* words 10-19, serial numbers as ascii */
	unsigned short ignore2[7]; /* words 20-26 */
	char modelnum[20]; /* words 27-46, model num as ascii */
	unsigned short ignore3[13]; /* words 47-59 */

	unsigned int lba_sectors; /* words 60-61 - "total number of user accessable logical sectors */
/* This field contains a value that is one greater than the maximum user accessible logical block address. The
maximum value that shall be placed in this field is 0FFF_FFFFh. */
/* TODO: find out the meaning of logical sectors - it seems that it doesn't mean the hard drive size */
	unsigned short ignore4[194]; /* words 62-127 */
} __attribute__ ((packed));

/* this struct holds the hd and partition table entries.
hd.exists = whether the hd is present (no CDROM etc support yet)
hd.ordinal = ordinal number used (or not) to separate partitions
struct partition holds the partition information. This loosely follows the 16 bit partition entry format, but not fully. */

/* struct hd only support 4 hds and 4 partitions on each hd */
struct hd {
int exists;
int ordinal; // hda = 0, hdd = 3
        struct partition { // partitions 1-4
	// this is the 16-bit partition entry. TODO: make sure the data types are correct
        unsigned char bootable;   // bootable flag 0x80
	// next 3 bytes = chs address of first absolute sector in this partition
        unsigned char shead;      // starting head
        unsigned char ssect;      // starting sector (bits5-0), and cyl high bytes 9-8 -> bits7-8
        unsigned char scyl;       // starting cylinder, bits  7-0
        unsigned char sysid;      // SysID (partition type), from 16-bit partition entry
	// next 3 bytes = chs address of last absolute sector in this partition
        unsigned char ehead;      // ending head, see format above
        unsigned char esect;      // ending sector, see format above
        unsigned char ecyl;       // ending cylinder, see format above
        unsigned int lba_start;   // LBA of first absolute sector, 4 bytes
        unsigned int totsect;    // total sectors in the partition, 4 bytes
	// there are extra fields used by the kernel
        int exists;
        char label[12]; // FS Label from FAT driver
        } part[3];
};


#endif
