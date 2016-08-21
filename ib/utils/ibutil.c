
#include "ibutil.h"
#include "structs.h"
#include "mad-handling.h"
#include "common.h"

static void fillMAD_Get_LFTRecord(UInt8 *umad, SInt64 len, UInt16 smLid, UInt16 destLid, UInt64 trId)
{
	UInt64 mask;
	struct _LinearForwardingTableRecord lft;

	memset(umad, 0, len);
	umad_set_addr(umad, smLid, MAD_QP1, MAD_DEFAULT_SL, IB_DEFAULT_QP1_QKEY);
	/* Ignore GRH */

	mad_set_field  (umad_get_mad(umad), 0, IB_MAD_METHOD_F, IB_MAD_METHOD_GET_TABLE);
	mad_set_field  (umad_get_mad(umad), 0, IB_MAD_CLASSVER_F, MAD_HEADER_CLASS_VERSION_SA);
	mad_set_field  (umad_get_mad(umad), 0, IB_MAD_MGMTCLASS_F, IB_SA_CLASS);
	mad_set_field  (umad_get_mad(umad), 0, IB_MAD_BASEVER_F, 1);
	mad_set_field64(umad_get_mad(umad), 0, IB_MAD_TRID_F, trId);
	mad_set_field  (umad_get_mad(umad), 0, IB_MAD_ATTRID_F, IB_SA_ATTR_LFTRECORD);

	if (destLid > 0) {
		memset(&lft, 0, sizeof(lft));
		lft.lid = hton16(destLid);

		mask = 0x1;	/* lid */
		memcpy((UInt8 *)umad_get_mad(umad) + IB_SA_DATA_OFFS, &lft, sizeof(lft));
	}
	mad_set_field64(umad_get_mad(umad), 0, IB_SA_COMPMASK_F, mask);
}

static void fillMAD_Get_NodeRecord(UInt8 *umad, SInt64 len, UInt16 smLid, UInt16 destLid, UInt64 trId)
{
	UInt64 mask = 0;
	struct _NodeRecord nr;

	memset(umad, 0, len);
	umad_set_addr(umad, smLid, MAD_QP1, MAD_DEFAULT_SL, IB_DEFAULT_QP1_QKEY);
	/* Ignore GRH */

	mad_set_field  (umad_get_mad(umad), 0, IB_MAD_METHOD_F, IB_MAD_METHOD_GET_TABLE);
	mad_set_field  (umad_get_mad(umad), 0, IB_MAD_CLASSVER_F, MAD_HEADER_CLASS_VERSION_SA);
	mad_set_field  (umad_get_mad(umad), 0, IB_MAD_MGMTCLASS_F, IB_SA_CLASS);
	mad_set_field  (umad_get_mad(umad), 0, IB_MAD_BASEVER_F, 1);
	mad_set_field64(umad_get_mad(umad), 0, IB_MAD_TRID_F, trId);
	mad_set_field  (umad_get_mad(umad), 0, IB_MAD_ATTRID_F, IB_SA_ATTR_NODERECORD);

	if (destLid > 0) {
		memset(&nr, 0, sizeof(nr));
		nr.lid = hton16(destLid);

		mask = 0x1;	/* lid */
		memcpy((UInt8 *)umad_get_mad(umad) + IB_SA_DATA_OFFS, &nr, sizeof(nr));
	}
	mad_set_field64(umad_get_mad(umad), 0, IB_SA_COMPMASK_F, mask);
}

