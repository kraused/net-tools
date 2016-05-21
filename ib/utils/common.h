
#ifndef IB_UTILS_COMMON_H_INCLUDED
#define IB_UTILS_COMMON_H_INCLUDED 1

#include <linux/limits.h>
#include <stdint.h>
#include <inttypes.h>

#define LIKELY(x)	__builtin_expect(!!(x), 1)
#define UNLIKELY(x)	__builtin_expect(!!(x), 0)

#define PACKED		__attribute__((packed))
#define UNUSED		__attribute__((unused))

typedef uint8_t		UInt8;
typedef  int8_t		SInt8;

typedef uint16_t	UInt16;
typedef  int16_t	SInt16;

typedef uint32_t	UInt32;
typedef  int32_t	SInt32;

typedef uint64_t	UInt64;
typedef  int64_t	SInt64;

static inline UInt16 ntoh16(UInt16 x)
{
	return ((x & 0x00FF) << 8) |
	       ((x & 0xFF00) >> 8);
}

static inline UInt16 hton16(UInt16 x)
{
	return ntoh16(x);
}

static inline UInt64 ntoh64(UInt64 x)
{
	return ((x & 0x00000000000000FFULL) << 56) |
	       ((x & 0x000000000000FF00ULL) << 40) |
	       ((x & 0x0000000000FF0000ULL) << 24) |
	       ((x & 0x00000000FF000000ULL) <<  8) |
	       ((x & 0x000000FF00000000ULL) >>  8) |
	       ((x & 0x0000FF0000000000ULL) >> 24) |
	       ((x & 0x00FF000000000000ULL) >> 40) |
	       ((x & 0xFF00000000000000ULL) >> 56);
}

static inline UInt64 hton64(UInt64 x)
{
	return ntoh64(x);
}

/* Allocation function.
 */
typedef void *(*AllocFunction)(void *, void *, UInt64, UInt64);

/* Default memory allocation routine. Failures are fatal.
 */
void *defaultMemoryAlloc(void *ud, void *ptr, UInt64 osize, UInt64 nsize);

#define IB_SYSFS_DIR	"/sys/class/infiniband"
#define IB_PORT_ACTIVE	4

/* Query the LID of the local port.
 */
SInt32 portLocalIdentifier(const char *CA, SInt16 port);

/* Query the LID of the active SM.
 */
SInt32 subnetManagerLocalIdentifier(const char *CA, SInt16 port);

#include "error.h"

#endif

