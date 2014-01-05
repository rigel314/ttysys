/*
 * common.h
 *
 *  Created on: Jan 1, 2014
 *      Author: cody
 */

#include <ncurses.h>

#ifndef COMMON_H_
#define COMMON_H_

#define AppName "ttysys"
#define AppVers "0.7"

#define ASCIIshiftBit 0x20

#define min(x, y) ((x)<(y)?(x):(y))
#define max(x, y) ((x)>(y)?(x):(y))
#define LLforeach(type, ptr, list) for(type ptr = list; ptr != NULL; ptr = ptr->next)

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

void showHelp();
int strchrCount(char* s, char c);
void listShiftLeftAdd(float* list, int len, float new);
void listShiftRightAdd(float* list, int len, float new);

extern WINDOW* borders;

#endif /* COMMON_H_ */
