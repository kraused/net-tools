
/* ibutil003: Discover the fabric and retrieve all performance and extended performance counter
 *            values.
 *            This code is parallelized with OpenMP. Running it with env GOMP_CPU_AFFINITY=0 OMP_NUM_THREADS=128
 *            gave good results.
 */

#include "common.h"
#include "adapter.h"
#include "mad-handling.h"

#include <sys/time.h>

#include <infiniband/mad.h>
#include <infiniband/umad.h>
#include <infiniband/ibnetdisc.h>

#ifdef PARALLEL

#include <omp.h>

#define OMP(string)	_Pragma(string)
#define	MAX_THREADS	128

static inline SInt32 maxThreads()
{
	return omp_get_max_threads();
}

static inline SInt32 threadId()
{
	return omp_get_thread_num();
}

#else

#define OMP(string)
#define MAX_THREADS	1

static inline SInt32 maxThreads()
{
	return 1;
}

static inline SInt32 threadId()
{
	return 0;
}

#endif

#define MAX_PORTS_PER_NODE	48

struct Node
{
	SInt16		type;
	SInt16		nports;
	struct Port	*ports[MAX_PORTS_PER_NODE + 1];	/* ports[0] == NULL, length == (numPorts + 1) */
};

struct Port
{
	UInt64		guid;
	UInt64		lid;
	SInt16		port;
	UInt64		trId;

	UInt32		counter_IB_PC_PORT_SELECT_F;
	UInt32		counter_IB_PC_COUNTER_SELECT_F;
	UInt32		counter_IB_PC_ERR_SYM_F;
	UInt32		counter_IB_PC_LINK_RECOVERS_F;
	UInt32		counter_IB_PC_LINK_DOWNED_F;
	UInt32		counter_IB_PC_ERR_RCV_F;
	UInt32		counter_IB_PC_ERR_PHYSRCV_F;
	UInt32		counter_IB_PC_ERR_SWITCH_REL_F;
	UInt32		counter_IB_PC_XMT_DISCARDS_F;
	UInt32		counter_IB_PC_ERR_XMTCONSTR_F;
	UInt32		counter_IB_PC_ERR_RCVCONSTR_F;
	UInt32		counter_IB_PC_COUNTER_SELECT2_F;
	UInt32		counter_IB_PC_ERR_LOCALINTEG_F;
	UInt32		counter_IB_PC_ERR_EXCESS_OVR_F;
	UInt32		counter_IB_PC_VL15_DROPPED_F;
	UInt32		counter_IB_PC_XMT_WAIT_F;

	UInt64		counter_IB_PC_EXT_XMT_BYTES_F;
	UInt64		counter_IB_PC_EXT_XMT_PKTS_F;
	UInt64		counter_IB_PC_EXT_RCV_BYTES_F;
	UInt64		counter_IB_PC_EXT_RCV_PKTS_F;
};

struct Fabric
{
	struct {
		char		*CA;
		SInt16		port;
		int		fds[MAX_THREADS];
		int		agents[MAX_THREADS];
	}			source;

	SInt64			nnodes;
	struct Node		*nodes;
};

static inline double timediff(const struct timeval *x, const struct timeval *y)
{
	return 1E-3*((x->tv_sec - y->tv_sec)*1000000L + (x->tv_usec - y->tv_usec));
}

static void openSourcePorts(struct Fabric *fabric)
{
	SInt32 err;
	int    fd;
	int    agent;
	UInt16 lid;
	SInt32 i, n;

	err = firstActivePort(defaultMemoryAlloc, NULL, &fabric->source.CA, &fabric->source.port, &lid);
	if (UNLIKELY(err < 0)) {
		ERROR("firstActivePort() failed");
	}

#define MGMT_VERSION	1
#define RMPP_VERSION	0

	n = maxThreads();
	for (i = 0; i < n; ++i) {
		fd = umad_open_port(fabric->source.CA, fabric->source.port);
		if (UNLIKELY(fd < 0)) {
			FATAL("umad_open_port() failed with error %d: %s", -fd, strerror(-fd));
		}

		agent = umad_register(fd, IB_SA_CLASS, MGMT_VERSION, RMPP_VERSION, NULL);
		if (UNLIKELY(agent < 0)) {
			FATAL("umad_register() failed with error %d: %s", -agent, strerror(-agent));
		}

		fabric->source.fds[i]    = fd;
		fabric->source.agents[i] = agent;
	}
}

