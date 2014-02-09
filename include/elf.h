#ifndef letkuos_elf_h
#define letkuos_elf_h letkuos_elf_h
/* ELF specification used here is Portable Formats Specification 1.1 */

// Currently LetkuOS only supports executable files, not relocatable files or shared object files

// the posix family has many different functions for executing a file. We currently only have one
int exec(const char *path); // TODO: add parameters as "const char *arg, ..."
int parse_elf(char *filename);

// elf header

/* ELF structure explained by the specs:

An ELF header resides at the beginning and holds a ‘‘road map’’ describing the file’s organization.  Sec-
tions hold the bulk of object file information for the linking view: instructions, data, symbol table, reloca-
tion information, and so on [...]

A program header table, if present, tells the system how to create a process image. Files used to build a pro-
cess image (execute a program) must have a program header table; relocatable files do not need one. A
section header table contains information describing the file’s sections. Every section has an entry in the
table; each entry gives information such as the section name, the section size, etc. Files used during link-
ing must have a section header table; other object files may or may not have one.
*/

// ELF header table
#define EI_NIDENT 16

// data types & sizes from the specs again:
#define Elf32_Half unsigned int 	// 2 bytes
#define Elf32_Word unsigned long int 	// 4 bytes
#define Elf32_Off  unsigned long int 	// 4 bytes
#define Elf32_Addr  unsigned long int 	// 4 bytes

#define ET_EXEC 2
#define EM_386	3
typedef struct {
	unsigned char e_ident[EI_NIDENT]; // 0x7F 'E' 'L' 'F'
	Elf32_Half e_type;	// file type,  ET_EXEC if file is an executable file
	Elf32_Half e_machine;	// machine type, EM_386 for a Intel 386 machine
	Elf32_Word e_version;	// must be 1?
	Elf32_Addr e_entry;	// virtual address where execution of file should start, zero if none
	Elf32_Off e_phoff;	// program header offset in file in bytes. 0 if no program header
	Elf32_Off e_shoff;	// section header offset in file in bytes. 0 if no section header
	Elf32_Word e_flags;	// processor specific flags
	Elf32_Half e_ehsize;	// ELF header size in bytes
	Elf32_Half e_phentsize;	// size of one program header entry. All are the same size
	Elf32_Half e_phnum;	// number of program headers
	Elf32_Half e_shentsize;	// size of one section header entry. all are the same size
	Elf32_Half e_shnum;	// number of section headers
	Elf32_Half e_shstrndx;	// section header index of the table associated with section name string table
} Elf32_Ehdr;


// section header table


#endif
