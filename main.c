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
#include <math.h>
#include <sys/select.h>

struct cpuTime
{
	unsigned long long total;
	long user;
	long sys;
};
struct cpuPercent
{
	float total;
	float user;
	float sys;
};

void listShiftLeftAdd(float* list, int len, float new);
void drawScreen(char* buf, float* list, int width, int height);
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
	char* screen;
	float* list;
	int listLen;
	int rows;
	int columns;
	
	// creating structs
	then = malloc(sizeof(struct cpuTime) * (numCPUs+1));
	now = malloc(sizeof(struct cpuTime) * (numCPUs+1));
	cpu = malloc(sizeof(struct cpuPercent) * (numCPUs+1));
	
	// ncurses init stuff
	initscr();
	raw();
	noecho();
	curs_set(0);
	keypad(stdscr,TRUE);
	halfdelay(3);
	refresh();
	
	rows = LINES;
	columns = COLS;
	listLen = COLS;
	screen = calloc(COLS*LINES, sizeof(char));
	list = calloc(COLS, sizeof(float));
	
	while((c = getch()) != 10)
	{
		getCPUtime(cpu, numCPUs, then, now);
		clear();
//		for(int i = 0; i < numCPUs + 1; i++)
//			mvprintw(i, 0, "%.2f%%\t%.2f%%\t%.2f%%\n", cpu[i].total, cpu[i].user, cpu[i].sys);
		listShiftLeftAdd(list, listLen, cpu[0].total);
		drawScreen(screen, list, columns, rows);
		refresh();
	}
	
	endwin();
	return 0;
}

void listShiftLeftAdd(float* list, int len, float new)
{
	for(int i = 0; i < len-1; i++)
	{
		list[i] = list[i+1];
	}
	list[len-1] = new;
}

void drawScreen(char* buf, float* list, int width, int height)
{
	int indexes[width];
	int axes[5];
	
	for(int i = 1; i < 4; i++)
		axes[i] = roundf((float) height - (float) height * (25.0*i)/100.0) - 1;
	
	for(int i = 0; i < width; i++)
	{
		indexes[i] = roundf((float) height - (float) height * list[i]/100.0) - 1;
		
		for(int j = indexes[i]; j < height; j++)
			mvprintw(j, i, "*");
	}
	
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
	struct timeval wait = {.tv_sec = 1, .tv_usec = 0};
	fd_set set;
	long numEvents;
	
	readCPUs(numCPUs, first);

	// Zero out the set, then add 0 to the set to monitor stdin for keypresses.
	FD_ZERO(&set);
	FD_SET(0, &set);
	// Wait 1 second. Or gather input.
	select(1, &set, NULL, NULL, &wait);
	
	readCPUs(numCPUs, second);

	// do math to calculate percentage
	for(int i = 0; i < numCPUs + 1; i++)
	{
		float divby = 1;
		
		if(i == 0)
			divby = numCPUs;
		
		numEvents = second[i].total - first[i].total;
		if(numEvents == 0)
			return 0;
		
//		cpu[i].total = (float) (second[i].total - first[i].total) / (float) userHz * 100.0 / divby;
		cpu[i].user = (float) (second[i].user - first[i].user) / (float) numEvents * 100.0 / divby;
		cpu[i].sys = (float) (second[i].sys - first[i].sys) / (float) numEvents * 100.0 / divby;
		cpu[i].total = cpu[i].user + cpu[i].sys;
	}

	return 0;
}
