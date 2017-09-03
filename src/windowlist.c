/*
 * windowlist.c
 *
 *  Created on: Jan 1, 2014
 *      Author: cody
 */

#include <stdlib.h>
#include <math.h>
#include <ncurses.h>
#include <string.h>
#include <limits.h>
#include <float.h>
#include <dlfcn.h>
#include "windowlist.h"
#include "common.h"

/**
 * void drawScreen(struct windowlist* win)
 * 	win is a window to draw.
 * Updates win->contentwin to show the data in win->data.
 * Also draws the grid if wf_Grid is set.
 */
void drawScreen(struct windowlist* win)
{
	if(win->type == PercentChart)
	{
		int* indexes = malloc(sizeof(int) * win->dataLen);
		int height = getContentFrame(win, NULL).size.height;
		int width = win->dataLen;
		int axes[5];
		
		// Calculate the row for each grid line. 0, 25, 50, 75, 100
		for(int i = 0; i < 5; i++)
			axes[i] = roundf((float) height - (float) height * (25.0*i)/100.0);
		
		for(int i = 0; i < width; i++)
		{
			// Calculate the row for each data point.
			indexes[i] = roundf((float) height - (float) height * win->data[i]/100.0);
			
			// Print either a space or an ACS_PLUS in each row.
			// Only the differences will be actually be printed when the refresh() occurs.
			for(int j = 0; j < height; j++)
			{
				if(j >= indexes[i])
					mvwaddch(win->contentwin, j, i, ACS_PLUS);
				else
					mvwaddch(win->contentwin, j, i, ' ');
			}
			
			if(win->flags & wf_Grid)
			{
				for(int j = 1; j < 4; j++) // 1-4 instead of 0-5 because we don't care about 0% and 100%.
				{
					wattron(win->contentwin, COLOR_PAIR(1));
					if(axes[j] < indexes[i])
						mvwaddch(win->contentwin, axes[j], i, ACS_HLINE); // Draw a line.
					else
						mvwaddch(win->contentwin, axes[j], i, ACS_PLUS); // We still want to see a point if was on a grid line.
					wattroff(win->contentwin, COLOR_PAIR(1));
				}
			}
		}
		
		free(indexes);
	}
	else if(win->type == ScaledValueChart)
	{
		float dmin, dmax;
		
		minMaxList(win, &dmin, &dmax);
		
		if(dmin == dmax)
		{
			dmin--;
			dmax++;
		}
		
		resizeWindowToFrame(win, true);
		int* indexes = malloc(sizeof(int) * win->dataLen);
		int width = win->dataLen;
		int height = getContentFrame(win, NULL).size.height;
		
		for(int i = 0; i < width; i++)
		{
			// Calculate the row for each data point.
			indexes[i] = roundf((float) height - (float) height * (win->data[i]-dmin)/(dmax-dmin));
			
			// Print either a space or an ACS_PLUS in each row.
			// Only the differences will be actually be printed when the refresh() occurs.
			for(int j = 0; j < height; j++)
			{
				if(j >= indexes[i])
					mvwaddch(win->contentwin, j, i, ACS_PLUS);
				else
					mvwaddch(win->contentwin, j, i, ' ');
			}
			
			if(win->flags & wf_Grid)
			{
				;
			}
		}
		
		free(indexes);
	}
	
}

int minMaxList(struct windowlist* win, float* omin, float* omax)
{
	float dmin = FLT_MAX;
	float dmax = -FLT_MAX;
	
	for(int i=0; i < win->validDataLen; i++)
	{
		dmin = min(dmin, win->data[win->dataLen-i-1]);
		dmax = max(dmax, win->data[win->dataLen-i-1]);
	}
	
	if(win->validDataLen == 0)
	{
		dmin = 0;
		dmax = 0;
	}
	if(win->validDataLen == 1)
	{
		dmin = win->data[win->dataLen-1];
		dmax = win->data[win->dataLen-1];
	}

	*omin = dmin;
	*omax = dmax;
	
	return 0;
	
}


