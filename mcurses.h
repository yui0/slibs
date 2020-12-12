/* GPL2 Simple, Minimalistic, curses
 *	©2020 Yuichiro Nakada
 *
 * Basic usage:
 *	setFunction_putchar(putchar);
 *	initscr();
 *	move(11, 15);
 *	addstr("Hello, World");
 *
 * */

//return the length of result string. support only 10 radix for easy use and better performance
int itoa(int val, char* buf, unsigned int radix)
{
	//const unsigned int radix = 10;

	char* p;
	unsigned int a;        //every digit
	int len;
	char* b;            //start of the digit char
	char temp;
	unsigned int u;

	p = buf;

	if (val < 0) {
		*p++ = '-';
		val = 0 - val;
	}
	u = (unsigned int)val;

	b = p;

	do {
		a = u % radix;
		u /= radix;

		*p++ = a + '0';

	} while (u > 0);

	len = (int)(p - buf);

	*p-- = 0;

	//swap
	do {
		temp = *p;
		*p = *b;
		*b = temp;
		--p;
		++b;

	} while (b < p);

	return len;
}


/*-----------------------------------------------------------------------------------------------------------------
   line editor
   Revision History:
   V1.0 2017 01 18 ChrisMicro, initial version
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
  ---------------------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * mcurses.h - include file for mcurses lib
 *
 * Copyright (c) 2011-2015 Frank Meyer - frank(at)fli4l.de
 *
 * Revision History:
 * V1.0 2015 xx xx Frank Meyer, original version
 * V1.1 2017 01 13 ChrisMicro, addepted as Arduino library, MCU specific functions removed
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#ifdef __cplusplus
extern "C"
{
#endif
#ifndef __MCURSES__
#define __MCURSES__

#include <stdint.h>
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * mcurses-config.h - configuration file for mcurses lib
 *
 * Copyright (c) 2011-2015 Frank Meyer - frank(at)fli4l.de
 *
  * Revision History:
 * V1.0 2015 xx xx Frank Meyer, original version
 * V1.1 2017 01 13 ChrisMicro , serial interface specific functions removed
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define MCURSES_LINES               24              // 24 lines
#define MCURSES_COLS                80              // 80 columns
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * some constants
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define LINES                   MCURSES_LINES
#define COLS                    MCURSES_COLS
#ifndef TRUE
#define TRUE                    (1)                                                 // true
#define FALSE                   (0)                                                 // false
#endif
#define OK                      (0)                                                 // yet not used
#define ERR                     (255)                                               // yet not used
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * attributes, may be ORed
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define A_NORMAL                0x0000                                              // normal
#define A_UNDERLINE             0x0001                                              // underline
#define A_REVERSE               0x0002                                              // reverse
#define A_BLINK                 0x0004                                              // blink
#define A_BOLD                  0x0008                                              // bold
#define A_DIM                   0x0010                                              // dim
#define A_STANDOUT              A_BOLD                                              // standout (same as bold)
#define F_BLACK                 0x0100                                              // foreground black
#define F_RED                   0x0200                                              // foreground red
#define F_GREEN                 0x0300                                              // foreground green
#define F_BROWN                 0x0400                                              // foreground brown
#define F_BLUE                  0x0500                                              // foreground blue
#define F_MAGENTA               0x0600                                              // foreground magenta
#define F_CYAN                  0x0700                                              // foreground cyan
#define F_WHITE                 0x0800                                              // foreground white
#define F_YELLOW                F_BROWN                                             // some terminals show brown as yellow (with A_BOLD)
#define F_COLOR                 0x0F00                                              // foreground mask
#define B_BLACK                 0x1000                                              // background black
#define B_RED                   0x2000                                              // background red
#define B_GREEN                 0x3000                                              // background green
#define B_BROWN                 0x4000                                              // background brown
#define B_BLUE                  0x5000                                              // background blue
#define B_MAGENTA               0x6000                                              // background magenta
#define B_CYAN                  0x7000                                              // background cyan
#define B_WHITE                 0x8000                                              // background white
#define B_YELLOW                B_BROWN                                             // some terminals show brown as yellow (with A_BOLD)
#define B_COLOR                 0xF000                                              // background mask
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * mcurses variables
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
extern uint_fast8_t             mcurses_is_up;                                      // flag: mcurses is up
extern uint_fast8_t             mcurses_cury;                                       // do not use, use getyx() instead!
extern uint_fast8_t             mcurses_curx;                                       // do not use, use getyx() instead!
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * mcurses functions
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void                     setFunction_putchar(void (*functionPoitner)(uint8_t ch));
void                     setFunction_getchar(char (*functionPoitner)(void));
uint_fast8_t             initscr (void);                                     // initialize mcurses
void                     move (uint_fast8_t, uint_fast8_t);                  // move cursor to line, column (home = 0, 0)
void                     attrset (uint_fast16_t);                            // set attribute(s)
void                     addch (uint_fast8_t);                               // add a character
void                     addstr (const char *);                              // add a string
void                     addstr_P (const char *);                            // add a string (PROGMEM)
void                     getnstr (char * str, uint_fast8_t maxlen);          // read a string (with mini editor functionality)
void                     setscrreg (uint_fast8_t, uint_fast8_t);             // set scrolling region
void                     deleteln (void);                                    // delete line at current line position
void                     insertln (void);                                    // insert a line at current line position
void                     scroll (void);                                      // scroll line up
void                     clear (void);                                       // clear total screen
void                     clrtobot (void);                                    // clear screen from current line to bottom
void                     clrtoeol (void);                                    // clear from current column to end of line
void                     delch (void);                                       // delete character at current position
void                     insch (uint_fast8_t);                               // insert character at current position
void                     nodelay (uint_fast8_t);                             // set/reset nodelay
void                     halfdelay (uint_fast8_t);                           // set/reset halfdelay
uint_fast8_t             getch (void);                                       // read key
void                     curs_set(uint_fast8_t);                             // set cursor to: 0=invisible 1=normal 2=very visible
void                     refresh (void);                                     // flush output
void                     endwin (void);                                      // end mcurses
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * mcurses macros
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define erase()                 clear()                                             // clear total screen, same as clear()
#define mvaddch(y,x,c)          move((y),(x)), addch((c))                           // move cursor, then add character
#define mvaddstr(y,x,s)         move((y),(x)), addstr((s))                          // move cursor, then add string
#define mvaddstr_P(y,x,s)       move((y),(x)), addstr_P((s))                        // move cursor, then add string (PROGMEM)
#define mvinsch(y,x,c)          move((y),(x)), insch((c))                           // move cursor, then insert character
#define mvdelch(y,x)            move((y),(x)), delch()                              // move cursor, then delete character
#define mvgetnstr(y,x,s,n)      move((y),(x)), getnstr(s,n)                         // move cursor, then get string
#define getyx(y,x)              y = mcurses_cury, x = mcurses_curx                  // get cursor coordinates
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * mcurses keys
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define KEY_TAB                 '\t'                                                // TAB key
#define KEY_CR                  '\r'                                                // RETURN key
#define KEY_BACKSPACE           '\b'                                                // Backspace key
#define KEY_ESCAPE              0x1B                                                // ESCAPE (pressed twice)
#define KEY_DOWN                0x80                                                // Down arrow key
#define KEY_UP                  0x81                                                // Up arrow key
#define KEY_LEFT                0x82                                                // Left arrow key
#define KEY_RIGHT               0x83                                                // Right arrow key
#define KEY_HOME                0x84                                                // Home key
#define KEY_DC                  0x85                                                // Delete character key
#define KEY_IC                  0x86                                                // Ins char/toggle ins mode key
#define KEY_NPAGE               0x87                                                // Next-page key
#define KEY_PPAGE               0x88                                                // Previous-page key
#define KEY_END                 0x89                                                // End key
#define KEY_BTAB                0x8A                                                // Back tab key
#define KEY_F1                  0x8B                                                // Function key F1
#define KEY_F(n)                (KEY_F1+(n)-1)                                      // Space for additional 12 function keys
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * graphics: draw boxes
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define ACS_LRCORNER            0x8a                                                // DEC graphic 0x6a: lower right corner
#define ACS_URCORNER            0x8b                                                // DEC graphic 0x6b: upper right corner
#define ACS_ULCORNER            0x8c                                                // DEC graphic 0x6c: upper left corner
#define ACS_LLCORNER            0x8d                                                // DEC graphic 0x6d: lower left corner
#define ACS_PLUS                0x8e                                                // DEC graphic 0x6e: crossing lines
#define ACS_HLINE               0x91                                                // DEC graphic 0x71: horizontal line
#define ACS_LTEE                0x94                                                // DEC graphic 0x74: left tee
#define ACS_RTEE                0x95                                                // DEC graphic 0x75: right tee
#define ACS_BTEE                0x96                                                // DEC graphic 0x76: bottom tee
#define ACS_TTEE                0x97                                                // DEC graphic 0x77: top tee
#define ACS_VLINE               0x98                                                // DEC graphic 0x78: vertical line
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * graphics: other symbols
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define ACS_DIAMOND             0x80                                                // DEC graphic 0x60: diamond
#define ACS_CKBOARD             0x81                                                // DEC graphic 0x61: checker board
#define ACS_DEGREE              0x86                                                // DEC graphic 0x66: degree symbol
#define ACS_PLMINUS             0x87                                                // DEC graphic 0x66: plus/minus
#define ACS_S1                  0x8f                                                // DEC graphic 0x6f: scan line 1
#define ACS_S3                  0x90                                                // DEC graphic 0x70: scan line 3
#define ACS_S5                  0x91                                                // DEC graphic 0x71: scan line 5
#define ACS_S7                  0x92                                                // DEC graphic 0x72: scan line 7
#define ACS_S9                  0x93                                                // DEC graphic 0x73: scan line 9
#define ACS_LEQUAL              0x99                                                // DEC graphic 0x79: less/equal
#define ACS_GEQUAL              0x9a                                                // DEC graphic 0x7a: greater/equal
#define ACS_PI                  0x9b                                                // DEC graphic 0x7b: Pi
#define ACS_NEQUAL              0x9c                                                // DEC graphic 0x7c: not equal
#define ACS_STERLING            0x9d                                                // DEC graphic 0x7d: uk pound sign
#define ACS_BULLET              0x9e                                                // DEC graphic 0x7e: bullet

#endif // __MCURSES__
#ifdef __cplusplus
}
#endif
#include <stdlib.h>     // strtol 
#include <string.h>
/*-----------------------------------------------------------------------------------------------------------------
 string line editor editor
 input: string buffer: the content of the buffer will be displayed and you can edit it
        lineLength   : maximum number of characters the line can have and can be edited
  output: *str       : edited string
 This function is a modified and extended version of
 void getnstr (char * str, uint_fast8_t maxlen)
 which allows predefined strings.
 getnstr was written by  Frank Meyer - frank(at)fli4l.de

------------------------------------------------------------------------------------------------------------------*/

