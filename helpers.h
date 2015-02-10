
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <errno.h>
#include <pthread.h>
#include <netdb.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>
#include <assert.h>



#define FILE_STORAGE_BUFFER_SIZE 15000000
#define SOCKET_ERROR -1
#define FILE_STORAGE_BUFFER_SIZE 15000000
#define FILE_TRANSFER_BUFFER_SIZE 2048
#define QUEUE_SIZE 1024
#define HOST_NAME_SIZE      255
#define WORKER_QUEUE_SIZE 5000

//--------------STRUCTURE DECLARATIONS--------------

//--------------WORKER STRUCTURES
struct worker_args_struct2 {
    //char* file_name;
    int thread_id;
    char* download_directory;
    int file_paths_size;
    char* file_paths[1024];
};
struct boss_metrics{
    int t_start;
    int total_msec;
    int t_end;
    int total_files;
    int total_bytes_sent;
};
struct thread_metrics{
    float bytes_sent;
    int finished_files;
};
struct client_params{
    char* PARAM_SERVER_ADDRESS;
    int PARAM_PORT;
    int PARAM_NUM_WORKER_THREADS;
    char* PARAM_PATH_TO_WORKLOAD_FILE;
    char* PARAM_PATH_TO_DOWNLOAD_DIRECTORY;
    int PARAM_NUM_OF_REQUESTS;
};
//--------------SERVER STRUCTURES
struct server_params{
    int PARAM_PORT;
    int PARAM_NUM_WORKER_THREADS;
    char* PARAM_PATH;
};
struct server_worker_args {
    int hsocket;
};
struct server_worker_arg_queue {
    struct server_worker_args worker_args_array[WORKER_QUEUE_SIZE];
    int NEXT_TO_ADD;
    int NEXT_TO_REMOVE;
    int SIZE;
};
//--------------FUNCTION DECLARATIONS---------------

//----QUEUE------
void InitializeQueue(struct server_worker_arg_queue *wq);
void AddToEnd(struct server_worker_arg_queue *wq,struct server_worker_args wa);
struct server_worker_args RemoveFromFront(struct server_worker_arg_queue *wq);

//----FILE I/O----
void WriteToFile(char buffer[FILE_STORAGE_BUFFER_SIZE],char* filePath);
char* ReadFile(char* fileAddress);
char* ReadFile_wSize(char* fileAddress, long *fileSize);

//---STRING MANIPULATION-----
char *substring(char *string, int position, int length);
size_t explode(const char *delim, const char *str,char **pointers_out, char *bytes_out);

//---SOCKET HELPER-----
void FillAddress(struct sockaddr_in *Address,int nHostPort);
int CreateServerSocket(int *hServerSocket);

//--------------CODE-----------------------

//----QUEUE----
void InitializeQueue(struct server_worker_arg_queue *wq){
    wq->NEXT_TO_ADD = 0;
    wq->NEXT_TO_REMOVE = 0;
    wq->SIZE = WORKER_QUEUE_SIZE;
    printf("---QUEUE INITALIZED----");
}
void AddToEnd(struct server_worker_arg_queue *wq,struct server_worker_args wa){

    printf("\nINSIDE ADD TO END\n");
    // get array of worker queue, goto end, set it equal to the new set of arguments
    wq->worker_args_array[wq->NEXT_TO_ADD] = wa;

    if(wq->NEXT_TO_ADD != WORKER_QUEUE_SIZE)
    {
        wq->NEXT_TO_ADD ++;
    }
    else {
        wq->NEXT_TO_ADD = 0;
    }
    printf("---ITEM ADDED SOCKED DESCRIPTOR %d IN ARRAY POS. %d--- NEXT TO REMOVE %d \n", wq->worker_args_array[wq->NEXT_TO_ADD].hsocket, wq->NEXT_TO_ADD, wq->NEXT_TO_REMOVE);

}
struct server_worker_args RemoveFromFront(struct server_worker_arg_queue *wq){

    printf("\nINSIDE REMOVE FROM FRONT\n");
    int returnVal = wq->NEXT_TO_REMOVE;

