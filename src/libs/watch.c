/*
 * watch.c
 *
 *  Created on: Oct 14, 2017
 *      Author: cody
 */

#include "../ttysys_api.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include "watch.h"

#define BUFSIZE 65536
#define TIMEOUT .5

int nextVal(void** context, float* outs)
{
	int fds[2];
	static char buf[BUFSIZE];
//	int sret;
//	int err;
	int len = 0;
	struct watchinfo* wi = *context;
	struct timeval tvloop;
	struct timeval tv1, tv2;
	int* blar = &errno;
	fd_set set;
	bool shouldkill = 0;
	
	if(pipe(fds))
	{
		return 1;
	}
	pid_t pid = fork();
	if(pid < 0)
	{
		return 1;
	}
	if(!pid)
	{
		close(1);
		close(2);
		close(fds[0]);
		dup(fds[1]);
		dup(fds[1]);
		execl("/bin/sh", "/bin/sh", "-c", wi->cmd, (char*)NULL);
		exit(1);
	}
	close(fds[1]);
	
	gettimeofday(&tv1, NULL);
	
	while(1)
	{
		int ret;
		FD_ZERO(&set);
		FD_SET(fds[0], &set);
		
		tvloop.tv_sec = 0;
		tvloop.tv_usec = 1000;
		
		ret = select(fds[0]+1, &set, NULL, NULL, &tvloop);
		if(ret > 0)
		{
			ret = read(fds[0], buf+len, BUFSIZE-len-1);
			if(ret < 0)
			{
				ret = 0;
			}
			if(ret == 0)
			{
				break;
			}
			len += ret;
		}
//		if(ret < 0)
//		{
//			shouldkill = true;
//		}
		
		gettimeofday(&tv2, NULL);
		float elapsed = (tv2.tv_sec+(double)tv2.tv_usec/1000000.0) - (tv1.tv_sec+(double)tv1.tv_usec/1000000.0);
		if(len >= BUFSIZE-1 || elapsed > TIMEOUT || shouldkill)
		{
			kill(pid, SIGKILL);
			break;
		}
	}
	
	close(fds[0]);
	buf[len] = '\0';
	if(len > 0)
		setText(buf);
	
	return 0;
}

void destroy(void** context)
{
	free(*context);
}

struct initData init(void** context, int argc, char** argv)
{
	struct initData id = {initStatus_GeneralFailure, NULL, NULL};
	char title[TITLE_LEN];
	
	*context = malloc(sizeof(struct watchinfo));
	struct watchinfo* wi = *context;

	if(argc != 2)
	{
		id.status = initStatus_ArgFailure;
		return id;
	}
	
	strcpy(wi->cmd, argv[1]);
	
	sprintf(title, "every %1.1fs: %s", (double)getRefreshRate() / TIMER_FREQ, argv[1]);
	setTitle(title);
	
	id.status = initStatus_Success;
	id.nextValue = &nextVal;
	id.cleanUp = &destroy;
	id.type = TextChart;
	
	return id;
}
