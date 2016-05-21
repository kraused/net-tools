
/* ibutil004: Query node records. A list of lids can be parsed to the program. If
 *            no lid is given it will return the node records of all components.
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
	SInt32 i;
	SInt32 numNRs;
	struct NodeRecord *NRs;

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

	getNodeRecords(defaultMemoryAlloc, NULL, CA, port,
                       numLIDs, LIDs, &numNRs, &NRs);

	for (i = 0; i < numNRs; ++i) {
		fprintf(stdout, "%d %d %d 0x%" PRIx64 " \"%s\"\n", (int )NRs[i].lid, (int )NRs[i].nodeType, (int )NRs[i].numPorts, NRs[i].portGuid, NRs[i].info);
	}
	defaultMemoryAlloc(NULL, NRs, numNRs*sizeof(struct NodeRecord), 0);

	return 0;
}