/**
 * void remapArrows(struct windowlist* wins, struct windowlist* win)
 * 	wins is the beginning of the linked list.
 * 	win is a specific window to be remaped.
 * Both arguments should be the same on the first call.
 * This function calls itself recursively to remap the keys for all windows.
 */
void remapArrows(struct windowlist* wins, struct windowlist* win)
{
	// Used for calculating top-most and left-most windows.  NULLs to make unchanged ones NULL, and INT_MAX to make it always lose the first test.
	struct bestPtr mins[4] = {{NULL, INT_MAX}, {NULL, INT_MAX}, {NULL, INT_MAX}, {NULL, INT_MAX}};
	
	// We've reached the end of the list.
	if(!win)
		return;

	// Running in n^2 time won't be too bad when n has a small max.  You'd need a HUGE monitor to get more than a 3 digit max.  And even then, you probably wouldn't notice the crappy running time.
	LLforeach(struct windowlist*, ptr, wins)
	{
		// Is ptr immediately to the left of win?  && Are the two frames "touching"?
		if(ptr->frame.origin.x + ptr->frame.size.width + 1 == win->frame.origin.x && ptr->frame.origin.y <= win->frame.origin.y+win->frame.size.height && ptr->frame.origin.y+ptr->frame.size.height >= win->frame.origin.y)
		{
			if(ptr->frame.origin.y < mins[0].diff)
			{
				mins[0].diff = ptr->frame.origin.y;
				mins[0].ptr = ptr;
			}
		}
		// Is ptr immediately to the right of win?  && Are the two frames "touching"?
		if(ptr->frame.origin.x == win->frame.origin.x + win->frame.size.width + 1 && ptr->frame.origin.y <= win->frame.origin.y+win->frame.size.height && ptr->frame.origin.y+ptr->frame.size.height >= win->frame.origin.y)
		{
			if(ptr->frame.origin.y < mins[1].diff)
			{
				mins[1].diff = ptr->frame.origin.y;
				mins[1].ptr = ptr;
			}
		}
		// Is ptr immediately above win?  && Are the two frames "touching"?
		if(ptr->frame.origin.y + ptr->frame.size.height + 1 == win->frame.origin.y && ptr->frame.origin.x <= win->frame.origin.x+win->frame.size.width && ptr->frame.origin.x+ptr->frame.size.width >= win->frame.origin.x)
		{
			if(ptr->frame.origin.x < mins[2].diff)
			{
				mins[2].diff = ptr->frame.origin.x;
				mins[2].ptr = ptr;
			}
		}
		// Is ptr immediately below win?  && Are the two frames "touching"?
		if(ptr->frame.origin.y == win->frame.origin.y + win->frame.size.height + 1 && ptr->frame.origin.x <= win->frame.origin.x+win->frame.size.width && ptr->frame.origin.x+ptr->frame.size.width >= win->frame.origin.x)
		{
			if(ptr->frame.origin.x < mins[3].diff)
			{
				mins[3].diff = ptr->frame.origin.x;
				mins[3].ptr = ptr;
			}
		}
	}

	// Set all of the arrowpointers.
	win->surrounding.left = mins[0].ptr;
	win->surrounding.right = mins[1].ptr;
	win->surrounding.up = mins[2].ptr;
	win->surrounding.down = mins[3].ptr;
	
	// remap the next window.
	remapArrows(wins, win->next);
}

/**
 * struct GRect getContentFrame(struct windowlist* win)
 * 	win is the window that you want the content frame of
 * uses win's frame and flags to determine what the frame of the contentwin should be.
 */
struct GRect getContentFrame(struct windowlist* win, struct GRect* labelFrame)
{
	struct GRect frame = win->frame;
	struct GRect label = win->frame;
	
	if(win->flags & wf_Title)
	{ // Title is always 1 row tall.
		frame.origin.y++;
		frame.size.height--;
		
		label.origin.y++;
		label.size.height--;
	}
	
