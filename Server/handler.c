#include "avs.h"
const char success[] = "000 ";
const char scan_success[] = "001 ";
const char virus_detected[] = "002 ";
const char goodbye[] = "099 ";
const char fail[] = "200 ";
const char unknown_command[] = "201 ";
const char wrong_arg[] = "202 ";
const char wrong_path_to_file[] = "250 ";
const char fail_server[] = "500 ";

// Main function for processing commands
void handler(int desc)
{
    fprintf(stderr,"handler\n");
    size_t bytes;
    char buff[BUFFSIZE];
    char ans[BUFFSIZE];
    pthr = 0;
    int status = 1;
    while(1)
    {
        if(!status)
            return;
        memset(ans,0,BUFFSIZE);
        memset(buff,0,BUFFSIZE);
        while( (bytes = read(desc, buff, BUFFSIZE)) > 0)
        {
            buff[bytes-1] = '\0';
            if(fputs(buff, stderr) == EOF)
                err_sys("Error: fputs");
            if(bytes < BUFFSIZE)
                break;
        }
        logwrite(buff, connfd);
        int ident = 0;
        // Validation
        if(!valid_command(buff, &ident))
        {
            memcpy(ans, unknown_command, strlen(unknown_command));
            strcat(ans, "Don't know how to '");
            strcat(buff, "'\r\n");
            strcat(ans, buff);
            status = Writen(desc,ans,strlen(ans));
            continue;
        }

        switch (ident)
        {
        case 1:
        {
            status = info_handler(desc);
            break;
        }
        case 2:
        {
            // Parallel scanning of files
            if(pthr != processed_files)
            {
                pthread_t thread;
                processes[pthr] = (struct scan_struct *)malloc( sizeof(struct scan_struct));
                if(processes[pthr] == NULL) perror("malloc");
                processes[pthr]->desc = desc;
                processes[pthr]->ptbuff = (char *)malloc(sizeof(char) * strlen(buff));
                strcpy(processes[pthr]->ptbuff, buff);
                processes[pthr]->proc = 0;
                processes[pthr]->remove = 0;
                processes[pthr]->thread_id = thread;
                pthr++;
                pthread_create(&thread,NULL,&scan_handler, processes[pthr-1]);
            }

            break;
        }
        case 3:
        {
            status = bye_handler(desc, processes);
            return;
        }
        case 4:
        {
            status = stat_handler(desc, processes);
            break;
        }
        case 5:
        {
            status = report_handler(desc, processes);
            break;
        }
        default:
            break;
        }
    }
}
// Validation commands
int valid_command(char * str, int * ident)
{
    fprintf(stderr,"valid_handler\n");
    char * space = strchr(str,' ');
    int shift;
    if(space == NULL)
        shift = strlen(str);
    else
        shift = space - str;

    if(shift == 3)
    {
        if(!strncmp(str, "BYE", shift))
        {
            *ident = 3;
            return 1;
        }
    }
    if(shift == 4)
    {
        if(!strncmp(str, "INFO", shift))
        {
            *ident = 1;
            return 1;
        }
        if(!strncmp(str, "SCAN", shift))
        {
            *ident = 2;
            return 1;
        }

        if(!strncmp(str, "STAT", shift))
        {
            *ident = 4;
            return 1;
        }
        if(!strncmp(str, "REPO", shift))
        {
            *ident = 5;
            return 1;
        }
    }
    str[shift] = '\0';
    return 0;
}
int info_handler(int desc)
{
    char buff[BUFFSIZESML];
    memset(buff,0,BUFFSIZESML);
    memcpy(buff, success, strlen(success));
    strcat(buff, "(VER_1_0) Jupiter Development Scanning Server\r\n");
    return Writen(desc,buff,strlen(buff));
}
// Function to scan ( thread-safe)
void * scan_handler(void * ar)
{
    struct scan_struct * arg = (struct scan_struct *)ar;
    int desc = arg->desc;
    char * args = arg->ptbuff;
    double * proc = &arg->proc;

    char buff[BUFFSIZESML];

    memset(buff,0,BUFFSIZESML);
    char * space_begin = strchr(args,' ');
    char * space_end = strrchr(args,' ');
    if(space_begin != space_end)
    {
        memcpy(buff, wrong_arg, strlen(wrong_arg));
        strcat(buff, space_begin);
        strcat(buff, "\r\n");
        Writen(desc,buff,strlen(buff));
        arg->remove = 1;
        return NULL;
    }

    FILE * pFileVDB;
    char * virusdb = getenv("VDB_PATH");
    if(virusdb == NULL)
        err_sys("Error: log_VDB");

    pFileVDB = fopen(virusdb,"rb");
    if(pFileVDB == NULL)
    {
        memcpy(buff, fail_server, strlen(fail_server));
        strcat(buff, "Unable to access VDB\r\n");
        Writen(desc,buff,strlen(buff));
        arg->remove = 1;
        return NULL;
    }
    FILE * pFile;
    pFile = fopen(space_begin+1,"rb");
    if(pFile == NULL)
    {
        memcpy(buff, wrong_path_to_file, strlen(wrong_path_to_file));
        strcat(buff, "No such file");
        strcat(buff, space_begin);
        strcat(buff, "\r\n");
        Writen(desc,buff,strlen(buff));
        arg->remove = 1;
        return NULL;
    }
    fseek(pFileVDB, 0L, SEEK_END);
    long int size_file_VDB = ftell(pFileVDB), pointer = 0;
    fseek(pFileVDB, 0L, SEEK_SET);

    char * virus_word = NULL;
    size_t len = 0;
    ssize_t bytes;

    while ((bytes = getline(&virus_word, &len, pFileVDB)) != -1)
    {
        memset(buff,0,BUFFSIZESML);
        *proc = (double)ftell(pFileVDB)/(double)size_file_VDB;
        virus_word[bytes - 1] = '\0';
        int c;
        size_t sizeVW = strlen(virus_word), i = 0;
        while (( c = fgetc(pFile)) != EOF)
        {
            if( c == virus_word[i])
            {
                ++i;
                if( i == sizeVW)
                {
                    memcpy(buff, virus_detected, strlen(virus_detected));
                    strcat(buff, virus_word);
                    strcat(buff, space_begin);
                    strcat(buff, "\r\n");
                    Writen(desc,buff,strlen(buff));
                    break;
                }
                pointer = 1;
                while (( c = fgetc(pFile)) != EOF)
                {
                    if( c == virus_word[i])
                    {
                        ++i;
                        if( i == sizeVW)
                        {
                            memcpy(buff, virus_detected, strlen(virus_detected));
                            strcat(buff, virus_word);
                            strcat(buff, space_begin);
                            strcat(buff, "\r\n");
                            Writen(desc,buff,strlen(buff));
                            break;
                        }
                        ++pointer;
                    }
                    else
                    {
                        fseek(pFile, -pointer, SEEK_CUR);
                        i = 0;
                        break;
                    }
                }
                if(i != 0)
                    break;
            }
        }
        fseek(pFile, 0, SEEK_SET);
        free(virus_word);
        virus_word = NULL;
        sleep(1);
    }
    *proc = 1;
    return NULL;
}
// Obtaining progress and complete removal of threads
int stat_handler(int desc, struct scan_struct * * process)
{
    fprintf(stderr,"stat_handler\n");
    int i = 0, j = 0;
    char num[16];
    double load;

    char buff[BUFFSIZESML];
    memset(buff,0,BUFFSIZESML);
    if(pthr == 0)
    {
        memcpy(buff, success, strlen(success));
        strcat(buff, "(files are missing)\r\n");
        return Writen(desc, buff, strlen(buff));
    }
    for(; i < pthr; ++i)
    {
        memset(buff,0,BUFFSIZESML);
        if(process[i]->remove == 1)
        {
            if(i != (pthr - 1))
            {
                free_scan_struct(process, i);
                i--;
                --pthr;
                continue;
            }
            else
            {
                free(process[i]);
                --pthr;
                break;
            }
        }
        load = process[i]->proc * 100;
        memcpy(buff, success, strlen(success));
        strcat(buff, "(");
        snprintf(num, 5, "%f", load);
        strcat(buff, num);
        strcat(buff, "%) [");
        for(; j < 10; ++j)
        {
            if((load / 10.0) > j)
                num[j]='>';
            else
                num[j]='.';
        }
        j = 0;
        num[10] = '\0';
        strcat(buff, num);
        strcat(buff, "]\r\n");
        Writen(process[i]->desc, buff, strlen(buff));
        if(load == 100.0)
        {
            free_scan_struct(process, i);
            i--;
            --pthr;
        }
    }
    memset(buff,0,BUFFSIZESML);
    if(i == 0 && load!=100)
    {
        memcpy(buff, success, strlen(success));
        strcat(buff, "(files are missing)\r\n");
        return Writen(desc, buff, strlen(buff));
    }
    return 1;
}
void free_scan_struct(struct scan_struct * * process, int k)
{
    fprintf(stderr,"free_scan_struct\n");
    free(process[k]);
    int p = k;
    for(; p < pthr - 1; ++p)
        process[p] = process[p + 1];
}
// To complete the connection
int bye_handler(int desc, struct scan_struct * *  process)
{
    fprintf(stderr,"bye_handler\n");
    int i = 0;
    for(; i < pthr; ++i)
    {
        pthread_cancel(process[i]->thread_id);
        if(!pthread_equal(pthread_self(), process[i]->thread_id))
            pthread_join(process[i]->thread_id, NULL);
        fprintf(stderr,"Thread cancelled: %lu\n", process[i]->thread_id);
        free(process[i]);
    }
    if(errno == EINTR)
        return 0;
    char buff[BUFFSIZESML];
    memset(buff,0,BUFFSIZESML);
    if(errno == EIO)
    {
        memcpy(buff, goodbye, strlen(goodbye));
        strcat(buff, "Server is unavailable\r\n");
        Writen(desc, buff, strlen(buff));
        Close(desc);
        return 0;
    }

    else
    {
        memcpy(buff, goodbye, strlen(goodbye));
        strcat(buff, "Goodbye\r\n");
    }
    return Writen(desc, buff, strlen(buff));
}
// Getting report
int report_handler(int desc, struct scan_struct * *  process)
{
    fprintf(stderr,"report_handler\n");
    char buff[BUFFSIZESML];
    memset(buff,0,BUFFSIZESML);
    memcpy(buff, success, strlen(success));
    strcat(buff, "REPORT\r\n");
    return Writen(desc, buff, strlen(buff));
}

