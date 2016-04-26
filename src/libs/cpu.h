/*
 * cpu.h
 *
 *  Created on: Jan 1, 2014
 *      Author: cody
 */

#ifndef CPU_H_
#define CPU_H_

#include <stdbool.h>

// Holds CPU time in different CPU modes.
struct cpuTime
{
	unsigned long long total;
	long user;
	long sys;
};

// Holds percentages for a CPU.
struct cpuPercent
{
	float total;
	float user;
	float sys;
};

// Plugin Context
struct cpuCtx
{
	int whichCPU;
	int numCPUs;
	struct cpuTime* cpuTimesLast;
	bool valid;
};

int getNumCPUs();
struct cpuTime parseCPUline(char* str, int len);
int readCPUs(int numCPUs, struct cpuTime* now);
int getCPUtime(struct cpuPercent* cpu, int numCPUs, struct cpuTime* first, struct cpuTime* second);

#endif /* CPU_H_ */