	if(win->flags & wf_Label)
	{
		int size = 0;
		
		if(win->type == VoidChart)
			size = 3;
		
		if(win->type == PercentChart)
			size = 3; // Label is always 3 columns wide.
		
		if(win->type == ScaledValueChart)
		{
			float dmin, dmax;
			minMaxList(win, &dmin, &dmax);

			int iminLen = snprintf(NULL, 0, "%d", (int)dmin);
			int imaxLen = snprintf(NULL, 0, "%d", (int)dmax);
			int dminLen = snprintf(NULL, 0, "%1.5E", (double)dmin);
			int dmaxLen = snprintf(NULL, 0, "%1.5E", (double)dmax);
			
			if(iminLen <= 6 && imaxLen <= 6)
			{
				size = 6;
			}
			else
			{
				size = max(dminLen, dmaxLen);
				size = 13;
			}
		}
		frame.origin.x += size;
		frame.size.width -= size;
		
		label.size.width = size;
	}
	
	if(labelFrame)
		*labelFrame = label;
	
	return frame;
}

/**
 * void resizeWindowToFrame(struct windowlist* win)
 * 	win is the window that should be resized.
 * This resizes and moves all WINDOW*s to whatever win->frame and win->flags says it should be.
 * This also realloc()'s the data array for win.
 */
void resizeWindowToFrame(struct windowlist* win, bool clearContent)
{
	int diff;
	int newLen;
	struct GRect labelFrame;
	struct GRect contentFrame = getContentFrame(win, &labelFrame);
	
	// Resize.
	wresize(win->titlewin, 1, win->frame.size.width);
	wresize(win->labelwin, labelFrame.size.height, labelFrame.size.width);
	wresize(win->contentwin, contentFrame.size.height, contentFrame.size.width);
	// Move.
	mvwin(win->titlewin, win->frame.origin.y, win->frame.origin.x);
	mvwin(win->labelwin, labelFrame.origin.y, labelFrame.origin.x);
	mvwin(win->contentwin, contentFrame.origin.y, contentFrame.origin.x);

	// Clear so they won't leave garbage around.
	werase(win->titlewin);
	werase(win->labelwin);

	if(clearContent)
		werase(win->contentwin);
	
	newLen = contentFrame.size.width;
	diff = newLen - win->dataLen;
	
	// If data was never initialized, we want it to be initialized with all zeros.  Hence calloc() instead of realloc().
	if(win->dataLen == 0)
	{
		diff = 0;
		win->data = calloc(newLen, sizeof(float));
		win->validDataLen = 0;
	}
	
	// A negative diff means data got smaller.  realloc() truncates, so we need to shift all data by diff first.
	if(diff < 0)
	{
		for(int i = 0; i < abs(diff); i++)
			listShiftLeftAdd(win->data, win->dataLen, 0);
		win->validDataLen = max(0,win->validDataLen+diff);
	}
	
	win->dataLen = newLen;
	win->data = realloc(win->data, sizeof(float) * newLen);
	
	// A positive diff means data got bigger.  realloc() gave us more space, so we need to shift all the data to the end now.
	if(diff > 0)
		for(int i = 0; i < diff; i++)
			listShiftRightAdd(win->data, win->dataLen, 0);
	
	// Print 25%, 50%, and 75%.
	if(win->flags & wf_Label)
	{
		int height = getContentFrame(win, NULL).size.height;
		
		if(win->type == PercentChart)
		{
			for(int i = 1; i < 4; i++)
			{
				int row = roundf((float) height - (float) height * (25.0*i)/100.0);
				char str[4];
				
				sprintf(str, "%d%%", 25*i);
				
				mvwaddstr(win->labelwin, row, 0, str);
			}
		}
		
		if(win->type == ScaledValueChart)
		{
			float dmin, dmax;
			minMaxList(win, &dmin, &dmax);
			
			int iminLen = snprintf(NULL, 0, "%d", (int)dmin);
			int imaxLen = snprintf(NULL, 0, "%d", (int)dmax);
			
			if(iminLen <= 6 && imaxLen <= 6)
			{
				mvwprintw(win->labelwin, 0, 0, "%d", (int)dmax);
				mvwprintw(win->labelwin, height-1, 0, "%d", (int)dmin);
			}
			else
			{
				mvwprintw(win->labelwin, 0, 0, "%1.5E", (double)dmax);
				mvwprintw(win->labelwin, height-1, 0, "%1.5E", (double)dmin);
			}
		}
	}
}

