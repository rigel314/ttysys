/*
 * memInfo.h
 *
 *  Created on: Jan 5, 2014
 *      Author: cody
 */

#ifndef MEMINFO_H_
#define MEMINFO_H_

struct memNow
{
	long ramTotal;
	long ramFree;
	long ramBuffered;
	long ramCached;
	long swapTotal;
	long swapFree;
};

struct memPercent
{
	float ram;
	float swap;
};

long parseMemLine(char* str, int len);
struct memNow readMem();
void getMemInfo(struct memPercent* mem);

#endif /* MEMINFO_H_ */
