/* fs.c
* file system stuff - probably Virtual FS abstraction in the future, thus it's called vfs in some parts of the code
*/
#ifndef  _letkuos_fs_h
#define _letkuos_fs_h _letkuos_fs_h
int init_vfs();

/* in the MBR, these are the offsets for partition table entries and BOOTSIG. These are the offsets we start to scan  */

#define MBR_PART1       0x1BE
#define MBR_PART2       0x1CE
#define MBR_PART3       0x1DE
#define MBR_PART4       0x1EE
#define MBR_BOOTSIG_LOC 0x1FE

#define MBR_BOOTSIG1    0x55
#define MBR_BOOTSIG2    0xAA


// file_entry stores info about a file or directory
// note that file_entry is FAT specific because it's using startcluster
typedef struct file_entry
{
        char type;      // 0x01 for DIR, 0x02 for FILE
        char name[11];  // follows the FAT convention
        unsigned int size;      // filesize in bytes
        unsigned long startcluster; // starting cluster in FAT32. Currently the FS driver is FAT32 specific
} file_entry;

#define FILE_ISDIR      0x01
#define FILE_ISFILE     0x02

#endif
