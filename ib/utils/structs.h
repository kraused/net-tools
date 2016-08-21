
#ifndef IB_UTILS_STRUCTS_H_INCLUDED
#define IB_UTILS_STRUCTS_H_INCLUDED 1

#include "common.h"

struct _LinearForwardingTableRecord
{
	UInt16	lid;
	UInt16	block;
	UInt32	resv;
	UInt8	lft[64];
} PACKED;

struct _NodeRecord
{
	UInt16	lid;
	UInt16	resv;
	UInt8	baseVersion;
	UInt8	classVersion;
	UInt8	nodeType;
	UInt8	numPorts;
	UInt64	sysGuid;
	UInt64	nodeGuid;
	UInt64	portGuid;
	UInt16	partitionCap;
	UInt16	deviceId;
	UInt32	revision;
	UInt32	portNumVendorId;
	UInt8	info[64];
} PACKED;

struct _MCMemberRecord
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
	/*
	 * UInt8	serviceLevel : 4;
	 * UInt32	flowLabel : 20;
	 * UInt8	hopLimit;
	*/
	UInt32	serviceLevel_flowLabel_hopLimit;
	/* We could use bit fields here to simplify the access. However,
	 * bit fields have the disadvantage of being endianess dependent.
	 * On a big endian machine the right order is:
	 * UInt8	scope : 4;
	 * UInt8	joinState : 4;
	 * On a little endian machine the order must be:
	 * UInt8	joinState : 4;
	 * UInt8	scope : 4;
	 */
	UInt8	scope_joinState;
	UInt8	proxyJoin : 1;
	UInt8	reserved[2];
} PACKED;

#endif

