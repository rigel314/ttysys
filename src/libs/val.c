/*
 * val.c
 *
 *  Created on: Jun 5, 2016
 *      Author: cody
 */

#include "../ttysys_api.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "val.h"

void printTitle(char* file)
{
	char* fname = strrchr(file, '/');
	if(fname)
		setTitle(fname);
	else
		setTitle(file);
}

int nextVal(void** context, float* outs)
{
	FILE* fp = fopen(*context, "r");
	if(!fp)
	{
		outs[0] = 0;
		return 1;
	}
	
	fscanf(fp, "%f", &outs[0]);
	fclose(fp);
	
	return 0;
}

void destroy(void** context)
{
	free(*context);
}

struct initData init(void** context, int argc, char** argv)
{
	struct initData id = {initStatus_GeneralFailure, NULL, NULL};
	
	if(argc == 2)
	{
		*context = malloc(strlen(argv[1])+1);
		
		FILE* fp = fopen(argv[1], "r");
		if(!fp)
		{
			id.status = initStatus_ArgFailure;
			return id;
		}
		fclose(fp);
		
		id.status = initStatus_Success;
		
		memcpy(*context, argv[1], strlen(argv[1])+1);
	}
	else
	{
		id.status = initStatus_ArgFailure;
		return id;
	}
	
	id.status = initStatus_Success;
	printTitle(*context);
	id.nextValue = &nextVal;
	id.cleanUp = &destroy;
	id.type = ScaledValueChart;
	
	return id;
}