static void discoverFabric(AllocFunction alloc, void *allocUd, struct Fabric *fabric)
{
	struct ibnd_fabric *f;
	struct ibnd_config config = {0};
	struct ibnd_node *node;
	struct timeval t1, t2;
	SInt32 i, j, k;

	gettimeofday(&t1, NULL);
	f = ibnd_discover_fabric(NULL, 0, NULL, &config);
	gettimeofday(&t2, NULL);

	fprintf(stderr, "Discovery took %.3f milliseconds\n", timediff(&t2, &t1));

	fabric->nnodes = 0;
	for (node = f->nodes; node; node = node->next) {
		++fabric->nnodes;
	}

	fabric->nodes = alloc(allocUd, NULL, 0, fabric->nnodes*sizeof(struct Node));

	i = 0;
	k = 0;
	for (node = f->nodes; node; node = node->next) {
		memset(&fabric->nodes[i], 0, sizeof(struct Port));

		fabric->nodes[i].type   = node->type;
		fabric->nodes[i].nports = node->numports;

		for (j = 1; j < (node->numports + 1); ++j) {
			fabric->nodes[i].ports[j] = alloc(allocUd, NULL, 0, sizeof(struct Port));
			memset(fabric->nodes[i].ports[j], 0, sizeof(struct Port));

			fabric->nodes[i].ports[j]->guid = node->guid;
			fabric->nodes[i].ports[j]->lid  = node->ports[j]->base_lid;
			fabric->nodes[i].ports[j]->port = j;
			fabric->nodes[i].ports[j]->trId = 256*(k++);
		}

		++i;
	}
}

static void queryPortCounters(int fd, int agent, struct Port *port)
{
	UInt8  umad[256];
	UInt8  *buf = NULL;
	SInt64 len;
	SInt32 status;

	const SInt32 timeout = 1000;

	memset(umad, 0, sizeof(umad));
	umad_set_addr(umad, port->lid, MAD_QP1, MAD_DEFAULT_SL, IB_DEFAULT_QP1_QKEY);
	/* Ignore GRH */

	mad_set_field  (umad_get_mad(umad), 0, IB_MAD_METHOD_F, IB_MAD_METHOD_GET);
	mad_set_field  (umad_get_mad(umad), 0, IB_MAD_CLASSVER_F, 1);
	mad_set_field  (umad_get_mad(umad), 0, IB_MAD_MGMTCLASS_F, IB_PERFORMANCE_CLASS);
	mad_set_field  (umad_get_mad(umad), 0, IB_MAD_BASEVER_F, 1);
	mad_set_field64(umad_get_mad(umad), 0, IB_MAD_TRID_F, (port->trId++));
	mad_set_field  (umad_get_mad(umad), 0, IB_MAD_ATTRID_F, IB_GSI_PORT_COUNTERS);

	mad_set_field  ((UInt8 *)umad_get_mad(umad) + IB_PC_DATA_OFFS, 0, IB_PC_PORT_SELECT_F, port->port);

	libibumad_Send_MAD(fd, agent, umad, sizeof(umad), timeout, 0);
	libibumad_Recv_MAD(defaultMemoryAlloc, NULL, fd, &buf, &len, timeout);

	status = mad_get_field(umad_get_mad(buf), 0, IB_MAD_STATUS_F);
	if (UNLIKELY(0 != status)) {
		FATAL("status is %d", status);
	}

#define COPY32(COUNTER, MUL)	port->counter_ ## COUNTER = (mad_get_field  (umad_get_mad(buf) + IB_PC_DATA_OFFS, 0, COUNTER) * MUL)
#define COPY64(COUNTER, MUL)	port->counter_ ## COUNTER = (mad_get_field64(umad_get_mad(buf) + IB_PC_DATA_OFFS, 0, COUNTER) * MUL)

	COPY32(IB_PC_PORT_SELECT_F, 1);
	COPY32(IB_PC_COUNTER_SELECT_F, 1);
	COPY32(IB_PC_ERR_SYM_F, 1);
	COPY32(IB_PC_LINK_RECOVERS_F, 1);
	COPY32(IB_PC_LINK_DOWNED_F, 1);
	COPY32(IB_PC_ERR_RCV_F, 1);
	COPY32(IB_PC_ERR_PHYSRCV_F, 1);
	COPY32(IB_PC_ERR_SWITCH_REL_F, 1);
	COPY32(IB_PC_XMT_DISCARDS_F, 1);
	COPY32(IB_PC_ERR_XMTCONSTR_F, 1);
	COPY32(IB_PC_ERR_RCVCONSTR_F, 1);
	COPY32(IB_PC_COUNTER_SELECT2_F, 1);
	COPY32(IB_PC_ERR_LOCALINTEG_F, 1);
	COPY32(IB_PC_ERR_EXCESS_OVR_F, 1);
	COPY32(IB_PC_VL15_DROPPED_F, 1);
	COPY32(IB_PC_XMT_WAIT_F, 1);

	buf = defaultMemoryAlloc(NULL, buf, len, 0);

	memset(umad, 0, sizeof(umad));
	umad_set_addr(umad, port->lid, MAD_QP1, MAD_DEFAULT_SL, IB_DEFAULT_QP1_QKEY);
	/* Ignore GRH */

	mad_set_field  (umad_get_mad(umad), 0, IB_MAD_METHOD_F, IB_MAD_METHOD_GET);
	mad_set_field  (umad_get_mad(umad), 0, IB_MAD_CLASSVER_F, 1);
	mad_set_field  (umad_get_mad(umad), 0, IB_MAD_MGMTCLASS_F, IB_PERFORMANCE_CLASS);
	mad_set_field  (umad_get_mad(umad), 0, IB_MAD_BASEVER_F, 1);
	mad_set_field64(umad_get_mad(umad), 0, IB_MAD_TRID_F, (port->trId++));
	mad_set_field  (umad_get_mad(umad), 0, IB_MAD_ATTRID_F, IB_GSI_PORT_COUNTERS_EXT);

	mad_set_field  ((UInt8 *)umad_get_mad(umad) + IB_PC_DATA_OFFS, 0, IB_PC_PORT_SELECT_F, port->port);

	libibumad_Send_MAD(fd, agent, umad, sizeof(umad), timeout, 0);
	libibumad_Recv_MAD(defaultMemoryAlloc, NULL, fd, &buf, &len, timeout);

	status = mad_get_field(umad_get_mad(buf), 0, IB_MAD_STATUS_F);
	if (UNLIKELY(0 != status)) {
		FATAL("status is %d", status);
	}

	COPY64(IB_PC_EXT_XMT_BYTES_F, 32);
	COPY64(IB_PC_EXT_XMT_PKTS_F, 1);
	COPY64(IB_PC_EXT_RCV_BYTES_F, 32);
	COPY64(IB_PC_EXT_RCV_PKTS_F, 1);

	buf = defaultMemoryAlloc(NULL, buf, len, 0);
}

