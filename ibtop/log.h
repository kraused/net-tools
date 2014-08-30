
#ifndef IBTOP_LOG_H_INCLUDED
#define IBTOP_LOG_H_INCLUDED 1

void ibtop_log_init();
void ibtop_log_fini();

void ibtop_fatal(const char *format, ...) __attribute__((format(printf, 1, 2))) __attribute__((noreturn));
void ibtop_error(const char *format, ...) __attribute__((format(printf, 1, 2)));
void ibtop_log(const char *format, ...) __attribute__((format(printf, 1, 2)));
void ibtop_debug(int level, const char *format, ...) __attribute__((format(printf, 2, 3)));

#endif

