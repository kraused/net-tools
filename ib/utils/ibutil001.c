
/* ibutil001: Query first usable (HCA, port) combination.
 */

#include <stdio.h>

#include "common.h"
#include "adapter.h"

int main(int argc, char **argv)
{
	char   *CA;
	SInt16 port;
	SInt32 err;
	UInt16 lid;

	err = firstActivePort(defaultMemoryAlloc, NULL, &CA, &port, &lid);
	if (UNLIKELY(err < 0)) {
		ERROR("firstActivePort() failed");
	}

	fprintf(stdout, "First usable device: '%s' port %d (lid %d)\n", CA, port, lid);

	return 0;
}

