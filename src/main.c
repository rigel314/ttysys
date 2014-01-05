/*
 * main.c
 *
 *  Created on: Dec 26, 2013
 *      Author: cody
 *
 *	TODO:
 *		Comment this code.
 *		New input method. + New help window.
 *		Corners in the border.
 */

#include <ncurses.h>
#include <stdlib.h>
#include "windowlist.h"
#include "cpuInfo.h"
#include "memInfo.h"
#include "common.h"

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
	
	while((c = getch()))
	{
		if((c | ASCIIshiftBit) == 'q')
			break;
		
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
				focus->flags ^= wf_Grid;
				break;
			case 'e':
				focus->flags ^= wf_ExpandedTitle;
				break;
			case 't':
				focus->flags ^= wf_Title;
				resizeWindowToFrame(focus);
				break;
			case 'l':
				focus->flags ^= wf_Label;
				resizeWindowToFrame(focus);
				break;
			
			case 'm':
				focus->dataType = MemData;
				focus->dataSource = 0;
				break;
			
			case 's':
				focus->dataType = MemData;
				focus->dataSource = 1;
				break;
			
			case 'h':
				splitH(focus);
				remapArrows(wins, wins);
				break;
			case 'v':
				splitV(focus);
				remapArrows(wins, wins);
				break;
			case 'c':
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
		if(c >= '0' && c <= '9')
		{
			if(c - '0' < numCPUs + 1)
			{
				focus->dataType = CPUData;
				focus->dataSource = c - '0';
			}
		}
		
		LLforeach(struct windowlist*, ptr, wins)
			drawScreen(ptr);

//		refresh();
		wrefresh(borders);
		wrefresh(status);
		refreshAll(wins, focus);
		
		getMemInfo(mem);
		if(getCPUtime(cpu, numCPUs, CPUthen, CPUnow) == 1)
			continue;
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
	
	endwin();
	return 0;
}
