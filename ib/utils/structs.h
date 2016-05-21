
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

#endif

