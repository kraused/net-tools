
#ifndef IBTOP_FABRIC_UTILS_H_INCLUDED
#define IBTOP_FABRIC_UTILS_H_INCLUDED 1

#include <sys/time.h>


/*
 * Get the time difference (x - y) in microseconds.
 */
static inline unsigned long long timediff_usec(const struct timeval *x, const struct timeval *y)
{
	return (x->tv_sec - y->tv_sec)*1000000L + (x->tv_usec - y->tv_usec);
}

/*
 * Get the time difference (x - y) in milliseconds.
 */
static inline double timediff_msec(const struct timeval *x, const struct timeval *y)
{
	return timediff_usec(x, y)*1e-3;
}

/*
 * Get the time difference (x - y) in microseconds.
 */
static inline double timediff_sec(const struct timeval *x, const struct timeval *y)
{
	return timediff_usec(x, y)*1E-6;
}

#endif

