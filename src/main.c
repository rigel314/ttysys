/*
 * main.c
 *
 *  Created on: Dec 26, 2013
 *      Author: cody
 */

#include <ncurses.h>
#include <stdlib.h>
#include "windowlist.h"
#include "cpuInfo.h"

int main(int argc, char** argv)
{
	int c;
	struct cpuTime* then;
	struct cpuTime* now;
	struct cpuPercent* cpu;
	int numCPUs = getNumCPUs();
//	float* list = NULL;
//	int listLen;
//	int rows;
//	int columns = 0;
//	bool startFlag = true;
	struct windowlist* wins = NULL;
	struct windowlist* focus;
	struct windowlist* ptr;
	
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
		getCPUtime(cpu, numCPUs, then, now);
		for(ptr = wins; ptr != NULL; ptr = ptr->next)
		{
			listShiftLeftAdd(ptr->data, ptr->dataLen, cpu[0].total);
			drawScreen(ptr);
		}
	}
	
	endwin();
	return 0;
	
//	while((c = getch()) != 10 && c != 'q' && c != 'Q')
//	{
//		if(startFlag)
//			c = KEY_RESIZE;
//		if(c == KEY_RESIZE)
//		{
//			int diff = COLS - columns;
//			
//			if(startFlag)
//			{
//				startFlag = false;
//				diff = 0;
//				list = calloc(COLS, sizeof(float));
//			}
//			
//			if(diff < 0)
//				for(int i = 0; i < abs(diff); i++)
//					listShiftLeftAdd(list, listLen, 0);
//			
//			rows = LINES;
//			columns = COLS;
//			listLen = COLS;
//			
//			list = realloc(list, sizeof(float) * listLen);
//			
//			if(diff > 0)
//				for(int i = 0; i < diff; i++)
//					listShiftRightAdd(list, listLen, 0);
//		}
//
//		clear();
////		for(int i = 0; i < numCPUs + 1; i++)
////			mvprintw(i, 0, "%.2f%%\t%.2f%%\t%.2f%%\n", cpu[i].total, cpu[i].user, cpu[i].sys);
//		drawScreen(list, columns, rows);
//		refresh();
//		getCPUtime(cpu, numCPUs, then, now);
//		listShiftLeftAdd(list, listLen, cpu[0].total);
//	}
//	
//	endwin();
//	return 0;
}