static void fillMAD_Get_PathRecord(UInt8 *umad, SInt64 len, UInt16 smLid, UInt64 guid, UInt64 trId)
{
	UInt8 dgid[16];

	memset(umad, 0, len);
	umad_set_addr(umad, smLid, MAD_QP1, MAD_DEFAULT_SL, IB_DEFAULT_QP1_QKEY);
	/* Ignore GRH */

	mad_set_field  (umad_get_mad(umad), 0, IB_MAD_METHOD_F, IB_MAD_METHOD_GET);
	mad_set_field  (umad_get_mad(umad), 0, IB_MAD_CLASSVER_F, MAD_HEADER_CLASS_VERSION_SA);
	mad_set_field  (umad_get_mad(umad), 0, IB_MAD_MGMTCLASS_F, IB_SA_CLASS);
	mad_set_field  (umad_get_mad(umad), 0, IB_MAD_BASEVER_F, 1);
	mad_set_field64(umad_get_mad(umad), 0, IB_MAD_TRID_F, trId);
	mad_set_field  (umad_get_mad(umad), 0, IB_MAD_ATTRID_F, IB_SA_ATTR_PATHRECORD);
	mad_set_field64(umad_get_mad(umad), 0, IB_SA_COMPMASK_F, 0x4);

	mad_set_field64(dgid, 0, IB_GID_PREFIX_F, IB_DEFAULT_SUBN_PREFIX);
	mad_set_field64(dgid, 0, IB_GID_GUID_F  , guid);

	/* 128 bit data
	 */
	mad_encode_field((UInt8 *)umad_get_mad(umad) + IB_SA_DATA_OFFS, IB_SA_PR_DGID_F, dgid);
}

static void fillMAD_Get_MCMemberRecord(UInt8 *umad, SInt64 len, UInt16 smLid, UInt64 guid, UInt64 trId)
{
	memset(umad, 0, len);
	umad_set_addr(umad, smLid, MAD_QP1, MAD_DEFAULT_SL, IB_DEFAULT_QP1_QKEY);
	/* Ignore GRH */

	mad_set_field  (umad_get_mad(umad), 0, IB_MAD_METHOD_F, IB_MAD_METHOD_GET_TABLE);
	mad_set_field  (umad_get_mad(umad), 0, IB_MAD_CLASSVER_F, MAD_HEADER_CLASS_VERSION_SA);
	mad_set_field  (umad_get_mad(umad), 0, IB_MAD_MGMTCLASS_F, IB_SA_CLASS);
	mad_set_field  (umad_get_mad(umad), 0, IB_MAD_BASEVER_F, 1);
	mad_set_field64(umad_get_mad(umad), 0, IB_MAD_TRID_F, trId);
	mad_set_field  (umad_get_mad(umad), 0, IB_MAD_ATTRID_F, IB_SA_ATTR_MCRECORD);
}

static void fillMAD_Set_MCMemberRecord(UInt8 *umad, SInt64 len, UInt16 smLid, UInt64 guid, UInt64 trId, UInt8 *mgid, UInt8 *portGid)
{
	UInt64 mask = 0;
	struct _MCMemberRecord mr;

	memset(umad, 0, len);
	umad_set_addr(umad, smLid, MAD_QP1, MAD_DEFAULT_SL, IB_DEFAULT_QP1_QKEY);
	/* Ignore GRH */

	mad_set_field  (umad_get_mad(umad), 0, IB_MAD_METHOD_F, IB_MAD_METHOD_SET);
	mad_set_field  (umad_get_mad(umad), 0, IB_MAD_CLASSVER_F, MAD_HEADER_CLASS_VERSION_SA);
	mad_set_field  (umad_get_mad(umad), 0, IB_MAD_MGMTCLASS_F, IB_SA_CLASS);
	mad_set_field  (umad_get_mad(umad), 0, IB_MAD_BASEVER_F, 1);
	mad_set_field64(umad_get_mad(umad), 0, IB_MAD_TRID_F, trId);
	mad_set_field  (umad_get_mad(umad), 0, IB_MAD_ATTRID_F, IB_SA_ATTR_MCRECORD);

	memset(&mr, 0, sizeof(mr));

	memcpy(mr.mgid, mgid, 16);
	mask |= (1 << 0);

	memcpy(mr.portGid, portGid, 16);
	mask |= (1 << 1);

	mr.qkey = hton32(0x1);
	mask |= (1 << 2);

	mr.pkey = hton16(0xffff);
	mask |= (1 << 7);

	/* SL */
	mask |= (1 << 12);

	/* FlowLabel */
	mask |= (1 << 13);
	/* Traffic class */
	mask |= (1 << 6);

	mr.scope_joinState = (0x5 << 4) | 0x1;
	mask |= (1 << 15);
	mask |= (1 << 16);

	memcpy((UInt8 *)umad_get_mad(umad) + IB_SA_DATA_OFFS, &mr, sizeof(mr));

	mad_set_field64(umad_get_mad(umad), 0, IB_SA_COMPMASK_F, mask);
}

