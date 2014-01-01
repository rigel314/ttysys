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
	int height = win->frame.size.height - 1;
	int width = win->dataLen;
	
	for(int i = 0; i < 5; i++)
		axes[i] = roundf((float) height - (float) height * (25.0*i)/100.0) - 1;
	
	for(int i = 0; i < width; i++)
	{
		indexes[i] = roundf((float) height - (float) height * win->data[i]/100.0) - 1;
		
		for(int j = 0; j < height; j++)
		{
			if(j >= indexes[i])
				mvwaddch(win->contentwin, j, i, '*');
			else
				mvwaddch(win->contentwin, j, i, ' ');
		}
		
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

void resizeWindowToFrame(struct windowlist* win)
{
	int diff;
	int newLen;
	
	wresize(win->titlewin, 1, win->frame.size.width);
	wresize(win->labelwin, win->frame.size.height-1, 3);
	wresize(win->contentwin, win->frame.size.height-1, win->frame.size.width-3);
	mvwin(win->titlewin, win->frame.origin.y, win->frame.origin.x);
	mvwin(win->labelwin, win->frame.origin.y+1, win->frame.origin.x);
	mvwin(win->contentwin, win->frame.origin.y+1, win->frame.origin.x+3);
	wclear(win->titlewin);
	wclear(win->labelwin);
	wclear(win->contentwin);
	
	newLen = win->frame.size.width-3;
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
	
//	mvwprintw(win->titlewin, 0, 3, "----Test Title----");
//	win->title = "----Test Title----";
	mvwprintw(win->labelwin, 3, 0, "20%%");
//	mvwprintw(win->contentwin, 3, 3, "%d, %d, %d, %d        ", win->frame.origin.x, win->frame.origin.y, win->frame.size.width, win->frame.size.height);
}

void splitV(struct windowlist* old)
{
	struct windowlist* new;
	bool parity;
	
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
//	new->frame.origin.x = old->frame.origin.x;
	
//	new->surrounding = old->surrounding;
//	old->surrounding.down = new;
//	new->surrounding.up = old;
//	if(new->surrounding.down && new->surrounding.down->frame.origin.x == new->frame.origin.x)
//		new->surrounding.down->surrounding.up = new;
	
	resizeWindowToFrame(old);
	resizeWindowToFrame(new);
	
	printLine(new->frame.origin.y - 1, new->frame.origin.x, HORIZ, new->frame.size.width);
}

void splitH(struct windowlist* old)
{
	struct windowlist* new;
	bool parity;
	
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
	
//	new->frame.origin.y = old->frame.origin.y;
	new->frame.origin.x = old->frame.origin.x + old->frame.size.width + 1;
	
//	new->surrounding = old->surrounding;
//	old->surrounding.right = new;
//	new->surrounding.left = old;
//	if(new->surrounding.right && new->surrounding.right->frame.origin.y == new->frame.origin.y)
//		new->surrounding.right->surrounding.left = new;
	
	resizeWindowToFrame(old);
	resizeWindowToFrame(new);
	
	printLine(new->frame.origin.y, new->frame.origin.x - 1, VERT, new->frame.size.height);
}

//void writeAllRefresh(struct windowlist* list)
//{
//	resizeAll(list);
//	writeContents(list);
//	writeTitles(list);
//	refreshAll(list);
//	return;
//}

void refreshAll(struct windowlist* wins, struct windowlist* focus)
{
	struct windowlist* ptr;
	
	for(ptr = wins; ptr != NULL; ptr = ptr->next)
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
			strcat(ptr->title, " - ");
			sprintf(ptr->title + strlen(ptr->title), "%.2f", ptr->data[ptr->dataLen-1]);
			strcat(ptr->title, "%");
			for(int i = strlen(ptr->title); i < 23; i++)
				ptr->title[i] = ' ';
			ptr->title[24] = '\0';
		}
		mvwaddstr(ptr->titlewin, 0, 3, ptr->title);
		
		if(ptr == focus)
			wattroff(ptr->titlewin, COLOR_PAIR(2));
		
		wrefresh(ptr->titlewin);
		wrefresh(ptr->labelwin);
		wrefresh(ptr->contentwin);
	}
}

void printLine(int row, int col, enum lineDir direction, int len)
{
	int i;
	switch (direction) {
		case HORIZ:
			for(i=0;i<len;i++)
				mvhline(row,col+i,ACS_HLINE,1);
			break;
		case VERT:
			for(i=0;i<len;i++)
				mvvline(row+i,col,ACS_VLINE,1);
			break;
		default:
			break;
	}
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
	new->flags = 0;
	new->type = PercentChart;
	new->surrounding.left = NULL;
	new->surrounding.right = NULL;
	new->surrounding.up = NULL;
	new->surrounding.down = NULL;
	new->frame = GRect(1, 1, COLS - 2, LINES - 2);
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
	
	for(ptr = *wins; ptr->next != win && ptr->next != NULL; ptr = ptr->next);
	
	if(!ptr->next)
		return;
	
	ptr->next = win->next;
	
	werase(win->contentwin);
	werase(win->labelwin);
	werase(win->titlewin);
	delwin(win->contentwin);
	delwin(win->labelwin);
	delwin(win->titlewin);
	free(win->data);
}