    if(wq->NEXT_TO_REMOVE != WORKER_QUEUE_SIZE) {
        printf("\nNEXT TO REMOVe DOES NOT EQUAL WORKER QUEUE SIZE\n");
        wq->NEXT_TO_REMOVE ++;
    }
    else{
        printf("\nNEXT TO REMOVe DOES EQUAL WORKER QUEUE SIZE\n");
        wq->NEXT_TO_REMOVE = 0;
    }

    printf("---ITEM REMOVED ARRAY ITEM NUMBER %d---\n",returnVal);
    return wq->worker_args_array[returnVal];
}

//----FILE I/O----
void WriteToFile(char buffer[FILE_STORAGE_BUFFER_SIZE],char* filePath)
{
    FILE *fptr;
    //char* filePath = "payload.txt";
    fptr = fopen(filePath,"a+");
    fprintf(fptr,"%s",buffer);
    fclose(fptr);
}
char* ReadFile(char* fileAddress){

    char *buffer;
    FILE *fh = fopen(fileAddress, "rb");
    if ( fh != NULL )
    {
        fseek(fh, 0L, SEEK_END);
        long s = ftell(fh);
        rewind(fh);
        buffer = malloc(s);
        if ( buffer != NULL )
        {
            fread(buffer, s, 1, fh);
            // we can now close the file
            fclose(fh); fh = NULL;

            // do something, e.g.
            //fwrite(buffer, s, 1, stdout);
            //free(buffer);
        }
        if (fh != NULL) fclose(fh);
    }

    return buffer;
}
char* ReadFile_wSize(char* fileAddress, long *fileSize){

    char *buffer;
    FILE *fh = fopen(fileAddress, "rb");
    if ( fh != NULL )
    {
        fseek(fh, 0L, SEEK_END);
        long s = ftell(fh);
        *fileSize = s;
        rewind(fh);

        buffer = malloc(s);
        if ( buffer != NULL )
        {
            fread(buffer, s, 1, fh);
            // we can now close the file
            fclose(fh); fh = NULL;

            // do something, e.g.
            //fwrite(buffer, s, 1, stdout);
            //free(buffer);
        }
        if (fh != NULL) fclose(fh);
    }

    return buffer;
}

//---STRING MANIPULATION-----
char *substring(char *string, int position, int length)
{
    char *pointer;
    int c;

    pointer = malloc(length+1);

    if (pointer == NULL)
    {
        printf("Unable to allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    for (c = 0 ; c < position -1 ; c++)
        string++;

    for (c = 0 ; c < length ; c++)
    {
        *(pointer+c) = *string;
        string++;
    }

    *(pointer+c) = '\0';

    return pointer;
}
size_t explode(const char *delim, const char *str, char **pointers_out, char *bytes_out)
{
    size_t  delim_length        = strlen(delim);
    char   **pointers_out_start = pointers_out;
    assert(delim_length > 0);
    for (;;) {
        const char *delim_pos = strstr(str, delim);
        *pointers_out++ = bytes_out;
        if (delim_pos == NULL) {
            strcpy(bytes_out, str);
            return pointers_out - pointers_out_start;
        } else {
            while (str < delim_pos)
                *bytes_out++ = *str++;
            *bytes_out++ = '\0';
            str += delim_length;
        }
    }
}

//---SOCKET HELPER-----
void FillAddress(struct sockaddr_in *Address,int nHostPort) {
    //FILL ADDRESS STRUCTURE
    printf("\ncreating address...\n");
    Address->sin_addr.s_addr=INADDR_ANY;
    //google what INADDR_ANY is...basically tells to not worry about address
    Address->sin_port=htons(nHostPort);
    //hook onto port passed by parameter
    Address->sin_family=AF_INET;
    //ipv4
}
int CreateServerSocket(int *hServerSocket){

    *hServerSocket = socket(AF_INET,SOCK_STREAM,0);
    //socket() function creates a socket and returns a descriptor
    //which can be passed to other functions
    // AF_INET - address family (ipv4)
    // SOCK_STREAM - TCP....SOCK_DGRAM is UDP
    // 0 - designates IP Protocol
    //socket error is just -1
    if(*hServerSocket == SOCKET_ERROR)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}
