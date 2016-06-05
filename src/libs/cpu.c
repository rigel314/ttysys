/*
 * cpu.c
 *
 *  Created on: Jan 1, 2014
 *      Author: cody
 */

#include "../ttysys_api.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ncurses.h>
#include <sys/select.h>
#include "cpu.h"

/**
 * int strchrCount(char* s, char c)
 * 	s is a string.
 * 	c is a character to look for.
 * returns number of occurrences of c in s.
 */
int strchrCount(char* s, char c)
{
	int i;
	for (i = 0; s[i]; (s[i] == c) ? (void) i++ : (void) s++); // always increment s or i.  Casts to void to avoid warnings.
	return i;
}

/**
 * int getNumCPUs()
 * reads /proc/stat and counts the lines that match "/^cpu[0-9]/"
 */
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
	
	// read one line.
	while((err = getline(&line, &dum, fp)) != -1)
	{
		// If it doesn't begin with cpu, exit the loop.
		// /proc/stat only begins with lines that start with "cpu". Once we hit a line that doesn't, we're done.
		if(strncmp(line, "cpu", 3))
		{
			free(line);
			break;
		}
		
		// The next character must be a number.
		if(line[3] >= '0' && line[3] <= '9')
			count++;
		
		free(line);
		line = NULL;
	}
	
	fclose(fp);
	
	return count;
}

/**
 * struct cpuTime parseCPUline(char* str, int len)
 * 	str is a string read from /proc/stat.
 * 	len in the length of the string.
 * Counts the number of single space delimited arguments, and splits them into an argc/argv layout.
 * uses atol() to get the information.
 */
struct cpuTime parseCPUline(char* str, int len)
{
	char copy[len+1];
	char** args;
	int j = 1;
	struct cpuTime out;
	unsigned long long sum = 0;
	int argc = strchrCount(str, ' ') + 1;
	
	// make a list of char*s to record each argument.
	args = malloc(sizeof(char*) * argc);
	
	args[0] = NULL;
	
	// make a local copy of str to mess with.
	memcpy(copy, str, len+1);
	
	for(int i = 0; i < len; i++)
	{
		if(copy[i] < ' ') // I don't want any character lower than a space.
			copy[i] = '\0';
		if(copy[i] == ' ')
		{
			if(args[j-1] < &copy[i]) // if the last arg is less than the current place
				args[j++] = &copy[i+1]; // Set the beginning of the next arg to the next place in the string.
			else // the last arg is equal to the current place because there were multiple spaces in a row.
				args[j-1]++; // increment last arg.
			copy[i] = '\0'; // Always set ' ' to '\0'.
		}
	}
	
	out.user = atol(args[1]) + atol(args[2]); // User mode CPU time is arg[1] + arg[2]. See `man 5 proc`.
	out.sys = atol(args[3]); // System mode CPU time is arg[3].
	for(int i = 1; i < j; i++)
		sum += atol(args[i]); // The total CPU time is the sum of all modes.
	out.total = sum;
	
	free(args);
	
	return out;
}

/**
 * int readCPUs(int numCPUs, struct cpuTime* now)
 * 	numCPUs is the number of CPUs.
 * 	now is a struct to be modified that holds the user, system, and total CPU times.
 * reads /proc/stat and passes them to parseCPUline for parsing.
 */
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
		{ // get a line. And parse it.
			now[i] = parseCPUline(line, len);
			free(line); // This line is important.
			line = NULL; // So is this one. See man 3 getline.
		}
	}
	
	fclose(fp);
	
	return i;
}

/**
 * int getCPUtime(struct cpuPercent* cpu, int numCPUs, struct cpuTime* first, struct cpuTime* second)
 * 	cpu is the output that should end up with percentages.
 * 	numCPUS is the number of CPUs.
 * 	first is a cpuTime before a wait.
 * 	second is a cpuTime after a wait.
 * Reads CPUs, waits, reads CPUs again and calculates percent.
 * returns 1 if interrupted by keypress. 0 otherwise.
 */
int getCPUtime(struct cpuPercent* cpu, int numCPUs, struct cpuTime* first, struct cpuTime* second)
{
	long numEvents;
	
	// read once.
	readCPUs(numCPUs, first);
	
	// Read again.
	readCPUs(numCPUs, second);

	// Do math to calculate percentage.
	for(int i = 0; i < numCPUs + 1; i++)
	{
		numEvents = second[i].total - first[i].total;
		if(numEvents == 0)
			continue; // Skip if no events happened for this processor.  Shouldn't happen unless wait didn't work and we didn't return.
		
		cpu[i].user = (float) (second[i].user - first[i].user) / (float) numEvents * 100.0;
		cpu[i].sys = (float) (second[i].sys - first[i].sys) / (float) numEvents * 100.0;
		cpu[i].total = cpu[i].user + cpu[i].sys;
	}

	return 0;
}

int nextVal(void** context, float* outs)
{
	long numEvents;
	char str[100];

	struct cpuCtx* ctx = (struct cpuCtx*) *context;
	struct cpuTime cts[ctx->numCPUs+1];
	struct cpuPercent cpu[ctx->numCPUs + 1];
	
	if(!ctx->valid)
	{
		readCPUs(ctx->numCPUs, ctx->cpuTimesLast);
		ctx->valid = true;
		outs[0] = 0;
		return 0;
	}
	else
		readCPUs(ctx->numCPUs, cts);
	
	for(int i = 0; i < ctx->numCPUs + 1; i++)
	{
		numEvents = cts[i].total - ctx->cpuTimesLast[i].total;
		if(numEvents == 0)
			continue; // Skip if no events happened for this processor.  Shouldn't happen unless wait didn't work and we didn't return.
		
		cpu[i].user = (float) (cts[i].user - ctx->cpuTimesLast[i].user) / (float) numEvents * 100.0;
		cpu[i].sys = (float) (cts[i].sys - ctx->cpuTimesLast[i].sys) / (float) numEvents * 100.0;
		cpu[i].total = cpu[i].user + cpu[i].sys;
	}
	
	if(ctx->whichCPU > 0)
	{
		outs[0] = cpu[ctx->whichCPU + 1].total;
		sprintf(str,"CPU %d", ctx->whichCPU);
	}
	else
	{
		outs[0] = cpu[0].total;
		sprintf(str,"CPU Summary");
	}
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
	
	*context = malloc(sizeof(struct cpuCtx));
	struct cpuCtx* ctx = (struct cpuCtx*) *context;
	
	ctx->whichCPU = -1;

	if(argc == 2)
		ctx->whichCPU = atoi(argv[1]);
	if(argc < 2 || !strcmp(argv[1],"all"))
		ctx->whichCPU = -1;

	ctx->numCPUs = getNumCPUs();
	ctx->cpuTimesLast = malloc(sizeof(struct cpuTime) * (ctx->numCPUs + 1));
	ctx->valid = false;
	
	id.status = initStatus_Success;
	if(ctx->whichCPU > ctx->numCPUs)
		id.status = initStatus_ArgFailure;
	id.nextValue = &nextVal;
	id.cleanUp = &destroy;
	id.type = PercentChart;
	
	return id;
}
