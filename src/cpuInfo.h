/*
 * cpuInfo.h
 *
 *  Created on: Jan 1, 2014
 *      Author: cody
 */

#ifndef CPUINFO_H_
#define CPUINFO_H_

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

int getNumCPUs();
struct cpuTime parseLine(char* str, int len);
int readCPUs(int numCPUs, struct cpuTime* now);
int getCPUtime(struct cpuPercent* cpu, int numCPUs, struct cpuTime* first, struct cpuTime* second);

#endif /* CPUINFO_H_ */
