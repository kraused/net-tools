
#ifndef IB_UTILS_ADAPTER_H_INCLUDED
#define IB_UTILS_ADAPTER_H_INCLUDED 1

#include "common.h"

/* Get the first usable (CA, port) tuple.
 */
SInt32 firstActivePort(AllocFunction alloc, void *allocUd, char **CA, SInt16 *port, UInt16 *lid);

#endif

