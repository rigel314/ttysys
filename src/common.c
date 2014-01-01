/*
 * common.c
 *
 *  Created on: Jan 1, 2014
 *      Author: cody
 */

int strchrCount(char* s, char c)
{
	int i;
	for (i = 0;
			s[i];
			s[i]==c ? i++ : (long) s++);
	return i;
}


void listShiftLeftAdd(float* list, int len, float new)
{
	for(int i = 0; i < len-1; i++)
	{
		list[i] = list[i+1];
	}
	list[len-1] = new;
}

void listShiftRightAdd(float* list, int len, float new)
{
	for(int i = len - 1; i > 0; i--)
	{
		list[i] = list[i-1];
	}
	list[0] = new;
}
