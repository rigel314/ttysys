/*
 * mem.c
 *
 *  Created on: Sep 11, 2015
 *      Author: cody
 */

#include "../ttysys_api.h"
#include <stdio.h>

int nextVal(float* outs)
{
	float out = 0;
	FILE* fp = fopen("/dev/urandom", "r");
	out = (float)fgetc(fp);
	fclose(fp);
	outs[0] = out*100.0/255;
	return 0;
}
