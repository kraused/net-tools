
#ifndef IBTOP_FABRIC_H_INCLUDED
#define IBTOP_FABRIC_H_INCLUDED 1

#include <time.h>

#include <infiniband/mad.h>
#include <infiniband/umad.h>
#include <infiniband/ibnetdisc.h>


struct ibtop_perfcounter {
	unsigned long long		bytes;
	unsigned long long		pkts;
};

struct ibtop_node {
	ibnd_node_t			*node;
	/* Number of failures of MAD rpcs to the node
	 * since the last success. */
	int				fails;
	struct ibtop_perfcounter	rx_pc[2];
	double				rx_bw;
	struct ibtop_perfcounter	tx_pc[2];
	double				tx_bw;
};

struct ibtop_fabric {
	ibnd_fabric_t			*ndf;
	int				nnodes;
	struct ibtop_node		*nodes;
};


/*
 * Discover the InfiniBand fabric.
 */
int ibtop_fabric_discover(struct ibtop_fabric *f);

/*
 * Destroy an instance of ibtop_fabric.
 */
int ibtop_fabric_destroy(struct ibtop_fabric *f);

/*
 * Read the latest performance counters. 
 */
int ibtop_fabric_update_perfcounters(struct ibtop_fabric *f, 
                                     struct ibmad_port *srcport,
                                     const int timeout);

/*
 * Compute bandwidth bandwidth from the gathered performance counters.
 */
int ibtop_fabric_compute_bandwidth(struct ibtop_fabric *f,
                                   const struct timespec *ts);

#endif