static void fillMAD_Delete_MCMemberRecord(UInt8 *umad, SInt64 len, UInt16 smLid, UInt64 guid, UInt64 trId, UInt8 *mgid, UInt8 *portGid)
{
	UInt64 mask = 0;
	struct _MCMemberRecord mr;

	memset(umad, 0, len);
	umad_set_addr(umad, smLid, MAD_QP1, MAD_DEFAULT_SL, IB_DEFAULT_QP1_QKEY);
	/* Ignore GRH */

	mad_set_field  (umad_get_mad(umad), 0, IB_MAD_METHOD_F, IB_MAD_METHOD_DELETE);
	mad_set_field  (umad_get_mad(umad), 0, IB_MAD_CLASSVER_F, MAD_HEADER_CLASS_VERSION_SA);
	mad_set_field  (umad_get_mad(umad), 0, IB_MAD_MGMTCLASS_F, IB_SA_CLASS);
	mad_set_field  (umad_get_mad(umad), 0, IB_MAD_BASEVER_F, 1);
	mad_set_field64(umad_get_mad(umad), 0, IB_MAD_TRID_F, trId);
	mad_set_field  (umad_get_mad(umad), 0, IB_MAD_ATTRID_F, IB_SA_ATTR_MCRECORD);

	memset(&mr, 0, sizeof(mr));

	memcpy(mr.mgid, mgid, 16);
	mask |= (1 << 0);

	memcpy(mr.portGid, portGid, 16);
	mask |= (1 << 1);

	mr.scope_joinState = 0x1;
	/* Scope remains unset */
	mask |= (1 << 16);

	mask |= (1 << 17);

	memcpy((UInt8 *)umad_get_mad(umad) + IB_SA_DATA_OFFS, &mr, sizeof(mr));

	mad_set_field64(umad_get_mad(umad), 0, IB_SA_COMPMASK_F, mask);
}

static void setup(char *CA, SInt16 port, int *fd, int *agent)
{
	umad_init();

	*fd = umad_open_port(CA, port);
	if (UNLIKELY((*fd) < 0)) {
		FATAL("umad_open_port() failed with error %d: %s", -(*fd), strerror(-(*fd)));
	}

#define MGMT_VERSION	MAD_HEADER_CLASS_VERSION_SA
#define RMPP_VERSION	1

	*agent = umad_register((*fd), IB_SA_CLASS, MGMT_VERSION, RMPP_VERSION, NULL);
	if (UNLIKELY((*agent) < 0)) {
		FATAL("umad_register() failed with error %d: %s", -(*agent), strerror(-(*agent)));
	}
}

static void done(int fd)
{
	umad_close_port(fd);
	umad_done();
}

static void getAllLinearForwardingTables(AllocFunction alloc, void *allocUd,
                                        int fd, int agent, UInt16 smLid,
                                        SInt32 *numLFTs, struct LinearForwardingTable **LFTs)
{
	UInt8  umad[256];
	UInt8  *buf = NULL;
	SInt64 len;
	SInt32 status;
	SInt64 i, n;
	SInt32 j, offset;
	struct _LinearForwardingTableRecord *p;
	struct LinearForwardingTable *current;

	const SInt32 timeout = 1000;

	fillMAD_Get_LFTRecord(umad, sizeof(umad), smLid, 0, __LINE__);

	libibumad_Send_MAD(fd, agent, umad, sizeof(umad), timeout, 0);
	libibumad_Recv_MAD(alloc, allocUd, fd, &buf, &len, timeout);

	status = mad_get_field(umad_get_mad(buf), 0, IB_MAD_STATUS_F);
	if (UNLIKELY(0 != status)) {
		FATAL("status is %d", status);
	}

	/* RMPP packets.
	 */
	offset = mad_get_field(umad_get_mad(buf), 0, IB_SA_ATTROFFS_F);
	if (UNLIKELY(0 == offset)) {
		FATAL("SA attribute offset is zero.");
	}
	n      = (len - IB_SA_DATA_OFFS)/(offset << 3);	/* offset is in 8 byte units */

	j = 0;
	for (i = 0 ; i < n; ++i) {
		p  = (struct _LinearForwardingTableRecord *)((UInt8 *)umad_get_mad(buf) + IB_SA_DATA_OFFS + i*(offset << 3));
		j += (0 == p->block);
	}

	*numLFTs = j;
	*LFTs    = alloc(allocUd, NULL, 0, (*numLFTs)*sizeof(struct LinearForwardingTable));

	/* All LFTs have the same size.
	 */
	j = 64*n/(*numLFTs);
	for (i = 0; i < (*numLFTs); ++i) {
		(*LFTs)[i].len = j;
		(*LFTs)[i].lft = alloc(allocUd, NULL, 0, j*sizeof(UInt16));
	}

	current = NULL;
	for (i = 0 ; i < n; ++i) {
		p  = (struct _LinearForwardingTableRecord *)((UInt8 *)umad_get_mad(buf) + IB_SA_DATA_OFFS + i*(offset << 3));

		if (0 == p->block) {
			if (NULL == current) {
				current = (*LFTs);
			} else {
				++current;
			}

			current->lid = ntoh16(p->lid);
		}

		for (j = 0; j < 64; ++j) {
			current->lft[64*ntoh16(p->block) + j] = p->lft[j];
		}
	}

	buf = alloc(allocUd, buf, len, 0);
}

