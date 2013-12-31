/*
 * main.c
 *
 *  Created on: Dec 26, 2013
 *      Author: cody
 */

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <sys/select.h>

enum lineDir { HORIZ, VERT };

struct cpuTime
{
	unsigned long long total;
	long user;
	long sys;
};

struct cpuPercent
{
	float total;
	float user;
	float sys;
};

enum winFlags { TitleOff = 1, LabelOff = 1<<1, GridOff = 1<<2, ExpandedTitleOff = 1<<3, BorderOff = 1<<4 };
enum winType { PercentChart };

struct GPoint
{
	int x;
	int y;
};
struct GSize
{
	int width;
	int height;
};
struct GRect
{
	struct GPoint origin;
	struct GSize size;
};
#define GRect(x, y, w, h) ((struct GRect){{(x), (y)}, {(w), (h)}})

struct windowlist
{
	struct windowlist* next;
	WINDOW* titlewin;
	WINDOW* contentwin;
	WINDOW* labelwin;
	char* title;
	int flags;
	enum winType type;
	struct GRect frame;
	struct arrowPointers
	{
		struct windowlist* left;
		struct windowlist* right;
		struct windowlist* up;
		struct windowlist* down;
	} surrounding;
	float* data;
	int dataLen;
};

void listShiftLeftAdd(float* list, int len, float new);
void listShiftRightAdd(float* list, int len, float new);
void drawScreen(float* list, int width, int height);
int getNumCPUs();
int strchrCount(char* s, char c);
struct cpuTime parseLine(char* str, int len);
int readCPUs(int numCPUs, struct cpuTime* now);
int getCPUtime(struct cpuPercent* cpu, int numCPUs, struct cpuTime* first, struct cpuTime* second);

void remapArrows(struct windowlist* wins, struct windowlist* win);
void resizeWindowToFrame(struct windowlist* win);
void splitV(struct windowlist* old);
void splitH(struct windowlist* old);
//void writeAllRefresh(struct windowlist* list);
void refreshAll(struct windowlist* wins, struct windowlist* focus);
void printLine(int row, int col, enum lineDir direction, int len);
struct windowlist* addWin(struct windowlist** wins);
void freeWin(struct windowlist** wins, struct windowlist* win);

int main(int argc, char** argv)
{
	int c;
	struct cpuTime* then;
	struct cpuTime* now;
	struct cpuPercent* cpu;
	int numCPUs = getNumCPUs();
	float* list = NULL;
	int listLen;
	int rows;
	int columns = 0;
	bool startFlag = true;
	struct windowlist* wins = NULL;
	struct windowlist* focus;
	
	// creating structs
	then = malloc(sizeof(struct cpuTime) * (numCPUs+1));
	now = malloc(sizeof(struct cpuTime) * (numCPUs+1));
	cpu = malloc(sizeof(struct cpuPercent) * (numCPUs+1));
	
	// ncurses init stuff
	initscr();
	start_color();
	init_pair(1, COLOR_RED, COLOR_BLACK);
	init_pair(2, COLOR_YELLOW, COLOR_BLACK);
	raw();
	noecho();
	curs_set(0);
	keypad(stdscr,TRUE);
	halfdelay(1);
	refresh();
	
	focus = addWin(&wins);
	resizeWindowToFrame(focus);
	
	box(stdscr, 0, 0);
	
	while((c = getch()))
	{
		refresh();
		refreshAll(wins, focus);
		
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
	}
	
	endwin();
	return 0;
	
	while((c = getch()) != 10 && c != 'q' && c != 'Q')
	{
		if(startFlag)
			c = KEY_RESIZE;
		if(c == KEY_RESIZE)
		{
			int diff = COLS - columns;
			
			if(startFlag)
			{
				startFlag = false;
				diff = 0;
				list = calloc(COLS, sizeof(float));
			}
			
			if(diff < 0)
				for(int i = 0; i < abs(diff); i++)
					listShiftLeftAdd(list, listLen, 0);
			
			rows = LINES;
			columns = COLS;
			listLen = COLS;
			
			list = realloc(list, sizeof(float) * listLen);
			
			if(diff > 0)
				for(int i = 0; i < diff; i++)
					listShiftRightAdd(list, listLen, 0);
		}

		clear();
//		for(int i = 0; i < numCPUs + 1; i++)
//			mvprintw(i, 0, "%.2f%%\t%.2f%%\t%.2f%%\n", cpu[i].total, cpu[i].user, cpu[i].sys);
		drawScreen(list, columns, rows);
		refresh();
		getCPUtime(cpu, numCPUs, then, now);
		listShiftLeftAdd(list, listLen, cpu[0].total);
	}
	
	endwin();
	return 0;
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
	win->title = "----Test Title----";
	mvwprintw(win->labelwin, 3, 0, "20%%");
	mvwprintw(win->contentwin, 3, 3, "%d, %d, %d, %d        ", win->frame.origin.x, win->frame.origin.y, win->frame.size.width, win->frame.size.height);
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
//		box(ptr->titlewin, 0, 0);
//		box(ptr->labelwin, 0, 0);
//		box(ptr->contentwin, 0, 0);
		
		if(ptr == focus)
			wattron(ptr->titlewin, COLOR_PAIR(2));
		
		mvwprintw(ptr->titlewin, 0, 3, ptr->title);
		
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
	new->title = NULL;
	new->flags = 0;
	new->type = PercentChart;
	new->surrounding.left = NULL;
	new->surrounding.right = NULL;
	new->surrounding.up = NULL;
	new->surrounding.down = NULL;
	new->frame = GRect(1, 1, COLS - 2, LINES - 2);
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
	free(win->title);
}


