/*
 * memInfo.c
 *
 *  Created on: Jan 5, 2014
 *      Author: cody
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memInfo.h"

long parseMemLine(char* str, int len)
{
	int i, j;
	char copy[len+1];
	
	memcpy(copy, str, len);
	copy[len] = '\0';
	
	for(i = 0; i < len && !(copy[i] >= '0' && copy[i] <= '9'); i++);
	for(j = i; j < len && copy[j] >= '0' && copy[j] <= '9'; j++);
	copy[j] = '\0';
	
	return atol(copy + i);
}

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
	{
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
		
		free(line);
		line = NULL;
	}
	
	fclose(fp);
	
	return now;
}

void getMemInfo(struct memPercent* mem)
{
	struct memNow now = readMem();
	
	if(now.ramTotal == 0)
		mem->ram = 0;
	else
		mem->ram = (float) (now.ramTotal - (now.ramFree + now.ramBuffered + now.ramCached)) / (float) now.ramTotal * 100.0;
	if(now.swapTotal == 0)
		mem->swap = 0;
	else
		mem->swap = (float) (now.swapTotal - now.swapFree) / (float) now.swapTotal * 100.0;
}
