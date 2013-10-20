/* stdio.c
* (loosely) standard c library
*/

/* the OS glue writech resides in scrn.c */
#include "stdio.h"
#include "scrn.h"

/* TODO: make %s use helpstring as well, use a common printing code */

/* a home made printf implementation, probably quite buggt */

void printf(const char *fmt, ...) {
unsigned char *pos;
int num = 0;
int print_int = 0;
va_list args;
va_start(args, fmt);

while (*fmt) {

int radix = 10; // default to base decimal, hex = 16
print_int = 0;
/* This is done to prevent some pretty crazy errors:
1. running "printf("%d %s",int a, char[10] b); leads to
printf("%d %s",int a, char[10] b); AND strncat(b,last-digit-of-a);
2. keyboard stops responding.
So, while we have no idea what it does, we'll use pos = something
in here as a workaround
*/
/* 
we needed this because the print_num section had *pos instead of pos.. shouldn't need it anymore
int tmp = 0;
pos = tmp; 
*/
if (*fmt != '%') 	/* not a control char, print it straight away */
	writech(*fmt);
else {
	fmt++; /* fmt == %, skip to the next one */
	switch (*fmt) {
		case 'x': // hex, change the base
			radix = 16;
		case 'd':
			num = va_arg(args, unsigned int);
			print_int = 1;
			break;

		case 's':
			pos = va_arg(args, unsigned char * );
			while (*pos != '\0') {
				writech(*pos);
				pos++;
				}
			break;

		case 'c':
			pos = va_arg(args, unsigned char *);
			writech(*pos);
			break;

		case '%':
			writech('%');
			break;
		}
}

if (print_int == 1) {
// a simple writech(num) would print the number corresponding to the hex ascii value
/* This is basically a bad itoa function */
/* get the numbers in reverse order */
int help = 0, help2 = 0, help3 = 0;
unsigned char helpstring[]="xxxxxxxxxxx";
/*
if (num == 127) {
	printf("!!!\n");
	num = 126;
	}
*/
// this loop through numbers higher than 10
while (num / radix > 0) {
	help3 = num % radix + '0';
	if (help3 > '9') // it's a hex!
		help3 = help3 + 7; // make it a number, '9' + 7 = 'A' (10 in hex)
	helpstring[help++] = help3;	
	num = num / radix;
	help2++;
	}

// the last one aka the first digit ( number < 10)
help3 = num % radix + '0';
if (help3 > '9') // it's a hex!
	help3 = help3 + 7; // make it a number
helpstring[help++] = help3;	

/* reverse to original order */
int temp = 0, first = 0;
int last = help2;

while (first < last) {
	temp = helpstring[first];
	helpstring[first] = helpstring[last];
	helpstring[last] = temp;
	last--;
	first++;
	}
/* now, print*/ 			
help = 0;
last = help2;
pos = helpstring;
while (*pos != '\0' && help <= last) {
	*pos = helpstring[help++]; // help++ means use help, only then ++!
	writech(*pos);
	}

}
fmt++;

}
va_end(args);
/* don't return anything, be a sucker */
}
