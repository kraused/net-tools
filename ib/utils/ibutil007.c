
/* ibutil007: Query MC member records.
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
	SInt32 i;
	SInt32 numMRs;
	struct MCMemberRecord *MRs;
	char mgid[INET6_ADDRSTRLEN];

	err = firstActivePort(defaultMemoryAlloc, NULL, &CA, &port, &lid);
	if (UNLIKELY(err < 0)) {
		ERROR("firstActivePort() failed");
	}

	getMCMemberRecords(defaultMemoryAlloc, NULL, CA, port, &numMRs, &MRs);

	for (i = 0; i < numMRs; ++i) {
		printf("%-32s 0x%x 0x%x 0x%x 0x%x\n", inet_ntop(AF_INET6, MRs[i].mgid, mgid, sizeof(mgid)), MRs[i].mlid, MRs[i].pkey, MRs[i].scope, MRs[i].joinState);
	}

	defaultMemoryAlloc(NULL, MRs, numMRs*sizeof(struct MCMemberRecord), 0);

	return 0;
}

