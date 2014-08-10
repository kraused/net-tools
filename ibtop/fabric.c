
#include "common.h"
#include "fabric.h"


static void increment(ibnd_node_t *node, void *udata)
{
	int *x = (int *)udata;

	if (IB_NODE_CA == node->type)
		++(*x);
}

static void copy(ibnd_node_t *node, void *udata)
{
	struct ibtop_node **x = (struct ibtop_node **)udata;

	if (IB_NODE_CA == node->type) {
		(*x)->node = node;
		++(*x);		
	}
}

static int count_nodes_in_fabric(ibnd_fabric_t *f)
{
	int i;

	i = 0;
	ibnd_iter_nodes(f, increment, &i);

	return i;
}

static void copy_node_pointers(ibnd_fabric_t *f, struct ibtop_node *node)
{
	ibnd_iter_nodes(f, copy, &node);
}

int ibtop_fabric_discover(struct ibtop_fabric *f)
{
	struct ibnd_config config = {0};

	memset(f, 0, sizeof(struct ibtop_fabric));

	f->ndf = ibnd_discover_fabric(NULL, 0, NULL, &config);
	if (unlikely(!f->ndf))
		goto fail;

	f->nnodes = count_nodes_in_fabric(f->ndf);

	f->nodes  = malloc(f->nnodes*sizeof(struct ibtop_node));
	if (unlikely(!f->nodes))
		goto fail;

	memset(f->nodes, 0, f->nnodes*sizeof(struct ibtop_node));
	copy_node_pointers(f->ndf, f->nodes);

	return 0;

fail:
	ibtop_fabric_destroy(f);
	return 1;
}

int ibtop_fabric_destroy(struct ibtop_fabric *f)
{
	if (likely(f->ndf))
		ibnd_destroy_fabric(f->ndf);
	if (likely(f->nodes))
		free(f->nodes);

	return 0;
}

static void update_perfcounters(struct ibtop_node *node, 
                                struct ibmad_port *srcport,
                                const int timeout)
{
	unsigned char pc[1024];
	int lid;
	ib_portid_t portid = {0};

	if (unlikely(!node->node->ports[1]))
		goto fail;

	ib_portid_set(&portid, node->node->ports[1]->base_lid, 0, 0);

	memset(pc, 0, sizeof(pc));
	if (unlikely(!pma_query_via(pc, &portid, 1, timeout,
	                            IB_GSI_PORT_COUNTERS_EXT,
	                            srcport)))
		goto fail;

	node->tx_pc[0].bytes = node->tx_pc[1].bytes;
	node->tx_pc[0].pkts  = node->tx_pc[1].pkts;
	node->rx_pc[0].bytes = node->rx_pc[1].bytes;
	node->rx_pc[0].pkts  = node->rx_pc[1].pkts;

	node->tx_pc[1].bytes = mad_get_field64(pc, 0, IB_PC_EXT_XMT_BYTES_F);
	node->tx_pc[1].pkts  = mad_get_field64(pc, 0, IB_PC_EXT_XMT_PKTS_F);
	node->rx_pc[1].bytes = mad_get_field64(pc, 0, IB_PC_EXT_RCV_BYTES_F);
	node->rx_pc[1].pkts  = mad_get_field64(pc, 0, IB_PC_EXT_RCV_PKTS_F);

success:
	node->fails = 0;
	return;

fail:
	++node->fails;
	return;	
}

int ibtop_fabric_update_perfcounters(struct ibtop_fabric *f, 
                                     struct ibmad_port *srcport,
                                     const int timeout)
{
	int i;

	for (i = 0; i < f->nnodes; ++i)
		update_perfcounters(&f->nodes[i], srcport, timeout);

	return 0;
}

static void compute_bandwidth(struct ibtop_node *node,
                              const struct timespec *ts)
{
	const double delay = ts->tv_sec + 1e-9*ts->tv_nsec;

	node->tx_bw = (node->tx_pc[1].bytes - node->tx_pc[0].bytes)/delay;
	node->rx_bw = (node->rx_pc[1].bytes - node->rx_pc[0].bytes)/delay;
}

int ibtop_fabric_compute_bandwidth(struct ibtop_fabric *f,
                                   const struct timespec *ts)
{
	int i;

	for (i = 0; i < f->nnodes; ++i)
		compute_bandwidth(&f->nodes[i], ts);

	return 0;
}
