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
#include "windowlist.h"

void drawScreen(struct windowlist* win)
{
	int* indexes = malloc(sizeof(int) * win->dataLen);
	int axes[5];
	int height = getContentFrame(win).size.height;
	int width = win->dataLen;
	
	for(int i = 0; i < 5; i++)
		axes[i] = roundf((float) height - (float) height * (25.0*i)/100.0) - 1;
	
	for(int i = 0; i < width; i++)
	{
		indexes[i] = roundf((float) height - (float) height * win->data[i]/100.0);
		
		for(int j = 0; j < height; j++)
		{
			if(j >= indexes[i])
				mvwaddch(win->contentwin, j, i, '*');
			else
				mvwaddch(win->contentwin, j, i, ' ');
		}
		
		if(win->flags & wf_Grid)
		{
			for(int j = 1; j < 4; j++)
			{
				wattron(win->contentwin, COLOR_PAIR(1));
				if(axes[j] != indexes[i])
					mvwaddch(win->contentwin, axes[j], i, ACS_HLINE);
				else
					mvwaddch(win->contentwin, indexes[i], i, '*');
				wattroff(win->contentwin, COLOR_PAIR(1));
			}
		}
	}
	
	free(indexes);
}

void remapArrows(struct windowlist* wins, struct windowlist* win)
{
	struct windowlist* ptr;
	
	if(!win)
		return;
	
	win->surrounding = (struct arrowPointers){NULL, NULL, NULL, NULL};
	
	for(ptr = wins; ptr != NULL; ptr = ptr->next)
	{
		if(ptr->frame.origin.y == win->frame.origin.y && ptr->frame.origin.x == win->frame.origin.x + win->frame.size.width + 1)
			win->surrounding.right = ptr;
		if(ptr->frame.origin.y == win->frame.origin.y && ptr->frame.origin.x + ptr->frame.size.width +1 == win->frame.origin.x)
			win->surrounding.left = ptr;
		if(ptr->frame.origin.x == win->frame.origin.x && ptr->frame.origin.y == win->frame.origin.y + win->frame.size.height + 1)
			win->surrounding.down = ptr;
		if(ptr->frame.origin.x == win->frame.origin.x && ptr->frame.origin.y + ptr->frame.size.height +1 == win->frame.origin.y)
			win->surrounding.up = ptr;
	}
	
	remapArrows(wins, win->next);
}

struct GRect getContentFrame(struct windowlist* win)
{
	struct GRect frame = win->frame;
	
	if(win->flags & wf_Title)
	{
		frame.origin.y++;
		frame.size.height--;
	}
	
	if(win->flags & wf_Label)
	{
		frame.origin.x += 3;
		frame.size.width -= 3;
	}
	
	return frame;
}

void resizeWindowToFrame(struct windowlist* win)
{
	int diff;
	int newLen;
	struct GRect contentFrame = getContentFrame(win);
	
	wresize(win->titlewin, 1, win->frame.size.width);
	wresize(win->labelwin, contentFrame.size.height, 3);
	wresize(win->contentwin, contentFrame.size.height, contentFrame.size.width);
	mvwin(win->titlewin, win->frame.origin.y, win->frame.origin.x);
	mvwin(win->labelwin, contentFrame.origin.y, win->frame.origin.x);
	mvwin(win->contentwin, contentFrame.origin.y, contentFrame.origin.x);
	wclear(win->titlewin);
	wclear(win->labelwin);
	wclear(win->contentwin);
	
	newLen = contentFrame.size.width;
	diff = newLen - win->dataLen;
	
	if(win->dataLen == 0)
	{
		diff = 0;
		win->data = calloc(newLen, sizeof(float));
	}
	
	if(diff < 0)
		for(int i = 0; i < abs(diff); i++)
			listShiftLeftAdd(win->data, win->dataLen, 0);
	
	win->dataLen = newLen;
	win->data = realloc(win->data, sizeof(float) * newLen);
	
	if(diff > 0)
		for(int i = 0; i < diff; i++)
			listShiftRightAdd(win->data, win->dataLen, 0);
	
	if(win->flags & wf_Label)
	{
		for(int i = 1; i < 4; i++)
		{
			int height = getContentFrame(win).size.height;
			int row = roundf((float) height - (float) height * (25.0*i)/100.0) - 1;
			char str[4];
			
			sprintf(str, "%d%%", 25*i);
			
			mvwaddstr(win->labelwin, row, 0, str);
		}
	}
}

void splitV(struct windowlist* old)
{
	struct windowlist* new;
	bool parity;
	
	if(old->frame.size.height < 4)
		return;

	new = addWin(&old);
	if(!new)
		return;
	
	new->frame = old->frame;
	
	parity = old->frame.size.height%2;
	
	if(parity) // Read: "if old height is odd"
	{
		old->frame.size.height = (old->frame.size.height - 1)/2;
		new->frame.size.height = old->frame.size.height;
	}
	else
	{
		old->frame.size.height = (old->frame.size.height)/2;
		new->frame.size.height = old->frame.size.height - 1;
	}
	
	new->frame.origin.y = old->frame.origin.y + old->frame.size.height + 1;
	
	resizeWindowToFrame(old);
	resizeWindowToFrame(new);
	
	printLine(borders, new->frame.origin.y - 1, new->frame.origin.x, HORIZ, new->frame.size.width);
}

