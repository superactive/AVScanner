#include "avs.h"

// Functions for transmission and processing of signals
void sig_term(int sign, siginfo_t * si, void * context)
{
    logwrite("SIGTERM", 0);
    fprintf(stderr, "sig_term\n");
    int i = 0;
    for(; i < NUMPROC; ++i)
    {
        if(kill(pids[i], SIGTERM))
        {
            fprintf(stderr, "Error kill: sig_term\n");
            return;
        }
    }
}
void sig_int(int sign, siginfo_t * si, void * context)
{
    logwrite("SIGINT", 0);
    fprintf(stderr, "sig_int\n");
    int i = 0;
    for(; i < NUMPROC; ++i)
    {
        if(kill(pids[i], SIGINT))
        {
            fprintf(stderr, "Error kill: sig_int\n");
            return;
        }
    }
}
void sig_hup(int sign, siginfo_t * si, void * context)
{
    setenv("LOG_PATH", "/bd/newlogfile.txt", 1);
    logwrite("SIGHUP", 0);

    fprintf(stderr, "sig_hup\n");
    int i = 0;
    struct hup_restore hr;
    hr.list = listenfd;
    hr.addr = server_len;

    union sigval value;
    value.sival_ptr = (void *)&hr;
    fprintf(stderr, "sig_hup_val\n");
    fprintf(stderr,"%d %d\n",((struct hup_restore *)(value.sival_ptr))->list,
            ((struct hup_restore *)(value.sival_ptr))->addr);
    for(; i < NUMPROC; ++i)
    {
        if ( sigqueue(pids[i], SIGHUP, value))
            perror("ERROR SIGQUEUE\n");
    }
}
void eternal()
{
    for ( ; ; )
        pause();	/* everything done by children */
}
// Writting to logfile
void logwrite(char * str, int con)
{
    char * log_path = getenv("LOG_PATH");
    if(log_path == NULL)
        err_sys("Error: log_path");

    FILE * fileLog;
    fileLog = fopen(log_path,"ab");
    if(fileLog == NULL)
        err_sys("Error: log_path");
    char buffer[128];
    char number[16];
    sprintf(number, "%d", con);
    time_t now;
    struct tm tm_now;
    now = time(NULL);
    localtime_r(&now, &tm_now);
    strftime(buffer, sizeof(buffer), "%Y.%m.%d %H:%M:%S", &tm_now);
    strcat(buffer, "\t");
    strcat(buffer, str);
    strcat(buffer, " #");
    strcat(buffer, number);
    strcat(buffer, "\n");
    fwrite(buffer, sizeof(char), strlen(buffer),fileLog);
    fclose(fileLog);
}
pid_t child_make(int listenfd, socklen_t addrlen)
{
    pid_t	pid;

    if ( (pid = Fork()) > 0)
        return(pid);		/* parent */

    child_main(listenfd, addrlen);	/* never returns */
    return 0;
}
void child_main(int listenfd, socklen_t addrlen)
{
    socklen_t		clilen;
    struct sockaddr	* cliaddr;

    cliaddr = (struct sockaddr	*)malloc(addrlen);

    // Processing signals
    Signal(SIGTERM, sig_term_2);
    Signal(SIGINT, sig_int_2);

    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    act.sa_handler = &sig_hup_2;
    act.sa_sigaction = &sig_hup_2s;
    if(sigaction(SIGHUP, &act ,NULL) == -1)
    {
        fprintf(stderr, "ERROR SIGASTION\n");
        return;
    }


    printf("child %ld starting\n", (long) getpid());
    for ( ; ; )
    {
        while (errno == EIO)
            sleep(1);
        fprintf(stderr,"child_main_for");
        clilen = addrlen;
        connfd = Accept(listenfd, cliaddr, &clilen);
        logwrite("Client joined", connfd);
        fprintf(stderr,"child_main_accept");

        // Processing commands
        handler(connfd);

        Close(connfd);
    }
}
void sig_chld(int sign)
{
    pid_t pid;
    int stat;
    // kill zombies
    while((pid = waitpid(pid, &stat, WNOHANG)) > 0)
        printf("child %d terminated\n", pid);
    return;
}
void sig_term_2(int sign)
{
    logwrite("SIGTERM", connfd);
    fprintf(stderr, "sig_term_2\n");
    errno = EIO;
    bye_handler(connfd, processes);
    fprintf(stderr, "sig_term_2.5\n");
    while (errno != EAGAIN)
        sleep(1);
    fprintf(stderr, "sig_term_2-2\n");
    return;
}
void sig_int_2(int sign)
{
    logwrite("SIGINT", connfd);
    errno = EINTR;
    bye_handler(connfd, processes);
    while (errno != EAGAIN)
        sleep(1);
    return;
}
void sig_hup_2(int sign)
{
    logwrite("SIGHUP", connfd);
    fprintf(stderr, "sig_hup_2\n");
    errno = EAGAIN;
    return;
}
void sig_hup_2s(int sign, siginfo_t * si, void * ucontext)
{
    setenv("LOG_PATH", "/AVS/log/newlogfile.txt", 1);
    logwrite("SIGHUP", connfd);
    errno = EAGAIN;
    fprintf(stderr,"sig_hup_2s");
    sigval_t s = si->si_value;
    struct hup_restore * str = (struct hup_restore *)s.sival_ptr;
    fprintf(stderr,"%d %d\n",str->list,str->addr);
    fprintf(stderr,"%d %d\n",listenfd, server_len);
    child_main(listenfd, server_len);
}
void init_sig()
{
    Signal(SIGPIPE, SIG_IGN);
    Signal(SIGCHLD, sig_chld);

    struct sigaction act_1;
    sigemptyset(&act_1.sa_mask);
    act_1.sa_flags = SA_SIGINFO;
    act_1.sa_sigaction = &sig_term;
    if(sigaction(SIGTERM, &act_1 ,NULL) == -1)
    {
        fprintf(stderr, "Error sigaction: SIGTERM\n");
        exit(EXIT_FAILURE);
    }
    struct sigaction act_2;
    sigemptyset(&act_2.sa_mask);
    act_2.sa_flags = SA_SIGINFO;
    act_2.sa_sigaction = &sig_int;
    if(sigaction(SIGINT, &act_2 ,NULL) == -1)
    {
        fprintf(stderr, "Error sigaction: SIGINT\n");
        exit(EXIT_FAILURE);
    }
    struct sigaction act_3;
    sigemptyset(&act_3.sa_mask);
    act_3.sa_flags = SA_SIGINFO;
    act_3.sa_sigaction = &sig_hup;
    if(sigaction(SIGHUP, &act_3 ,NULL) == -1)
    {
        fprintf(stderr, "Error sigaction: SIGHUP\n");
        exit(EXIT_FAILURE);
    }
}
