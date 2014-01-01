/*
 * common.h
 *
 *  Created on: Jan 1, 2014
 *      Author: cody
 */

#include <ncurses.h>

#ifndef COMMON_H_
#define COMMON_H_

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

int strchrCount(char* s, char c);
void listShiftLeftAdd(float* list, int len, float new);
void listShiftRightAdd(float* list, int len, float new);

extern WINDOW* borders;

#endif /* COMMON_H_ */
