/*
 * cpuInfo.h
 *
 *  Created on: Jan 1, 2014
 *      Author: cody
 */

#ifndef CPUINFO_H_
#define CPUINFO_H_

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

int getNumCPUs();
struct cpuTime parseCPUline(char* str, int len);
int readCPUs(int numCPUs, struct cpuTime* now);
int getCPUtime(struct cpuPercent* cpu, int numCPUs, struct cpuTime* first, struct cpuTime* second);

#endif /* CPUINFO_H_ */
