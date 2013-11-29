/* fat.c
* FAT32 driver
*/

#include "letkuos-common.h"
#include "fat.h"
#include "ata.h"
#include "stdio.h"
#include "string.h"

/* LetkuOS currently supports 1 partition only. The FAT Boot Record of this partition is stored in rootdevice */
struct fat32_BS rootdevice;

int debug = 0;

/* fat_scan is similiar to partition_scan - populate to structs */
/* should be run once at boot time for rootdevice */
void fat_scan(struct partition *part) {

/* read the FAT boot record. This is the first sector of the partition -> part->lba_start */
// TODO: make sure ata_drsel is correct
// TODO: link fat32_BS *fatptr to drive and part?
// TODO: is this the correct way to use pointers..?
unsigned char *fatptr;
fatptr = ata_readblock(part->lba_start);
memcpy ((void *) &rootdevice, (void *)fatptr, sizeof(rootdevice));

if (rootdevice.boot_signature != 0x29 && rootdevice.boot_signature != 0x28)
	{
	printf("bootsig is: %xh\n",rootdevice.boot_signature);
	panic("No valid FAT32 partition table found on rootdevice!\n");
	}
/*
else
	{
	printf("Found a valid partition: ");
	printf("%s\n",rootdevice.oem_name);

	printf("bps: %d\n",rootdevice.bytes_per_sector);
	printf("spc: %d\n",rootdevice.sectors_per_cluster);
	printf("rsc: %d\n",rootdevice.reserved_sector_count);
	printf("tc: %d\n",rootdevice.FAT_count);
	printf("rec: %d\n",rootdevice.root_entry_count);
	printf("ts16: %d\n",rootdevice.total_sectors_16);
	}
*/

return;
}



////////////////////////////////////////////
// displays a directory listing
////////////////////////////////////////////
// read all the 16 32bit blocks in one (SECTOR or CLUSTER??)
// basically this amounts to a directory listing of the rootdir
int fat_readdir(int cluster) {

// table holds info about one file at a time. Could also be table[16] and no memcpy in for loop??
struct dir_struct *table = NULL;

// convert the relative cluster used internally by FAT to an absolute sector used by ATA.
// from http://www.pjrc.com/tech/8051/ide/fat32.html
//cluster_begin_lba = Partition_LBA_Begin + Number_of_Reserved_Sectors + (Number_of_FATs * Sectors_Per_FAT);
// lba_addr = cluster_begin_lba + (cluster_number - 2) * sectors_per_cluster;

cluster = drive[0].part[0].lba_start + rootdevice.reserved_sector_count + (rootdevice.FAT_count * rootdevice.sectors_per_fat) + ((cluster -2) * rootdevice.sectors_per_cluster);

// Then, read the sector from disk
unsigned char *fatbuffer;
fatbuffer  = ata_readblock(cluster);

printf("showing files in directory, sector %d\n",cluster);
// for < 16 'cos sector/tablesize = 512/32 = 16. 16 entries in one sector
// .. note cluster size here: is one cluster 4 sectors?
int i;
for (i = 0; i < 16; i++)
	{
	memcpy((void *) table, (void *)fatbuffer, 32);
	fatbuffer = fatbuffer + 32; // next loop = next file.

	if (table->file_attr == FAT_ATTR_longfile)
		{
		// debug note: currently just skip long file names, don't implement anything
		continue;
		// long file names work in this way:
		// until file_name[0] is 0x40 + [0x01 to 0x0F], these only contain names. The actual file
		// comes after the 0x40. The 0x01-0x0F means the order of the files. Thus, 0x41 means it's the first and last long file name "file"

		// we need to parse the file for the proper file name
/*
		char temp[11] = "";
		temp[0] = table->file_name[1];
		temp[1] = table->file_name[3];
		temp[2] = table->file_name[5];
		temp[3] = table->file_name[7];
		temp[4] = table->file_name[9];
		temp[5] = table->file_name[14];
		temp[6] = table->file_name[16]; // can we access above the array of file_name[11] ??
		temp[7] = table->file_name[18];
		temp[8] = table->file_name[20];
		temp[9] = table->file_name[22];
		temp[10] = table->file_name[28];
		temp[11] = table->file_name[30];
		int i;
		for (i = 0; i <= 11; i++)
			table->file_name[i] = temp[i];
*/
		}


	// if it is 0xE5 the file in the entry has been deleted
	if (table->file_name[0] == FAT_ATTR_deleted)
		continue;

	if (table->file_name[0] == FAT_ATTR_nomore) {
		// in this case no more files in the specified sector, stop reading
		printf("No more files in this directory, %d\n",i);
		break;
		}

	// else, it's a proper file, print it:
	printf("File #%d %s, size %d",i,table->file_name, table->file_size);
	if (table->file_attr & FAT_ATTR_directory)
		printf("<DIR>");
	else
		printf("\t");

	// print out the starting cluster of this file
	int clusternumber = table->cluster_number_hi;
	       clusternumber = (clusternumber << 8) + table->cluster_number_lo;
	printf("cluster %x\n",clusternumber);



	}
/* TODO: instead of printing the files, return the table for processing in other functions - findfile etc */
return 1;
}





//////////////////////////////////////////////////////////////////////
// only for debugging the fat table, no real function in the kernel
// displays a cluster "fat_offset" in a fat table
//////////////////////////////////////////////////////////////////////

int debug_showfat(int fatpage) { // fatpage means which page of FAT we will read

//  rootdevice is a FAT struct, so it stores info in relative form. It gives us the ABSOLUTE sector of the FAT by..

//   The first data sector (that is, the first sector in which directories and files may be stored):
// currently fatpage is static:
// TODO: find out where the FAT actually resides. Should it be the Boot record +1 ?
printf("first fat in %d\n",rootdevice.reserved_sector_count);
fatpage =  (rootdevice.reserved_sector_count);


unsigned char *clusterbuffer  = ata_readblock(drive[0].part[0].lba_start + fatpage);
unsigned char fatbuffer[512] = "";
memcpy ((void *) fatbuffer, clusterbuffer, 512);
// print the FAT for debug purposes. This can be used to manually follow chains
printf("0000 0001 0002 0003 0004 0005 0006 0007 0008 0009 000A 000B 000C 000D 000E 000F\n");
printf("------------------------------------------------------------------------------\n");

int h = 0;
for (h = 0; h < 512; h = h +4) // 128, since each cluster takes up 32 (28) bytes?
	{
	// A fat32 cluster is 28 bytes. fatbuffer is 8. Thus, we need to make a dword out of a char
	unsigned int cluster;
	cluster =  fatbuffer[h+1]; // low byte
	cluster = cluster <<  8;
	cluster =  cluster + fatbuffer[h]; // low byte

	cluster = cluster & 0x0FFFFFFF; // remove the 4 high bytes, thus making a 28 bit interger

	// to make it human readable, add padding
	if (cluster <= 0xFFF)
		printf("0");
	if (cluster <= 0xFF)
		printf("0");
	if (cluster <= 0x0F)
		printf("0");


	printf("%x ", cluster);
        }


return 1;
}