static void getOneLinearForwardingTable(AllocFunction alloc, void *allocUd,
                                        int fd, int agent, UInt16 smLid,
                                        UInt16 lid, struct LinearForwardingTable *LFT)
{
	UInt8  umad[256];
	UInt8  *buf = NULL;
	SInt64 len;
	SInt32 status;
	SInt64 i, n;
	SInt32 j, offset;
	struct _LinearForwardingTableRecord *p;

	const SInt32 timeout = 1000;

	/* Build the transaction id from the lid to ensure that we have unique transaction ids
	 * when looping over multiple lids.
	 */
	fillMAD_Get_LFTRecord(umad, sizeof(umad), smLid, lid, __LINE__ + lid);

	libibumad_Send_MAD(fd, agent, umad, sizeof(umad), timeout, 0);
	libibumad_Recv_MAD(alloc, allocUd, fd, &buf, &len, timeout);

	status = mad_get_field(umad_get_mad(buf), 0, IB_MAD_STATUS_F);
	if (UNLIKELY(0 != status)) {
		FATAL("status is %d", status);
	}

	/* RMPP packets.
	 */
	offset = mad_get_field(umad_get_mad(buf), 0, IB_SA_ATTROFFS_F);
	if (UNLIKELY(0 == offset)) {
		FATAL("SA attribute offset is zero.");
	}
	n      = (len - IB_SA_DATA_OFFS)/(offset << 3);	/* offset is in 8 byte units */

	LFT->len = 64*n;
	LFT->lft = alloc(allocUd, NULL, 0, LFT->len*sizeof(UInt16));

	for (i = 0 ; i < n; ++i) {
		p  = (struct _LinearForwardingTableRecord *)((UInt8 *)umad_get_mad(buf) + IB_SA_DATA_OFFS + i*(offset << 3));

		LFT->lid = ntoh16(p->lid);

		for (j = 0; j < 64; ++j) {
			LFT->lft[64*ntoh16(p->block) + j] = p->lft[j];
		}
	}

	buf = alloc(allocUd, buf, len, 0);
}

