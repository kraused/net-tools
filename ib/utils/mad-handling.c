
#include "mad-handling.h"

#include <errno.h>

void libibumad_Enable_Debugging()
{
	umad_debug(5);
}

void libibumad_Send_MAD(int fd, int agent, UInt8 *umad, UInt64 len, SInt32 timeout, SInt32 nretries)
{
	SInt32 err;

	err = umad_send(fd, agent, umad, len, timeout, nretries);
	if (UNLIKELY(err < 0)) {
		FATAL("umad_send() failed with error %d: %s", -err, strerror(-err));
	}
}

void libibumad_Recv_MAD(AllocFunction alloc, void *allocUd, int fd, UInt8 **buf, SInt64 *len, SInt32 timeout)
{
	SInt32 err, length;

	*len = 1024;
	*buf = alloc(allocUd, NULL, 0, (*len));
	if (UNLIKELY(!(*buf))) {
		FATAL("alloc() returned NULL.");
	}
	memset((*buf), 0, (*len));

	do {
		length = (*len);
		err    = umad_recv(fd, (*buf), &length, timeout);
		if (err >= 0) {
			*len = length;
			break;
		}

		if (ENOSPC == (-err)) {
			*buf = alloc(allocUd, (*buf), (*len), length + umad_size());
			if (UNLIKELY(!(*buf))) {
				FATAL("alloc() returned NULL.");
			}
			*len = length + umad_size();
			memset((*buf), 0, (*len));
		} else {
			FATAL("umad_recv() failed with error %d: %s", -err, strerror(-err));
		}
	} while(1);
}

