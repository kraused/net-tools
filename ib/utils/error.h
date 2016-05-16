
#ifndef IB_UTILS_ERROR_HXX_INCLUDED
#define IB_UTILS_ERROR_HXX_INCLUDED

#include <stdarg.h>

void errorFatal(const char *file, const char *func, long line, const char *fmt, ...);
void errorError(const char *file, const char *func, long line, const char *fmt, ...);
void errorWarn (const char *file, const char *func, long line, const char *fmt, ...);
void errorLog  (const char *file, const char *func, long line, const char *fmt, ...);
void errorDebug(const char *file, const char *func, long line, const char *fmt, ...);

/* Convenience wrappers around XEnv_Error_X() functions
 */
#define FATAL(FMT, ...)	errorFatal(__FILE__, __func__, __LINE__, FMT, ## __VA_ARGS__)
#define ERROR(FMT, ...)	errorError(__FILE__, __func__, __LINE__, FMT, ## __VA_ARGS__)
#define WARN(FMT, ...)	errorWarn (__FILE__, __func__, __LINE__, FMT, ## __VA_ARGS__)
#define LOG(FMT, ...)	errorLog  (__FILE__, __func__, __LINE__, FMT, ## __VA_ARGS__)
#define DEBUG(FMT, ...)	errorDebug(__FILE__, __func__, __LINE__, FMT, ## __VA_ARGS__)

#define	ASSERT(COND)							\
	do {								\
		if (UNLIKELY(!(COND))) {				\
			FATAL("Assertion " #COND " failed.");		\
		}							\
	} while(0)

#endif

