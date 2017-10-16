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
#include <sys/wait.h>
#include <errno.h>
#include "watch.h"

int nextVal(void** context, float* outs)
{
	int fds[2];
	static char buf[65536];
	int sret;
	int err;
	int len = 0;
	struct watchinfo* wi = *context;
	struct timeval tv = { .tv_sec = 0, .tv_usec = 100000 };
	int* blar = &errno;
	
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
	
	sret = select(0, NULL, NULL, NULL, NULL);
	err = errno;
	usleep(10000);
	len = read(fds[0], buf, 65535);
	if(len < 0)
		len = 0;
	if(sret == -1)
	{
		if(err == EINTR)
		{
			if(waitpid(pid, NULL, WNOHANG) != pid)
			{
				kill(pid, SIGKILL);
				usleep(1000);
				waitpid(pid, NULL, WNOHANG);
			}
		}
	}
	else
	{
		kill(pid, SIGKILL);
		usleep(1000);
		waitpid(pid, NULL, WNOHANG);
	}
	
	close(fds[0]);
	buf[len] = '\0';
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
