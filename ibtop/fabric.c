
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

static void fill_node_names(struct ibtop_node *nodes, int nnodes)
{
	int i, j, n;

	for (i = 0; i < nnodes; ++i) {
		j = 0;
		n = MIN(sizeof(nodes[i].name) - 1, strlen(nodes[i].node->nodedesc));

		for(; (j < n) && (' ' != nodes[i].node->nodedesc[j]); ++j)
			nodes[i].name[j] = nodes[i].node->nodedesc[j];
		nodes[i].name[j] = 0;
	}
}

static void fill_sorted(struct ibtop_fabric *f)
{
	int i;

	for (i = 0; i < f->nnodes; ++i)
		f->sorted[i] = &f->nodes[i];
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
	fill_node_names(f->nodes, f->nnodes);

	f->sorted = malloc(f->nnodes*sizeof(struct ibtop_node *));
	if (unlikely(!f->sorted))
		goto fail;

	fill_sorted(f);

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
	if (likely(f->sorted))
		free(f->sorted);

	return 0;
}

static void update_perfcounters(struct ibtop_node *node,
                                struct ibmad_port *srcport,
                                const int timeout)
{
	unsigned char pc[1024];
	ib_portid_t portid = {0};

	if (unlikely(!node->node->ports[1]))
		goto fail;

	ib_portid_set(&portid, node->node->ports[1]->base_lid, 0, 0);

	memset(pc, 0, sizeof(pc));
	if (unlikely(!pma_query_via(pc, &portid, 1, timeout,
	                            IB_GSI_PORT_COUNTERS_EXT,
	                            srcport)))
		goto fail;

	node->tx_pc[0].bits = node->tx_pc[1].bits;
	node->tx_pc[0].pkts = node->tx_pc[1].pkts;
	node->rx_pc[0].bits = node->rx_pc[1].bits;
	node->rx_pc[0].pkts = node->rx_pc[1].pkts;

	/* Multiply by four to get bytes (see man perfquery) and then times eight
	 * to convert to bits. */
	node->tx_pc[1].bits = mad_get_field64(pc, 0, IB_PC_EXT_XMT_BYTES_F) * 32;
	node->tx_pc[1].pkts = mad_get_field64(pc, 0, IB_PC_EXT_XMT_PKTS_F);
	node->rx_pc[1].bits = mad_get_field64(pc, 0, IB_PC_EXT_RCV_BYTES_F) * 32;
	node->rx_pc[1].pkts = mad_get_field64(pc, 0, IB_PC_EXT_RCV_PKTS_F);

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

	const long long tx = node->tx_pc[1].bits - node->tx_pc[0].bits;
	const long long rx = node->rx_pc[1].bits - node->rx_pc[0].bits;

	node->tx_bw = tx / (1000 * 1000L * delay);
	node->rx_bw = rx / (1000 * 1000L * delay);
}

int ibtop_fabric_compute_bandwidth(struct ibtop_fabric *f,
                                   const struct timespec *ts)
{
	int i;

	for (i = 0; i < f->nnodes; ++i)
		compute_bandwidth(&f->nodes[i], ts);

	return 0;
}

int compare_bandwidth_descending(const void *x1, const void *x2)
{
	const struct ibtop_node *node1 = *(const struct ibtop_node **)x1;
	const struct ibtop_node *node2 = *(const struct ibtop_node **)x2;

	const double tmp1 = MAX(node1->tx_bw, node1->rx_bw);
	const double tmp2 = MAX(node2->tx_bw, node2->rx_bw);

	if (tmp1 > tmp2)
		return -1;
	if (tmp1 < tmp2)
		return  1;

	return 0;
}

int ibtop_fabric_sort_by_bandwidth_descending(struct ibtop_fabric *f)
{
	qsort(f->sorted, f->nnodes, sizeof(struct ibtop_node *),
	      compare_bandwidth_descending);

	return 0;
}

