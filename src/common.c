/*
 * common.c
 *
 *  Created on: Jan 1, 2014
 *      Author: cody
 */

#include <ncurses.h>
#include "common.h"

/**
 * void showHelp()
 * Presents a window showing a help screen.
 */
void showHelp()
{
	int c;
	// Make frames.
	WINDOW* bwin = newwin(20, 70, LINES / 2 - 10, COLS / 2 - 35);
	WINDOW* hwin = newwin(18, 68, LINES / 2 - 10 + 1, COLS / 2 - 35 + 1);
	
	box(bwin, 0, 0); // Print border.
	mvwprintw(hwin, 0, 0, 	"%s v%s\n"
							"Lots of keys do things:\n"
							"  ?            This help.\n"
							"  h            Split current window horizontally.\n"
							"  v            Split current window vertically.\n"
							"  c            Close current window.\n"
							"  Tab          Move to next window in order of creation.\n"
							"  Arrow Keys   Move to next window on screen in direction pressed.\n"
							"  Numbers 0-9  Set data source to CPU #. '0' means give a summary.\n"
							"  m            Set data source to Ram.\n"
							"  s            Set data source to Swap.\n"
							"  g            Toggle grid for selected window.\n"
							"  e            Toggle value display in current window's title.\n"
							"  t            Toggle display of current window's title bar.\n"
							"  l            Toggle display of current window's label sidebar.\n"
							"  q            Quit this program.\n"
							"\n"
							"Press Enter to dismiss this help.",
							AppName, AppVers);
	
	wrefresh(bwin); // Actually write to screen.
	wrefresh(hwin);
	
	while((c = getch()))
	{ // Wait for enter.
		if(c == '\n')
			break;
	}
	
	delwin(hwin);
	delwin(bwin);
}

/**
 * char* getDirectionString(int num)
 * 	num is a number that represents an offset from the surrounding member of a windowlist struct, in sizeof(struct windowlist*) multiples
 * Helps polish the UI.
 */
char* getDirectionString(int num)
{
	switch(num)
	{
		case 0:
			return "Left";
		case 1:
			return "Right";
		case 2:
			return "Up";
		case 3:
			return "Down";
		default:
			return "";
	}
}

/**
 * int strchrCount(char* s, char c)
 * 	s is a string.
 * 	c is a character to look for.
 * returns number of occurrences of c in s.
 */
int strchrCount(char* s, char c)
{
	int i;
	for (i = 0; s[i]; (s[i] == c) ? (void) i++ : (void) s++); // always increment s or i.  Casts to void to avoid warnings.
	return i;
}

/**
 * void listShiftLeftAdd(float* list, int len, float new)
 * 	list is the list to be shifted.
 * 	len is the length of the list.
 * 	new is a new value to be added.
 * shifts list towards beginning and adds new at end.
 */
void listShiftLeftAdd(float* list, int len, float new)
{
	for(int i = 0; i < len-1; i++)
	{
		list[i] = list[i+1]; // shift
	}
	list[len-1] = new; // add
}

/**
 * void listShiftRightAdd(float* list, int len, float new)
 * 	list is the list to be shifted.
 * 	len is the length of the list.
 * 	new is a new value to be added.
 * shifts list towards end and adds new at beginning.
 */
void listShiftRightAdd(float* list, int len, float new)
{
	for(int i = len - 1; i > 0; i--)
	{
		list[i] = list[i-1]; // shift
	}
	list[0] = new; // add
}