/**
 * void splitV(struct windowlist* old)
 * 	old is the window to be split.
 * Divides window in half, vertically, as best it can.
 * Resizes all WINDOW*s and prints a new border line.
 */
void splitV(struct windowlist* old)
{
	struct windowlist* new;
	bool odd;
	
	if(old->frame.size.height < 10)
		return;

	new = addWin(&old);
	if(!new)
		return;
	
	new->frame = old->frame;
	
	// 1 if odd, 0 if even.
	odd = old->frame.size.height%2;
	
	if(odd) // Read: "if old height is odd"
	{
		old->frame.size.height = (old->frame.size.height - 1)/2;
		new->frame.size.height = old->frame.size.height;
	}
	else
	{
		old->frame.size.height = (old->frame.size.height)/2;
		new->frame.size.height = old->frame.size.height - 1;
	}
	
	// Plus 1 for the border.
	new->frame.origin.y = old->frame.origin.y + old->frame.size.height + 1;
	
	// Make sure each window is properly resized to its new frame.
	resizeWindowToFrame(old, true);
	resizeWindowToFrame(new, true);
	
	// Draw a line for the new border.
	printLine(borders, new->frame.origin.y - 1, new->frame.origin.x, HORIZ, new->frame.size.width);
}

/**
 * void splitH(struct windowlist* old)
 * 	old is the window to be split.
 * Divides window in half, vertically, as best it can.
 * Resizes all WINDOW*s and prints a new border line.
 */
void splitH(struct windowlist* old)
{
	struct windowlist* new;
	bool odd;

	if(old->frame.size.width < 10)
		return;
	
	new = addWin(&old);
	if(!new)
		return;
	
	new->frame = old->frame;
	
	// %2 is an even/odd check.
	odd = old->frame.size.width%2;
	
	if(odd) // Read: "if old width is odd"
	{
		old->frame.size.width = (old->frame.size.width - 1)/2;
		new->frame.size.width = old->frame.size.width;
	}
	else
	{
		old->frame.size.width = (old->frame.size.width)/2;
		new->frame.size.width = old->frame.size.width - 1;
	}
	
	new->frame.origin.x = old->frame.origin.x + old->frame.size.width + 1;
	
	resizeWindowToFrame(old, true);
	resizeWindowToFrame(new, true);
	
	printLine(borders, new->frame.origin.y, new->frame.origin.x - 1, VERT, new->frame.size.height);
}

/**
 * void unSplit(struct windowlist** wins, struct windowlist** win)
 * 	wins is a pointer to the beginning of the linked list.
 * 	win is a pointer to the window to be closed.
 * Closes a window and expands a nearby window to fill the void.
 * If more than one window might expand, this function blocks for user input.
 * 	Press the arrow key in the direction of the window that should expand.
 */
