#ifndef AVS_CLIENT_H
#define AVS_CLIENT_H
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <errno.h>
#include <syscall.h>
#include <pthread.h>
#include <stdarg.h>
#include <signal.h>
#define BUFFSIZE    1024
#define	MAXLINE		4096	/* max text line length */

void err_sys(const char *fmt, ...);
void err_doit(int errnoflag, const char *fmt, va_list ap);
ssize_t writen(int fd, const void *vptr, size_t n);
void Writen(int fd, void *ptr, size_t nbytes);
void Close(int fd);
void * func_read(void * arg);

char * files[BUFFSIZE];
int    files_id[BUFFSIZE];
int findex;


#endif // AVS_CLIENT_H
