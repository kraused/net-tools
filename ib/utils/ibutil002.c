
/* ibutil002: Query the LFT of a switch
 */

#include "common.h"
#include "lft.h"
#include "adapter.h"

#include <errno.h>

#include <infiniband/mad.h>
#include <infiniband/umad.h>

#define MGMT_VER 2	/* 15.21 C15-0.1.6 */

#if 0
static void enableDebugging()
{
	umad_debug(5);
}
#endif

static void fillMAD(char *umad, SInt64 len, UInt16 smLid, UInt16 destLid)
{
	UInt64 mask;
	struct LinearForwardingTableRecord lft;

	memset(&lft, 0, sizeof(lft));
	lft.lid = hton16(destLid);

	memset(umad, 0, len);
	umad_set_addr(umad, smLid, 1, 0, IB_DEFAULT_QP1_QKEY);
	/* Ignore GRH */

	mad_set_field  (umad_get_mad(umad), 0, IB_MAD_METHOD_F, IB_MAD_METHOD_GET_TABLE);
	mad_set_field  (umad_get_mad(umad), 0, IB_MAD_CLASSVER_F, MGMT_VER);
	mad_set_field  (umad_get_mad(umad), 0, IB_MAD_MGMTCLASS_F, IB_SA_CLASS);
	mad_set_field  (umad_get_mad(umad), 0, IB_MAD_BASEVER_F, 1);
	mad_set_field64(umad_get_mad(umad), 0, IB_MAD_TRID_F, 0x1);
	mad_set_field  (umad_get_mad(umad), 0, IB_MAD_ATTRID_F, IB_SA_ATTR_LFTRECORD);

	mask = 0x1;	/* lid */
	mad_set_field64(umad_get_mad(umad), 0, IB_SA_COMPMASK_F, mask);
	memcpy((char* )umad_get_mad(umad) + IB_SA_DATA_OFFS, &lft, sizeof(lft));
}

static void sendMAD(int fd, int agent, char *umad, UInt64 len, SInt32 timeout, SInt32 nretries)
{
	SInt32 err;

	err = umad_send(fd, agent, umad, len, timeout, nretries);
	if (UNLIKELY(err < 0)) {
		FATAL("umad_send() failed with error %d: %s", -err, strerror(-err));
	}
}

static void recvMAD(AllocFunction alloc, void *allocUd, int fd, char **buf, SInt64 *len, SInt32 timeout)
{
	SInt32 err, length;

	*len = 1024;
	*buf = alloc(allocUd, NULL, 0, (*len));
	memset((*buf), 0, (*len));

	do {
		length = (*len);
		err    = umad_recv(fd, (*buf), &length, timeout);
		if (err >= 0) {
			*len = length;
			break;
		}

		if (ENOSPC == (-err)) {
			*buf = alloc(allocUd, (*buf), (*len), length + umad_size());
			*len = length + umad_size();
			memset((*buf), 0, (*len));
		} else {
			FATAL("umad_recv() failed with error %d: %s", -err, strerror(-err));
		}
	} while(1);
}

static void printLinearForwardingTable(char *buf, SInt64 len)
{
	SInt32 status;
	SInt64 n;
	SInt32 i, j, offset;
	struct LinearForwardingTableRecord *p;

	status = mad_get_field(umad_get_mad(buf), 0, IB_MAD_STATUS_F);
	fprintf(stderr, "%d\n", status);

	/* RMPP packets.
  	 */
	offset = mad_get_field(umad_get_mad(buf), 0, IB_SA_ATTROFFS_F);
	n      = (len - IB_SA_DATA_OFFS)/(offset << 3);	/* offset is in 8 byte units */

	for (i = 0 ; i < n; ++i) {
		p = (struct LinearForwardingTableRecord *)(umad_get_mad(buf) + IB_SA_DATA_OFFS + i*(offset << 3));

		for (j = 0; j < 64; ++j)
			fprintf(stdout, "%4d %3d\n", (int )(64*ntoh16(p->block) + j), (SInt8 )p->lft[j]);
	}
}

int main(int argc, char **argv)
{
	char   *CA;
	SInt16 port;
	SInt32 err;
	UInt16 lid;
	int    fd, agent;
	char   umad[256];
	char   *buf;
	SInt64 len;
	UInt16 smLid, destLid;
	const SInt32 timeout = 1000;

	if (UNLIKELY(2 != argc)) {
		FATAL("usage: ibutil002.exe <lid>");
	}

	destLid = strtol(argv[1], NULL, 10);

	err = firstActivePort(defaultMemoryAlloc, NULL, &CA, &port, &lid);
	if (UNLIKELY(err < 0)) {
		ERROR("firstActivePort() failed");
	}

	smLid = subnetManagerLocalIdentifier(CA, port);

#if 0
	enableDebugging();
#endif

	umad_init();

	fd = umad_open_port(CA, port);
	if (UNLIKELY(fd < 0)) {
		FATAL("umad_open_port() failed with error %d: %s", -fd, strerror(-fd));
	}

	agent = umad_register(fd, IB_SA_CLASS, MGMT_VER, 1, NULL);
	if (UNLIKELY(agent < 0)) {
		FATAL("umad_register() failed with error %d: %s", -agent, strerror(-agent));
	}

	fprintf(stderr, "%d %d\n", (int )lid, (int )destLid);

	fillMAD(umad, sizeof(umad), smLid, destLid);
	sendMAD(fd, agent, umad, sizeof(umad), timeout, 0);

	recvMAD(defaultMemoryAlloc, NULL, fd, &buf, &len, timeout);

	printLinearForwardingTable(buf, len);

	umad_close_port(fd);
	umad_done();

	return 0;
}

