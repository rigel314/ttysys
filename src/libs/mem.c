/*
 * mem.c
 *
 *  Created on: Sep 11, 2015
 *      Author: cody
 */

#include "../ttysys_api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mem.h"

/**
 * long parseMemLine(char* str, int len)
 * 	str is a line from /proc/meminfo.
 * 	len is the lengh of str.
 * finds a number and uses atol() to get it's value.
 * ignores unit, assumes kiB.
 * returns value of meminfo entry.
 */
long parseMemLine(char* str, int len)
{
	int i, j;
	char copy[len+1];
	
	// make local copy for monkeying.
	memcpy(copy, str, len);
	copy[len] = '\0';
	
	for(i = 0; i < len && !(copy[i] >= '0' && copy[i] <= '9'); i++); // i is the beginning of the number.
	for(j = i; j < len && copy[j] >= '0' && copy[j] <= '9'; j++); // j is right after the end of the number.
	copy[j] = '\0'; // NULL terminate at end of number.
	
	return atol(copy + i); // Give atol() a string that starts at copy+i.
}

/**
 * struct memNow readMem()
 * Reads /proc/meminfo and looks for specific lines relating to RAM and Swap.
 * Parses them with parseMemLine().
 * returns struct memNow containing values from /proc/meminfo.
 */
struct memNow readMem()
{
	struct memNow now = {0, 0, 0, 0, 0, 0};
	FILE* fp;
	char* line = NULL;
	size_t dum;
	ssize_t len;
	int count = 0;
	
	fp = fopen("/proc/meminfo", "r");
	if(!fp)
		return now;
	
	while(count < 6 && (len = getline(&line, &dum, fp)) != -1)
	{ // Get a line and parse it if it is one of these lines:
		if(!strncmp(line, "MemTotal:", 9))
		{
			count++;
			now.ramTotal = parseMemLine(line, len);
		}
		if(!strncmp(line, "MemFree:", 8))
		{
			count++;
			now.ramFree = parseMemLine(line, len);
		}
		if(!strncmp(line, "Buffers:", 8))
		{
			count++;
			now.ramBuffered = parseMemLine(line, len);
		}
		if(!strncmp(line, "Cached:", 7))
		{
			count++;
			now.ramCached = parseMemLine(line, len);
		}
		if(!strncmp(line, "SwapTotal:", 10))
		{
			count++;
			now.swapTotal = parseMemLine(line, len);
		}
		if(!strncmp(line, "SwapFree:", 9))
		{
			count++;
			now.swapFree = parseMemLine(line, len);
		}
		
		free(line); // Important!
		line = NULL; // Also important.  See man 3 getline.
	}
	
	fclose(fp);
	
	return now;
}

/**
 * void getMemInfo(struct memPercent* mem)
 * 	mem is the output that should end up with percentages
 * Calls readMem() and calculates percentages.
 */
void getMemInfo(struct memPercent* mem)
{
	// Read meminfo.
	struct memNow now = readMem();
	
	// Do math to calculate percentage.
	if(now.ramTotal == 0)
		mem->ram = 0;
	else
		mem->ram = (float) (now.ramTotal - (now.ramFree + now.ramBuffered + now.ramCached)) / (float) now.ramTotal * 100.0;
	if(now.swapTotal == 0)
		mem->swap = 0;
	else
		mem->swap = (float) (now.swapTotal - now.swapFree) / (float) now.swapTotal * 100.0;
	
	mem->now = now;
}

int nextVal(void** context, float* outs)
{
	struct memPercent mem;
	
	getMemInfo(&mem);
	
	int* type = (int*) *context;
	
	if(*type == 1)
		outs[0] = mem.ram;
	else
		outs[0] = mem.swap;
	
	char str[100];
	sprintf(str,"%f",mem.swap);
	setTitle(str);
	
	return 0;
}

void destroy(void** context)
{
	free(*context);
}

struct initData init(void** context, int argc, char** argv)
{
	struct initData id;
	
	*context = malloc(sizeof(int));
	int* type = (int*) *context;
	
	*type = 0;
	
	if(argc < 2 || !strcmp(argv[1],"ram"))
		*type = 1;
	if(argc == 2 && !strcmp(argv[1],"swap"))
		*type = 2;
	
	id.status = initStatus_Success;
	if(*type == 0)
		id.status = initStatus_ArgFailure;
	id.nextValue = &nextVal;
	id.cleanUp = &destroy;
	id.type = PercentChart;
	
	return id;
}
