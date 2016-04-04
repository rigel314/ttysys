/*
 * ttysys_api.h
 *
 *  Created on: Feb 5, 2016
 *      Author: cody
 */

#ifndef SRC_TTYSYS_API_H_
#define SRC_TTYSYS_API_H_

#define TITLE_LEN 100

enum winType { VoidChart, PercentChart, ValueChart, TextChart };

struct initData;

// You should have a function called 'init' that matches this signature:
typedef struct initData (initFunc)(void** context, int argc, char** argv);
struct initData init(void** context, int argc, char** argv);

typedef int (nextValueFunc)(void** context, float* vals);
typedef int (titleFunc)(void** context, char title[TITLE_LEN]);
typedef void (cleanupFunc)(void** context);

// When a plugin returns this, NULL means ttysys won't call the function.
struct initData
{
	nextValueFunc* nextValue;
	cleanupFunc* cleanUp;
	titleFunc* title;
	enum winType type;
};

#endif /* SRC_TTYSYS_API_H_ */
