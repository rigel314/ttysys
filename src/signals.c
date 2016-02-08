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

sig_atomic_t got_sigitimer = false;

void sigHandler(int sig)
{
	;
}

void createTimer(int frequency)
{
	struct itimerval it;
	struct timeval tv;
	
	tv.tv_sec = 0;
	tv.tv_usec = 1000000/frequency;
	
	it.it_interval = tv;
	it.it_value = tv;
	
	setitimer(ITIMER_REAL, &it, NULL);
}

void registerHandlers()
{
	;
}
