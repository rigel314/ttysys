/*
 * cpuInfo.c
 *
 *  Created on: Jan 1, 2014
 *      Author: cody
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include <sys/select.h>
#include "cpuInfo.h"
#include "common.h"

int getNumCPUs()
{
	FILE* fp;
	char* line = NULL;
	size_t dum;
	ssize_t err;
	int count = 0;
	
	fp = fopen("/proc/stat", "r");
	if(!fp)
		return 0;
	
	while((err = getline(&line, &dum, fp)) != -1)
	{
		if(strncmp(line, "cpu", 3))
		{
			free(line);
			break;
		}
		
		if(line[3] >= '0' && line[3] <= '9')
			count++;
		
		free(line);
		line = NULL;
	}
	
	fclose(fp);
	
	return count;
}

struct cpuTime parseLine(char* str, int len)
{
	char copy[len+1];
	char** args;
	int j = 1;
	struct cpuTime out;
	unsigned long long sum = 0;
	int argc = strchrCount(str, ' ') + 1;
	
	args = malloc(sizeof(char*) * argc);
	
	args[0] = NULL;
	
	memcpy(copy, str, len+1);
	for(int i = 0; i < len; i++)
	{
//		printf("%d: %d\n", i, j);
		if(copy[i] < ' ')
			copy[i] = '\0';
		if(copy[i] == ' ')
		{
			if(args[j-1] < &copy[i])
				args[j++] = &copy[i+1];
			else
				args[j-1]++;
			copy[i] = '\0';
		}
	}
	
	out.user = atol(args[1]) + atol(args[2]);
	out.sys = atol(args[3]);
	for(int i = 1; i < j; i++)
		sum += atol(args[i]);
	out.total = sum;
	
	free(args);
	
	return out;
}

int readCPUs(int numCPUs, struct cpuTime* now)
{
	FILE* fp;
	char* line = NULL;
	size_t dum;
	ssize_t len;
	int i;
	
	fp = fopen("/proc/stat", "r");
	if(!fp)
		return 0;
	
	for(i = 0; i <  numCPUs + 1; i++)
	{
		if((len = getline(&line, &dum, fp)) != -1)
		{
			now[i] = parseLine(line, len);
			free(line);
			line = NULL;
		}
	}
	
	fclose(fp);
	
	return i;
}

int getCPUtime(struct cpuPercent* cpu, int numCPUs, struct cpuTime* first, struct cpuTime* second)
{
	struct timeval wait = {.tv_sec = 1, .tv_usec = 0};
	fd_set set;
	long numEvents;
	
	readCPUs(numCPUs, first);

	// Zero out the set, then add 0 to the set to monitor stdin for keypresses.
	FD_ZERO(&set);
	FD_SET(0, &set);
	// Wait 1 second. Or gather input.
	select(1, &set, NULL, NULL, &wait);
	
//	if(FD_ISSET(0, &set))
//		return 1;
	
	readCPUs(numCPUs, second);

	// do math to calculate percentage
	for(int i = 0; i < numCPUs + 1; i++)
	{
		numEvents = second[i].total - first[i].total;
		if(numEvents == 0)
			continue;
		
		cpu[i].user = (float) (second[i].user - first[i].user) / (float) numEvents * 100.0;
		cpu[i].sys = (float) (second[i].sys - first[i].sys) / (float) numEvents * 100.0;
		cpu[i].total = cpu[i].user + cpu[i].sys;
	}

	return 0;
}
