#include "scrn.h"

/* for info about the video memory, shorts and bytes, see
http://www.osdever.net/bkerndev/Docs/printing.htm for example
*/

short *vga_memptr;
int  vga_defaultcolor = VGA_GREY_ON_BLACK;

/* video memory is a block of contiguous memory. In contrast, humans tend to process information in chunks.
Therefore, we need an abstraction of x-y dimension to utilize newlines
*/
int vga_cursorx;
int vga_cursory;

int init_video()
{
vga_memptr = (short *) VIDMEM;
vga_cursorx=0;
vga_cursory=0;

cls();
}


/* clears the screen */
int cls()
{
short blank = VGA_BLANK;
int i, j;
short *temp_ptr;

/*
go trough every single char in 80x25 and set them to blank (=space).
The video memory is a short, so it has 16 bytes. The colors have to be pushed with the <<
*/

for (i = 0; i < 25; i++)
	for (j = 0; j <= 80; j++)
		{
		temp_ptr = (vga_memptr + (i*80) + j );
		*temp_ptr = ' '|VGA_BLANK << 8; /* << 8 means we push the attribute byte into the short */
		}
}



/* write a character to the screen */
int writech(char ch)
{

/* print a character to current x-y (=virtual cursor)  position */
/* handle special cases first */
switch (ch)
	{
	/* handle a new line */
	case '\n':
		vga_cursorx = 0;
		vga_cursory++;
		/* no need to do cursor stuff anymore */
		return 1;
	default:
		break;
	}

/* get vidmem position based on x and y and print to it*/
short *temp_ptr = vga_memptr + vga_cursory*80 + vga_cursorx;
*temp_ptr = ch | vga_defaultcolor << 8; /* << 8 means we push the attribute byte into the short */



/* advance the cursor */
vga_cursorx++;

/* start a new line for the virtual cursor if needed */
if (vga_cursorx == 80)
	{
	vga_cursory++;
	vga_cursorx = 0;
	}




/* TODO: scrolling */

return 1;
}

int writeline(char *line)
{
while (*line) /* TODO: should this have a check of some sort, != \n or something? */
	{
	writech(*line);
	line++;
	}

return 1;
}
