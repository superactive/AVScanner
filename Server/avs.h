#ifndef AVS_H
#define AVS_H
#include	<sys/types.h>
#include	<sys/socket.h>
#include	<errno.h>
#include	<fcntl.h>
#include	<netdb.h>
#include	<signal.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include    <sys/sysinfo.h>
#include	<unistd.h>
#include	<sys/wait.h>
#include	<sys/un.h>
#include    <time.h>
#include    <pthread.h>
#include    <stdarg.h>

#define NUMPROC     64      /* Number of processes to handle connections */
#define	LISTENQ		1024	/* 2nd argument to listen() */

#define	MAXLINE		4096	/* max text line length */
#define	BUFFSIZE	8192    /* max buffer's size */
#define	BUFFSIZESML	256

typedef	void	Sigfunc(int);	/* for signal handlers */

struct scan_struct
{
    int desc;
    char * ptbuff;
    double proc;
    int remove;
    pthread_t thread_id;
};
struct hup_restore
{
    int list;
    socklen_t addr;
};
enum {processed_files = 3};
struct scan_struct * processes[processed_files];
int connfd;
int pthr;
int listenfd;
socklen_t   server_len;
pid_t	* pids;
int		daemon_proc;

void    err_sys(const char *, ...);
void    logwrite(char * str, int con);
void    init_sig();
void    sig_chld(int sign);
void    sig_term(int sign, siginfo_t * si, void * context);
void    sig_int(int sign, siginfo_t * si, void * context);
void    sig_hup(int sign, siginfo_t * si, void * context);
void    eternal();
pid_t   child_make(int listenfd, socklen_t addrlen);
void    child_main(int listenfd, socklen_t addrlen);
void    sig_term_2(int sign);
void    sig_int_2(int sign);
void    sig_hup_2(int sign);
void    sig_hup_2s(int sign, siginfo_t * si, void * ucontext);
void    handler(int desc);
int     valid_command(char *str, int * ident);
int     info_handler(int desc);
void *  scan_handler(void * ar);
int     stat_handler(int desc, struct scan_struct **  process);
int     bye_handler(int desc, struct scan_struct **  process);
int     report_handler(int desc, struct scan_struct **  process);
void    free_scan_struct(struct scan_struct * * process, int k);

// Covers
Sigfunc * Signal(int, Sigfunc *);
int       Accept(int, struct sockaddr *, socklen_t *);
void	  Close(int);
int       Writen(int, void *, size_t);
void *    Calloc(size_t, size_t);
pid_t	  Fork(void);

#endif // AVS_H
