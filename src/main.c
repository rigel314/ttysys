/*
 * main.c
 *
 *  Created on: Dec 26, 2013
 *      Author: cody
 *
 *	TODO:
 *		New input method. + New help window.
 *		Corners in the border.
 *		Resizeability.
 */

#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include "windowlist.h"
#include "cpuInfo.h"
#include "memInfo.h"
#include "common.h"

// Global variable for WINDOW* to draw borders on.
WINDOW* borders;

int main(int argc, char** argv)
{
	int c;
	struct cpuTime* CPUthen;
	struct cpuTime* CPUnow;
	struct cpuPercent* cpu;
	struct memPercent* mem;
	int numCPUs = getNumCPUs();
	struct windowlist* wins = NULL;
	struct windowlist* focus;
	WINDOW* status;
	int argLen = 0;
	int argCtr = 0;
	
	// creating structs
	CPUthen = malloc(sizeof(struct cpuTime) * (numCPUs+1));
	CPUnow = malloc(sizeof(struct cpuTime) * (numCPUs+1));
	cpu = malloc(sizeof(struct cpuPercent) * (numCPUs+1));
	mem = malloc(sizeof(struct memPercent));
	
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
	
	if(argc == 2)
	{
		argLen = strlen(argv[1]);
		ungetch(argv[1][argCtr++]);
	}
	
	while((c = getch()))
	{
		if(argCtr < argLen)
			ungetch(argv[1][argCtr]);
			
		if((c | ASCIIshiftBit) == 'q') // Q or q can quit
			break; // Do this outside the switch so the break will actually break.
		
		switch (c)
		{
			case '?':
				showHelp();
				LLforeach(struct windowlist*, ptr, wins)
				{
					touchwin(ptr->titlewin);
					touchwin(ptr->labelwin);
					touchwin(ptr->contentwin);
				}
				touchwin(borders);
				touchwin(status);
				break;
			case 'g':
				focus->flags ^= wf_Grid; // Toggle grid
				break;
			case 'e':
				focus->flags ^= wf_ExpandedTitle; // Toggle Expanded title
				break;
			case 't':
				focus->flags ^= wf_Title; // Toggle title
				resizeWindowToFrame(focus);
				break;
			case 'o':
				focus->flags ^= wf_Label; // Toggle ordinate label
				resizeWindowToFrame(focus);
				break;
			
			case 'm':
				focus->dataType = MemData; // Set data source to memory
				focus->dataSource = 0;
				break;
			
			case 's':
				focus->dataType = MemData; // Set data source to swap space
				focus->dataSource = 1;
				break;
			
			case 'h': // horizontal split
				splitH(focus);
				remapArrows(wins, wins);
				break;
			case 'v': // vertical split
				splitV(focus);
				remapArrows(wins, wins);
				break;
			case 'c': // Close window
				unSplit(&wins, &focus);
				remapArrows(wins, wins);
				LLforeach(struct windowlist*, ptr, wins)
				{
					touchwin(ptr->titlewin);
					touchwin(ptr->labelwin);
					touchwin(ptr->contentwin);
				}
				touchwin(borders);
				touchwin(status);
				break;
			
			case '\t':
				focus = focus->next;
				if(focus == NULL)
					focus = wins;
				break;
			case 'r':
			case KEY_RIGHT:
				if(focus->surrounding.right != NULL)
					focus = focus->surrounding.right;
				break;
			case 'l':
			case KEY_LEFT:
				if(focus->surrounding.left != NULL)
					focus = focus->surrounding.left;
				break;
			case 'u':
			case KEY_UP:
				if(focus->surrounding.up != NULL)
					focus = focus->surrounding.up;
				break;
			case 'd':
			case KEY_DOWN:
				if(focus->surrounding.down != NULL)
					focus = focus->surrounding.down;
				break;
		}
		if(c >= '0' && c <= '9')
		{ // Do this outside the switch so I can specify a range without 10 cases.
			if(c - '0' < numCPUs + 1)
			{
				// Make sure the type is set to CPU
				focus->dataType = CPUData;
				focus->dataSource = c - '0';
			}
		}
		
		LLforeach(struct windowlist*, ptr, wins)
			drawScreen(ptr); // Draw each window's content.

		wrefresh(borders);
		wrefresh(status);
		refreshAll(wins, focus); // Draw each window's title and write it to the screen.
		
		if(argCtr <= argLen)
		{
			argCtr++;
			continue;
		}
		
		// Get all the system info.
		getMemInfo(mem);
		if(getCPUtime(cpu, numCPUs, CPUthen, CPUnow) == 1) // returning 1 means that a key was pressed.  Don't update any data when keys are pressed.
			continue;
		
		// Add a data point to each list from the appropriate source.
		LLforeach(struct windowlist*, ptr, wins)
		{
			if(ptr->dataType == CPUData)
			{
				listShiftLeftAdd(ptr->data, ptr->dataLen, cpu[ptr->dataSource].total);
			}
			else if(ptr->dataType == MemData)
			{
				if(ptr->dataSource == 0)
					listShiftLeftAdd(ptr->data, ptr->dataLen, mem->ram);
				else
					listShiftLeftAdd(ptr->data, ptr->dataLen, mem->swap);
			}
		}
	}
	
	// Only happens when the loop exits.
	endwin();
	return 0;
}