void unSplit(struct windowlist** wins, struct windowlist** win)
{
	struct windowlist** surrPtr;
	struct windowlist* dirs[4] = {NULL, NULL, NULL, NULL};
	int numDirs = 0;
	int choice = -1;
	
	// If someone called this badly, I don't want to deal with it.
	if(!wins || !*wins || !win || !*win)
		return;
	
	// surrPtr initially points into (*win)'s struct at it's surrounding.left pointer.
	// incrementing surrPtr makes it go through each surrounding window pointer.
	for(surrPtr = &(*win)->surrounding.left; surrPtr <= &(*win)->surrounding.down; surrPtr++)
	{
		struct windowlist* ptr = *surrPtr;
		
		if(ptr) // If *win shares a dimension and a starting coordinate with ptr.
			if( (ptr->frame.size.height == (*win)->frame.size.height && ptr->frame.origin.y == (*win)->frame.origin.y) ||
				(ptr->frame.size.width == (*win)->frame.size.width && ptr->frame.origin.x == (*win)->frame.origin.x))
			{
				// Yay! pointer subtraction. You don't divide by sizeof(void*) here. Bad things happen when you try.
				int i = (surrPtr - &(*win)->surrounding.left);
				dirs[i] = ptr; // save this ptr.
				numDirs++; // Increment number of available directions.
				choice = i; // set default choice to this one.
			}
	}
	
	// If we couldn't find a direction to unsplit on, return.
	if(numDirs == 0)
		return;

	// If there is more than one choice, display a window to make the user decide a direction.
	if(numDirs > 1)
	{
		int c;
		char str[(5+numDirs+2) * 62];
		
		// Make frames.
		WINDOW* bwin = newwin(5+numDirs+2, 64, LINES / 2 - (5+numDirs+2)/2, COLS / 2 - 32);
		WINDOW* hwin = newwin(5+numDirs, 62, LINES / 2 - (5+numDirs+2)/2 + 1, COLS / 2 - 32 + 1);
		
		strcpy(str, "Selected window will close. Which window should expand?\n"
					"\n"
					"Press an arrow key to indicate the window that should expand.\n");
		for(int i = 0; i < 4; i++)
			if(dirs[i])
				sprintf(str + strlen(str), "  %s: %s\n", getDirectionString(i), dirs[i]->title);
		strcat(str, "\n"
					"Press Enter to cancel.");
		
		box(bwin, 0, 0); // Draw a border frame.
		mvwaddstr(hwin, 0, 0,str);
		
		// refresh WINDOW*s.
		wrefresh(bwin);
		wrefresh(hwin);
		
		// force while to happen.
		choice = 4;
		
		while(choice > 3 || dirs[choice] == NULL)
		{
			c = getch();
			
			if(c == '\n')
			{ // Do this outside the switch to break the while.
				choice = -1;
				break;
			}
			
			switch (c)
			{
				case 'l':
				case KEY_LEFT:
					choice = 0;
					break;
				case 'r':
				case KEY_RIGHT:
					choice = 1;
					break;
				case 'u':
				case KEY_UP:
					choice = 2;
					break;
				case 'd':
				case KEY_DOWN:
					choice = 3;
					break;
			}
		}
		
		delwin(hwin);
		delwin(bwin);
	}
	
	// If user hits enter, return.
	if(choice == -1)
		return;
	
	// Horizontal splits are 0 and 1.  Vertical are 2 and 3.
	if(choice < 2)
		dirs[choice]->frame = GRect(min(dirs[choice]->frame.origin.x, (*win)->frame.origin.x),
									min(dirs[choice]->frame.origin.y, (*win)->frame.origin.y),
									dirs[choice]->frame.size.width + (*win)->frame.size.width + 1,
									dirs[choice]->frame.size.height);
	else
		dirs[choice]->frame = GRect(min(dirs[choice]->frame.origin.x, (*win)->frame.origin.x),
									min(dirs[choice]->frame.origin.y, (*win)->frame.origin.y),
									dirs[choice]->frame.size.width,
									dirs[choice]->frame.size.height + (*win)->frame.size.height + 1);
	
	resizeWindowToFrame(dirs[choice], true);
	freeWin(wins, *win);
	*win = dirs[choice]; // Change the pointed to struct windowlist*.
}

/**
 * void refreshAll(struct windowlist* wins, struct windowlist* focus)
 * 	wins is the beginning of the linked list.
 * 	focus is the address of the window that should get a special color in the title.
 * Write's titles and calls refresh() on each window.
 */
