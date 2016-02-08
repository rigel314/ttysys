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
#include "ncurses-help.h"
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
	struct windowlist* focus = NULL;
	WINDOW* status = NULL;
	int argLen = 0;
	int argCtr = 0;
	
	// creating structs
	CPUthen = malloc(sizeof(struct cpuTime) * (numCPUs+1));
	CPUnow = malloc(sizeof(struct cpuTime) * (numCPUs+1));
	cpu = malloc(sizeof(struct cpuPercent) * (numCPUs+1));
	mem = malloc(sizeof(struct memPercent));
	
	// ncurses init stuff
	wins = ncurses_init();
	status = addStatusLine();
	focus = wins;
	
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
			case '~':
				
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
		
		if(argCtr <= argLen)
		{
			argCtr++;
			continue;
		}

		LLforeach(struct windowlist*, ptr, wins)
			drawScreen(ptr); // Draw each window's content.
		
		wrefresh(borders);
		wrefresh(status);
		refreshAll(wins, focus); // Draw each window's title and write it to the screen.
		
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
				{
					listShiftLeftAdd(ptr->data, ptr->dataLen, mem->ram);
					ptr->maxVal = mem->now.ramTotal;
				}
				else
				{
					listShiftLeftAdd(ptr->data, ptr->dataLen, mem->swap);
					ptr->maxVal = mem->now.swapTotal;
				}
			}
		}
	}
	
	// Only happens when the loop exits.
	endwin();
	return 0;
}
