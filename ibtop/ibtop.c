
#include "common.h"
#include "fabric.h"

#include <time.h>
#include <curses.h>

#include <infiniband/mad.h>


struct ibmad_port *open_mad_port()
{
	int classes[1] = {IB_PERFORMANCE_CLASS};

	return mad_rpc_open_port(NULL, 0, classes,
	                         sizeof(classes)/sizeof(classes[0]));
}

void print(struct ibtop_fabric* f)
{
	int i;
	int y = 1;

#define ONE_OVER_FDR_SPEED	66/(14*64*4*1000L)

	for (i = 0; i < f->nnodes; ++i) {
		if (unlikely(f->sorted[i]->fails))
			continue;

		mvprintw(y++, 0, "    %-20s\t|\t%-10g Mbps (%6.2f%%)\t|\t%-10g Mbps (%6.2f%%)\n",
		         f->sorted[i]->node->nodedesc,
		         f->sorted[i]->rx_bw,
		         f->sorted[i]->rx_bw * ONE_OVER_FDR_SPEED * 100,
		         f->sorted[i]->tx_bw,
		         f->sorted[i]->tx_bw * ONE_OVER_FDR_SPEED * 100);
	}

	move(1, 0);
        refresh();
}

static int loop(struct ibtop_fabric* f,
                struct ibmad_port *srcport,
                struct timespec *ts)
{
	int ch;
	int sort = 0;	/* Initialize with zero so that we sort the first
	                 * time we execute the loop body. */

	do {
		ibtop_fabric_update_perfcounters(f, srcport, 1000);
		ibtop_fabric_compute_bandwidth(f, ts);

		if (!(sort--)) {
			ibtop_fabric_sort_by_bandwidth_descending(f);
			sort = 10; /* TODO Make this configurable */
		}

		print(f);

		timeout(0);
		ch = getch();

		if (ERR != ch)
			break;

		nanosleep(ts, 0);
	} while(1);

	return 0;
}

int main(int argc, char **argv)
{
	struct ibmad_port *srcport;
	struct ibtop_fabric f;
	int ret;
	struct timespec ts;

	srcport = open_mad_port();
	if (unlikely(!srcport))
		return 1;

	ret = ibtop_fabric_discover(&f);
	if (unlikely(ret))
		return 1;

	ts.tv_sec  = 1;
	ts.tv_nsec = 0;

	ibtop_fabric_update_perfcounters(&f, srcport, 1000);
	nanosleep(&ts, 0);

	initscr();

	ret = loop(&f, srcport, &ts);

	endwin();

	ibtop_fabric_destroy(&f);
	mad_rpc_close_port(srcport);

	return ret;
}

