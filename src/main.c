/*
 * main.c
 *
 *  Created on: Dec 26, 2013
 *      Author: cody
 *
 *	TODO:
 *		make the api cooler
 *			required function for shortdescription (fixed name)
 *		startup CLI args
 *		Fix titles
 *			length
 *			per chart type format
 *		Make command entry cooler
 *			show errors on plugin load
 *			support escaping special chars
 *		New help window.
 *			show help in voiddata
 *			display list of found plugins with shortdescription
 *		Do something about close window prompt
 *			maybe use the command line, emacs-style?
 *		Cleanup plugin window when changing plugin and new plugin fails init.
 *			maybe set back to void data?
 *		
 *		Next version:
 *			default startup file
 *			Corners in the border
 *			Resizeability
 *			make the api cooler
 *				add elapsed time api call
 */

#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <errno.h>
#include <stdbool.h>
#include <dlfcn.h>
#include <sys/time.h>
#include "windowlist.h"
#include "ncurses-help.h"
#include "common.h"
#include "signals.h"

// Global variable for WINDOW* to draw borders on.
WINDOW* borders = NULL;
WINDOW* status = NULL;
struct windowlist* focus = NULL;
struct windowlist* plgWin;

int main(int argc, char** argv)
{
	int c;
	struct windowlist* wins = NULL;
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
	drawScreen(wins);
	refreshAll(wins, focus); // Draw each window's title and write it to the screen.
	
	// timer
	registerHandlers();
	createTimer(TIMER_FREQ);

	while(1)
	{
		// When a timer event occurs, update windows that are supposed to be updated.
		if(got_sigitimer)
		{ // Set in signal handler for sigItimer
			got_sigitimer = false;
			bool shouldRefresh = false;
			
			// loop through each window and call nextValFunc if it's timer has expired.
			LLforeach(struct windowlist*, ptr, wins)
			{
				plgWin = ptr;
				if(ptr->refreshPrd && ptr->plgHandle && !(itimerCount % ptr->refreshPrd))
				{ // itimerCount % ptr->refreshPrd is 0 when this window should be refreshed
					shouldRefresh = true;
					nextValueFunc* funcptr = ptr->plgData.nextValue;
					if(funcptr)
					{
						float out[2] = {0};
						#ifdef DEBUG
							struct timeval tv;
							gettimeofday(&tv, NULL);
							double thisfreq = 1/((tv.tv_sec+(double)tv.tv_usec/1000000.0) - (ptr->lasttime.tv_sec+(double)ptr->lasttime.tv_usec/1000000.0));
							ptr->freq = .0025*ptr->filtdata[0] + .005*ptr->filtdata[1] + .0025*thisfreq - .81*ptr->filtdata[2] + 1.8*ptr->filtdata[3];
							ptr->filtdata[0] = ptr->filtdata[1];
							ptr->filtdata[1] = thisfreq;
							ptr->filtdata[2] = ptr->filtdata[3];
							ptr->filtdata[3] = ptr->freq;
							ptr->lasttime = tv;
						#endif
						funcptr(&(ptr->plgContext),out); // Call the nextValue function
						listShiftLeftAdd(ptr->data[0], ptr->dataLen, out[0]);
						listShiftLeftAdd(ptr->data[1], ptr->dataLen, out[1]);
						ptr->validDataLen = (ptr->validDataLen < ptr->dataLen) ? ptr->validDataLen+1 : ptr->dataLen;
					}
				}
			}
			itimerCount++;
			
			if(shouldRefresh)
			{ // only set if a window has been marked for update.
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
			if((c >= ' ' && c <= '~') && cmdLen < sizeof(cmdStr)-1)
			{ // Printable ascii.
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
				int timePrd;
				char name[sizeof(cmdStr)] = "";
				char args[sizeof(cmdStr)] = "";
				int ret;
				
				// TODO: support embedded parens
				ret = sscanf(cmdStr, "%d %[^( ](%[^)])", &timePrd, (char*)&name, (char*)&args);
				
				if(ret > 1)
				{
					char dlName[sizeof(cmdStr)+3];
					strcpy(dlName, name);
					strcat(dlName, ".so");
					
					plgWin = focus;
					
					if(focus->plgHandle)
						cleanupPlugin(focus);
					focus->plgHandle = dlopen(dlName, RTLD_LAZY);
					if(focus->plgHandle)
					{
						focus->refreshPrd = timePrd;
						if(initializePlugin(focus, args))
						{
							cleanupPlugin(focus);
						}
					}
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
					resizeWindowToFrame(focus, true);
					break;
				case 'o':
					focus->flags ^= wf_Label; // Toggle ordinate label
					resizeWindowToFrame(focus, true);
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
