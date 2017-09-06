/*
 * net.c
 *
 *  Created on: Sep 3, 2017
 *      Author: cody
 */

#include "../ttysys_api.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "net.h"

void printTitle(char* iface)
{
	int x = strlen(iface);
	char msg[IFACE_LEN+100];
	
	strcpy(msg, iface);
	
	if(x == 0)
	{
		strcpy(msg, "All  Ifaces");
	}
	else if(msg[x-1] == ':')
	{
		msg[x-1] = '\0';
	}
	else
	{
		strcat(msg, "*");
	}

	strcat(msg, " KiB/s");
	
	setTitle(msg);
}

int readNets(unsigned long long* rx, unsigned long long* tx, const struct netinfo* ni)
{
	char* line = NULL;
	FILE* fp;
	size_t dum = 0;
	ssize_t err;
	
	*rx = *tx = 0;
	
	fp = fopen("/proc/net/dev", "r");
	if(!fp)
		return 0;

	// read one line.
	while((err = getline(&line, &dum, fp)) != -1)
	{
		// lines with values have a colon
		if(strchr(line, ':'))
		{
			char* subline = line;
			
			while(*subline == ' ')
				subline++;
			
			if(!strncmp(ni->iface, subline, strlen(ni->iface)))
			{
				int count = 0;
				
				while(count < min(ni->rbytescol, ni->tbytescol))
				{
					while(*subline != ' ')
						subline++;
					while(*subline == ' ')
						subline++;
					count++;
				}
				
				unsigned long addVal = strtoul(subline, NULL, 10);
				
				if(count == ni->rbytescol)
					*rx += addVal;
				if(count == ni->tbytescol)
					*tx += addVal;
				
				while(count < max(ni->rbytescol, ni->tbytescol))
				{
					while(*subline != ' ')
						subline++;
					while(*subline == ' ')
						subline++;
					count++;
				}
				
				addVal = strtoul(subline, NULL, 10);
				
				if(count == ni->rbytescol)
					*rx += addVal;
				if(count == ni->tbytescol)
					*tx += addVal;
			}
		}
		
		free(line);
		line = NULL;
	}
	
	fclose(fp);
	
	return 1;
}

int nextVal(void** context, float* outs)
{
	struct netinfo* ni = *context;
	unsigned long long tx, rx;
	
	readNets(&rx, &tx, ni);
	
	printTitle(ni->iface);
	
	if(!ni->valid)
	{
		ni->valid = true;
		outs[0] = outs[1] = 0;
		ni->last.rbytes = rx;
		ni->last.tbytes = tx;
		return 0;
	}
	
	float div = getRefreshRate() / ((float)TIMER_FREQ) * 1024.0f;
	outs[0] = ((float)(tx - ni->last.tbytes)) / div;
	outs[1] = ((float)(rx - ni->last.rbytes)) / div;
	
	ni->last.rbytes = rx;
	ni->last.tbytes = tx;
	
	return 0;
}

void destroy(void** context)
{
	free(*context);
}

struct initData init(void** context, int argc, char** argv)
{
	struct initData id = {initStatus_GeneralFailure, NULL, NULL};
	
	*context = calloc(1, sizeof(struct netinfo));
	struct netinfo* ni = *context;
	strcpy(ni->iface, "");
	
	ni->rbytescol = 1;
	ni->tbytescol = 9;
	
	if(argc == 2)
	{
		strncpy(ni->iface, argv[1], IFACE_LEN-1);
	}
	else if(argc > 2)
	{
		id.status = initStatus_ArgFailure;
		return id;
	}
	
	printTitle(ni->iface);
	
	id.status = initStatus_Success;
	id.nextValue = &nextVal;
	id.cleanUp = &destroy;
	id.type = UpDownChart;
	
	return id;
}
