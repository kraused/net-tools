
#include <stdlib.h>
#include <stdio.h>	/* snprintf() */
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/syscall.h>

#include "adapter.h"

struct DirectoryEntry
{
	UInt64	ino;
	UInt64	off;
	UInt16	reclen;
	UInt8	type;
	char	name[];
};

static _Bool portIsActive(const char *CA, SInt16 port)
{
	int  fd;
	char buf[1024];
	SInt64 i, n;

	n = snprintf(buf, sizeof(buf), "%s/%s/ports/%d/state", IB_SYSFS_DIR, CA, port);
	if (UNLIKELY((n < 0) || (n >= sizeof(buf)))) {
		ERROR("snprintf() returned %d", n);
		return 0;
	}

	fd = open(buf, O_RDONLY);
	/* Not necessarily an error.
	 */
	if (UNLIKELY(fd < 0)) {
		return 0;
	}

	n = read(fd, buf, sizeof(buf) - 1);
	if (UNLIKELY(n < 0)) {
		ERROR("read() failed with error %d: %s", errno, strerror(errno));
		return 0;
	}
	if (UNLIKELY(n == (sizeof(buf) - 1))) {
		ERROR("read(): message truncated");
		return 0;
	}

	for (i = 0; i < n; ++i) {
		if (':' == buf[i]) {
			buf[i] = 0;
		}
	}

	return (IB_PORT_ACTIVE == strtol(buf, NULL, 10));
}

static SInt16 firstActivePortOfCA(const char *CA)
{
	int  fd;
	char buf[1024];
	SInt64 n, pos;
	SInt16 q, port;
	struct DirectoryEntry *d;

	port = -1;

	n = snprintf(buf, sizeof(buf), "%s/%s/ports", IB_SYSFS_DIR, CA);
	if (UNLIKELY((n < 0) || (n >= sizeof(buf)))) {
		ERROR("snprintf() returned %d", n);
		return -1;
	}

	fd = open(buf, O_RDONLY | O_DIRECTORY);
	if (UNLIKELY(fd < 0)) {
		ERROR("read() failed with error %d: %s", errno, strerror(errno));
		return -1;
	}

	for (;;) {
		n = syscall(SYS_getdents64, fd, buf, sizeof(buf));
		if (UNLIKELY(n < 0)) {
			FATAL("getdents64() failed with error %d: %s", errno, strerror(errno));
		}
		if (0 == n) {
			break;
		}

		for (pos = 0; pos < n;) {
			d = (struct DirectoryEntry *)(buf + pos);

			pos += d->reclen;
			/* Skip '.' and '..' 
			 */
			if ((d->name[0] < 0x30) || (d->name[0] > 0x39)) {
				continue;
			}

			q = strtol(d->name, NULL, 10);
			if (portIsActive(CA, q)) {
				port = q;
				break;
			}
		}
	}

	close(fd);

	return port;
}

SInt32 firstActivePort(AllocFunction alloc, void *allocUd, char **CA, SInt16 *port, UInt16 *lid)
{
	int  fd;
	char buf[1024];
	SInt64 n, pos;
	SInt16 q;
	struct DirectoryEntry *d;

	*CA   = NULL;
	*port = -1;

	fd = open(IB_SYSFS_DIR, O_RDONLY | O_DIRECTORY);
	for (;;) {
		n = syscall(SYS_getdents64, fd, buf, sizeof(buf));
		if (UNLIKELY(n < 0)) {
			FATAL("getdents64() failed with error %d: %s", errno, strerror(errno));
		}
		if (0 == n) {
			break;
		}

		for (pos = 0; pos < n;) {
			d = (struct DirectoryEntry *)(buf + pos);
			
			pos += d->reclen;
			/* The relevant entries are links to the PCIe device sysfs entry.
 			 */
			if (DT_LNK != d->type)
				continue;

			q = firstActivePortOfCA(d->name);

			if (LIKELY(q >= 0)) {
				*port = q;

				n   = strlen(d->name);
				*CA = alloc(allocUd, NULL, 0, (n + 1));
				*((char *)mempcpy(*CA, d->name, n)) = 0;

				break;
			}
		}


		if (LIKELY(*CA)) {
			break;
		}		
	}

	close(fd);

	if (!(*CA)) {
		return -1;
	}

	if (lid) {
		*lid = portLocalIdentifier((*CA), *port);

		if (UNLIKELY((*lid) < 1)) {
			*CA   = alloc(allocUd, (*CA), 0, 0);
			*port = -1;

			return -1;
		}
	}

	return 0;
}

