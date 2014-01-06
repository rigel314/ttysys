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
	struct timeval wait = {.tv_sec = 1, .tv_usec = 0};
	fd_set set;
	long numEvents;
	
	// read once.
	readCPUs(numCPUs, first);

	// Zero out the set, then add 0 to the set to monitor stdin for keypresses.
	FD_ZERO(&set);
	FD_SET(0, &set);
	// Wait 1 second. Or gather input.
	select(1, &set, NULL, NULL, &wait);
	
	// If input was gathered.
	if(FD_ISSET(0, &set))
		return 1;
	
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
