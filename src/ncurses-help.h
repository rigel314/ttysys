/*
 * ncurses-help.h
 *
 *  Created on: Dec 1, 2015
 *      Author: cody
 */

#ifndef NCURSES_HELP_H_
#define NCURSES_HELP_H_

#include "windowlist.h"
#include <ncurses.h>

struct windowlist* ncurses_init();
WINDOW* addStatusLine();

#endif /* NCURSES_HELP_H_ */
