
/* ibutil002: Query the LFT of switches. A list of lids can be parsed to the program. If
 *            no lid is given it will return the LFTs from all switches.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "ibutil.h"
#include "adapter.h"

int main(int argc, char **argv)
{
	char   *CA;
	SInt16 port;
	UInt16 lid;
	SInt32 err;
	SInt32 numLIDs;
	UInt16 *LIDs;
	SInt32 i, j;
	SInt32 numLFTs;
	struct LinearForwardingTable *LFTs;

	err = firstActivePort(defaultMemoryAlloc, NULL, &CA, &port, &lid);
	if (UNLIKELY(err < 0)) {
		ERROR("firstActivePort() failed");
	}

	numLIDs = 0;
	LIDs    = NULL;

	if (argc > 1) {
		numLIDs = argc - 1;
		LIDs    = defaultMemoryAlloc(NULL, NULL, 0, numLIDs*sizeof(UInt16));

		for (i = 0; i < numLIDs; ++i ) {
			LIDs[i] = strtol(argv[i + 1], NULL, 10);
			if (UNLIKELY(0 == LIDs[i])) {
				FATAL("Invalid lid in slot %d", i);
			}
		}
	}

	getLinearForwardingTables(defaultMemoryAlloc, NULL, CA, port,
                                  numLIDs, LIDs, &numLFTs, &LFTs);

	for (i = 0; i < numLFTs; ++i) {
		fprintf(stdout, "%4d\n", (int )LFTs[i].lid);
		for (j = 1; j < LFTs[i].len; ++j) {
			fprintf(stdout, "\t%4d %4d\n", j, LFTs[i].lft[j]);
		}

		defaultMemoryAlloc(NULL, LFTs[i].lft, LFTs[i].len*sizeof(UInt16), 0);
	}
	defaultMemoryAlloc(NULL, LFTs, numLFTs*sizeof(struct LinearForwardingTable), 0);

	return 0;
}

