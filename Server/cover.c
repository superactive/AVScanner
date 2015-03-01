#include "avs.h"
#include <syslog.h>
static void	err_doit(int, int level, const char *, va_list);

// Function to handle errors
void err_sys(const char *fmt, ...)
{
    va_list		ap;
    va_start(ap, fmt);
    err_doit(1, LOG_ERR, fmt, ap);
    va_end(ap);
    exit(1);
}
static void err_doit(int errnoflag, int level, const char *fmt, va_list ap)
{
    int		errno_save, n;
    char	buf[MAXLINE];

    errno_save = errno;		/* value caller might want printed */
    vsprintf(buf, fmt, ap);					/* this is not safe */
    n = strlen(buf);
    if (errnoflag)
        snprintf(buf+n, sizeof(buf)-n, ": %s", strerror(errno_save));
    strcat(buf, "\n");

    if (daemon_proc)
        syslog(level, buf);
    else
    {
        fflush(stdout);		/* in case stdout and stderr are the same */
        fputs(buf, stderr);
        fflush(stderr);
    }
    return;
}
Sigfunc * signal(int signo, Sigfunc *func)
{
    struct sigaction	act, oact;

    act.sa_handler = func;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    if (sigaction(signo, &act, &oact) < 0)
        return(SIG_ERR);
    return(oact.sa_handler);
}

Sigfunc * Signal(int signo, Sigfunc *func)	/* for our signal() function */
{
    Sigfunc	*sigfunc;

    if ( (sigfunc = signal(signo, func)) == SIG_ERR)
        err_sys("signal error");
    return(sigfunc);
}
int Accept(int fd, struct sockaddr *sa, socklen_t *salenptr)
{
    int		n;

again:
    if ( (n = accept(fd, sa, salenptr)) < 0) {
#ifdef	EPROTO
        if (errno == EPROTO || errno == ECONNABORTED)
#else
        if (errno == ECONNABORTED)
#endif
            goto again;
        else
            err_sys("accept error");
    }
    return(n);
}
void Close(int fd)
{
    if (close(fd) == -1)
        err_sys("close error");
}
pid_t Fork(void)
{
    pid_t	pid;

    if ( (pid = fork()) == -1)
        err_sys("fork error");
    return(pid);
}
void * Calloc(size_t n, size_t size)
{
    void	*ptr;

    if ( (ptr = calloc(n, size)) == NULL)
        err_sys("calloc error");
    return(ptr);
}
ssize_t writen(int fd, const void *vptr, size_t n)
{
    size_t		nleft;
    ssize_t		nwritten;
    const char	*ptr;

    ptr = vptr;
    nleft = n;
    while (nleft > 0) {
        if ( (nwritten = write(fd, ptr, nleft)) <= 0)
        {
            if (errno == EPIPE)
            {
                fprintf(stderr,"\nEPIPE\n");
                return 0;
            }
            if (errno == EINTR)
                nwritten = 0;
            else
                return(-1);
        }
        nleft -= nwritten;
        ptr   += nwritten;
    }
    return(n);
}

int Writen(int fd, void *ptr, size_t nbytes)
{
    if (writen(fd, ptr, nbytes) != (ssize_t)nbytes)
        return 0;
    return 1;
}
