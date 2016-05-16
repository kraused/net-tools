
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "error.h"

static char __thread errorBuf[4096];

static void errorReport(const char* prefix, const char* file, const char* func, long line,
		        const char* fmt, va_list vl);

void errorFatal(const char *file, const char *func, long line, const char *fmt, ...)
{
	va_list vl;

	va_start(vl, fmt);
	errorReport("fatal: ", file, func, line, fmt, vl);
	va_end(vl);

	fflush(0);
	_exit(1);
}

void errorError(const char *file, const char *func, long line, const char *fmt, ...)
{
	va_list vl;

	va_start(vl, fmt);
	errorReport("error: ", file, func, line, fmt, vl);
	va_end(vl);
}

void errorWarn(const char *file, const char *func, long line, const char *fmt, ...)
{
	va_list vl;

	va_start(vl, fmt);
	errorReport("warning: ", file, func, line, fmt, vl);
	va_end(vl);
}

void errorLog(const char *file, const char *func, long line, const char *fmt, ...)
{
	va_list vl;

	va_start(vl, fmt);
	errorReport("", file, func, line, fmt, vl);
	va_end(vl);
}

void errorDebug(const char *file, const char *func, long line, const char *fmt, ...)
{
	va_list vl;

	va_start(vl, fmt);
	errorReport("debug: ", file, func, line, fmt, vl);
	va_end(vl);
}

static void errorReport(const char* prefix, const char* file, const char* func, long line,
                        const char* fmt, va_list vl)
{
        vsnprintf(errorBuf, sizeof(errorBuf), fmt, vl);

	fprintf(stderr, "<%s(), %s:%ld> %s%s\n", func, file, line, prefix, errorBuf);
	fflush (stderr);
}