void editLine (char * str, uint8_t lineLength )
{
	uint_fast8_t ch;
	uint_fast8_t curlen = 0;
	uint_fast8_t curpos = 0;
	uint_fast8_t starty;
	uint_fast8_t startx;
	uint_fast8_t i;
	curlen=strlen(str);
	curpos=curlen;
	getyx (starty, startx);           // get current cursor position
	for (i=0; i<lineLength; i++) {
		addch(' ');        // clear region
	}
	attrset(A_REVERSE);

	move(starty,startx);
	addstr (str);    // show text
	move (starty, startx+curpos);
	while ((ch = getch ()) != KEY_CR) {
		switch (ch) {
		case KEY_LEFT:
			if (curpos > 0) {
				curpos--;
			}
			break;
		case KEY_RIGHT:
			if (curpos < curlen) {
				curpos++;
			}
			break;
		case KEY_HOME:
			curpos = 0;
			break;
		case KEY_END:
			curpos = curlen;
			break;
		case KEY_BACKSPACE:
			if (curpos > 0) {
				curpos--;
				curlen--;
				move (starty, startx + curpos);
				for (i = curpos; i < curlen; i++) {
					str[i] = str[i + 1];
				}
				str[i] = '\0';

				for (i=curpos; i<curlen; i++) {
					addch(str[i]);
				}
				attrset(A_REVERSE);
				attrset(A_NORMAL);
				addch(' ');
				attrset(A_REVERSE);
			}
			break;
		case KEY_DC: // delete-key
			if (curlen > 0) {
				curlen--;
				for (i = curpos; i < curlen; i++) {
					str[i] = str[i + 1];
				}
				str[i] = '\0';
				for (i=curpos; i<curlen; i++) {
					addch(str[i]);
				}
				attrset(A_REVERSE);
				attrset(A_NORMAL);
				addch(' ');
				attrset(A_REVERSE);
			}
			break;
		default:
			if (curlen < lineLength && (ch & 0x7F) >= 32 && (ch & 0x7F) < 127) {    // printable ASCII 7bit or printable 8bit ISO8859
				for (i = curlen; i > curpos; i--) {
					str[i] = str[i - 1];
				}
				str[curpos] = ch;
				curlen++;
				for (i=curpos; i<curlen; i++) {
					addch(str[i]);
				}
				curpos++;
			}
		}
		move (starty, startx + curpos);
	}
	str[curlen] = '\0';

	// redraw the string with normal attribute
	attrset(A_REVERSE);
	attrset(A_NORMAL);
	move(starty,startx);
	addstr (str);    // show text
}
/*-----------------------------------------------------------------------------------------------------------------
 edit a given number
 input: number: initial number. It will be displayed
 output: edited number

-----------------------------------------------------------------------------------------------------------------*/
int16_t editInt16(int16_t initialNumber)
{
	char text[10];
	uint8_t starty;
	uint8_t startx;

	getyx (starty, startx);   // get current cursor position
	itoa(initialNumber,text,10);
	editLine (text,6);

	int16_t number = strtol(text, 0, 10);
	itoa(number,text,10);
	move (starty, startx);    // set cursor position from the beginning
	addstr (text);            // show text
	return number;
}
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * @file hexedit.c - full screen hex editor
 *
 * Copyright (c) 2014 Frank Meyer - frank(at)fli4l.de
 *
 * Revision History:
 * V1.0 2015 xx xx Frank Meyer, original version
 * V1.1 2017 01 14 ChrisMicro, converted to Arduino example
 * V1.2 2019 01 22 ChrisMicro, memory access functions can now be set
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Possible keys:
 *  LEFT                back one byte
 *  RIGHT               forward one byte
 *  DOWN                one line down
 *  UP                  one line up
 *  TAB                 toggle between hex and ascii columns
 *  any other           input as hex or ascii value
 *  2 x ESCAPE          exit
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void itox(uint8_t val);
void itoxx(unsigned char i);
uint8_t xtoi(uint8_t ch);
void print_hex_line(uint8_t line, uint16_t off);
void hexedit(uint16_t offset);
#define FIRST_LINE      1
#define LAST_LINE       (LINES - 1)
#define BYTES_PER_ROW   16
#define FIRST_HEX_COL   7
#define FIRST_ASCII_COL 57
#define LAST_BYTE_COL   (FIRST_HEX_COL + 3 * BYTES_PER_ROW)
#define IS_PRINT(ch)    (((ch) >= 32 && (ch) < 0x7F) || ((ch) >= 0xA0))
#define MODE_HEX        0
#define MODE_ASCII      1
#define IS_HEX(ch)      (((ch) >= 'A' && (ch) <= 'F') || ((ch) >= 'a' && (ch) <= 'f') || ((ch) >= '0' && (ch) <= '9'))
/*
#ifdef unix
#define PEEK(x)         memory[x]
#define POKE(x,v)       memory[x] = (v)
unsigned char           memory[65536];
#else // SDCC
#define PEEK(x)         *((unsigned char *) (x))
#define POKE(x,v)       *((unsigned char *) (x)) = (v)
#endif
*/
uint8_t peekMemory(uint16_t address)
{
	uint8_t *p;
	return p[address];
}
void    pokeMemory(uint16_t address, uint8_t value)
{
	uint8_t *p;
	p[address]=value;
}
uint8_t (*FunctionPointer_readMemory)(uint16_t address)=peekMemory; // set default function
void  (*FunctionPointer_writeMemory)(uint16_t address, uint8_t value)=pokeMemory; // set default function
#define PEEK(x) FunctionPointer_readMemory(x)
#define POKE(x,v) FunctionPointer_writeMemory(x,v)
void setFunction_readMemory(uint8_t (*functionPointer)(uint16_t address))
{
	FunctionPointer_readMemory = functionPointer;
}
void setFunction_writeMemory(void (*functionPointer)(uint16_t address, uint8_t value))
{
	FunctionPointer_writeMemory = functionPointer;
}
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * itox: convert a decimal value 0-15 into hexadecimal digit
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void itox (uint8_t val)
{
	uint8_t ch;
	val &= 0x0F;
	if (val <= 9) {
		ch = val + '0';
	} else {
		ch = val - 10 + 'A';
	}
	addch (ch);
}
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * itoxx: convert a decimal value 0-255 into 2 hexadecimal digits
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void itoxx (unsigned char i)
{
	itox (i >> 4);
	itox (i & 0x0F);
}
uint8_t xtoi (uint8_t ch)
{
	uint8_t val;
	if (ch >= 'A' && ch <= 'F') {
		val = (ch - 'A') + 10;
	} else if (ch >= 'a' && ch <= 'f') {
		val = (ch - 'a') + 10;
	} else {
		val = (ch - '0');
	}
	itox (val);     // print value in hex
	return val;
}
void print_hex_line (uint8_t line, uint16_t off)
{
	uint8_t         col;
	uint8_t         ch;
	move (line, 0);
	itoxx (off >> 8);
	itoxx (off & 0xFF);
	move (line, FIRST_HEX_COL);
	for (col = 0; col < BYTES_PER_ROW; col++) {
		itoxx (PEEK(off));
		addch (' ');
		off++;
	}
	off -= BYTES_PER_ROW;
	move (line, FIRST_ASCII_COL);
	for (col = 0; col < BYTES_PER_ROW; col++) {
		ch = PEEK(off);
		if (IS_PRINT(ch)) {
			addch (ch);
		} else {
			addch ('.');
		}
		off++;
	}
}
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * hexdit: hex editor
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void hexedit (uint16_t offset)
{
	uint8_t         ch;
	uint8_t         line;
	uint8_t         col;
	uint16_t        off;
	uint8_t         byte;
	uint8_t         mode = MODE_HEX;
	clear ();
	setscrreg (FIRST_LINE, LAST_LINE);
	off = offset;
	move (0, 0);
	attrset (A_REVERSE);
	for (col = 0; col < FIRST_HEX_COL; col++) {
		addch (' ');
	}
	for (byte = 0; byte < BYTES_PER_ROW; byte++) {
		itoxx (byte);
		addch (' ');
		col += 3;
	}
	for ( ; col < FIRST_ASCII_COL; col++) {
		addch (' ');
	}
	for (byte = 0; byte < BYTES_PER_ROW; byte++) {
		itox (byte);
	}
	attrset (A_NORMAL);
	for (line = FIRST_LINE; line < LAST_LINE; line++) {
		print_hex_line (line, off);
		off += BYTES_PER_ROW;
	}
	off = offset;
	line = FIRST_LINE;
	col = FIRST_HEX_COL;
	byte = 0;
	do {
		move (line, col);
		ch = getch ();
		switch (ch) {
		case KEY_RIGHT: {
			if (byte < BYTES_PER_ROW - 1) {
				byte++;
				col += (mode == MODE_HEX) ? 3 : 1;
			}
			break;
		}
		case KEY_LEFT: {
			if (byte > 0) {
				byte--;
				col -= (mode == MODE_HEX) ? 3 : 1;
			}
			break;
		}
		case KEY_DOWN: {
			if (off < 65520) {
				off += BYTES_PER_ROW;
				if (line == LAST_LINE - 1) {
					scroll ();
					print_hex_line (line, off);
				} else {
					line++;
				}
			}
			break;
		}
		case KEY_UP: {
			if (off >= BYTES_PER_ROW) {
				off -= BYTES_PER_ROW;
				if (line == FIRST_LINE) {
					insertln ();
					print_hex_line (line, off);
				} else {
					line--;
				}
			}
			break;
		}
		case KEY_TAB: {
			if (mode == MODE_HEX) {
				mode = MODE_ASCII;
				col = FIRST_ASCII_COL + byte;
			} else {
				mode = MODE_HEX;
				col = FIRST_HEX_COL + 3 * byte;
			}
			}    /* fall through */
		default: {
			if (mode == MODE_HEX) {
				if (IS_HEX(ch)) {
					uint16_t    addr  = off + byte;
					uint8_t     value = xtoi (ch) << 4;
					ch = getch ();
					if (IS_HEX(ch)) {
						value |= xtoi (ch);
						POKE(addr, value);
						move (line, FIRST_ASCII_COL + byte);
						if (IS_PRINT(value)) {
							addch (value);
						} else {
							addch ('.');
						}
					}
				}
			} else { // MODE_ASCII
				if (IS_PRINT(ch)) {
					uint16_t addr = off + byte;
					POKE(addr, ch);
					addch (ch);
					move (line, FIRST_HEX_COL + 3 * byte);
					itoxx (ch);
				}
			}
		}
		}
	} while (ch != KEY_ESCAPE);
}
/*
   hexedit.h
   Created: 18.01.2017 08:24:04
    Author: ChrisMicro
*/
#ifdef __cplusplus
extern "C"
{
#endif
#ifndef HEXEDIT_H_
#define HEXEDIT_H_
void hexedit (uint16_t offset);
void setFunction_readMemory(uint8_t (*functionPointer)(uint16_t address));
void setFunction_writeMemory(void (*functionPointer)(uint16_t address, uint8_t value));
#endif /* HEXEDIT_H_ */
#ifdef __cplusplus
}
#endif
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * mcurses.c - mcurses lib
 *
 * Copyright (c) 2011-2015 Frank Meyer - frank(at)fli4l.de
 *
 * Revision History:
 * V1.0 2015 xx xx Frank Meyer, original version
 * V1.1 2017 01 13 ChrisMicro, addepted as Arduino library, MCU specific functions removed
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#ifdef __AVR__
#include <avr/pgmspace.h>
#else
#define PROGMEM
#define PSTR(x)                                 (x)
#define pgm_read_byte(s)                        (*s)
#endif
#define SEQ_CSI                                 PSTR("\033[")                   // code introducer
#define SEQ_CLEAR                               PSTR("\033[2J")                 // clear screen
#define SEQ_CLRTOBOT                            PSTR("\033[J")                  // clear to bottom
#define SEQ_CLRTOEOL                            PSTR("\033[K")                  // clear to end of line
#define SEQ_DELCH                               PSTR("\033[P")                  // delete character
#define SEQ_NEXTLINE                            PSTR("\033E")                   // goto next line (scroll up at end of scrolling region)
#define SEQ_INSERTLINE                          PSTR("\033[L")                  // insert line
#define SEQ_DELETELINE                          PSTR("\033[M")                  // delete line
#define SEQ_ATTRSET                             PSTR("\033[0")                  // set attributes, e.g. "\033[0;7;1m"
#define SEQ_ATTRSET_REVERSE                     PSTR(";7")                      // reverse
#define SEQ_ATTRSET_UNDERLINE                   PSTR(";4")                      // underline
#define SEQ_ATTRSET_BLINK                       PSTR(";5")                      // blink
#define SEQ_ATTRSET_BOLD                        PSTR(";1")                      // bold
#define SEQ_ATTRSET_DIM                         PSTR(";2")                      // dim
#define SEQ_ATTRSET_FCOLOR                      PSTR(";3")                      // forground color
#define SEQ_ATTRSET_BCOLOR                      PSTR(";4")                      // background color
#define SEQ_INSERT_MODE                         PSTR("\033[4h")                 // set insert mode
#define SEQ_REPLACE_MODE                        PSTR("\033[4l")                 // set replace mode
#define SEQ_RESET_SCRREG                        PSTR("\033[r")                  // reset scrolling region
#define SEQ_LOAD_G1                             PSTR("\033)0")                  // load G1 character set
#define SEQ_CURSOR_VIS                          PSTR("\033[?25")                // set cursor visible/not visible
static uint_fast8_t                             mcurses_scrl_start = 0;         // start of scrolling region, default is 0
static uint_fast8_t                             mcurses_scrl_end = LINES - 1;   // end of scrolling region, default is last line
static uint_fast8_t                             mcurses_nodelay;                // nodelay flag
static uint_fast8_t                             mcurses_halfdelay;              // halfdelay value, in tenths of a second
uint_fast8_t                                    mcurses_is_up = 0;              // flag: mcurses is up
uint_fast8_t                                    mcurses_cury = 0xff;            // current y position of cursor, public (getyx())
uint_fast8_t                                    mcurses_curx = 0xff;            // current x position of cursor, public (getyx())
static void                                     mcurses_puts_P (const char *);
char (*FunctionPointer_getchar)(void);
void  (*FunctionPointer_putchar)(uint_fast8_t ch);
void setFunction_getchar(char (*functionPoitner)(void))
{
	FunctionPointer_getchar = functionPoitner;
}
void setFunction_putchar(void (*functionPoitner)(uint8_t ch))
{
	FunctionPointer_putchar = functionPoitner;
}
static uint_fast8_t mcurses_phyio_init (void)
{
	return 0;
}
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * PHYIO: done (AVR)
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void mcurses_phyio_done (void)
{

}
static void mcurses_phyio_putc (uint_fast8_t ch)
{
	if (FunctionPointer_putchar!=0)	{
		FunctionPointer_putchar(ch);
	}
}
static uint_fast8_t mcurses_phyio_getc (void)
{
	if (FunctionPointer_getchar!=0)	{
		return FunctionPointer_getchar();
	} else {
		return 0;
	}
}
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * PHYIO: set/reset nodelay (AVR)
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void mcurses_phyio_nodelay (uint_fast8_t flag)
{
	mcurses_nodelay = flag;
}
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * PHYIO: set/reset halfdelay (AVR)
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void mcurses_phyio_halfdelay (uint_fast8_t tenths)
{
	mcurses_halfdelay = tenths;
}
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * PHYIO: flush output (AVR)
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void mcurses_phyio_flush_output ()
{

}
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * INTERN: put a character (raw)
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
mcurses_putc (uint_fast8_t ch)
{
	mcurses_phyio_putc (ch);
}
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * INTERN: put a string from flash (raw)
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
mcurses_puts_P (const char * str)
{
	uint_fast8_t ch;
	while ((ch = pgm_read_byte(str)) != '\0') {
		mcurses_putc (ch);
		str++;
	}
}
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * INTERN: put a 3/2/1 digit integer number (raw)
 *
 * Here we don't want to use sprintf (too big on AVR/Z80) or itoa (not available on Z80)
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
mcurses_puti (uint_fast8_t i)
{
	uint_fast8_t ii;
	if (i >= 10) {
		if (i >= 100) {
			ii = i / 100;
			mcurses_putc (ii + '0');
			i -= 100 * ii;
		}
		ii = i / 10;
		mcurses_putc (ii + '0');
		i -= 10 * ii;
	}
	mcurses_putc (i + '0');
}
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * INTERN: addch or insch a character
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define CHARSET_G0      0
#define CHARSET_G1      1
static void
mcurses_addch_or_insch (uint_fast8_t ch, uint_fast8_t insert)
{
	static uint_fast8_t  charset = 0xff;
	static uint_fast8_t  insert_mode = FALSE;
	if (ch >= 0x80 && ch <= 0x9F) {
		if (charset != CHARSET_G1) {
			mcurses_putc ('\016');                                              // switch to G1 set
			charset = CHARSET_G1;
		}
		ch -= 0x20;                                                             // subtract offset to G1 characters
	} else {
		if (charset != CHARSET_G0) {
			mcurses_putc ('\017');                                              // switch to G0 set
			charset = CHARSET_G0;
		}
	}
	if (insert) {
		if (! insert_mode) {
			mcurses_puts_P (SEQ_INSERT_MODE);
			insert_mode = TRUE;
		}
	} else {
		if (insert_mode) {
			mcurses_puts_P (SEQ_REPLACE_MODE);
			insert_mode = FALSE;
		}
	}
	mcurses_putc (ch);
	mcurses_curx++;
}
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * INTERN: set scrolling region (raw)
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
mysetscrreg (uint_fast8_t top, uint_fast8_t bottom)
{
	if (top == bottom) {
		mcurses_puts_P (SEQ_RESET_SCRREG);                                      // reset scrolling region
	} else {
		mcurses_puts_P (SEQ_CSI);
		mcurses_puti (top + 1);
		mcurses_putc (';');
		mcurses_puti (bottom + 1);
		mcurses_putc ('r');
	}
}
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * move cursor (raw)
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
mymove (uint_fast8_t y, uint_fast8_t x)
{
	mcurses_puts_P (SEQ_CSI);
	mcurses_puti (y + 1);
	mcurses_putc (';');
	mcurses_puti (x + 1);
	mcurses_putc ('H');
}
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: initialize
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
uint_fast8_t
initscr (void)
{
	uint_fast8_t rtc;
	if (mcurses_phyio_init ()) {
		mcurses_puts_P (SEQ_LOAD_G1);                                               // load graphic charset into G1
		attrset (A_NORMAL);
		clear ();
		move (0, 0);
		mcurses_is_up = 1;
		rtc = OK;
	} else {
		rtc = ERR;
	}
	return rtc;
}
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: add character
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
addch (uint_fast8_t ch)
{
	mcurses_addch_or_insch (ch, FALSE);
}
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: add string
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
addstr (const char * str)
{
	while (*str) {
		mcurses_addch_or_insch (*str++, FALSE);
	}
}
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: add string
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
addstr_P (const char * str)
{
	uint_fast8_t ch;
	while ((ch = pgm_read_byte(str)) != '\0') {
		mcurses_addch_or_insch (ch, FALSE);
		str++;
	}
}
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: set attribute(s)
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
attrset (uint_fast16_t attr)
{
	static uint_fast8_t mcurses_attr = 0xff;                    // current attributes
	uint_fast8_t        idx;
	if (attr != mcurses_attr) {
		mcurses_puts_P (SEQ_ATTRSET);
		idx = (attr & F_COLOR) >> 8;
		if (idx >= 1 && idx <= 8) {
			mcurses_puts_P (SEQ_ATTRSET_FCOLOR);
			mcurses_putc (idx - 1 + '0');
		}
		idx = (attr & B_COLOR) >> 12;
		if (idx >= 1 && idx <= 8) {
			mcurses_puts_P (SEQ_ATTRSET_BCOLOR);
			mcurses_putc (idx - 1 + '0');
		}
		if (attr & A_REVERSE) {
			mcurses_puts_P (SEQ_ATTRSET_REVERSE);
		}
		if (attr & A_UNDERLINE) {
			mcurses_puts_P (SEQ_ATTRSET_UNDERLINE);
		}
		if (attr & A_BLINK) {
			mcurses_puts_P (SEQ_ATTRSET_BLINK);
		}
		if (attr & A_BOLD) {
			mcurses_puts_P (SEQ_ATTRSET_BOLD);
		}
		if (attr & A_DIM) {
			mcurses_puts_P (SEQ_ATTRSET_DIM);
		}
		mcurses_putc ('m');
		mcurses_attr = attr;
	}
}
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: move cursor
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
move (uint_fast8_t y, uint_fast8_t x)
{
	if (mcurses_cury != y || mcurses_curx != x) {
		mcurses_cury = y;
		mcurses_curx = x;
		mymove (y, x);
	}
}
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: delete line
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
deleteln (void)
{
	mysetscrreg (mcurses_scrl_start, mcurses_scrl_end);                         // set scrolling region
	mymove (mcurses_cury, 0);                                                   // goto to current line
	mcurses_puts_P (SEQ_DELETELINE);                                            // delete line
	mysetscrreg (0, 0);                                                         // reset scrolling region
	mymove (mcurses_cury, mcurses_curx);                                        // restore position
}
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: insert line
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
insertln (void)
{
	mysetscrreg (mcurses_cury, mcurses_scrl_end);                               // set scrolling region
	mymove (mcurses_cury, 0);                                                   // goto to current line
	mcurses_puts_P (SEQ_INSERTLINE);                                            // insert line
	mysetscrreg (0, 0);                                                         // reset scrolling region
	mymove (mcurses_cury, mcurses_curx);                                        // restore position
}
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: scroll
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
scroll (void)
{
	mysetscrreg (mcurses_scrl_start, mcurses_scrl_end);                         // set scrolling region
	mymove (mcurses_scrl_end, 0);                                               // goto to last line of scrolling region
	mcurses_puts_P (SEQ_NEXTLINE);                                              // next line
	mysetscrreg (0, 0);                                                         // reset scrolling region
	mymove (mcurses_cury, mcurses_curx);                                        // restore position
}
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: clear
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
clear (void)
{
	mcurses_puts_P (SEQ_CLEAR);
}
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: clear to bottom of screen
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
clrtobot (void)
{
	mcurses_puts_P (SEQ_CLRTOBOT);
}
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: clear to end of line
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
clrtoeol (void)
{
	mcurses_puts_P (SEQ_CLRTOEOL);
}
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: delete character at cursor position
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
delch (void)
{
	mcurses_puts_P (SEQ_DELCH);
}
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: insert character
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
insch (uint_fast8_t ch)
{
	mcurses_addch_or_insch (ch, TRUE);
}
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: set scrolling region
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
setscrreg (uint_fast8_t t, uint_fast8_t b)
{
	mcurses_scrl_start = t;
	mcurses_scrl_end = b;
}
void
curs_set (uint_fast8_t visibility)
{
	mcurses_puts_P (SEQ_CURSOR_VIS);
	if (visibility == 0) {
		mcurses_putc ('l');
	} else {
		mcurses_putc ('h');
	}
}
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: refresh: flush output
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
refresh (void)
{
	mcurses_phyio_flush_output ();
}
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: set/reset nodelay
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
nodelay (uint_fast8_t flag)
{
	if (mcurses_nodelay != flag) {
		mcurses_phyio_nodelay (flag);
	}
}
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: set/reset halfdelay
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
halfdelay (uint_fast8_t tenths)
{
	mcurses_phyio_halfdelay (tenths);
}
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: read key
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define MAX_KEYS                ((KEY_F1 + 12) - 0x80)
static const char * function_keys[MAX_KEYS] = {
	"B",                        // KEY_DOWN                 0x80                // Down arrow key
	"A",                        // KEY_UP                   0x81                // Up arrow key
	"D",                        // KEY_LEFT                 0x82                // Left arrow key
	"C",                        // KEY_RIGHT                0x83                // Right arrow key
	"1~",                       // KEY_HOME                 0x84                // Home key
	"3~",                       // KEY_DC                   0x85                // Delete character key
	"2~",                       // KEY_IC                   0x86                // Ins char/toggle ins mode key
	"6~",                       // KEY_NPAGE                0x87                // Next-page key
	"5~",                       // KEY_PPAGE                0x88                // Previous-page key
	"4~",                       // KEY_END                  0x89                // End key
	"Z",                        // KEY_BTAB                 0x8A                // Back tab key
#if 0 // VT400:
	"11~",                      // KEY_F(1)                 0x8B                // Function key F1
	"12~",                      // KEY_F(2)                 0x8C                // Function key F2
	"13~",                      // KEY_F(3)                 0x8D                // Function key F3
	"14~",                      // KEY_F(4)                 0x8E                // Function key F4
	"15~",                      // KEY_F(5)                 0x8F                // Function key F5
#else // Linux console
	"[A",                       // KEY_F(1)                 0x8B                // Function key F1
	"[B",                       // KEY_F(2)                 0x8C                // Function key F2
	"[C",                       // KEY_F(3)                 0x8D                // Function key F3
	"[D",                       // KEY_F(4)                 0x8E                // Function key F4
	"[E",                       // KEY_F(5)                 0x8F                // Function key F5
#endif
	"17~",                      // KEY_F(6)                 0x90                // Function key F6
	"18~",                      // KEY_F(7)                 0x91                // Function key F7
	"19~",                      // KEY_F(8)                 0x92                // Function key F8
	"20~",                      // KEY_F(9)                 0x93                // Function key F9
	"21~",                      // KEY_F(10)                0x94                // Function key F10
	"23~",                      // KEY_F(11)                0x95                // Function key F11
	"24~"                       // KEY_F(12)                0x96                // Function key F12
};
uint_fast8_t
getch (void)
{
	char    buf[4];
	uint_fast8_t ch;
	uint_fast8_t idx;
	refresh ();
	ch = mcurses_phyio_getc ();
	if (ch == 0x7F) {                                                           // BACKSPACE on VT200 sends DEL char
		ch = KEY_BACKSPACE;                                                     // map it to '\b'
	} else if (ch == '\033') {                                                  // ESCAPE
		while ((ch = mcurses_phyio_getc ()) == ERR) {
			;
		}
		if (ch == '\033') {                                                     // 2 x ESCAPE
			return KEY_ESCAPE;
		} else if (ch == '[') {
			for (idx = 0; idx < 3; idx++) {
				while ((ch = mcurses_phyio_getc ()) == ERR) {
					;
				}
				buf[idx] = ch;
				if ((ch >= 'A' && ch <= 'Z') || ch == '~') {
					idx++;
					break;
				}
			}
			buf[idx] = '\0';
			for (idx = 0; idx < MAX_KEYS; idx++) {
				if (! strcmp (buf, function_keys[idx])) {
					ch = idx + 0x80;
					break;
				}
			}
			if (idx == MAX_KEYS) {
				ch = ERR;
			}
		} else {
			ch = ERR;
		}
	}
	return ch;
}
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: read string (with mini editor built-in)
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
getnstr (char * str, uint_fast8_t maxlen)
{
	uint_fast8_t ch;
	uint_fast8_t curlen = 0;
	uint_fast8_t curpos = 0;
	uint_fast8_t starty;
	uint_fast8_t startx;
	uint_fast8_t i;
	maxlen--;                               // reserve one byte in order to store '\0' in last position
	getyx (starty, startx);                 // get current cursor position
	while ((ch = getch ()) != KEY_CR) {
		switch (ch) {
		case KEY_LEFT:
			if (curpos > 0) {
				curpos--;
			}
			break;
		case KEY_RIGHT:
			if (curpos < curlen) {
				curpos++;
			}
			break;
		case KEY_HOME:
			curpos = 0;
			break;
		case KEY_END:
			curpos = curlen;
			break;
		case KEY_BACKSPACE:
			if (curpos > 0) {
				curpos--;
				curlen--;
				move (starty, startx + curpos);
				for (i = curpos; i < curlen; i++) {
					str[i] = str[i + 1];
				}
				str[i] = '\0';
				delch();
			}
			break;
		case KEY_DC:
			if (curlen > 0) {
				curlen--;
				for (i = curpos; i < curlen; i++) {
					str[i] = str[i + 1];
				}
				str[i] = '\0';
				delch();
			}
			break;
		default:
			if (curlen < maxlen && (ch & 0x7F) >= 32 && (ch & 0x7F) < 127) {    // printable ascii 7bit or printable 8bit ISO8859
				for (i = curlen; i > curpos; i--) {
					str[i] = str[i - 1];
				}
				insch (ch);
				str[curpos] = ch;
				curpos++;
				curlen++;
			}
		}
		move (starty, startx + curpos);
	}
	str[curlen] = '\0';
}
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: endwin
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
endwin (void)
{
	move (LINES - 1, 0);                                                        // move cursor to last line
	clrtoeol ();                                                                // clear this line
	mcurses_putc ('\017');                                                      // switch to G0 set
	curs_set (TRUE);                                                            // show cursor
	mcurses_puts_P(SEQ_REPLACE_MODE);                                           // reset insert mode
	refresh ();                                                                 // flush output
	mcurses_phyio_done ();                                                      // end of physical I/O
	mcurses_is_up = 0;
}
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * mcurses-config.h - configuration file for mcurses lib
 *
 * Copyright (c) 2011-2015 Frank Meyer - frank(at)fli4l.de
 *
  * Revision History:
 * V1.0 2015 xx xx Frank Meyer, original version
 * V1.1 2017 01 13 ChrisMicro , serial interface specific functions removed
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define MCURSES_LINES               24              // 24 lines
#define MCURSES_COLS                80              // 80 columns
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * mcurses.h - include file for mcurses lib
 *
 * Copyright (c) 2011-2015 Frank Meyer - frank(at)fli4l.de
 *
 * Revision History:
 * V1.0 2015 xx xx Frank Meyer, original version
 * V1.1 2017 01 13 ChrisMicro, addepted as Arduino library, MCU specific functions removed
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#ifdef __cplusplus
extern "C"
{
#endif
#ifndef __MCURSES__
#define __MCURSES__

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * some constants
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define LINES                   MCURSES_LINES
#define COLS                    MCURSES_COLS
#ifndef TRUE
#define TRUE                    (1)                                                 // true
#define FALSE                   (0)                                                 // false
#endif
#define OK                      (0)                                                 // yet not used
#define ERR                     (255)                                               // yet not used
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * attributes, may be ORed
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define A_NORMAL                0x0000                                              // normal
#define A_UNDERLINE             0x0001                                              // underline
#define A_REVERSE               0x0002                                              // reverse
#define A_BLINK                 0x0004                                              // blink
#define A_BOLD                  0x0008                                              // bold
#define A_DIM                   0x0010                                              // dim
#define A_STANDOUT              A_BOLD                                              // standout (same as bold)
#define F_BLACK                 0x0100                                              // foreground black
#define F_RED                   0x0200                                              // foreground red
#define F_GREEN                 0x0300                                              // foreground green
#define F_BROWN                 0x0400                                              // foreground brown
#define F_BLUE                  0x0500                                              // foreground blue
#define F_MAGENTA               0x0600                                              // foreground magenta
#define F_CYAN                  0x0700                                              // foreground cyan
#define F_WHITE                 0x0800                                              // foreground white
#define F_YELLOW                F_BROWN                                             // some terminals show brown as yellow (with A_BOLD)
#define F_COLOR                 0x0F00                                              // foreground mask
#define B_BLACK                 0x1000                                              // background black
#define B_RED                   0x2000                                              // background red
#define B_GREEN                 0x3000                                              // background green
#define B_BROWN                 0x4000                                              // background brown
#define B_BLUE                  0x5000                                              // background blue
#define B_MAGENTA               0x6000                                              // background magenta
#define B_CYAN                  0x7000                                              // background cyan
#define B_WHITE                 0x8000                                              // background white
#define B_YELLOW                B_BROWN                                             // some terminals show brown as yellow (with A_BOLD)
#define B_COLOR                 0xF000                                              // background mask
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * mcurses variables
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
extern uint_fast8_t             mcurses_is_up;                                      // flag: mcurses is up
extern uint_fast8_t             mcurses_cury;                                       // do not use, use getyx() instead!
extern uint_fast8_t             mcurses_curx;                                       // do not use, use getyx() instead!
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * mcurses functions
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void                     setFunction_putchar(void (*functionPoitner)(uint8_t ch));
void                     setFunction_getchar(char (*functionPoitner)(void));
uint_fast8_t             initscr (void);                                     // initialize mcurses
void                     move (uint_fast8_t, uint_fast8_t);                  // move cursor to line, column (home = 0, 0)
void                     attrset (uint_fast16_t);                            // set attribute(s)
void                     addch (uint_fast8_t);                               // add a character
void                     addstr (const char *);                              // add a string
void                     addstr_P (const char *);                            // add a string (PROGMEM)
void                     getnstr (char * str, uint_fast8_t maxlen);          // read a string (with mini editor functionality)
void                     setscrreg (uint_fast8_t, uint_fast8_t);             // set scrolling region
void                     deleteln (void);                                    // delete line at current line position
void                     insertln (void);                                    // insert a line at current line position
void                     scroll (void);                                      // scroll line up
void                     clear (void);                                       // clear total screen
void                     clrtobot (void);                                    // clear screen from current line to bottom
void                     clrtoeol (void);                                    // clear from current column to end of line
void                     delch (void);                                       // delete character at current position
void                     insch (uint_fast8_t);                               // insert character at current position
void                     nodelay (uint_fast8_t);                             // set/reset nodelay
void                     halfdelay (uint_fast8_t);                           // set/reset halfdelay
uint_fast8_t             getch (void);                                       // read key
void                     curs_set(uint_fast8_t);                             // set cursor to: 0=invisible 1=normal 2=very visible
void                     refresh (void);                                     // flush output
void                     endwin (void);                                      // end mcurses
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * mcurses macros
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define erase()                 clear()                                             // clear total screen, same as clear()
#define mvaddch(y,x,c)          move((y),(x)), addch((c))                           // move cursor, then add character
#define mvaddstr(y,x,s)         move((y),(x)), addstr((s))                          // move cursor, then add string
#define mvaddstr_P(y,x,s)       move((y),(x)), addstr_P((s))                        // move cursor, then add string (PROGMEM)
#define mvinsch(y,x,c)          move((y),(x)), insch((c))                           // move cursor, then insert character
#define mvdelch(y,x)            move((y),(x)), delch()                              // move cursor, then delete character
#define mvgetnstr(y,x,s,n)      move((y),(x)), getnstr(s,n)                         // move cursor, then get string
#define getyx(y,x)              y = mcurses_cury, x = mcurses_curx                  // get cursor coordinates
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * mcurses keys
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define KEY_TAB                 '\t'                                                // TAB key
#define KEY_CR                  '\r'                                                // RETURN key
#define KEY_BACKSPACE           '\b'                                                // Backspace key
#define KEY_ESCAPE              0x1B                                                // ESCAPE (pressed twice)
#define KEY_DOWN                0x80                                                // Down arrow key
#define KEY_UP                  0x81                                                // Up arrow key
#define KEY_LEFT                0x82                                                // Left arrow key
#define KEY_RIGHT               0x83                                                // Right arrow key
#define KEY_HOME                0x84                                                // Home key
#define KEY_DC                  0x85                                                // Delete character key
#define KEY_IC                  0x86                                                // Ins char/toggle ins mode key
#define KEY_NPAGE               0x87                                                // Next-page key
#define KEY_PPAGE               0x88                                                // Previous-page key
#define KEY_END                 0x89                                                // End key
#define KEY_BTAB                0x8A                                                // Back tab key
#define KEY_F1                  0x8B                                                // Function key F1
#define KEY_F(n)                (KEY_F1+(n)-1)                                      // Space for additional 12 function keys
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * graphics: draw boxes
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define ACS_LRCORNER            0x8a                                                // DEC graphic 0x6a: lower right corner
#define ACS_URCORNER            0x8b                                                // DEC graphic 0x6b: upper right corner
#define ACS_ULCORNER            0x8c                                                // DEC graphic 0x6c: upper left corner
#define ACS_LLCORNER            0x8d                                                // DEC graphic 0x6d: lower left corner
#define ACS_PLUS                0x8e                                                // DEC graphic 0x6e: crossing lines
#define ACS_HLINE               0x91                                                // DEC graphic 0x71: horizontal line
#define ACS_LTEE                0x94                                                // DEC graphic 0x74: left tee
#define ACS_RTEE                0x95                                                // DEC graphic 0x75: right tee
#define ACS_BTEE                0x96                                                // DEC graphic 0x76: bottom tee
#define ACS_TTEE                0x97                                                // DEC graphic 0x77: top tee
#define ACS_VLINE               0x98                                                // DEC graphic 0x78: vertical line
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * graphics: other symbols
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define ACS_DIAMOND             0x80                                                // DEC graphic 0x60: diamond
#define ACS_CKBOARD             0x81                                                // DEC graphic 0x61: checker board
#define ACS_DEGREE              0x86                                                // DEC graphic 0x66: degree symbol
#define ACS_PLMINUS             0x87                                                // DEC graphic 0x66: plus/minus
#define ACS_S1                  0x8f                                                // DEC graphic 0x6f: scan line 1
#define ACS_S3                  0x90                                                // DEC graphic 0x70: scan line 3
#define ACS_S5                  0x91                                                // DEC graphic 0x71: scan line 5
#define ACS_S7                  0x92                                                // DEC graphic 0x72: scan line 7
#define ACS_S9                  0x93                                                // DEC graphic 0x73: scan line 9
#define ACS_LEQUAL              0x99                                                // DEC graphic 0x79: less/equal
#define ACS_GEQUAL              0x9a                                                // DEC graphic 0x7a: greater/equal
#define ACS_PI                  0x9b                                                // DEC graphic 0x7b: Pi
#define ACS_NEQUAL              0x9c                                                // DEC graphic 0x7c: not equal
#define ACS_STERLING            0x9d                                                // DEC graphic 0x7d: uk pound sign
#define ACS_BULLET              0x9e                                                // DEC graphic 0x7e: bullet

#endif // __MCURSES__
#ifdef __cplusplus
}
#endif
