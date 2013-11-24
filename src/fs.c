/* fs.c
scan partitions
*/

#include "stdio.h"
#include "ata.h"
#include "fs.h"
#include "fat.h"
#include "string.h"
#include "letkuos-common.h"

extern struct hd drive[3]; // these are our hard drives
void partition_scan(struct hd *drive);
char *partition_types[];


/* load the (virtual) file system */
int init_vfs()
{
/* TODO: scan all drives */
if (drive[0].exists == 1)
	{
	/* needed for readblock to function. Should figure out this mess, maybe readblock should take these parameters? */
	ata_drsel(ATA_PRI_DATAPORT, MASTER_HD);
	partition_scan(&drive[0]);
	fat_scan(&drive[0].part[0]);
	}

return 1;
}


/* partition_scan checks to MBR for valid partitions */
void partition_scan(struct hd *drive) {
/* read the MBR (sector 0 of a hard disk to buffer */
unsigned char *mbrbuffer = ata_readblock(0);

/* see that the magic signature 0x55AA is there */
if (mbrbuffer[MBR_BOOTSIG_LOC] != MBR_BOOTSIG1 && mbrbuffer[MBR_BOOTSIG_LOC+1] != MBR_BOOTSIG2)
	panic("No valid MBR found on the drive!\n");
else
	printf("Found an MBR on the drive, all good!\n");

/* scan the partitions and fill the structs one by one */

int i = 0 ;
for (mbrbuffer = mbrbuffer + MBR_PART1; i <= 3; i++) // point the buffer to the first 16-byte partition and loop
	{
	/* memcpy the struct, check somewhere if exists or not */
	memcpy(&drive->part[i], mbrbuffer, 16);

	// if there's a sysid, the partition exists.. Is there a better way to check it?
	if (drive->part[i].sysid != 0)
		drive->part[i].exists = true;
	else
		drive->part[i].exists = false;
 	mbrbuffer = mbrbuffer + 16; /* point the buffer to the beginning of next partition */
	}
for (i = 0; i <= 3; i++)
	{
	if (drive->part[i].exists)
		{
		printf("I think we found a partition #%d..\n",i);
		printf("\tBootable: 0x%x\n",drive->part[i].bootable);
		printf("\tSysid: %s\n", partition_types[(int) drive->part[i].sysid]);
		printf("\tLBA sector start: %d\n",drive->part[i].lba_start);
		printf("\tTotal sectors in partition: %d\n",drive->part[i].totsect);
		printf("\t..thus, size of partition: %dkB\n",drive->part[i].totsect*512/1024);
		}
	}
}

/* list of known partition types based on sysID byte */
char *partition_types[] = {
"Empty",
"FAT12",
"XENIX root",
"XENIX usr",
"FAT16 <32M",
"Extended",
"FAT16",
"HPFS/NTFS",
"AIX",
"AIX bootable",
"OS/2 Boot Manager",
"W95 FAT32",
"W95 FAT32 (LBA)",
"UNKNOWN",
"W95 FAT16 (LBA)",
"W95 Ext'd (LBA)",
"OPUS",
"Hidden FAT12",
"Compaq diagnostics",
"Hidden FAT16 <32M",
"Hidden FAT16",
"Hidden HPFS/NTFS",
"AST Smartsleep"
};

/* TODO: write the rest of them */
/*
 01 FAT12                 4F QNX4.x 3rd part       A8 Darwin UFS
 02 XENIX root            50 OnTrack DM            A9 NetBSD
 03 XENIX usr             51 OnTrack DM6 Aux1      AB Darwin boot
 04 FAT16 <32M            52 CP/M                  B7 BSDI fs
 05 Extended              53 OnTrack DM6 Aux3      B8 BSDI swap
 06 FAT16                 54 OnTrackDM6            BB Boot Wizard hidden
 07 HPFS/NTFS             55 EZ-Drive              BE Solaris boot
 08 AIX                   56 Golden Bow            BF Solaris
 09 AIX bootable          5C Priam Edisk           C1 DRDOS/sec (FAT-12)
 0A OS/2 Boot Manager     61 SpeedStor             C4 DRDOS/sec (FAT-16
 0B W95 FAT32             63 GNU HURD or SysV      C6 DRDOS/sec (FAT-16)
 0C W95 FAT32 (LBA)       64 Novell Netware 286    C7 Syrinx
 0E W95 FAT16 (LBA)       65 Novell Netware 386    DA Non-FS data
 0F W95 Ext'd (LBA)       70 DiskSecure Multi-Boo  DB CP/M / CTOS / ...
 10 OPUS                  75 PC/IX                 DE Dell Utility
 11 Hidden FAT12          80 Old Minix             DF BootIt
 12 Compaq diagnostics    81 Minix / old Linux     E1 DOS access
 14 Hidden FAT16 <32M     82 Linux swap / Solaris  E3 DOS R/O
 16 Hidden FAT16          83 Linux                 E4 SpeedStor
 17 Hidden HPFS/NTFS      84 OS/2 hidden C: drive  EB BeOS fs
 18 AST SmartSleep        85 Linux extended        EE GPT
 1B Hidden W95 FAT32      86 NTFS volume set       EF EFI (FAT-12/16/32)
 1C Hidden W95 FAT32 (LB  87 NTFS volume set       F0 Linux/PA-RISC boot
 1E Hidden W95 FAT16 (LB  88 Linux plaintext       F1 SpeedStor
 24 NEC DOS               8E Linux LVM             F4 SpeedStor
 39 Plan 9                93 Amoeba                F2 DOS secondary
 3C PartitionMagic recov  94 Amoeba BBT            FB VMware VMFS
 40 Venix 80286           9F BSD/OS                FC VMware VMKCORE
 41 PPC PReP Boot         A0 IBM Thinkpad hiberna  FD Linux raid
autodetec
 42 SFS                   A5 FreeBSD               FE LANstep
 4D QNX4.x                A6 OpenBSD               FF BBT
 4E QNX4.x 2nd part       A7 NeXTSTEP
*/