void splitH(struct windowlist* old)
{
	struct windowlist* new;
	bool parity;

	if(old->frame.size.width < 6)
		return;
	
	new = addWin(&old);
	if(!new)
		return;
	
	new->frame = old->frame;
	
	parity = old->frame.size.width%2;
	
	if(parity) // Read: "if old width is odd"
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
	
	resizeWindowToFrame(old);
	resizeWindowToFrame(new);
	
	printLine(borders, new->frame.origin.y, new->frame.origin.x - 1, VERT, new->frame.size.height);
}

void unSplit(struct windowlist** wins, struct windowlist** win)
{
	struct windowlist** surrPtr;
	
	if(!wins || !*wins || !win || !*win)
		return;
	
	for(surrPtr = &(*win)->surrounding.left; surrPtr <= &(*win)->surrounding.down; surrPtr++)
	{
		struct windowlist* ptr = *surrPtr;
		if(ptr)
		{
			if( (ptr->frame.size.height == (*win)->frame.size.height && abs(ptr->frame.size.width - (*win)->frame.size.width) <= 1) ||
				(ptr->frame.size.width == (*win)->frame.size.width && abs(ptr->frame.size.height - (*win)->frame.size.height) <= 1))
			{
				if(surrPtr < &(*win)->surrounding.up)
					ptr->frame = GRect(	min(ptr->frame.origin.x, (*win)->frame.origin.x),
										min(ptr->frame.origin.y, (*win)->frame.origin.y),
										ptr->frame.size.width + (*win)->frame.size.width + 1,
										ptr->frame.size.height);
				else
					ptr->frame = GRect(	min(ptr->frame.origin.x, (*win)->frame.origin.x),
										min(ptr->frame.origin.y, (*win)->frame.origin.y),
										ptr->frame.size.width,
										ptr->frame.size.height + (*win)->frame.size.height + 1);
				resizeWindowToFrame(ptr);
				freeWin(wins, *win);
				*win = ptr;
				break;
			}
		}
	}
}

void refreshAll(struct windowlist* wins, struct windowlist* focus)
{
	struct windowlist* ptr;
	
	for(ptr = wins; ptr != NULL; ptr = ptr->next)
	{
		if(ptr->flags & wf_Title)
		{
			if(ptr == focus)
				wattron(ptr->titlewin, COLOR_PAIR(2));
			
			if(ptr->dataType == CPUData)
			{
				strcpy(ptr->title, "CPU ");
				if(ptr->dataSource == 0)
					strcat(ptr->title, "Average");
				else
					sprintf(ptr->title + 4, "%d", ptr->dataSource);
				
				if(ptr->flags & wf_ExpandedTitle)
				{
					strcat(ptr->title, " - ");
					sprintf(ptr->title + strlen(ptr->title), "%.2f%%", ptr->data[ptr->dataLen-1]);
				}
	
				for(int i = strlen(ptr->title); i < 23; i++)
					ptr->title[i] = ' ';
				ptr->title[24] = '\0';
			}
			mvwaddstr(ptr->titlewin, 0, 3, ptr->title);
			
			if(ptr == focus)
				wattroff(ptr->titlewin, COLOR_PAIR(2));
		}
		
		if(ptr->flags & wf_Title)
			wrefresh(ptr->titlewin);
		
		if(ptr->flags & wf_Label)
			wrefresh(ptr->labelwin);
		
		wrefresh(ptr->contentwin);
	}
}

void printLine(WINDOW* WIN, int row, int col, enum lineDir direction, int len)
{
	int i;
	
	wattron(WIN, COLOR_PAIR(3));
	switch (direction)
	{
		case HORIZ:
			for(i=0;i<len;i++)
				mvwhline(WIN, row,col+i,ACS_HLINE,1);
			break;
		case VERT:
			for(i=0;i<len;i++)
				mvwvline(WIN, row+i,col,ACS_VLINE,1);
			break;
		default:
			break;
	}
	wattroff(WIN, COLOR_PAIR(3));
}

struct windowlist* addWin(struct windowlist** wins)
{
	struct windowlist* ptr;
	struct windowlist* new;
	
	if(!wins)
		return NULL;
	
	new = malloc(sizeof(struct windowlist));
	if(!new)
		return NULL;
	
	new->next = NULL;
	new->titlewin = newwin(0, 0, 0, 0);
	new->contentwin = newwin(0, 0, 0, 0);
	new->labelwin = newwin(0, 0, 0, 0);
	new->title[0] = 0;
	new->flags = wf_Title | wf_Label | wf_Grid | wf_ExpandedTitle | wf_Border;
	new->type = PercentChart;
	new->surrounding.left = NULL;
	new->surrounding.right = NULL;
	new->surrounding.up = NULL;
	new->surrounding.down = NULL;
	new->frame = GRect(1, 1, COLS - 2, LINES - 3);
	new->dataType = CPUData;
	new->dataSource = 0;
	new->data = NULL;
	new->dataLen = 0;
	
	if(!*wins)
	{
		*wins = new;
	}
	else
	{
		for(ptr = *wins; ptr->next != NULL; ptr = ptr->next);
		
		ptr->next = new;
	}
	
	return new;
}

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
		for(ptr = *wins; ptr->next != win && ptr->next != NULL; ptr = ptr->next);
	
		if(!ptr->next)
			return;
		
		ptr->next = win->next;
	}
	
//	werase(win->contentwin);
//	werase(win->labelwin);
//	werase(win->titlewin);
	delwin(win->contentwin);
	delwin(win->labelwin);
	delwin(win->titlewin);
	free(win->data);
	free(win);
}
