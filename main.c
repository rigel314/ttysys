/*
 * main.c
 *
 *  Created on: Dec 26, 2013
 *      Author: cody
 */

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

struct cpuTime
{
	long total;
	long user;
	long sys;
};
struct cpuPercent
{
	float total;
	float user;
	float sys;
};

int getNumCPUs();
int strchrCount(char* s, char c);
struct cpuTime parseLine(char* str, int len);
int readCPUs(int numCPUs, struct cpuTime* now);
int getCPUtime(struct cpuPercent* cpu, int numCPUs, struct cpuTime* first, struct cpuTime* second);

int main(int argc, char** argv)
{
	int c;
	struct cpuTime* then;
	struct cpuTime* now;
	struct cpuPercent* cpu;
	int numCPUs = getNumCPUs();
	
	// creating structs
	then = malloc(sizeof(struct cpuTime) * (numCPUs+1));
	now = malloc(sizeof(struct cpuTime) * (numCPUs+1));
	cpu = malloc(sizeof(struct cpuPercent) * (numCPUs+1));
	
//	while(1)
//	{
//		getCPUtime(cpu, numCPUs, then, now);
//		for(int i = 0; i <= numCPUs; i++)
//			printf("%.2f%%\t%.2f%%\t%.2f%%\n", cpu[i].total, cpu[i].user, cpu[i].sys);
//		printf("\n");
//	}
//	readCPUs(numCPUs, now);
//	return 0;
	
	// ncurses init stuff
	initscr();
	raw();
	noecho();
	curs_set(0);
	keypad(stdscr,TRUE);
	halfdelay(3);
	refresh();
	
	while((c = getch()) != 10)
	{
		getCPUtime(cpu, numCPUs, then, now);
		clear();
		for(int i = 0; i < numCPUs + 1; i++)
			mvprintw(i, 0, "%.2f%%\t%.2f%%\t%.2f%%\n", cpu[i].total, cpu[i].user, cpu[i].sys);
		refresh();
	}
	
	endwin();
	return 0;
}

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

int strchrCount(char* s, char c)
{
	int i;
	for (i = 0;
			s[i];
			s[i]==c ? i++ : (long) s++);
	return i;
}

struct cpuTime parseLine(char* str, int len)
{
	char copy[len+1];
	char** args;
	int j = 1;
	struct cpuTime out;
	
	args = malloc((strchrCount(str, ' ') + 1) * sizeof(char*));
	
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
	
	out.user = atoi(args[1]) + atoi(args[2]);
	out.sys = atoi(args[3]);
	out.total = out.user + out.sys;
	
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
//		printf("%d\n", i);
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
	struct timespec wait = {.tv_sec = 1, .tv_nsec = 1000000 * 0};
	struct timespec rest;
	long userHz = sysconf(_SC_CLK_TCK);
	
	readCPUs(numCPUs, first);

	// Wait 1 second.
	nanosleep(&wait, &rest);
	
	readCPUs(numCPUs, second);

	// do math to calculate percentage
	for(int i = 0; i < numCPUs + 1; i++)
	{
		float divby = 1;
		
		if(i == 0)
			divby = numCPUs;
		
		cpu[i].total = (float) (second[i].total - first[i].total) / (float) userHz * 100.0 / divby;
		cpu[i].user = (float) (second[i].user - first[i].user) / (float) userHz * 100.0 / divby;
		cpu[i].sys = (float) (second[i].sys - first[i].sys) / (float) userHz * 100.0 / divby;
	}

	return 0;
}
