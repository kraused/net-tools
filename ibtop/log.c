
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

#include "common.h"
#include "utils.h"
#include "log.h"


static int verbosity = 0;
static int debug = 0;
static FILE *logfp = NULL;


void ibtop_log_init()
{
	/* TODO Read these from the command line. */
	verbosity = 1;
	debug = 7;
	logfp = fopen("ibtop.log", "w");
}

void ibtop_log_fini()
{
	if (likely(logfp))
		fclose(logfp);
}

void report(const char *prefix, const char *format, va_list args)
{
	time_t now;
	char buf[32];

	now = time(NULL);
	strftime(buf, sizeof(buf), "%FT%T", localtime(&now));

	if (likely(logfp)) {
		fprintf(logfp, " %s\t%s\t", buf, prefix);
		vfprintf(logfp, format, args);
		fprintf(logfp, "\n");
	}
	/* else? */

	fflush(logfp);
}

void ibtop_fatal(const char *format, ...)
{
	va_list args;
	
	va_start(args, format);

	report("fatal", format, args);

	va_end(args);

	exit(1);
}

void ibtop_error(const char *format, ...)
{
	va_list args;
	
	va_start(args, format);

	report("error", format, args);

	va_end(args);
}

void ibtop_log(const char *format, ...)
{
	va_list args;
	
	va_start(args, format);

	report("log", format, args);

	va_end(args);
}

void ibtop_debug(int level, const char *format, ...)
{
	va_list args;
	static const char *prefix[] = {
		"debug1", "debug2", "debug3",
		"debug4", "debug5", "debug6",
		"debug7", "debug8", "debug9",
	};

	if (likely(level > debug))
		return;

	va_start(args, format);
	
	if (unlikely((level < 1) || (level > 9)))
		report("debug", format, args);

	report(prefix[level - 1], format, args);

	va_end(args);
}

