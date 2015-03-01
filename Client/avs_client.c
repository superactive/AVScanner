#include "avs_client.h"
void err_sys(const char *fmt, ...)
{
    va_list		ap;
    va_start(ap, fmt);
    err_doit(1, fmt, ap);
    va_end(ap);
    exit(1);
}
void err_doit(int errnoflag, const char *fmt, va_list ap)
{
    int		errno_save, n;
    char	buf[MAXLINE];

    errno_save = errno;		/* value caller might want printed */
    vsprintf(buf, fmt, ap);					/* this is not safe */

    n = strlen(buf);
    if (errnoflag)
        snprintf(buf+n, sizeof(buf)-n, ": %s", strerror(errno_save));
    strcat(buf, "\n");

    fflush(stdout);		/* in case stdout and stderr are the same */
    fputs(buf, stderr);
    fflush(stderr);

    return;
}
ssize_t writen(int fd, const void *vptr, size_t n)
{
    size_t		nleft;
    ssize_t		nwritten;
    const char	*ptr;

    ptr = vptr;
    nleft = n;
    while (nleft > 0)
    {
        nwritten = write(fd, ptr, nleft);
        nleft -= nwritten;
        ptr   += nwritten;
    }
    return(n);
}
void Writen(int fd, void *ptr, size_t nbytes)
{
    if (writen(fd, ptr, nbytes) != (int)nbytes)
        err_sys("writen error");
}
void Close(int fd)
{
    if (close(fd) == -1)
        err_sys("close error");
}

