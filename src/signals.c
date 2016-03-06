/*
 * signals.c
 *
 *  Created on: Dec 2, 2015
 *      Author: cody
 */

#include <stdbool.h>
#include <sys/time.h>
#include <stdlib.h>
#include "signals.h"

volatile sig_atomic_t got_sigitimer = false;

void sigHandler(int sig)
{
	got_sigitimer = true;
}

int createTimer(int frequency)
{
	struct itimerval it;
	struct timeval tv;
	
	tv.tv_sec = 0;
	tv.tv_usec = 1000000/frequency;
	
	it.it_interval = tv;
	it.it_value = tv;
	
	return setitimer(ITIMER_REAL, &it, NULL);
}

int registerHandlers()
{
	int retval = 0;
	struct sigaction sa;

	sa.sa_handler = sigHandler;
	sa.sa_flags = 0; // or SA_RESTART
	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask,SIGALRM);
	
	if (sigaction(SIGALRM,&sa,NULL) == -1)
	{
		retval |= 1;
	}
	
	return retval;
}
