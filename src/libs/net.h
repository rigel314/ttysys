/*
 * net.h
 *
 *  Created on: Sep 3, 2017
 *      Author: cody
 */

#ifndef SRC_LIBS_NET_H_
#define SRC_LIBS_NET_H_

#include <stdbool.h>

// Usual min and max macros.
#define min(x, y) ((x)<(y)?(x):(y))
#define max(x, y) ((x)>(y)?(x):(y))

#define IFACE_LEN 20

struct netinfolast
{
	unsigned long long rbytes;
	unsigned long long tbytes;
};
struct netinfo
{
	struct netinfolast last;
	int rbytescol;
	int tbytescol;
	bool valid;
	char iface[IFACE_LEN];
};

#endif /* SRC_LIBS_NET_H_ */