static void printPort(struct Port *port)
{
	printf("0x%16" PRIx64 " " "%8" PRIu64 " " "%2d" " ", port->guid, port->lid, port->port);
	printf("%8" PRIu32 " " "%8" PRIu32 " " "%8" PRIu32 " " "%8" PRIu32 " "
	       "%8" PRIu32 " " "%8" PRIu32 " " "%8" PRIu32 " " "%8" PRIu32 " "
	       "%8" PRIu32 " " "%8" PRIu32 " " "%8" PRIu32 " " "%8" PRIu32 " "
	       "%8" PRIu32 " " "%8" PRIu32 " " "%8" PRIu32 " " "%8" PRIu32 " ",
	       port->counter_IB_PC_PORT_SELECT_F,
	       port->counter_IB_PC_COUNTER_SELECT_F,
	       port->counter_IB_PC_ERR_SYM_F,
	       port->counter_IB_PC_LINK_RECOVERS_F,
	       port->counter_IB_PC_LINK_DOWNED_F,
	       port->counter_IB_PC_ERR_RCV_F,
	       port->counter_IB_PC_ERR_PHYSRCV_F,
	       port->counter_IB_PC_ERR_SWITCH_REL_F,
	       port->counter_IB_PC_XMT_DISCARDS_F,
	       port->counter_IB_PC_ERR_XMTCONSTR_F,
	       port->counter_IB_PC_ERR_RCVCONSTR_F,
	       port->counter_IB_PC_COUNTER_SELECT2_F,
	       port->counter_IB_PC_ERR_LOCALINTEG_F,
	       port->counter_IB_PC_ERR_EXCESS_OVR_F,
	       port->counter_IB_PC_VL15_DROPPED_F,
	       port->counter_IB_PC_XMT_WAIT_F);
	printf("%16" PRIu64 " " "%16" PRIu64 " " "%16" PRIu64 " " "%16" PRIu64 "\n",
	       port->counter_IB_PC_EXT_XMT_BYTES_F,
	       port->counter_IB_PC_EXT_XMT_PKTS_F,
	       port->counter_IB_PC_EXT_RCV_BYTES_F,
	       port->counter_IB_PC_EXT_RCV_PKTS_F);
}

static void queryAllCounters(struct Fabric *fabric)
{
	struct timeval t1, t2;
	SInt64 i;
	SInt32 j;

	gettimeofday(&t1, NULL);

	OMP("omp parallel for")
	for (i = 0; i < fabric->nnodes; ++i) {
		for (j = 1; j < (fabric->nodes[i].nports + 1); ++j) {
			queryPortCounters(fabric->source.fds[threadId()],
			                  fabric->source.agents[threadId()],
			                  fabric->nodes[i].ports[j]);
		}
	}

	gettimeofday(&t2, NULL);
	fprintf(stderr, "Query took %.3f milliseconds\n", timediff(&t2, &t1));
}

static void printAllCounters(struct Fabric *fabric)
{
	SInt64 i;
	SInt32 j;

	for (i = 0; i < fabric->nnodes; ++i) {
		for (j = 1; j < (fabric->nodes[i].nports + 1); ++j) {
			printPort(fabric->nodes[i].ports[j]);
		}
	}
}

int main(int argc, char **argv)
{
	struct Fabric fabric;

	umad_init();

	openSourcePorts(&fabric);
	discoverFabric (defaultMemoryAlloc, NULL, &fabric);

	queryAllCounters(&fabric);
	printAllCounters(&fabric);

	umad_done();

	return 0;
}

