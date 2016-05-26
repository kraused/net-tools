
/* ibutil006: Map GUIDs to LIDs.
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
	SInt32 numGUIDs;
	UInt64 *GUIDs;
	UInt16 *LIDs;
	SInt32 i;

	err = firstActivePort(defaultMemoryAlloc, NULL, &CA, &port, &lid);
	if (UNLIKELY(err < 0)) {
		ERROR("firstActivePort() failed");
	}

	numGUIDs = 0;
	GUIDs    = NULL;

	if (argc > 1) {
		numGUIDs = argc - 1;
		GUIDs    = defaultMemoryAlloc(NULL, NULL, 0, numGUIDs*sizeof(UInt16));

		for (i = 0; i < numGUIDs; ++i ) {
			GUIDs[i] = strtoull(argv[i + 1], NULL, 16);
			if (UNLIKELY(0 == GUIDs[i])) {
				FATAL("Invalid lid in slot %d", i);
			}
		}
	}

	getLIDsFromGUIDs(defaultMemoryAlloc, NULL, CA, port,
	                 numGUIDs, GUIDs, &LIDs);

	for (i = 0; i < numGUIDs; ++i) {
		fprintf(stdout, "0x%" PRIx64 " %d\n", GUIDs[i], LIDs[i]);
	}
	defaultMemoryAlloc(NULL, LIDs, numGUIDs*sizeof(struct NodeRecord), 0);

	return 0;
}

