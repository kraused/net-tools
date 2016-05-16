
#ifndef IB_UTILS_LFT_H_INCLUDED
#define IB_UTILS_LFT_H_INCLUDED 1

#include "common.h"

struct LinearForwardingTableRecord
{
	UInt16	lid;
	UInt16	block;
	UInt8	resv;
	UInt8	lft[64];
} PACKED;

#endif

