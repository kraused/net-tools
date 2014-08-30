
#include "common.h"
#include "utils.h"
#include "fabric.h"
#include "log.h"

#include <time.h>
#include <math.h>
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
	int i, j;
	int x, y = 1;
	char line[160];
	long len;
	char tx[31], rx[31];
	long ptx, prx;

#define ONE_OVER_FDR_SPEED	66/(14*64*4*1000L)

	attron(COLOR_PAIR(3));

	snprintf(line, sizeof(line), "%-17s%6s%40s %-40s%6s", " NODE", "TX BW [MBPS]", "TX % OF 4X FDR PEAK    ", "    RX % OF 4X FDR PEAK", "RX BW [MBPS] ");

	mvaddstr(y++, 0, line);

	attroff(COLOR_PAIR(3));

	for (i = 0; i < f->nnodes; ++i) {
		ptx = MAX(0, MIN(100, lround(f->sorted[i]->tx_bw * ONE_OVER_FDR_SPEED * 100))) * 3 / 10;
		prx = MAX(0, MIN(100, lround(f->sorted[i]->rx_bw * ONE_OVER_FDR_SPEED * 100))) * 3 / 10;

		for (j = 0; j < ptx; ++j)
			tx[j] = '|';
		tx[ptx] = 0;

		for (j = 0; j < prx; ++j)
			rx[j] = '|';
		rx[prx] = 0;

		x   = 0;

		len = snprintf(line, sizeof(line), " %-16s", f->sorted[i]->name);

		mvaddstr(y, x, line);
		x += len;

		len = snprintf(line, sizeof(line), "  %10.2f", f->sorted[i]->rx_bw);

		mvaddstr(y, x, line);
		x += len;

		len = snprintf(line, sizeof(line), "%.2f%% %s", f->sorted[i]->rx_bw * ONE_OVER_FDR_SPEED * 100, rx);

		for (j = 0; j < (40 - len); ++j) {
			mvaddch(y, x, ' ');
			x += 1;
		}

		attron(COLOR_PAIR(1));

		mvaddstr(y, x, line);
		x += len;

		attroff(COLOR_PAIR(1));

		len = snprintf(line, sizeof(line), "|");

		mvaddstr(y, x, line);
		x += len;

		len = snprintf(line, sizeof(line), "%s %.2f%%", tx, f->sorted[i]->tx_bw * ONE_OVER_FDR_SPEED * 100);

		attron(COLOR_PAIR(2));

		mvaddstr(y, x, line);
		x += len;

		attroff(COLOR_PAIR(1));

		for (j = 0; j < (40 - len); ++j) {
			mvaddch(y, x, ' ');
			x += 1;
		}

		len = snprintf(line, sizeof(line), "  %10.2f", f->sorted[i]->tx_bw);

		mvaddstr(y, x, line);
		x += len;

		y += 1;
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

	ibtop_log("Entering loop().");

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

		if ('q' == ch)
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

	ibtop_log_init();

	ibtop_debug(3, "Opening MAD port.");

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
	start_color();
	use_default_colors();

	curs_set(0);

	init_pair(1, COLOR_RED , -1);
	init_pair(2, COLOR_BLUE, -1);
	init_pair(3, COLOR_BLACK, COLOR_WHITE);

	ret = loop(&f, srcport, &ts);

	endwin();

	ibtop_fabric_destroy(&f);
	mad_rpc_close_port(srcport);

	ibtop_log_fini();

	return ret;
}

