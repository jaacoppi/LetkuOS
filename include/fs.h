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

#endif
