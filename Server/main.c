#include "avs.h"
#include "daemon.h"
int main(int argc, char ** argv)
{
    int i;
    struct sockaddr_un server_address;
    char * socket_path = getenv("SOCKET_PATH");
    if(socket_path == NULL)
        err_sys("Error: socket_path");

    // Call the daemon
    daemon_init(argv[0],0);

    unlink(socket_path);
    listenfd = socket(AF_UNIX,SOCK_STREAM,0);
    server_address.sun_family = AF_UNIX;
    strcpy(server_address.sun_path, socket_path);
    server_len = sizeof(server_address);
    bind(listenfd,(struct sockaddr *)&server_address, server_len);
    listen(listenfd, LISTENQ);

    pids = Calloc(NUMPROC, sizeof(pid_t));
    init_sig();
    // Writting to logfile
    logwrite("Start Server", 0);

    // Creating child processes for to handle requests
    for (i = 0; i < NUMPROC; i++)
        pids[i] = child_make(listenfd, server_len);
    eternal();
    return 0;
}

