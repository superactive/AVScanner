#include "avs_client.h"
pthread_mutex_t	mlock = PTHREAD_MUTEX_INITIALIZER;
int main(int argc, char ** argv)
{
    if(argc < 2)
        err_sys("Error: few arguments");
    fprintf(stderr,"Valid commands:\n\n"
            "SCAN\t Scan a file\t\t\t \"SCAN /folder/file.txt\"\n"
            "STAT\t Get progress\t\t\t \"STAT\"\n"
            "BYE\t Disconnect\t\t\t \"BYE\"\n"
            "REPO\t Get a report\t\t\t \"REPO\"\n"
            "INFO\t Get information about server\t \"INFO\"\n\n"
            );
    findex = 0;
    int result, i;
    for(i = 0; i < BUFFSIZE; ++i)
        files_id[i] = 0;
    int                 sockfd;
    socklen_t           len;
    struct sockaddr_un  address;

    char * socket_path = argv[1];
//    "/AVS/socket/server_socket");

    if ((sockfd = socket(AF_UNIX,SOCK_STREAM,0)) < 0)
        err_sys("Error: socket");

    address.sun_family = AF_UNIX;
    strcpy(address.sun_path,socket_path); // ~
    len = sizeof(address);


    if((result = connect(sockfd, (struct sockaddr *)&address, len)) == -1)
        err_sys("Error: connect");

    int pthreadResult;
    pthread_t thread;
    // Thread for reading
    pthreadResult=pthread_create(&thread,NULL,&func_read,&sockfd);
    if(pthreadResult!=0)
        perror("Error pthread: 1");
    char buff[BUFFSIZE];
    for(i = 2; i < argc; ++i)
    {
        files[findex] = (char *)malloc(sizeof(char) * strlen(argv[i]));
        strcpy(files[findex], argv[i]);
        ++findex;

        memset(buff,0,BUFFSIZE);
        memcpy(buff, "SCAN ",5);
        strcat(buff, argv[i]);
        strcat(buff, "\n");
        fputs(buff, stderr);
        Writen(sockfd, buff, strlen(buff));
        sleep(1);
    }
    while(1)
    {
        fgets (buff, BUFFSIZE, stdin);
        Writen(sockfd, buff, strlen(buff));
        if(!strncmp(buff, "SCAN /", 6))
        {
            pthread_mutex_lock(&mlock);
            buff[strlen(buff)-1] = '\0';
            char * str = strchr(buff,' ');
            files[findex] = (char *)malloc(sizeof(char) * strlen(str + 1));
            strcpy(files[findex], str + 1 );
            ++findex;
            pthread_mutex_unlock(&mlock);
        }
    }
    Close(sockfd);
    return 0;
}
void * func_read(void * arg)
{
    int sockfd = *(int *)arg;
    char buff[BUFFSIZE];
    size_t bytes = 0;
    while(1)
    {
        while( (bytes = read(sockfd,buff, BUFFSIZE)) > 0)
        {
            buff[bytes] = '\0';
            if(fputs(buff, stderr) == EOF)
                err_sys("Error: fputs");
            if(bytes < BUFFSIZE)
                break;
        }
        if(!strncmp(buff,"002",3))
        {
            char * space_end = strrchr(buff,' ');
            int i = 0;
            pthread_mutex_lock(&mlock);
            for(; i < findex; ++i)
            {
                if(!strncmp(space_end + 1, files[i], strlen(files[i])))
                    files_id[i] = 1;
            }
            pthread_mutex_unlock(&mlock);
        }
        if(!strncmp(buff,"250",3))
        {
            char * space_end = strrchr(buff,' ');
            int i = 0;
            pthread_mutex_lock(&mlock);
            for(; i < findex; ++i)
            {
                if(!strncmp(space_end + 1, files[i], strlen(files[i])))
                {
                    files_id[i] = 2;
                    break;
                }
            }
            pthread_mutex_unlock(&mlock);
        }

        if(!strncmp(buff,"099",3))
            exit(EXIT_SUCCESS);
        if(!strncmp(buff,"000 REPORT",10))
        {
            int i = 0;
            for(; i < findex; ++i)
            {
                fprintf(stderr,"%s ",files[i]);
                if(files_id[i] == 1)
                    fprintf(stderr,"\tinfected\n");
                else
                {
                    if (files_id[i] == 2)
                        fprintf(stderr,"\tnot found\n");
                    else
                        fprintf(stderr,"\tclean\n");
                }
            }
        }
        bytes = 0;
    }
}

