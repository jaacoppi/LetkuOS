/* keymap_fi */
#include "letkuos-common.h"
#include "keyboard.h"

char keymap_fi[3][256] =
{
	/* layer 0 - basic */
	{
/*	00	01	02	03	04	05	06	07	08	09	0A	0B	0C	0D	0E	0F*/
/* 00*/ NULL,	KEY_F9,	NULL,	KEY_F5,	KEY_F3,	KEY_F1,	KEY_F2,	KEY_F12,NULL,	KEY_F10,KEY_F8,	KEY_F6,	KEY_F4,	KEY_TAB,'?',	NULL,
/* 10*/ NULL,	KEY_LALT,KEY_LS,NULL,	KEY_LCTRL,'q',	'1',	NULL,	NULL,	NULL,	'z',	's',	'a',	'w',	'2',	NULL,
/* 20*/ NULL,	'c',	'x',	'd',	'e',	'4',	'3',	NULL,	NULL,	KEY_SPACE,'v',	'f',	't',	'r',	'5',	NULL,
/* 30*/ NULL,	'n',	'b',	'h',	'g',	'y',	'6',	NULL,	NULL,	NULL,	'm',	'j',	'u',	'7',	'8',	NULL,
/* 40*/ NULL,	',',	'k',	'i',	'o',	'0',	'9',	NULL,	NULL,	'.',	'&',	'l',	';',	'p',	'-',	NULL,
/* 50*/ NULL,	NULL,	'\'',	NULL,	'[',	'+',	NULL,	NULL,	KEY_CAPS,KEY_RS,KEY_ENT,']',	NULL,	'\\',	NULL,	NULL,
/* 60*/ NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	KEY_BSPACE,NULL,NULL,	KEY_NUM1,NULL,	KEY_NUM4,KEY_NUM7,NULL,	NULL,	NULL,
/* 70*/ NULL,	KEY_NUM0,KEY_NUMDOT,KEY_NUM2,'5',KEY_NUM6,KEY_NUM8,KEY_ESC,KEY_NUMLOCK,KEY_F11, '+',KEY_NUM3, '-','*',KEY_NUM9,KEY_SCROLLLOCK, NULL,
/* 80*/ NULL,	NULL,	NULL,	KEY_F7
	},

	/* layer 1 - shift pressed */
	{
/*	00	01	02	03	04	05	06	07	08	09	0A	0B	0C	0D	0E	0F*/
/* 00*/ NULL,	KEY_F9,	NULL,	KEY_F5,	KEY_F3,	KEY_F1,	KEY_F2,	KEY_F12,NULL,	KEY_F10,KEY_F8,	KEY_F6,	KEY_F4,	KEY_TAB,'?',	NULL,
/* 10*/ NULL,	KEY_LALT,KEY_LS,NULL,	KEY_LCTRL,'Q',	'!',	NULL,	NULL,	NULL,	'Z',	'S',	'A',	'W',	'"',	NULL,
/* 20*/ NULL,	'C',	'X',	'D',	'E',	NULL,	'#',	NULL,	NULL,	KEY_SPACE,'V',	'F',	'T',	'R',	'%',	NULL,
/* 30*/ NULL,	'N',	'B',	'H',	'G',	'Y',	'&',	NULL,	NULL,	NULL,	'M',	'J',	'U',	'&',	'*',	NULL,
/* 40*/ NULL,	'<',	'K',	'I',	'O',	')',	'(',	NULL,	NULL,	'>',	'?',	'L',	':',	'P',	'_',	NULL,
/* 50*/ NULL,	NULL,	'\'',	NULL,	'[',	'=',	NULL,	NULL,	KEY_CAPS,KEY_RS,KEY_ENT,']',	NULL,	'\\',	NULL,	NULL,
/* 60*/ NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	KEY_BSPACE,NULL,NULL,	KEY_NUM1,NULL,	KEY_NUM4,KEY_NUM7,NULL,	NULL,	NULL,
/* 70*/ NULL,	KEY_NUM0,KEY_NUMDOT,KEY_NUM2,'5',KEY_NUM6,KEY_NUM8,KEY_ESC,KEY_NUMLOCK,KEY_F11, '+',KEY_NUM3, '-','*',KEY_NUM9,KEY_SCROLLLOCK, NULL,
/* 80*/ NULL,	NULL,	NULL,	KEY_F7
	},
	/* layer 2 - alt gr pressed */
	{
	't','o','d','o'
	}
};

/* Scan code set 2 from http://www.osdever.net/documents/kbd.php?the_id=14
code key        code key        code key        code key
---- ---        ---- ---        ---- ---        ---- ---
01   F9                                         66   BackSpace
                21   C          41   ,<
03   F5         22   X          42   K          69   End 1
04   F3         23   D          43   I
05   F1         24   E          44   O          6B   (left) 4
06   F2         25   4$         45   0)         6C   Home 7
07   F12        26   3#         46   9(
                                                70   Ins 0
09   F10        29   Space      49   .>         71   Del .
0A   F8         2A   V          4A   /?         72   (down) 2
0B   F6         2B   F          4B   L          73   5
0C   F4         2C   T          4C   ;:         74   (right) 6
0D   Tab        2D   R          4D   P          75   (up) 8
0E   `~         2E   5%         4E   -_         76   Esc
                                                77   NumLock
11   L Alt      31   N          52   '"         78   F11
12   L Shift    32   B                          79   +
                33   H          54   [{         7A   PageDown 3
14   L Ctrl     34   G          55   =+         7B   -
15   Q          35   Y                          7C   *
16   1!         36   6^         58   CapsLock   7D   PageUp 9
                                59   R Shift    7E   ScrollLock
1A   Z          3A   M          5A   Enter
1B   S          3B   J          5B   ]}         83   F7
1C   A          3C   U
1D   W          3D   7&         5D   \|
1E   2@         3E   8*

code            key
----            ---
E011            R Alt
E012E07C        PrintScreen make code
E014            R Ctrl
E01F            L Win
E027            R Win
E02F            Menu
E04A            /
E05A            Enter (on numeric keypad)
E069            End
E06B            Left
E06C            Home
E070            Ins
E071            Del
E072            (down)
E074            (right)
E075            (up)
E07A            PageDown
E07C            PrintScreen repeat code
E07D            PageUp
E0F07CE0F012    PrintScreen break code
E11477E1F014F077 Pause
*/
