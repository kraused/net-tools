
/* ibutil009: Leave a MC group
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#include "common.h"
#include "ibutil.h"
#include "adapter.h"

int main(int argc, char **argv)
{
	char   *CA;
	SInt16 port;
	UInt16 lid;
	SInt32 err;
	UInt8  mgid[INET6_ADDRSTRLEN];

	err = firstActivePort(defaultMemoryAlloc, NULL, &CA, &port, &lid);
	if (UNLIKELY(err < 0)) {
		ERROR("firstActivePort() failed");
	}

	err = inet_pton(AF_INET6, argv[1], mgid);
	if (UNLIKELY(1 != err)) {
		ERROR("inet_pton() failed");
	}

	leaveMCGroup(defaultMemoryAlloc, NULL, CA, port, mgid);

	return 0;
}

