
#ifndef IB_UTILS_IBUTILS_H_INCLUDED
#define IB_UTILS_IBUTILS_H_INCLUDED 1

#include "common.h"
#include "structs.h"

struct LinearForwardingTable
{
	UInt16	lid;
	SInt32	len;
	UInt8	*lft;
};

/* Get the LFTs from selected components or (if 0 == numLIDs) from all components.
 */
void getLinearForwardingTables(AllocFunction alloc, void *allocUd, char *CA, SInt16 port,
                               SInt32 numLIDs, const UInt16 *LIDs,
                               SInt32 *numLFTs, struct LinearForwardingTable **LFTs);

struct NodeRecord
{
	UInt16	lid;
	UInt8	nodeType;
	UInt8	numPorts;
	UInt64	sysGuid;
	UInt64	nodeGuid;
	UInt64	portGuid;
	UInt8	info[64];
};

/* Get the node records from selected components or (if 0 == numLIDs) from all components.
 */
void getNodeRecords(AllocFunction alloc, void *allocUd, char *CA, SInt16 port,
                    SInt32 numLIDs, const UInt16 *LIDs,
                    SInt32 *numNRs, struct NodeRecord **NRs);

#endif
