/*
 * ttysys_api.h
 *
 *  Created on: Feb 5, 2016
 *      Author: cody
 */

#ifndef SRC_TTYSYS_API_H_
#define SRC_TTYSYS_API_H_

#define TITLE_LEN 100

enum winType { VoidChart, PercentChart, ScaledValueChart, TextChart, UpDownChart };

struct initData;

// You must have a function called 'init' that matches this signature:
typedef struct initData (initFunc)(void** context, int argc, char** argv);
struct initData init(void** context, int argc, char** argv);

// You must have a function that matches this signature
typedef int (nextValueFunc)(void** context, float* vals);

// You may define a function to cleanup anything that needs cleaned when properly exiting.
typedef void (cleanupFunc)(void** context);

enum initStatus {initStatus_Success, initStatus_ArgFailure, initStatus_GeneralFailure = 255};

// When a plugin returns this, NULL means ttysys won't call the function.
struct initData
{
	enum initStatus status;
	nextValueFunc* nextValue;
	cleanupFunc* cleanUp;
	enum winType type;
};

// You can call this during any of your functions to set the title of the window your function is called for. 
extern void setTitle(char* test);

extern int getRefreshRate();

#endif /* SRC_TTYSYS_API_H_ */
