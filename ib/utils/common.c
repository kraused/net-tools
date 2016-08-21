
#include <stdlib.h>
#include <stdio.h>	/* snprintf() */
#include <string.h>
#include <inttypes.h>
#include <errno.h>
#include <arpa/inet.h>

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

static SInt32 sysfsRead(char *path, char *buf, SInt64 bufLen, SInt64 *len)
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
	buf[n] = 0;

	if (LIKELY(*len)) {
		*len = n;
	}

	close(fd);

	return 0;
}

/* Read an integer from a sysfs file. path and buf may alias.
 */
static SInt32 sysfsReadInt(char *path, char *buf, SInt64 bufLen, SInt32 base)
{
	SInt32 err;
	SInt64 len;

	err = sysfsRead(path, buf, bufLen, &len);
	if (UNLIKELY(err))
		return -1;

	if ((len > 0) && ('\n' == buf[len - 1])) {
		buf[--len] = 0;
	}

	return strtol(buf, NULL, base);
}

static SInt32 sysfsReadInet6(char *path, char *buf, SInt64 bufLen, UInt8 *addr)
{
	SInt32 err;
	SInt64 len;

	err = sysfsRead(path, buf, bufLen, &len);
	if (UNLIKELY(err))
		return -1;

	if ((len > 0) && ('\n' == buf[len - 1])) {
		buf[--len] = 0;
	}

	err = inet_pton(AF_INET6, buf, addr);
	if (UNLIKELY(1 != err)) {
		ERROR("inet_pton() returned %d\n", err);
		return -1;
	}

	return 0;
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

SInt32 portGlobalIdentifier(const char *CA, SInt16 port, SInt16 whichGid, UInt8 *gid)
{
	char   buf[1024];
	SInt64 n;

	n = snprintf(buf, sizeof(buf), "%s/%s/ports/%d/gids/%d", IB_SYSFS_DIR, CA, port, whichGid);
	if (UNLIKELY((n < 0) || (n >= sizeof(buf)))) {
		ERROR("snprintf() returned %d", n);
		return -1;
	}

	return sysfsReadInet6(buf, buf, sizeof(buf), gid);
}

