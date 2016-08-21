
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

/* Convert GUIDs to LIDs.
 */
void getLIDsFromGUIDs(AllocFunction alloc, void *allocUd, char *CA, SInt16 port,
                      SInt32 numGUIDs, const UInt64 *GUIDs, UInt16 **LIDs);

struct MCMemberRecord
{
	UInt8	mgid[16];
	UInt8	portGid[16];
	UInt32	qkey;
	UInt16	mlid;
	UInt8	mtu;
	UInt8	tclass;
	UInt16	pkey;
	UInt8	rate;
	UInt8	packetLife;
	UInt8	serviceLevel;
	UInt32	flowLabel;
	UInt8	hopLimit;
	UInt8	scope;
	UInt8	joinState;
	UInt8	proxyJoin;
};

/* Get multicast member records.
 */
void getMCMemberRecords(AllocFunction alloc, void *allocUd, char *CA, SInt16 port,
                        SInt32 *numMRs, struct MCMemberRecord **MRs);

/* Join/create a multicast group.
 */
void joinMCGroup(AllocFunction alloc, void *allocUd, char *CA, SInt16 port, UInt8 *mgid);

/* Leave/delete a multicast group.
 */
void leaveMCGroup(AllocFunction alloc, void *allocUd, char *CA, SInt16 port, UInt8 *mgid);

#endif

