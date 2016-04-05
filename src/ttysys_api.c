/*
 * ttysys_api.c
 *
 *  Created on: Apr 5, 2016
 *      Author: cody
 */

#include <ncurses.h>

extern WINDOW* status;

void setTitle(char* test)
{
	mvwprintw(status,0,60,test);
}
