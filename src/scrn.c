#include "scrn.h"

/* for info about the video memory, shorts and bytes, see
http://www.osdever.net/bkerndev/Docs/printing.htm for example
*/

short *vga_memptr;
int  vga_attrib=VGA_GREY_ON_BLACK;

int init_video()
{
vga_memptr = (short *) VIDMEM;
cls();
}


/* clears the screen */
int cls()
{
short blank = VGA_BLANK;
int i, j;
short *temp_ptr;

/* 
go trough every single char in 80x25 and set them to a space.
The video memory is a short, so it has 16 bytes. The colors have to be pushed with the <<
*/

for (i = 0; i < 25; i++)
	for (j = 0; j <= 80; j++)
		{
		temp_ptr = (vga_memptr + (i*80) + j );
		*temp_ptr = ' '|VGA_BLANK << 8; /* << 8 means we push the attribute byte into the short */
		}
}
