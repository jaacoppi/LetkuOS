/* scrn.h - screen related things */

#define VIDMEM 0xB8000	/* video memory resides in 0xB8000), assuming it's a color monitor */ 

/* in color, there's a bit for foreground and background. figure them out */
#define VGA_WHITE_ON_BLUE 0x1F
#define VGA_GREY_ON_BLACK 0x07
#define VGA_BLANK 0x00

int init_video();
int cls();
