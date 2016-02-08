/*
 * signals.h
 *
 *  Created on: Dec 2, 2015
 *      Author: cody
 */

#ifndef SIGNALS_H_
#define SIGNALS_H_

#include <signal.h>

extern sig_atomic_t got_sigitimer;

void sigHandler(int sig);
void createTimer(int frequency);
void registerHandlers();

#endif /* SIGNALS_H_ */