void listShiftLeftAdd(float* list, int len, float new)
{
	for(int i = 0; i < len-1; i++)
	{
		list[i] = list[i+1];
	}
	list[len-1] = new;
}

void listShiftRightAdd(float* list, int len, float new)
{
	for(int i = len - 1; i > 0; i--)
	{
		list[i] = list[i-1];
	}
	list[0] = new;
}

void drawScreen(float* list, int width, int height)
{
	int indexes[width];
	int axes[5];
	
	for(int i = 0; i < 5; i++)
		axes[i] = roundf((float) height - (float) height * (25.0*i)/100.0) - 1;
	
	for(int i = 0; i < width; i++)
	{
		indexes[i] = roundf((float) height - (float) height * list[i]/100.0) - 1;
		
		for(int j = indexes[i]; j < height; j++)
		{
			mvprintw(j, i, "*");
		}
		
		for(int j = 1; j < 4; j++)
		{
			attron(COLOR_PAIR(1));
			if(axes[j] != indexes[i])
				mvaddch(axes[j], i, ACS_HLINE);
			else
				mvprintw(indexes[i], i, "*");
			attroff(COLOR_PAIR(1));
		}
	}
}

int getNumCPUs()
{
	FILE* fp;
	char* line = NULL;
	size_t dum;
	ssize_t err;
	int count = 0;
	
	fp = fopen("/proc/stat", "r");
	if(!fp)
		return 0;
	
	while((err = getline(&line, &dum, fp)) != -1)
	{
		if(strncmp(line, "cpu", 3))
		{
			free(line);
			break;
		}
		
		if(line[3] >= '0' && line[3] <= '9')
			count++;
		
		free(line);
		line = NULL;
	}
	
	fclose(fp);
	
	return count;
}

int strchrCount(char* s, char c)
{
	int i;
	for (i = 0;
			s[i];
			s[i]==c ? i++ : (long) s++);
	return i;
}

struct cpuTime parseLine(char* str, int len)
{
	char copy[len+1];
	char** args;
	int j = 1;
	struct cpuTime out;
	unsigned long long sum = 0;
	int argc = strchrCount(str, ' ') + 1;
	
	args = malloc(sizeof(char*) * argc);
	
	args[0] = NULL;
	
	memcpy(copy, str, len+1);
	for(int i = 0; i < len; i++)
	{
//		printf("%d: %d\n", i, j);
		if(copy[i] < ' ')
			copy[i] = '\0';
		if(copy[i] == ' ')
		{
			if(args[j-1] < &copy[i])
				args[j++] = &copy[i+1];
			else
				args[j-1]++;
			copy[i] = '\0';
		}
	}
	
	out.user = atol(args[1]) + atol(args[2]);
	out.sys = atol(args[3]);
	for(int i = 1; i < j; i++)
		sum += atol(args[i]);
	out.total = sum;
	
	free(args);
	
	return out;
}

int readCPUs(int numCPUs, struct cpuTime* now)
{
	FILE* fp;
	char* line = NULL;
	size_t dum;
	ssize_t len;
	int i;
	
	fp = fopen("/proc/stat", "r");
	if(!fp)
		return 0;
	
	for(i = 0; i <  numCPUs + 1; i++)
	{
		if((len = getline(&line, &dum, fp)) != -1)
		{
			now[i] = parseLine(line, len);
			free(line);
			line = NULL;
		}
	}
	
	fclose(fp);
	
	return i;
}

int getCPUtime(struct cpuPercent* cpu, int numCPUs, struct cpuTime* first, struct cpuTime* second)
{
	struct timeval wait = {.tv_sec = 1, .tv_usec = 0};
	fd_set set;
	long numEvents;
	
	readCPUs(numCPUs, first);

	// Zero out the set, then add 0 to the set to monitor stdin for keypresses.
	FD_ZERO(&set);
	FD_SET(0, &set);
	// Wait 1 second. Or gather input.
	select(1, &set, NULL, NULL, &wait);
	
	readCPUs(numCPUs, second);

	// do math to calculate percentage
	for(int i = 0; i < numCPUs + 1; i++)
	{
		numEvents = second[i].total - first[i].total;
		if(numEvents == 0)
			return 0;
		
		cpu[i].user = (float) (second[i].user - first[i].user) / (float) numEvents * 100.0;
		cpu[i].sys = (float) (second[i].sys - first[i].sys) / (float) numEvents * 100.0;
		cpu[i].total = cpu[i].user + cpu[i].sys;
	}

	return 0;
}
