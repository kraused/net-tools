
#ifndef IB_UTILS_MAD_HANDLING_H_INCLUDED
#define IB_UTILS_MAD_HANDLING_H_INCLUDED 1

/* This file is explicitly not named "mad.h" to avoid conflicts with the
 * libibmad header file. 
 */

#include "common.h"

#include <infiniband/mad.h>
#include <infiniband/umad.h>

/* C15-0.1.6: The MADHeader:ClassVersion component for the SA class shall be 2 for this version of
 *            the specification.
 */
#define MAD_HEADER_CLASS_VERSION_SA	2

#define MAD_QP1		1
#define MAD_DEFAULT_SL	0

void libibumad_Enable_Debugging();

/* Send a MAD using libibumad. Errors are fatal.
 */
void libibumad_Send_MAD(int fd, int agent, char *umad, UInt64 len, SInt32 timeout, SInt32 nretries);

/* Recv a response via libibumad. Errors are fatal.
 */
void libibumad_Recv_MAD(AllocFunction alloc, void *allocUd, int fd, char **buf, SInt64 *len, SInt32 timeout);

#endif

