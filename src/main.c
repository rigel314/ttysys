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
WINDOW* borders = NULL;
WINDOW* status = NULL;

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
	bool commandEntry = 0;
	bool leavingCmdEntry = 0;
	char cmdStr[100] = {0};
	int cmdLen = 0;
	int itimerCount = 0;

	// ncurses init stuff
	wins = ncurses_init();
	status = addStatusLine();
	focus = wins;

	// Display first window
	refreshAll(wins, focus); // Draw each window's title and write it to the screen.
	
	// timer
	registerHandlers();
	createTimer(TIMER_FREQ);

	while(1)
	{
		if(got_sigitimer)
		{
			case '?':
				showHelp();
				LLforeach(struct windowlist*, ptr, wins)
				{
					touchwin(ptr->titlewin);
					touchwin(ptr->labelwin);
					touchwin(ptr->contentwin);
				}
			}
			itimerCount++;
			
			if(shouldRefresh)
			{
				LLforeach(struct windowlist*, ptr, wins)
				{
					drawScreen(ptr); // Draw each window's content.
				}
				refreshAll(wins, focus);
			}
		}
		
		// Wait for available input on stdin
		fd_set rfds;
		FD_ZERO(&rfds);
		FD_SET(0,&rfds);
		if(select(1, &rfds, NULL, NULL, NULL) == -1)
		{
			if(errno == EINTR)
			{ // continue if interrupted by signal
				continue;
			}
			else
			{ // Die if some strange error happened.
				// What happened?
				break;
			}
		}
		
		// continue if there isn't any available input
		if(!FD_ISSET(0, &rfds))
			continue;
		
		c = getch();
		
		if(commandEntry && c > 0)
		{
			wattron(status, COLOR_PAIR(4));
			if((((c|ASCIIshiftBit) >= 'a' && (c|ASCIIshiftBit) <= 'z') || (c >= '0' && c <='9') || c == ' ') && cmdLen < sizeof(cmdStr)-1)
			{
				cmdStr[cmdLen++] = c;
				cmdStr[cmdLen] = '\0';
			}
			if((c == KEY_BACKSPACE || c == 0x08 || c == 0x7f) && cmdLen > 0)
			{ //                      BackSpace    Ascii Del
				cmdLen--;
				mvwaddch(status,0,cmdLen,' ');
				cmdStr[cmdLen] = 0;
			}
			if(c == 27 || c == 3 || c == 4)
			{ // ESC      CTRL-C    CTRL-D
				leavingCmdEntry = true;
			}
			if(c == 10 || c == 13)
			{ // '\n'       '\r'
				leavingCmdEntry = true;

				// split input
				char cmdCpy[sizeof(cmdStr)];
				strncpy(cmdCpy, cmdStr, cmdLen);
				cmdCpy[cmdLen] = '\0';
				
				char* space = strchr(cmdCpy, ' ');
				if(space)
				{
					listShiftLeftAdd(ptr->data, ptr->dataLen, mem->swap);
					ptr->maxVal = mem->now.swapTotal;
				}
			}
			
			if(leavingCmdEntry)
			{
				leavingCmdEntry = false;
				memset(cmdStr,' ', sizeof(cmdStr));
				cmdStr[sizeof(cmdStr)-1] = '\0';
				cmdLen = 0;
				commandEntry = 0; // go back to regular mode
				curs_set(0);
			}
			
			mvwaddstr(status,0,0,cmdStr);
			wattroff(status, COLOR_PAIR(4));
		}
		else
		{
			switch(c)
			{
				case '~':
					// clear cmdStr, reset len
					commandEntry = 1;
					curs_set(1);
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
			if((c | ASCIIshiftBit) == 'q') // Q or q can quit
				break; // Do this outside the switch so the break will actually break.
		}

		LLforeach(struct windowlist*, ptr, wins)
		{
			drawScreen(ptr); // Draw each window's content.
		}
		
		refreshAll(wins, focus);
		
		if(commandEntry)
		{
			wmove(status, 0, cmdLen);
			wrefresh(status);
		}
	}

	// Only happens when the loop exits.
	endwin();
	return 0;
}