static void getAllNodeRecords(AllocFunction alloc, void *allocUd,
                              int fd, int agent, UInt16 smLid,
                              SInt32 *numNRs, struct NodeRecord **NRs)
{
	UInt8  umad[256];
	UInt8  *buf = NULL;
	SInt64 len;
	SInt32 status;
	SInt64 i, n;
	SInt32 offset;
	struct _NodeRecord *p;

	const SInt32 timeout = 1000;

	fillMAD_Get_NodeRecord(umad, sizeof(umad), smLid, 0, __LINE__);

	libibumad_Send_MAD(fd, agent, umad, sizeof(umad), timeout, 0);
	libibumad_Recv_MAD(alloc, allocUd, fd, &buf, &len, timeout);

	status = mad_get_field(umad_get_mad(buf), 0, IB_MAD_STATUS_F);
	if (UNLIKELY(0 != status)) {
		FATAL("status is %d", status);
	}

	/* RMPP packets.
	 */
	offset = mad_get_field(umad_get_mad(buf), 0, IB_SA_ATTROFFS_F);
	if (UNLIKELY(0 == offset)) {
		FATAL("SA attribute offset is zero.");
	}
	n      = (len - IB_SA_DATA_OFFS)/(offset << 3);	/* offset is in 8 byte units */

	*numNRs = n;
	*NRs    = alloc(allocUd, NULL, 0, (*numNRs)*sizeof(struct NodeRecord));

	for (i = 0 ; i < n; ++i) {
		p = (struct _NodeRecord *)((UInt8 *)umad_get_mad(buf) + IB_SA_DATA_OFFS + i*(offset << 3));

		(*NRs)[i].lid      = ntoh16(p->lid);
		(*NRs)[i].nodeType = p->nodeType;
		(*NRs)[i].numPorts = p->numPorts;
		(*NRs)[i].sysGuid  = ntoh64(p->sysGuid);
		(*NRs)[i].nodeGuid = ntoh64(p->nodeGuid);
		(*NRs)[i].portGuid = ntoh64(p->portGuid);
		memcpy((*NRs)[i].info, p->info, 64*sizeof(UInt8));
	}

	buf = alloc(allocUd, buf, len, 0);
}

static void getOneNodeRecord(AllocFunction alloc, void *allocUd,
                             int fd, int agent, UInt16 smLid,
                             UInt16 lid, struct NodeRecord *NR)
{
	UInt8  umad[256];
	UInt8  *buf = NULL;
	SInt64 len;
	SInt32 status;
	SInt64 n;
	SInt32 offset;
	struct _NodeRecord *p;

	const SInt32 timeout = 1000;

	fillMAD_Get_NodeRecord(umad, sizeof(umad), smLid, lid, __LINE__ + lid);

	libibumad_Send_MAD(fd, agent, umad, sizeof(umad), timeout, 0);
	libibumad_Recv_MAD(alloc, allocUd, fd, &buf, &len, timeout);

	status = mad_get_field(umad_get_mad(buf), 0, IB_MAD_STATUS_F);
	if (UNLIKELY(0 != status)) {
		FATAL("status is %d", status);
	}

	/* RMPP packets.
	 */
	offset = mad_get_field(umad_get_mad(buf), 0, IB_SA_ATTROFFS_F);
	if (UNLIKELY(0 == offset)) {
		FATAL("SA attribute offset is zero.");
	}
	n      = (len - IB_SA_DATA_OFFS)/(offset << 3);	/* offset is in 8 byte units */

	if (UNLIKELY(1 != n)) {
		FATAL("Expected one node record but found %d in answer", n);
	}

	p = (struct _NodeRecord *)((UInt8 *)umad_get_mad(buf) + IB_SA_DATA_OFFS + 0*(offset << 3));

	NR->lid      = ntoh16(p->lid);
	NR->nodeType = p->nodeType;
	NR->numPorts = p->numPorts;
	NR->sysGuid  = ntoh64(p->sysGuid);
	NR->nodeGuid = ntoh64(p->nodeGuid);
	NR->portGuid = ntoh64(p->portGuid);
	memcpy(NR->info, p->info, 64*sizeof(UInt8));

	buf = alloc(allocUd, buf, len, 0);
}

static void getLIDFromGUID(AllocFunction alloc, void *allocUd,
                           int fd, int agent, UInt16 smLid, UInt64 guid, UInt16 *lid)
{
	UInt8  umad[256];
	UInt8  *buf = NULL;
	SInt64 len;
	SInt32 status;

	const SInt32 timeout = 1000;

	fillMAD_Get_PathRecord(umad, sizeof(umad), smLid, guid, __LINE__ + guid);

	libibumad_Send_MAD(fd, agent, umad, sizeof(umad), timeout, 0);
	libibumad_Recv_MAD(alloc, allocUd, fd, &buf, &len, timeout);

	status = mad_get_field(umad_get_mad(buf), 0, IB_MAD_STATUS_F);
	if (UNLIKELY(0 != status)) {
		FATAL("status is %d", status);
	}

	mad_decode_field((UInt8 *)umad_get_mad(buf) + IB_SA_DATA_OFFS, IB_SA_PR_DLID_F, lid);

	buf = alloc(allocUd, buf, len, 0);
}

