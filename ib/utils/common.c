
#include <stdlib.h>
#include <stdio.h>	/* snprintf() */
#include <string.h>
#include <inttypes.h>
#include <errno.h>

#include <unistd.h>
#include <fcntl.h>

#include "common.h"

void *defaultMemoryAlloc(void *UNUSED ud, void *ptr, UInt64 osize, UInt64 nsize)
{
	void *qtr;

	if (0 == nsize) {
		free(ptr);
		return NULL;
	}

	qtr = realloc(ptr, nsize);
	if (UNLIKELY(!qtr)) {
		FATAL("Failed to allocate %" PRIu64 "bytes", nsize);
	}

	return qtr;
}

/* Read an integer from a sysfs file. path and buf may alias.
 */
static SInt32 sysfsReadInt(char *path, char *buf, SInt64 bufLen, SInt32 base)
{
	int  fd;
	SInt64 n;

	fd = open(buf, O_RDONLY);
	/* Not necessarily an error.
	 */
	if (UNLIKELY(fd < 0)) {
		ERROR("open() failed with error %d: %s", errno, strerror(errno));
		return -1;
	}

	n = read(fd, buf, bufLen - 1);
	if (UNLIKELY(n < 0)) {
		ERROR("read() failed with error %d: %s", errno, strerror(errno));
		return -1;
	}
	if (UNLIKELY(n == (bufLen - 1))) {
		ERROR("read(): message truncated");
		return -1;
	}

	close(fd);

	return strtol(buf, NULL, base);
}

SInt32 portLocalIdentifier(const char *CA, SInt16 port)
{
	char   buf[1024];
	SInt64 n;

	n = snprintf(buf, sizeof(buf), "%s/%s/ports/%d/lid", IB_SYSFS_DIR, CA, port);
	if (UNLIKELY((n < 0) || (n >= sizeof(buf)))) {
		ERROR("snprintf() returned %d", n);
		return -1;
	}

	return sysfsReadInt(buf, buf, sizeof(buf), 16);
}

SInt32 subnetManagerLocalIdentifier(const char *CA, SInt16 port)
{
	char   buf[1024];
	SInt64 n;

	n = snprintf(buf, sizeof(buf), "%s/%s/ports/%d/sm_lid", IB_SYSFS_DIR, CA, port);
	if (UNLIKELY((n < 0) || (n >= sizeof(buf)))) {
		ERROR("snprintf() returned %d", n);
		return -1;
	}

	return sysfsReadInt(buf, buf, sizeof(buf), 16);
}

