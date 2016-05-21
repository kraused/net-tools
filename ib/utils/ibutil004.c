
/* ibutil004: Query node records.
 */

#include "common.h"
#include "structs.h"
#include "adapter.h"
#include "mad-handling.h"

static void fillMAD(char *umad, SInt64 len, UInt16 smLid, UInt16 destLid)
{
	UInt64 mask = 0;
	struct NodeRecord nr;

	memset(umad, 0, len);
	umad_set_addr(umad, smLid, MAD_QP1, MAD_DEFAULT_SL, IB_DEFAULT_QP1_QKEY);
	/* Ignore GRH */

	mad_set_field  (umad_get_mad(umad), 0, IB_MAD_METHOD_F, IB_MAD_METHOD_GET_TABLE);
	mad_set_field  (umad_get_mad(umad), 0, IB_MAD_CLASSVER_F, MAD_HEADER_CLASS_VERSION_SA);
	mad_set_field  (umad_get_mad(umad), 0, IB_MAD_MGMTCLASS_F, IB_SA_CLASS);
	mad_set_field  (umad_get_mad(umad), 0, IB_MAD_BASEVER_F, 1);
	mad_set_field64(umad_get_mad(umad), 0, IB_MAD_TRID_F, 0x1);
	mad_set_field  (umad_get_mad(umad), 0, IB_MAD_ATTRID_F, IB_SA_ATTR_NODERECORD);

	if (destLid > 0) {
		memset(&nr, 0, sizeof(nr));
		nr.lid = hton16(destLid);

		mask = 0x1;	/* lid */
		memcpy((char* )umad_get_mad(umad) + IB_SA_DATA_OFFS, &nr, sizeof(nr));
	}
	mad_set_field64(umad_get_mad(umad), 0, IB_SA_COMPMASK_F, mask);
}

static void printNodeRecords(char *buf, SInt64 len)
{
	SInt32 status;
	SInt64 n;
	SInt32 i, offset;
	struct NodeRecord *p;

	status = mad_get_field(umad_get_mad(buf), 0, IB_MAD_STATUS_F);
	if (UNLIKELY(0 != status)) {
		FATAL("status is %d", status);
	}

	/* RMPP packets.
  	 */
	offset = mad_get_field(umad_get_mad(buf), 0, IB_SA_ATTROFFS_F);
	n      = (len - IB_SA_DATA_OFFS)/(offset << 3);	/* offset is in 8 byte units */

	for (i = 0 ; i < n; ++i) {
		p = (struct NodeRecord *)(umad_get_mad(buf) + IB_SA_DATA_OFFS + i*(offset << 3));

		fprintf(stdout, "%d %d %d 0x%" PRIx64 "\n", ntoh16(p->lid), p->nodeType, p->numPorts, ntoh64(p->portGuid));
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

	if (UNLIKELY(argc > 2)) {
		FATAL("usage: ibutil002.exe [<lid>]");
	}

	destLid = 0;
	if (2 == argc) {
		destLid = strtol(argv[1], NULL, 10);
	}

	err = firstActivePort(defaultMemoryAlloc, NULL, &CA, &port, &lid);
	if (UNLIKELY(err < 0)) {
		ERROR("firstActivePort() failed");
	}

	smLid = subnetManagerLocalIdentifier(CA, port);

#if 0
	libibumad_Enable_Debugging();
#endif

	umad_init();

	fd = umad_open_port(CA, port);
	if (UNLIKELY(fd < 0)) {
		FATAL("umad_open_port() failed with error %d: %s", -fd, strerror(-fd));
	}

#define	MGMT_VERSION	MAD_HEADER_CLASS_VERSION_SA
#define RMPP_VERSION	1

	agent = umad_register(fd, IB_SA_CLASS, MGMT_VERSION, RMPP_VERSION, NULL);
	if (UNLIKELY(agent < 0)) {
		FATAL("umad_register() failed with error %d: %s", -agent, strerror(-agent));
	}

	fillMAD(umad, sizeof(umad), smLid, destLid);

	libibumad_Send_MAD(fd, agent, umad, sizeof(umad), timeout, 0);
	libibumad_Recv_MAD(defaultMemoryAlloc, NULL, fd, &buf, &len, timeout);

	printNodeRecords(buf, len);

	umad_close_port(fd);
	umad_done();

	return 0;
}

