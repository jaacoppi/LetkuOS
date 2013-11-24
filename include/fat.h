/* fat.h
* FAT32 driver
*/

#ifndef _letkuos_fat_h
#define _letkuos_fat_h _letkuos_fat_h

#include "ata.h"
#include "letkuos-common.h"

void fat_scan(struct partition *part);
int fat_readdir(int sector);
int debug_showfat(int fatpage);
/* from osdev.org wiki, adapted a bit */
/* also from http://support.microsoft.com/kb/q140418 */
/* FAT Boot record, always at block 0 of the partition */
struct fat32_BS {
	// BPB - Bios Paramater Block
        unsigned char           bootjmp[3];
        unsigned char           oem_name[8];	// meaningless according to specs
        unsigned short          bytes_per_sector; // we expect it to be 512
        unsigned char           sectors_per_cluster;
        unsigned short          reserved_sector_count; // sectors before the first FAT
        unsigned char           FAT_count;	// number of FAT's, probably 2
        unsigned short          root_entry_count;	// ??
        unsigned short          total_sectors_16;	// if 0, the real count is in byte 32-35
        unsigned char           media_type;		// physical media descriptor, we don't care
        unsigned short          sectors_per_fat16;	// sectors per fat, FAT12/16 only. Meaningless
        unsigned short          sectors_per_track;	// ??
        unsigned short          head_side_count;	// physical media stuff, we don't care
        unsigned int            hidden_sector_count;	// used to calculate absolute offset to root directory
        unsigned int            total_sectors_32;	// if total_sectors16 is 0
        // EBPB, extended bios parameter block stuff for FAT32, not for FAT12/16
// for testing if it's a FAT12 instead of FAT32:
//	unsigned char		signature[3];
        unsigned int		sectors_per_fat;	// size of FAT in sectors
	unsigned short		flags;			// some flags
	unsigned short		fat_version;		// fat version number
	unsigned int		root_cluster;		// relative cluster for the root dir, probably 2
	unsigned short		fsinfo_cluster;		// relative cluster for FSinfo struct
	unsigned short		backup_cluster;		// relative cluster for backup boot sector
	unsigned char		reserved[12];		// should be zero?
	unsigned char		drive_num;		// 0x80 for a hard disk, 0x00 for floppies
        unsigned char           reserved1;		// windows NT flags
        unsigned char           boot_signature;		// must be 0x28 or 0x29 for Windows NT
        unsigned int            volume_id;		// random id of the disk - can be ignored
        unsigned char           unused_volume_label[11];// nowadays volume label is in the rootdir
        unsigned char           fat_type_label[8];	// "FAT32   ", but should not be needed
} __attribute__((packed));

struct dir_struct {
        unsigned char           file_name[11];
        unsigned char           file_attr;
        unsigned char           reserv1;
        unsigned char           time_created_tens;
        unsigned short          time_created;
        unsigned short          date_created;
        unsigned short          date_accessed;
        unsigned short          cluster_number_hi;
        unsigned short          time_modified;
        unsigned short          date_modified;
        unsigned short          cluster_number_lo;
        unsigned int            file_size;
} __attribute__((packed)) dir_struct;

/* FAT directory attributes */

/* define FAT directory entry attributes */
#define FAT_ATTR_readonly	0x01
#define FAT_ATTR_hidden		0x02
#define FAT_ATTR_system		0x04
#define FAT_ATTR_volumeid	0x08
#define FAT_ATTR_directory	0x10
#define FAT_ATTR_archive	0x20
#define FAT_ATTR_longfile	0x0F


#define FAT_ATTR_nomore		0x00
#define FAT_ATTR_deleted	0xE5


#endif
