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

void setTitle(char* test)
{
//	mvwprintw(focus->titlewin,0,0,test);
	strncpy(plgWin->title, test, TITLE_LEN);
}

int getRefreshRate()
{
	return plgWin->refreshPrd;
}