static void getAllMCMemberRecords(AllocFunction alloc, void *allocUd,
                              int fd, int agent, UInt16 smLid,
                              SInt32 *numMRs, struct MCMemberRecord **MRs)
{
	UInt8  umad[256];
	UInt8  *buf = NULL;
	SInt64 len;
	SInt32 status;
	SInt64 i, n;
	SInt32 offset;
	struct _MCMemberRecord *p;

	const SInt32 timeout = 1000;

	fillMAD_Get_MCMemberRecord(umad, sizeof(umad), smLid, 0, __LINE__);

	libibumad_Send_MAD(fd, agent, umad, sizeof(umad), timeout, 0);
	libibumad_Recv_MAD(alloc, allocUd, fd, &buf, &len, timeout);

	status = mad_get_field(umad_get_mad(buf), 0, IB_MAD_STATUS_F);
	if (UNLIKELY(0 != status)) {
		FATAL("status is %d", status);
	}

	/* RMPP packets.
	 */
	offset = mad_get_field(umad_get_mad(buf), 0, IB_SA_ATTROFFS_F);
	if (UNLIKELY(0 == offset)) {
		FATAL("SA attribute offset is zero.");
	}
	n      = (len - IB_SA_DATA_OFFS)/(offset << 3);	/* offset is in 8 byte units */

	*numMRs = n;
	*MRs    = alloc(allocUd, NULL, 0, (*numMRs)*sizeof(struct MCMemberRecord));

	for (i = 0 ; i < n; ++i) {
		p = (struct _MCMemberRecord *)((UInt8 *)umad_get_mad(buf) + IB_SA_DATA_OFFS + i*(offset << 3));

		memcpy((*MRs)[i].mgid, p->mgid, 16);
		memcpy((*MRs)[i].portGid, p->portGid, 16);
		(*MRs)[i].qkey         = ntoh32(p->qkey);
		(*MRs)[i].mlid         = ntoh16(p->mlid);
		(*MRs)[i].mtu          = p->mtu;
		(*MRs)[i].tclass       = p->tclass;
		(*MRs)[i].pkey         = ntoh16(p->pkey);
		(*MRs)[i].rate         = p->rate;
		(*MRs)[i].packetLife   = p->packetLife;
		/* FIXME
		(*MRs)[i].serviceLevel = p->serviceLevel;
		(*MRs)[i].flowLabel    = ntoh32(p->flowLabel);
		(*MRs)[i].hopLimit     = p->hopLimit;
		 */
		(*MRs)[i].scope        = (p->scope_joinState & 0xf0) >> 4;
		(*MRs)[i].joinState    =  p->scope_joinState & 0x0f;
		(*MRs)[i].proxyJoin    = p->proxyJoin;
	}

	buf = alloc(allocUd, buf, len, 0);
}

void getLinearForwardingTables(AllocFunction alloc, void *allocUd, char *CA, SInt16 port,
                               SInt32 numLIDs, const UInt16 *LIDs,
                               SInt32 *numLFTs, struct LinearForwardingTable **LFTs)
{
	int    fd, agent;
	SInt32 i;
	UInt16 smLid;

	setup(CA, port, &fd, &agent);
	smLid = subnetManagerLocalIdentifier(CA, port);

	if (0 == numLIDs) {
		getAllLinearForwardingTables(alloc, allocUd, fd, agent, smLid, numLFTs, LFTs);
	} else {
		*numLFTs = numLIDs;
		*LFTs    = alloc(allocUd, NULL, 0, (*numLFTs)*sizeof(struct LinearForwardingTable));

		for (i = 0; i < numLIDs; ++i) {
			getOneLinearForwardingTable(alloc, allocUd, fd, agent, smLid, LIDs[i], &(*LFTs)[i]);
		}
	}

	done(fd);
}

