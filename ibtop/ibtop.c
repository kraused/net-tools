
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
		if (unlikely(f->nodes[i].fails))
			continue;

		mvprintw(y++, 0, "    %-20s\t%-10g (%6.2f)\t%-10g (%6.2f)\n", 
		         f->nodes[i].node->nodedesc,
		         f->nodes[i].rx_bw,
		         f->nodes[i].rx_bw * ONE_OVER_FDR_SPEED * 100,
		         f->nodes[i].tx_bw,
		         f->nodes[i].tx_bw * ONE_OVER_FDR_SPEED * 100);
	}

	move(1, 0);
        refresh();
}

static int loop(struct ibtop_fabric* f, 
                struct ibmad_port *srcport, 
                struct timespec *ts)
{
	int ch;

	do {
		ibtop_fabric_update_perfcounters(f, srcport, 1000);
		ibtop_fabric_compute_bandwidth(f, ts);

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
