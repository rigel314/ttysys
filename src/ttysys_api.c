/*
 * ttysys_api.c
 *
 *  Created on: Apr 5, 2016
 *      Author: cody
 */

#include <ncurses.h>
#include <string.h>
#include "windowlist.h"
#include "ttysys_api.h"

extern struct windowlist* plgWin;

void setTitle(char* title)
{
//	mvwprintw(focus->titlewin,0,0,test);
	strncpy(plgWin->title, title, TITLE_LEN);
}

int getRefreshRate()
{
	return plgWin->refreshPrd;
}