void getNodeRecords(AllocFunction alloc, void *allocUd, char *CA, SInt16 port,
                    SInt32 numLIDs, const UInt16 *LIDs,
                    SInt32 *numNRs, struct NodeRecord **NRs)
{
	int    fd, agent;
	SInt32 i;
	UInt16 smLid;

	setup(CA, port, &fd, &agent);
	smLid = subnetManagerLocalIdentifier(CA, port);

	if (0 == numLIDs) {
		getAllNodeRecords(alloc, allocUd, fd, agent, smLid, numNRs, NRs);
	} else {
		*numNRs = numLIDs;
		*NRs    = alloc(allocUd, NULL, 0, (*numNRs)*sizeof(struct NodeRecord));

		for (i = 0; i < numLIDs; ++i) {
			getOneNodeRecord(alloc, allocUd, fd, agent, smLid, LIDs[i], &(*NRs)[i]);
		}
	}

	done(fd);
}

void getLIDsFromGUIDs(AllocFunction alloc, void *allocUd, char *CA, SInt16 port,
                      SInt32 numGUIDs, const UInt64 *GUIDs, UInt16 **LIDs)
{
	int    fd, agent;
	SInt32 i;
	UInt16 smLid;

	setup(CA, port, &fd, &agent);
	smLid = subnetManagerLocalIdentifier(CA, port);

	*LIDs = alloc(allocUd, NULL, 0, numGUIDs*sizeof(UInt16));

	for (i = 0; i < numGUIDs; ++i) {
		getLIDFromGUID(alloc, allocUd, fd, agent, smLid, GUIDs[i], &(*LIDs)[i]);
	}

	done(fd);
}

void getMCMemberRecords(AllocFunction alloc, void *allocUd, char *CA, SInt16 port,
                        SInt32 *numMRs, struct MCMemberRecord **MRs)
{
	int    fd, agent;
	UInt16 smLid;

	setup(CA, port, &fd, &agent);
	smLid = subnetManagerLocalIdentifier(CA, port);

	getAllMCMemberRecords(alloc, allocUd, fd, agent, smLid, numMRs, MRs);

	done(fd);
}

void joinMCGroup(AllocFunction alloc, void *allocUd, char *CA, SInt16 port, UInt8 *mgid)
{
	int    fd, agent;
	UInt16 smLid;
	UInt8  umad[256];
	SInt32 status;
	SInt32 err;
	UInt8  *buf = NULL;
	SInt64 len;
	UInt8  portGid[16];

	setup(CA, port, &fd, &agent);
	smLid = subnetManagerLocalIdentifier(CA, port);

	err = portGlobalIdentifier(CA, port, 0, portGid);
	if (UNLIKELY(0 != err)) {
		FATAL("portGlobalIdentifier() failed.");
	}

	const SInt32 timeout = 1000;

	fillMAD_Set_MCMemberRecord(umad, sizeof(umad), smLid, 0, __LINE__, mgid, portGid);

	libibumad_Send_MAD(fd, agent, umad, sizeof(umad), timeout, 0);
	libibumad_Recv_MAD(alloc, allocUd, fd, &buf, &len, timeout);

	status = mad_get_field(umad_get_mad(buf), 0, IB_MAD_STATUS_F);
	if (UNLIKELY(0 != status)) {
		FATAL("status is %d", status);
	}

	done(fd);

	buf = alloc(allocUd, buf, len, 0);
}

void leaveMCGroup(AllocFunction alloc, void *allocUd, char *CA, SInt16 port, UInt8 *mgid)
{
	int    fd, agent;
	UInt16 smLid;
	UInt8  umad[256];
	SInt32 status;
	SInt32 err;
	UInt8  *buf = NULL;
	SInt64 len;
	UInt8  portGid[16];

	setup(CA, port, &fd, &agent);
	smLid = subnetManagerLocalIdentifier(CA, port);

	err = portGlobalIdentifier(CA, port, 0, portGid);
	if (UNLIKELY(0 != err)) {
		FATAL("portGlobalIdentifier() failed.");
	}

	const SInt32 timeout = 1000;

	fillMAD_Delete_MCMemberRecord(umad, sizeof(umad), smLid, 0, __LINE__, mgid, portGid);

	libibumad_Send_MAD(fd, agent, umad, sizeof(umad), timeout, 0);
	libibumad_Recv_MAD(alloc, allocUd, fd, &buf, &len, timeout);

	status = mad_get_field(umad_get_mad(buf), 0, IB_MAD_STATUS_F);
	if (UNLIKELY(0 != status)) {
		FATAL("status is %d", status);
	}

	done(fd);

	buf = alloc(allocUd, buf, len, 0);
}

