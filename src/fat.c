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

void fat_raw_to_humanreadable(unsigned char filename[11]);
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
int sector = 0; // sector to be read from disk
int loop = 1;
	while (loop)
	{
	// directories can span multiple cluster in FAT. Thus, we need to check if need be to read another cluster
	unsigned int clustervalue = get_cluster_value(cluster);
	if (clustervalue >= 0xFFF8) // no more cluster in this dir
		loop = 0;

	// convert the relative cluster used internally by FAT to an absolute sector used by ATA.
	// from http://www.pjrc.com/tech/8051/ide/fat32.html
	//cluster_begin_lba = Partition_LBA_Begin + Number_of_Reserved_Sectors + (Number_of_FATs * Sectors_Per_FAT);
	// lba_addr = cluster_begin_lba + (cluster_number - 2) * sectors_per_cluster;

	sector = drive[0].part[0].lba_start + rootdevice.reserved_sector_count + (rootdevice.FAT_count * rootdevice.sectors_per_fat) + ((cluster -2) * rootdevice.sectors_per_cluster);

	// point cluster to the next cluster for looping purposes
	cluster = clustervalue;

	// Then, read the sector from disk
	unsigned char *fatbuffer;
	fatbuffer  = ata_readblock(sector);

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
	}

/* TODO: instead of printing the files, return the table for processing in other functions - findfile etc */
return 1;
}




//////////////////////////////////////////////////////////////////////
// return a cluster value; useful for parsing directories, for example
//////////////////////////////////////////////////////////////////////

unsigned int get_cluster_value(unsigned int cluster)
{
// setup some helper ints for calculations

// cluster size means the space on disk that 1 cluster points to.
int clustersize = rootdevice.bytes_per_sector * rootdevice.sectors_per_cluster;

unsigned char *clusterbuffer;

// the reasoning for fat_offset is a bit unclear.. this is what might be happening:
// one cluster is 32 bits. Thus, with 512 byte sectors, one cluster occupies 4 x 8 bytes (4 chars)
// in any case, fat_offset means the offset of the cluster from the beginning of FAT
unsigned int fat_offset = cluster * 4;

// find out the absolute sector of hard disk where the cluster resides
unsigned int sector =  drive[0].part[0].lba_start + rootdevice.reserved_sector_count + (fat_offset / clustersize);

// read 512 bytes from the proper hda sector
clusterbuffer = ata_readblock(sector);

// to read the value of the cluster we care about, use offset (in the current sector)
unsigned int offset = fat_offset % clustersize;

// read 28 bits to an unsigned int, taking endianndess into account
unsigned int table_value =  clusterbuffer[offset+1]; // low byte
table_value = table_value << 8;
table_value = table_value + clusterbuffer[offset];

//remember to ignore the high 4 bits.
table_value = table_value & 0x0FFFFFFF;

return table_value;
}




//////////////////////////////////////////////////////////////////////
// parse a full path to find the correct cluster
//////////////////////////////////////////////////////////////////////

unsigned int fat_parse_path(char *path)
{
#define FILENAME_LEN 11 // hardcode the file system to use 8+3 format. A stupid decision..
char pathcomponent[FILENAME_LEN] = "";

// currently, only absolute path names are used. Thus, if the path doesn't begin with \, don't allow it)
if (path[0] != '\\')
	return -1; // function returns -1 on error

path++; // skip the first "\"

int i;
unsigned int cluster = rootdevice.root_cluster; // start parsing from the root cluster
 // loop the directory tree until the end is found
while (true)
	{
	if (path == '\0')
		break;

	// this loops searchs for directories in the middle of path
	// thus, from \boot\grub\menu.lst it finds \boot and \boot\grub, but not the last part, menu.lst
	int c = 1;
	for (i = 0; i < FILENAME_LEN; i++)
		{
		c = path[i]; // just strcmp((const char *) '\\', (const char *) path[0]) doesn't work
		if (c == '\\')	// we found a \, there's one more dir
			{
			memcpy(pathcomponent, path, i); // parse the directory / file name
			path = path + i + 1; // advance pointer to the beginning of next file (also skip trailing \)

			// at this point, pathcomponent contains the directory/file we want to search
			// find the cluster that holds the dir we want
			printf("trying to find %s in cluster %d\n",pathcomponent, cluster);

			cluster = fat_findcluster(pathcomponent, cluster);
			if (cluster == 0)
				{
				printf("Invalid directory or file %s\n", pathcomponent);
				return -1;
				}

			printf("dir/file %s is in cluster %d.\n",pathcomponent, cluster);
			memcpy(pathcomponent, (const void *) 0, 11);
			break;
			}
		}
		// travelled until the end of path but didn't find a "\"
		if (i == FILENAME_LEN)
			{
			memcpy(pathcomponent, path, i);
			cluster = fat_findcluster(pathcomponent, cluster);
			if (cluster == 0)
				{
				printf("Invalid directory or file %s\n", pathcomponent);
				return -1;
				}
			printf("last dir/file %s is in cluster 0x%xh.\n",pathcomponent, cluster);
			break;
			}
	}
return cluster; // return the starting cluster of the last file/dir in path
}

