/*
 * mem.h
 *
 *  Created on: Sep 11, 2015
 *      Author: cody
 */

#ifndef MEM_H_
#define MEM_H_

// Holds numbers from /proc/meminfo in kiB.
struct memNow
{
	long ramTotal;
	long ramFree;
	long ramBuffered;
	long ramCached;
	long swapTotal;
	long swapFree;
};

// Holds percentages for RAM and Swap.
struct memPercent
{
	struct memNow now;
	float ram;
	float swap;
};

long parseMemLine(char* str, int len);
struct memNow readMem();
void getMemInfo(struct memPercent* mem);

#endif /* MEM_H_ */
