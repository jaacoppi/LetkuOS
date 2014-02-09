/* linker script for user space programs.
Entry point is at 04000000 - that is, 64MB
*/
ENTRY (_start)
SECTIONS{
    . = 0x004000000;

    .text :{
        *(.text)
    }

    .rodata ALIGN (0x1000) : {
        *(.rodata)
    }

    .data ALIGN (0x1000) : {
        *(.data)
    }

    .bss : {
        sbss = .;
        *(COMMON)
        *(.bss)
        ebss = .;
    }
}