void refreshAll(struct windowlist* wins, struct windowlist* focus)
{
	wnoutrefresh(borders);
	wnoutrefresh(status);

	LLforeach(struct windowlist*, ptr, wins)
	{
		// Only do title related stuff if wf_Title is set.
		if(ptr->flags & wf_Title)
		{
			// Turn on color for the focused window.
			if(ptr == focus)
				wattron(ptr->titlewin, COLOR_PAIR(2));
			
			if(ptr->dataType == CPUData)
			{
				strcpy(ptr->title, "CPU ");
				if(ptr->dataSource == 0)
					strcat(ptr->title, "Summary"); // "CPU Summary"
				else
					sprintf(ptr->title + 4, "%d", ptr->dataSource); // "CPU 1"
			}
			else if(ptr->dataType == MemData)
			{
				if(ptr->dataSource == 0)
					strcpy(ptr->title, "RAM"); // "RAM"
				else
					strcpy(ptr->title, "Swap"); // "Swap"
			}
			else if(ptr->dataType == VoidData)
			{
				strcpy(ptr->title,"(VoidData)");
			}
			
			if((ptr->flags & wf_ExpandedTitle) && ptr->dataType != VoidData)
			{
				strcat(ptr->title, " - ");
				sprintf(ptr->title + strlen(ptr->title), "%.2f%%", ptr->data[ptr->dataLen-1]); // "CPU Summary - 17.26%"
				
				if((ptr->flags & wf_ShowMax) && ptr->dataType == MemData)
				{
					int expon=3;
					float x = ptr->maxVal;
					
					// Divide by multiples of 2^10 until we have a reasonable number.
					// If it's an unreasonable number of GB, this program will not display correctly.
					while(x / 1024.0 > 1 && expon < 9)
					{
						x /= 1024.0;
						expon += 3;
					}
					// I used an interpolation function to map 3 -> K, 6 -> M, and 9 -> G.
					snprintf(ptr->title + strlen(ptr->title), 39-strlen(ptr->title), " of %.2f %ciB", x, 'A' + (-4*expon*expon/9+14*expon/3));
					// "RAM - 17.26% of 3.86 GiB"
				}
				
				#ifdef DEBUG
					sprintf(ptr->title + strlen(ptr->title), "(%.2fHz)", ptr->freq);
				#endif
			}
			
			// Pad the rest of title with spaces to be written over anything that got shorter.
			for(int i = strlen(ptr->title); i < 39; i++)
				ptr->title[i] = ' ';
			ptr->title[39] = '\0'; // Null terminate title.
			
			// Actually print title. (not actually, just schedule it for the next refresh() call.)
			mvwaddstr(ptr->titlewin, 0, 3, ptr->title);
			
			// Turn off color if it was on.
			if(ptr == focus)
				wattroff(ptr->titlewin, COLOR_PAIR(2));
		}
		
		// Only refesh WINDOW*s if they are in use.
		if(ptr->flags & wf_Title)
			wnoutrefresh(ptr->titlewin);
		
		if(ptr->flags & wf_Label)
			wnoutrefresh(ptr->labelwin);
		
		wnoutrefresh(ptr->contentwin);
	}
	
	doupdate();
}

/**
 * void printLine(WINDOW* WIN, int row, int col, enum lineDir direction, int len)
 * 	WIN is a WINDOW* that needs a line printed.
 * 	(row, col) are the starting coordinates.
 * 	direction is HORIZ or VERT and tells the function which direction of line to draw.
 * 	len is the length of the line.
 * Wrapper for mvwhline() and mvwvline().
 * Intended for border only, so COLOR_PAIR(3) is used.
 */
void printLine(WINDOW* WIN, int row, int col, enum lineDir direction, int len)
{
	int i;
	
	wattron(WIN, COLOR_PAIR(3));
	switch (direction)
	{
		case HORIZ:
			for(i=0;i<len;i++)
				mvwhline(WIN, row,col+i,ACS_HLINE,1); // Draw line.
			break;
		case VERT:
			for(i=0;i<len;i++)
				mvwvline(WIN, row+i,col,ACS_VLINE,1); // Draw line.
			break;
		default:
			break;
	}
	wattroff(WIN, COLOR_PAIR(3));
}

/**
 * struct windowlist* addWin(struct windowlist** wins)
 * 	wins is a pointer to a part of the linked list.
 * if *wins is null, it is set to a newly created linked list with one element.
 * otherwise, addWin adds an element to the end of the list.
 * returns address of new window.
 */
struct windowlist* addWin(struct windowlist** wins)
{
	struct windowlist* ptr;
	struct windowlist* new;
	
	if(!wins)
		return NULL;
	
	new = malloc(sizeof(struct windowlist));
	if(!new)
		return NULL;
	
