#include "scrn.h"

/* for info about the video memory, shorts and bytes, see
http://www.osdever.net/bkerndev/Docs/printing.htm for example
*/

short *vga_memptr;
int  vga_defaultcolor = VGA_GREY_ON_BLACK;
short blank = VGA_BLANK;

/* video memory is a block of contiguous memory. In contrast, humans tend to process information in chunks.
Therefore, we need an abstraction of x-y dimension to utilize newlines
*/
int vga_cursorx;
int vga_cursory;

void screen_scroll();

int init_video()
{
vga_memptr = (short *) VIDMEM;
vga_cursorx=0;
vga_cursory=0;

cls();
return 1;
}


/* clears the screen */
int cls()
{
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
		*temp_ptr = ' '| blank << 8; /* << 8 means we push the attribute byte into the short */
		}

/* move the cursor to upper left corner so the next line will start "from the beginning" */
vga_cursorx = 0;
vga_cursory = 0;

return 1;
}



/* write a character to the screen */
int writech(unsigned char ch)
{
/* print a character to current x-y (=virtual cursor)  position */
/* handle special cases first */
switch (ch)
	{
	/* handle a new line */
	case '\n':
		vga_cursorx = 0;
		vga_cursory++;
		break;
	default:
		break;
	}

/* scrolling */
if (vga_cursory >= 25) /* 25 lines, start from 0 */
	{
	screen_scroll();
	vga_cursorx = 0;
	}



/* get vidmem position based on x and y and print to it*/
short *temp_ptr = vga_memptr + vga_cursory*80 + vga_cursorx;
*temp_ptr = ch | vga_defaultcolor << 8; /* << 8 means we push the attribute byte into the short */



/* advance the cursor, except when the char is a newline */
if (ch != '\n')
	vga_cursorx++;

/* start a new line for the virtual cursor if needed */
if (vga_cursorx == 80)
	{
	vga_cursory++;
	vga_cursorx = 0;
	}

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

/* scroll screen down one line */
void screen_scroll()
{
vga_cursory--;

short *temp_ptr;
/* TODO: use memcopy or something to copy the whole line, don't do char by char */
int i;

for (i = 0; i < 25*80; i++) /* 25*80 = whole buffer */
	{
	temp_ptr = (vga_memptr + i);

	/* take the char from direcly one line above and print it to the screen */
	short *next_ptr = (temp_ptr + 80);
	short ch = (short ) (*next_ptr);
	*temp_ptr = (char )ch | vga_defaultcolor << 8; 
	}

/* clear the last line so it can be used for writing after this function returns */
for (i = 0; i < 80; i++)
	{
	temp_ptr = (vga_memptr + (24*80) + i);
	*temp_ptr = ' '| blank << 8;
	}
}
