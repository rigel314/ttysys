/*
 * main.c
 *
 *  Created on: Dec 26, 2013
 *      Author: cody
 *
 *	TODO:
 *		Comment this code.
 *		Memory + Swap.
 *		Unsplit windows.
 *		New input method.
 *		Implement more window flags.
 *		More reasonable arrow keys.
 *		Corners in the border.
 *		Status bar.
 */

#include <ncurses.h>
#include <stdlib.h>
#include "windowlist.h"
#include "cpuInfo.h"

WINDOW* borders;

int main(int argc, char** argv)
{
	int c;
	struct cpuTime* then;
	struct cpuTime* now;
	struct cpuPercent* cpu;
	int numCPUs = getNumCPUs();
	struct windowlist* wins = NULL;
	struct windowlist* focus;
	struct windowlist* ptr;
	WINDOW* status;
	
	// creating structs
	then = malloc(sizeof(struct cpuTime) * (numCPUs+1));
	now = malloc(sizeof(struct cpuTime) * (numCPUs+1));
	cpu = malloc(sizeof(struct cpuPercent) * (numCPUs+1));
	
	// ncurses init stuff
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
	halfdelay(1);
	refresh();
	
	// Add first window and setup internal WINDOW*s
	focus = addWin(&wins);
	resizeWindowToFrame(focus);
	
	// Make the borders WINDOW*
	borders = newwin(LINES - 1, COLS, 0, 0);
	status = newwin(1, COLS, LINES - 1, 0);
	
	// Blue box for first border
	wattron(borders, COLOR_PAIR(3));
	box(borders, 0, 0);
	wattroff(borders, COLOR_PAIR(3));

	// Create status line
	move(LINES - 1, 0);
	wattron(status, COLOR_PAIR(4));
	for(int i = 0; i < COLS-12; i++)
		waddch(status, ' ');
	waddstr(status, "'?' for help");
	wattroff(status, COLOR_PAIR(4));
	
	while((c = getch()))
	{
		if(c == 10)
			break;
		
		switch (c)
		{
			case 'h':
				splitH(focus);
				remapArrows(wins, wins);
				break;
			case 'v':
				splitV(focus);
				remapArrows(wins, wins);
				break;
			case '\t':
				focus = focus->next;
				if(focus == NULL)
					focus = wins;
				break;
			case KEY_RIGHT:
				if(focus->surrounding.right != NULL)
					focus = focus->surrounding.right;
				break;
			case KEY_LEFT:
				if(focus->surrounding.left != NULL)
					focus = focus->surrounding.left;
				break;
			case KEY_UP:
				if(focus->surrounding.up != NULL)
					focus = focus->surrounding.up;
				break;
			case KEY_DOWN:
				if(focus->surrounding.down != NULL)
					focus = focus->surrounding.down;
				break;
		}
		if(c >= '0' && c <= '0'+numCPUs)
		{
			focus->dataSource = c - '0';
		}
		
		for(ptr = wins; ptr != NULL; ptr = ptr->next)
			drawScreen(ptr);

//		refresh();
		wrefresh(borders);
		wrefresh(status);
		refreshAll(wins, focus);
		
		if(getCPUtime(cpu, numCPUs, then, now) == 1)
			continue;
		for(ptr = wins; ptr != NULL; ptr = ptr->next)
			listShiftLeftAdd(ptr->data, ptr->dataLen, cpu[ptr->dataSource].total);
	}
	
	endwin();
	return 0;
}
