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

// Remember, any functions added below must be added to the ttysys_export_list.txt file

void setTitle(char* title)
{
	strncpy(plgWin->title, title, TITLE_LEN);
}

void setText(char* text)
{
	if(plgWin->type == TextChart)
	{
		werase(plgWin->contentwin);
		mvwprintw(plgWin->contentwin,0,0,"%s", text);
	}
}

int getRefreshRate()
{
	return plgWin->refreshPrd;
}
