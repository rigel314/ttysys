/*
 * common.h
 *
 *  Created on: Jan 1, 2014
 *      Author: cody
 */

#include <ncurses.h>

#ifndef COMMON_H_
#define COMMON_H_

// Program Information.
#define AppName "ttysys"
#define AppVers "1.0"

// Timer frequency of internal timer.  Max update frequency for windows.
#define TIMER_FREQ 10 // Hz

// ASCII shiftBit for ignoring case.
#define ASCIIshiftBit 0x20

// Usual min and max macros.
#define min(x, y) ((x)<(y)?(x):(y))
#define max(x, y) ((x)>(y)?(x):(y))

// Macro for iterating through any linked list.
#define LLforeach(type, ptr, list) for(type ptr = list; ptr != NULL; ptr = ptr->next)

// Frame structures.
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
// Helper macro for creating a GRect in one line.
#define GRect(x, y, w, h) ((struct GRect){{(x), (y)}, {(w), (h)}})

void showHelp();
char* getDirectionString(int num);
int strchrCount(char* s, char c);
void listShiftLeftAdd(float* list, int len, float new);
void listShiftRightAdd(float* list, int len, float new);

// Global variable for WINDOW* to draw borders on.
extern WINDOW* borders;
extern WINDOW* status;

#endif /* COMMON_H_ */
