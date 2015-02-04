/*
 * 		GlobalHelper.h
 *
 * 	 	Created on: 27.08.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#ifndef GLOBALHELPER_H_
#define GLOBALHELPER_H_

#include <ctime>

long long GetMillisecondTime () {
	struct timeval  te;
	gettimeofday(&te, 0);

	return te.tv_sec*1000LL + te.tv_usec/1000; // calculate milliseconds
}

#endif /* GLOBALHELPER_H_ */
