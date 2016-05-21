
/* ibutil003: Discover the fabric and retrieve all performance and extended performance counter
 *            values.
 *            This code is parallelized with OpenMP. Running it with env GOMP_CPU_AFFINITY=0 OMP_NUM_THREADS=128
 *            gave good results.
 */

#include "common.h"

#include <sys/time.h>

#include <infiniband/mad.h>
#include <infiniband/umad.h>
#include <infiniband/ibnetdisc.h>

#ifdef PARALLEL

#include <omp.h>

#define OMP(string)	_Pragma(string)
#define	MAX_THREADS	128

inline SInt32 maxThreads()
{
	return omp_get_max_threads();
}

inline SInt32 threadId()
{
	return omp_get_thread_num();
}

#else

#define OMP(string)
#define MAX_THREADS	1

inline SInt32 maxThreads()
{
	return 1;
}

inline SInt32 threadId()
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
	struct ibmad_port	*sourcePorts[MAX_THREADS];

	SInt64			nnodes;
	struct Node		*nodes;
};

static inline double timediff(const struct timeval *x, const struct timeval *y)
{
	return 1E-3*((x->tv_sec - y->tv_sec)*1000000L + (x->tv_usec - y->tv_usec));
}

static void openSourcePorts(struct Fabric *fabric)
{
	int classes[1] = {IB_PERFORMANCE_CLASS};
	SInt32 i, n;

	n = maxThreads();
	for (i = 0; i < n; ++i) {
		fabric->sourcePorts[i] = mad_rpc_open_port(NULL, 0, classes,
		                                           sizeof(classes)/sizeof(classes[0]));
		if (UNLIKELY(!fabric->sourcePorts[i])) {
			exit(1);
		}
	}
}

static void discoverFabric(AllocFunction alloc, void *allocUd, struct Fabric *fabric)
{
	struct ibnd_fabric *f;
	struct ibnd_config config = {0};
	struct ibnd_node *node;
	struct timeval t1, t2;
	SInt32 i, j;

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
		}

		++i;
	}
}

static void queryPortCounters(struct ibmad_port *sourcePort, struct Port *port)
{
	unsigned char pc[1024];
	struct portid portId = { .lid = port->lid };
	void *p;

#define TIMEOUT	1000

	memset(pc, 0, sizeof(pc));
	p = pma_query_via(pc, &portId, port->port, TIMEOUT, IB_GSI_PORT_COUNTERS, sourcePort);

	if (!p) {
		fprintf(stderr, "pma_query_via() failed.\n");
		exit(1);
	}

#define COPY32(COUNTER, MUL)	port->counter_ ## COUNTER = (mad_get_field  (pc, 0, COUNTER) * MUL)
#define COPY64(COUNTER, MUL)	port->counter_ ## COUNTER = (mad_get_field64(pc, 0, COUNTER) * MUL)

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

	memset(pc, 0, sizeof(pc));
	p = pma_query_via(pc, &portId, port->port, TIMEOUT, IB_GSI_PORT_COUNTERS_EXT, sourcePort);

	if (!p) {
		fprintf(stderr, "pma_query_via() failed.\n");
		exit(1);
	}

	COPY64(IB_PC_EXT_XMT_BYTES_F, 32);
	COPY64(IB_PC_EXT_XMT_PKTS_F, 1);
	COPY64(IB_PC_EXT_RCV_BYTES_F, 32);
	COPY64(IB_PC_EXT_RCV_PKTS_F, 1);
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
			queryPortCounters(fabric->sourcePorts[threadId()], fabric->nodes[i].ports[j]);
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

	openSourcePorts(&fabric);
	discoverFabric (defaultMemoryAlloc, NULL, &fabric);

	queryAllCounters(&fabric);
	printAllCounters(&fabric);

	return 0;
}

