/*
 * common.c
 *
 *  Created on: Jan 1, 2014
 *      Author: cody
 */

#include <ncurses.h>
#include "common.h"

void showHelp()
{
	int c;
	WINDOW* bwin = newwin(18, 70, LINES / 2 - 9, COLS / 2 - 35);
	WINDOW* hwin = newwin(16, 68, LINES / 2 - 9 + 1, COLS / 2 - 35 + 1);
	
	box(bwin, 0, 0);
	mvwprintw(hwin, 0, 0, 	"%s v%s\n"
							"Lots of keys do things:\n"
							"  ?            This help.\n"
							"  h            Split current window horizontally.\n"
							"  v            Split current window vertically.\n"
							"  c            Close current window.\n"
							"  Tab          Move to next window in order of creation.\n"
							"  Arrow Keys   Move to next window on screen in direction pressed.\n"
							"  Numbers 0-9  Set data source to CPU #. '0' means give a summary.\n"
							"  g            Toggle grid for selected window.\n"
							"  e            Toggle value display in current window's title.\n"
							"  t            Toggle display of current window's title bar.\n"
							"  l            Toggle display of current window's label sidebar.\n"
							"  q            Quit this program.\n"
							"\n"
							"Press Enter to dismiss this help.",
							AppName, AppVers);
	
	wrefresh(bwin);
	wrefresh(hwin);
	
	while((c = getch()))
	{
		if(c == 10)
			break;
	}
	
	delwin(hwin);
	delwin(bwin);
}

int strchrCount(char* s, char c)
{
	int i;
	for (i = 0;
			s[i];
			s[i]==c ? i++ : (long) s++);
	return i;
}


void listShiftLeftAdd(float* list, int len, float new)
{
	for(int i = 0; i < len-1; i++)
	{
		list[i] = list[i+1];
	}
	list[len-1] = new;
}

void listShiftRightAdd(float* list, int len, float new)
{
	for(int i = len - 1; i > 0; i--)
	{
		list[i] = list[i-1];
	}
	list[0] = new;
}