//////////////////////////////////////////////////////////////////////
// read cluster and find the cluster that file resides in
//////////////////////////////////////////////////////////////////////
unsigned int fat_findcluster(char *file, unsigned int cluster)
{
struct dir_struct *table = NULL;
int sector = 0; // sector to be read from disk


int loop = 1;
        while (loop)
        {
        // directories can span multiple cluster in FAT. Thus, we need to check if need be to read another cluster
        unsigned int clustervalue = get_cluster_value(cluster);
        if (clustervalue >= 0xFFF8) // no more cluster in this dir
                loop = 0;

	// convert the relative cluster used internally by FAT to an absolute sector used by ATA.
	// from http://www.pjrc.com/tech/8051/ide/fat32.html
	//cluster_begin_lba = Partition_LBA_Begin + Number_of_Reserved_Sectors + (Number_of_FATs * Sectors_Per_FAT);
	// lba_addr = cluster_begin_lba + (cluster_number - 2) * sectors_per_cluster;

	sector = drive[0].part[0].lba_start + rootdevice.reserved_sector_count + (rootdevice.FAT_count * rootdevice.sectors_per_fat) + ((cluster -2) * rootdevice.sectors_per_cluster);

	// point cluster to the next cluster for looping purposes
	cluster = clustervalue;

	// Then, read the sector from disk
	unsigned char *fatbuffer;
	fatbuffer  = ata_readblock(sector);

	int i;
	for (i = 0; i < 16; i++)
		{
		memcpy((void *) table, (void *)fatbuffer, 32);
		fatbuffer = fatbuffer + 32; // next loop = next file.

		// skip long files
                if (table->file_attr == FAT_ATTR_longfile)
                        continue;

		// in this case no more files in the specified sector, stop reading
		if (table->file_name[0] == FAT_ATTR_nomore)
                        break;

		// change the file_name from FAT format to human readable
		fat_raw_to_humanreadable(table->file_name);
		if (strcmp((const char *)table->file_name,file) == 0) // we found the file/dir
			{
			int clusternumber = table->cluster_number_hi;
			       clusternumber = (clusternumber << 8) + table->cluster_number_lo;
			return clusternumber;	// return starting cluster of file
			}
		}
	}
// we didn't find the file/dir from this cluster, return an error
// long file names could cause some files sometimes to be read
// 0 can be an error since cluster must be an unsigned int and clusters start from 2

return 0;
}



//////////////////////////////////////////////////////////////////////
// follow a cluster chain
//////////////////////////////////////////////////////////////////////

int follow_clusterchain(unsigned int cluster)
{
cluster =7;

// setup some helper ints for calculations

// cluster size means the space on disk that 1 cluster points to.
int clustersize = rootdevice.bytes_per_sector * rootdevice.sectors_per_cluster;

unsigned int lastsector = 0;
unsigned char *clusterbuffer;
// loop until the end of cluster chain
int i;
printf("for a file starting in cluster %x..",cluster);
for (i = 1; i > 0; i++)
	{
	// the reasoning for fat_offset is a bit unclear.. this is what might be happening:
	// one cluster is 32 bits. Thus, with 512 byte sectors, one cluster occupies 4 x 8 bytes (4 chars)
	// in any case, fat_offset means the offset of the cluster from the beginning of FAT
	unsigned int fat_offset = cluster * 4;

	// find out the absolute sector of hard disk where the cluster resides
	unsigned int sector =  drive[0].part[0].lba_start + rootdevice.reserved_sector_count + (fat_offset / clustersize);

	/* for debugging -s debugfat instead of follow
	printf("cluster here: %d\n",cluster);
	debug_showfat(cluster - 2048 -32 );
	return;
	*/

	// read 512 bytes from the proper hda sector
	// only read if the sector has changed from last time. This reduces unnecessary ATA accesses
	if (lastsector != sector)
		{
		lastsector = sector;
		clusterbuffer = ata_readblock(sector);
		}

	// to read the value of the cluster we care about, use offset (in the current sector)
	unsigned int offset = fat_offset % clustersize;

	// read 28 bits to an unsigned int, taking endianndess into account
	unsigned int table_value =  clusterbuffer[offset+1]; // low byte
	table_value = table_value << 8;
	table_value = table_value + clusterbuffer[offset];

	//remember to ignore the high 4 bits.
	table_value = table_value & 0x0FFFFFFF;

	/*
	// for debug needs, print out the value
	printf("the value of cluster %x is ",cluster);

	// to make it human readable, add padding
	if (table_value <= 0xFFF)
		printf("0");
	if (table_value <= 0xFF)
		printf("0");
	if (table_value <= 0x0F)
		printf("0");

	printf("%x\n",table_value);
	*/
	// end debug

	// if the value is >= 0xFFF8, it's the last cluster
	if (table_value >= 0xFFF8)
		break;
	// otherwise, set cluster to the new value and continue looping
	cluster = table_value;
	}

printf("total number of clusters: %d\n", i);
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
fatpage = fatpage + rootdevice.reserved_sector_count;


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


////////////////////////////////////////////////////
// change file name from fat format to humanreadable
// that is, from "MENU    LST" to "menu.lst"
////////////////////////////////////////////////////
void fat_raw_to_humanreadable(unsigned char filename[11])
{
// this assumes all file names are always lowercase
// the extension is easy, just from uppercase to lowercase IF there's a letter
int i;


	// if it's a space, it means there' no more letters. Check if there's an extension
for (i = 0; i <= 11; i++)
	{
	// if it's an uppercase letter, lowercase it
	if (filename[i] >= 'A' && filename [i] <= 'Z')
		filename[i] = filename[i] + 32;

	if (filename[i] == ' ')	// padding starts
		{
		if (i <=8)	// we're padding the 8 part, not +3 part
			{
			if (filename[8] != 32) // there's an extension
				{
				// add the extension where the padding starts
				filename[i] = '.';
				filename[i+1] = filename[8];
				filename[i+2] = filename[9];
				filename[i+3] = filename[10];

				// pad the rest with with null bytes
				int k;
				for (k=i+4; k <= 11; k++)
					filename[k] = '\0';
				}
			else // no extension, pad everything with null bytes
				{
				filename[i] = '\0';
				break;
				}
			}
		}
	}
}
