/* elf.c
LetkuOS specifics for loading and executing ELF files
*/

#include "elf.h"
#include "fat.h"
#include "stdio.h"
#include "string.h"

//////////////////////////////////////////////////////////
// execute a file from disk pointed to by path
//////////////////////////////////////////////////////////
int exec(const char *path)
{

return 1;
}

//////////////////////////////////////////////////////////
//  parse the elf file structure to create a process image
// - load correct parts to correct places in (virtual?) memory
//////////////////////////////////////////////////////////
int parse_elf(char *filename)
{
// load a copy of the whole file into memory. This isn't actually needed, but it's a simple way..
char *memaddr = (int *)fat_loadfile(filename);

// load the ELF header
Elf32_Ehdr elfheader;
memcpy(&elfheader, memaddr, sizeof(Elf32_Ehdr));

// check the magic bits 0x7F, 'E', 'L', 'F'
if (elfheader.e_ident[0] != 0x7F || elfheader.e_ident[1] != 'E' || elfheader.e_ident[2] != 'L' || elfheader.e_ident[3] != 'F')
	{
	printf("Error! %s is not a valid ELF file!\n",filename);
	return 0;
	}

// find and fill the program headers using:
elfheader.phoff
elfheader.phnum

return 1;
}