	// Default values.
	new->next = NULL;
	new->titlewin = newwin(0, 0, 0, 0);
	new->contentwin = newwin(0, 0, 0, 0);
	new->labelwin = newwin(0, 0, 0, 0);
	new->surrounding.left = NULL;
	new->surrounding.right = NULL;
	new->surrounding.up = NULL;
	new->surrounding.down = NULL;
	new->title[0] = 0;
	new->flags = wf_Title | wf_Label | wf_Grid | wf_ExpandedTitle | wf_ShowMax | wf_Border;
	new->type = VoidChart;
	new->frame = GRect(1, 1, COLS - 2, LINES - 3);
	new->dataType = VoidData;
	new->dataSource = 0;
	new->data = NULL;
	new->dataLen = 0;
	new->validDataLen = 0;
	new->maxVal = 0;
	new->refreshPrd = 0;
	new->plgHandle = NULL;
	
	if(!*wins)
	{
		*wins = new;
	}
	else
	{
		// Find place to add element.
		for(ptr = *wins; ptr->next != NULL; ptr = ptr->next);
		
		ptr->next = new;
	}
	
	return new;
}

int initializePlugin(struct windowlist* win, char* args)
{
	int argc = 0;
	char** argv;
	initFunc* initfunc;
	
	dlerror();
	initfunc = dlsym(win->plgHandle, "init");
	if(dlerror())
	{
		return 1;
	}

	int len = strlen(args);
	for(int i=0, flag=false; i<=len && len > 0; i++)
	{
		if(args[i] == '"' && !flag)
		{
			args[i] = '\0';
			flag = true;
		}
		else if(args[i] == '"' && flag)
		{
			args[i] = '\0';
			flag = false;
		}
		
		if(args[i] == ' ' && !flag)
		{
			args[i] = '\0';
			argc++;
		}
		
		if(args[i] == ',' && !flag)
		{
			args[i] = '\0';
			argc++;
		}
		if(args[i] == '\0')
		{
			argc++;
		}
	}
	
	argc++;
	
	argv=malloc(sizeof(char*)*argc);
	if(!argv)
		return 2;
	
	argv[0] = NULL;
	
	for(int i=0,flag=false,c=1,off=0; i<=len; i++)
	{
		if(args[i] == '\0' && flag)
		{
			argv[c++] = args+off;
			flag = false;
		}
		
		if(args[i] != '\0' && !flag)
		{
			flag = true;
			off = i;
		}
	}
	
	struct initData id = initfunc(&(win->plgContext), argc, argv);
	
	if(id.status == initStatus_Success)
	{
		win->type = id.type;
		win->dataType = UserData;
		resizeWindowToFrame(win, true);
	}

	free(argv);
	
	return id.status;
}

void cleanupPlugin(struct windowlist* win)
{
	if(win->plgHandle)
	{
		if(win->plgData.cleanUp)
			win->plgData.cleanUp(&(win->plgContext));
		dlclose(win->plgHandle);
		win->plgHandle = NULL;
	}
	win->plgContext = NULL;
}

/**
 * void freeWin(struct windowlist** wins, struct windowlist* win)
 * 	wins is the beginning of the linked list.
 * 	win is the window to be freed.
 * if win is not found, nothing happens.
 * otherwise, win's memory is freed and removed from the list.
 * if win is *wins, *wins is changed to win->next.
 */
void freeWin(struct windowlist** wins, struct windowlist* win)
{
	struct windowlist* ptr;
	
	if(!wins || !*wins)
		return;
	
	if(*wins == win)
	{
		*wins = win->next;
	}
	else
	{
		// Find window.
		for(ptr = *wins; ptr->next != win && ptr->next != NULL; ptr = ptr->next);
	
		if(!ptr->next)
			return;
		
		ptr->next = win->next;
	}
	
	// free WINDOW*s
	delwin(win->contentwin);
	delwin(win->labelwin);
	delwin(win->titlewin);
	// free struct members that matter.
	free(win->data);
	if(win->plgHandle)
		free(win->plgHandle);
	free(win);
}
