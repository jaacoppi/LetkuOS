/* elf.c
LetkuOS specifics for loading and executing ELF files
*/

#include "elf.h"
#include "fat.h"
#include "stdio.h"
#include "string.h"
#include "mm.h"

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

// TODO: rename to exec? 
int parse_elf(char *filename)
{
// load a copy of the whole file into memory. This isn't actually needed, but it's a simple way..
char *memaddr = (char *)fat_loadfile(filename);

// load the ELF header
Elf32_Ehdr elfheader;
memcpy((void *)&elfheader, memaddr, sizeof(Elf32_Ehdr));

if (elfheader.e_ident[0] != 0x7F || elfheader.e_ident[1] != 'E' || elfheader.e_ident[2] != 'L' || elfheader.e_ident[3] != 'F')
	{
	printf("Error! %s is not a valid ELF file!\n",filename);
	return 0;
	}

// load all program segments to memory
char *segaddr = (int) memaddr + elfheader.e_phoff;
Elf32_Phdr progheader;

int i;
for (i = 0; i < (int) elfheader.e_phnum; i++)
	{
	memcpy((void *) &progheader, segaddr,elfheader.e_phentsize);
	if (progheader.p_type != PT_LOAD)
		panic("unsupported progheader type #%d\n",progheader.p_type);

	printf("v_addr: 0x%xh\n",progheader.p_vaddr);
	printf("memsz: 0x%xh\n",progheader.p_memsz);
	printf("filesz: 0x%xh\n",progheader.p_filesz);
	int loadaddr = kmalloc(progheader.p_memsz);
	// note that hard coding ELF_LOADADDR here doesn't work with looping
	if (i >= 1)
		panic("elf loading can't handle multiple program headers");
	#define ELF_LOADADDR 0x4000000	// LetkuOS loads user progs at 64Mb
	paging_remap(ELF_LOADADDR, loadaddr);

	// load the program to memory
	memcpy(ELF_LOADADDR, memaddr, progheader.p_filesz);
	// according to elf specs, pad zeroes if needed
	if (progheader.p_filesz > progheader.p_memsz);
		memcpy(loadaddr+progheader.p_filesz,'0',(progheader.p_filesz - progheader.p_memsz));
	/*
2. setup page privileges based on p_flags

3. memcpy(virtbase,p_offset,p_filesz) - note p_align
4. if memsz > filesz, memcpy(right after,0,memsz-filesz) (pad with zeroes)

-then just jmp to e_entry and execute.. - in C it might be (void *) e_entry;
*/
	memaddr = memaddr + elfheader.e_phentsize;
	}

return 1;
}
