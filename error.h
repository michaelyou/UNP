#ifndef _ERROR_H
#define _ERROR_H
#include <errno.h>
#endif

static void	err_doit(int, int, const char *, va_list);
void err_ret(const char *fmt, ...);
void err_sys(const char *fmt, ...);
void err_cont(int error, const char *fmt, ...);
void err_exit(int error, const char *fmt, ...);
void err_dump(const char *fmt, ...);
void err_msg(const char *fmt, ...);
void err_quit(const char *fmt, ...);
static void err_doit(int errnoflag, int error, const char *fmt, va_list ap);








