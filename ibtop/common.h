
#ifndef IBTOP_FABRIC_COMMON_H_INCLUDED
#define IBTOP_FABRIC_COMMON_H_INCLUDED 1

#define unlikely(x)	__builtin_expect(!!(x), 0)
#define   likely(x)	__builtin_expect(!!(x), 1)

#define MAX(x,y)	(((x) > (y)) ? (x) : (y))
#define MIN(x,y)	(((x) < (y)) ? (x) : (y))

#endif

