/*
 * ncurses.c
 *
 *  Created on: Nov 28, 2015
 *      Author: cody
 */
/*
 * #include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include "windowlist.h"
#include "cpuInfo.h"
#include "memInfo.h"
#include "common.h"
 * 
 * 
 */

#include <ncurses.h>
#include "windowlist.h"

struct windowlist* ncurses_init()
{
	struct windowlist* wins = NULL;
	initscr();
	start_color();
	init_pair(1, COLOR_RED, COLOR_BLACK);
	init_pair(2, COLOR_YELLOW, COLOR_BLACK);
	init_pair(3, COLOR_BLUE, COLOR_BLACK);
	init_pair(4, COLOR_BLACK, COLOR_WHITE);
	raw();
	noecho();
	curs_set(0);
	keypad(stdscr,TRUE);
	refresh();
	
	// Add first window and setup internal WINDOW*s
	struct windowlist* focus = addWin(&wins);
	resizeWindowToFrame(focus);
	
	// Make the borders WINDOW*
	borders = newwin(LINES - 1, COLS, 0, 0);
	
	// Blue box for first border
	wattron(borders, COLOR_PAIR(3));
	box(borders, 0, 0);
	wattroff(borders, COLOR_PAIR(3));

	return wins;
}

WINDOW* addStatusLine()
{
	WINDOW* status = newwin(1, COLS, LINES - 1, 0);

	// Create status line
	move(LINES - 1, 0);
	wattron(status, COLOR_PAIR(4));
	for(int i = 0; i < COLS; i++)
		waddch(status, ' ');
	wattroff(status, COLOR_PAIR(4));
	
	return status;
}
