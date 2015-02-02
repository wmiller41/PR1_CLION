#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>

#define SOCKET_ERROR        -1
#define FILE_STORAGE_BUFFER_SIZE 15000000
#define FILE_TRANSFER_BUFFER_SIZE 2048
#define HOST_NAME_SIZE      255


/*
Client Requirements
The client must satisfy the following requirements:
        Must be implemented using the Boss-Worker thread pattern
        Must spawn a pool of worker threads which will make GetFile requests
        Must have a queue mechanism for communication between the boss and worker threads
        Must create a workload for the server
Must collect and compute the following metrics and write them to a file
        Total bytes received units here [B]
Average response time units here [s]
Average throughput [B/s]
The client must generate a workload as follows:
The client must generate a total number of requests (R)
R should be uniformly distributed all worker threads (T) such that each worker performs approximately R/T requests
The client must request files specified in a workflow file
The workflow file must contain 1 or more filenames (ex: /myfile.html)
The workflow file must support up to 100 unique filenames
Filenames will be no longer than 200 characters
        Each filename should be on a separate line
        For each request, filenames will be assigned to the worker, either randomly or in a round robin manner.
The worker threads must satisfy the following requirements:
Must create socket and connect to an accepting server
        Must send request, receive response, and then close connection with the server
        If a download directory is specified at the command line, then the worker must take the response (file) and write the file to the download directory
If the download directory is specified as null, then the response data may be discarded
Must perform the above steps for a number of iterations specified by the Boss thread
        Must communicate metrics back to Boss thread
*/



char* ReadFile(char* fileAddress);
void WriteToFile(char buffer[FILE_STORAGE_BUFFER_SIZE],char* filePath);
char *substring(size_t start, size_t stop, const char *src, size_t size);
size_t explode(const char *delim, const char *str,char **pointers_out, char *bytes_out);
//int DoWork(char *FILE_NAME, int hSocket);
void *DoWorkThread(void* THREAD_ARGS);

struct worker_args_struct {
    char* file_name;
    int hsocket;
    int thread_id;
    char* full_file_path;
};






pthread_t threads[100];

/*
options:
-s server address (Default: 0.0.0.0)
-p server port (Default: 8888)
-t number of worker threads (Default: 1, Range: 1-100)
-w path to workload file (Default: workload.txt)
-d path to downloaded file directory (Default: null)
-r number of total requests (Default: 10, Range: 1-1000)
*/

struct client_params{
    char* PARAM_SERVER_ADDRESS;
    int PARAM_PORT;
    int PARAM_NUM_WORKER_THREADS;
    char* PARAM_PATH_TO_WORKLOAD_FILE;
    char* PARAM_PATH_TO_DOWNLOAD_DIRECTORY;
    int PARAM_NUM_OF_REQUESTS;
};

struct client_params PARAMETERS;

struct client_params get_parameters(int num_args, char** arguments);

struct client_params get_parameters(int num_args, char** arguments){


    struct client_params RETURN_PARAMS;
    RETURN_PARAMS.PARAM_PORT = 8888;
    RETURN_PARAMS.PARAM_NUM_WORKER_THREADS = 1;
    RETURN_PARAMS.PARAM_SERVER_ADDRESS = "0.0.0.0";
    RETURN_PARAMS.PARAM_PATH_TO_WORKLOAD_FILE = "workload.txt";
    RETURN_PARAMS.PARAM_NUM_OF_REQUESTS = 10;


    int c;

    opterr = 0;
    while ((c = getopt (num_args, arguments, "ptfh:")) != -1)
        switch (c)
        {
            case 'p':
                RETURN_PARAMS.PARAM_PORT = atoi(optarg);
                break;
            case 't':
                RETURN_PARAMS.PARAM_NUM_WORKER_THREADS = atoi(optarg);
                break;
            case 'w':
                RETURN_PARAMS.PARAM_PATH_TO_WORKLOAD_FILE = optarg;
                break;
            case 's':
                RETURN_PARAMS.PARAM_SERVER_ADDRESS = optarg;
                break;
            case 'd':
                RETURN_PARAMS.PARAM_PATH_TO_WORKLOAD_FILE = optarg;
                break;
            case 'r':
                RETURN_PARAMS.PARAM_NUM_OF_REQUESTS = atoi(optarg);
                break;
            default:
                abort ();
        }


    //printf("args...port:%d threads:%d path:%s",
            //RETURN_PARAMS.PARAM_PORT,
            //RETURN_PARAMS.PARAM_NUM_WORKER_THREADS);
            //RETURN_PARAMS.PARAM_PATH);

    return RETURN_PARAMS;


}



int main(int argc, char **argv) {
        printf("------WEB CLIENT STARTED-----\n");


        int hSocket;                 /* handle to socket */
        struct hostent *pHostInfo;   /* holds info about a machine */
        struct sockaddr_in Address;  /* Internet socket address stuct */
        long nHostAddress;
        //char pBuffer[FILE_STORAGE_BUFFER_SIZE];
        //unsigned nReadAmount;
        char strHostName[HOST_NAME_SIZE];
        int nHostPort;
        int NUM_REQUESTS;
        char *WORKLOAD_FILE_NAME;
        char *FULL_FILE_PATH = malloc(256);
        //int NUM_WORKER_THREADS;
        FULL_FILE_PATH[0] = '\0';
        //place in folder

        if (argc < 3) {
            printf("\nUsage: client host-name host-port\n");
            return 0;
        }
        else {
            // get parameters
            PARAMETERS = get_parameters(argc,argv);
            // assign parameters
            strcpy(strHostName,PARAMETERS.PARAM_SERVER_ADDRESS);
            nHostPort = PARAMETERS.PARAM_PORT;
            WORKLOAD_FILE_NAME = PARAMETERS.PARAM_PATH_TO_WORKLOAD_FILE;
            NUM_REQUESTS = PARAMETERS.PARAM_NUM_OF_REQUESTS;
            strcat(FULL_FILE_PATH,PARAMETERS.PARAM_PATH_TO_DOWNLOAD_DIRECTORY);
            //NUM_WORKER_THREADS = PARAMETERS.PARAM_NUM_WORKER_THREADS;
        }


        /*---TO DO---*/

        //1.) read workload file in

        char *WORKLOAD_FILE_CONTENTS = ReadFile(WORKLOAD_FILE_NAME);

        //2.) USE "GET FILE" protocol to request files
        // CLIENT SHOULD GENERATE A NUMBER OF "GETFILE" REQUESTS (eventually from different threads)
        //we should have a for loop...for each item in the array of files to retrieve...
        //create and send a proper "get file" request for each one and save it in the directory


        //------------------------WORKLOAD FILE DATA-------------------
        printf("\nWORKLOAD FILE CONTENTS \"%s\"", WORKLOAD_FILE_CONTENTS);
        char **TARGET_FILE_POINTERS = malloc((256 * (sizeof(char))));
        char *BYTES_OUT = malloc((256 * (sizeof(char))));
        int TARGET_FILES_SIZE = explode("\n", WORKLOAD_FILE_CONTENTS, TARGET_FILE_POINTERS, BYTES_OUT);
        printf("Seperated workload into \"%d\" files", TARGET_FILES_SIZE);


        //------------------------THREAD DATA--------------------------
        // one thread per file for now

        //-------POOL OF THREADS-------

        //for loop begin
        //for each file to get
        int i = 0;
        for (i = 0; i < NUM_REQUESTS; i++) {


            int ran_number = rand() % 4 + 1;

                //------------------SOCKET STUFF---------------------------
                printf("\nMaking a socket");
                /* make a socket */
                hSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
                if (hSocket == SOCKET_ERROR) {
                    printf("\nCould not make a socket\n");
                    return 0;
                }
                /* get IP address from name */
                pHostInfo = gethostbyname(strHostName);
                /* copy address into long */
                memcpy(&nHostAddress, pHostInfo->h_addr, pHostInfo->h_length);
                /* fill address struct */
                Address.sin_addr.s_addr = nHostAddress;
                Address.sin_port = htons(nHostPort);
                Address.sin_port = htons(nHostPort);
                Address.sin_family = AF_INET;
                printf("\nConnecting to %s on port %d", strHostName, nHostPort);
                /* connect to host */
                if (connect(hSocket, (struct sockaddr *) &Address, sizeof(Address)) == SOCKET_ERROR) {
                    printf("\nCould not connect to host\n");
                    return 0;
                }

                //-----------------END SOCKET STUFF-------------------------

                //-----------------THREAD STUFF-----------------

                int rc;
                long t = i;



                //maybe instead of sending a single structure we sohuld send a queue of structures...
                //more effecienit than constantly sending structures

                //-----CREATE PARAMS FOR THREAD-----
                struct worker_args_struct thread_args;
                thread_args.file_name = TARGET_FILE_POINTERS[ran_number];
                printf("THREAD ARG FILE NAME FOR THREAD %ld is %s \n", t, thread_args.file_name);
                thread_args.hsocket = hSocket;
                printf("THREAD ARG SOCKET HANDLER FOR THREAD %ld is %d \n", t, thread_args.hsocket);
                thread_args.thread_id = t;
                printf("THREAD ARG ID FOR THREAD %ld is %d \n", t, thread_args.thread_id);
                // need file path too
                thread_args.full_file_path = PARAMETERS.PARAM_PATH_TO_DOWNLOAD_DIRECTORY;

                printf("\nIn file loop: creating thread %ld\n", t);
                rc = pthread_create(&threads[t], NULL, DoWorkThread, (void *) &thread_args);
                if (rc) {
                    printf("\nERROR; return code from pthread_create() is %d\n", rc);
                    exit(-1);
                }
        }

        int iter;
        for (iter = 0; iter < 100; iter++) {
            //try to rejoin main thread in worker function
            pthread_join(threads[iter], NULL);
        }

        return 0;
    }


     //int DoWork(char *FILE_NAME, int hSocket) {
     void *DoWorkThread(void* THREAD_ARGS){


                struct worker_args_struct *arguments;
                arguments = (struct worker_args_struct*) THREAD_ARGS;

                int thread_id = arguments->thread_id;
                printf("THREAD #%d SPINNING UP\n",thread_id);

                char* FILE_NAME = arguments->file_name;
                int hSocket = arguments->hsocket;


                //long REQUESTED_FILE_SIZE;
                char *GET_FILE_RESPONSE = malloc((256 * (sizeof(char))));
                char *GET_FILE_REQUEST = malloc((256 * (sizeof(char))));
                char *GET_FILE_TOTAL_SIZE = malloc((256 * (sizeof(char))));
                // destination path


                //----#1------------FORM GETFILE REQUEST--------------
                GET_FILE_REQUEST[0] = '\0';
                strcat(GET_FILE_REQUEST, "GetFile GET ");
                //target file pointer is file name with \ attached
                strcat(GET_FILE_REQUEST, FILE_NAME);
                strcat(GET_FILE_REQUEST, "\0");
                printf("\nTHREAD %d FULL GET FILE REQUEST TO SEND: \"%s\"",thread_id,GET_FILE_REQUEST);

                //----#2------------SEND GET FILE REQUEST----------------


                //SLEEP FOR 4 SECONDS TO ALLOW SERVER TO BE READY???

                write(hSocket, GET_FILE_REQUEST, 256 * sizeof(char));
                sleep(1);

                //----#3------------READ GET FILE RESPONSE/SIZE TO EXPECT---------------
                //GetFile STATUS filesize FILE
                //get response
                read(hSocket, GET_FILE_RESPONSE, 256 * (sizeof(char)));
                // get file size
                read(hSocket, GET_FILE_TOTAL_SIZE, 256 * (sizeof(char)));
                printf("\nReceived \"%s\" from server\n", GET_FILE_RESPONSE);
                printf("\n File to expect is size \"%s\"\n", GET_FILE_TOTAL_SIZE);

                //----#4------------PARSE REQUEST STATUS-----------------
                char *RESPONSE_STATUS = substring(8, 2, GET_FILE_RESPONSE, 3);
                printf("\nStatus of response is  \"%s\", read amount \"%ld\"\n", RESPONSE_STATUS, sizeof(RESPONSE_STATUS));
                if (strcmp(RESPONSE_STATUS, "OK") != 0) {
                    printf("\nThere was an error!");
                    close(hSocket);
                    return 0;
                }

                //----#5------------CREATE DESTINATION PATH-------------
                //file name already has /
                char* FULL_FILE_PATH = arguments->full_file_path;
                strcat(FULL_FILE_PATH, FILE_NAME);
                printf("Dest. final name is \"%s\" \n", FULL_FILE_PATH);

                FILE *fp = fopen(FULL_FILE_PATH, "ab");
                if (NULL == fp) {
                    printf("Error opening file");
                    exit(1);
                }

                int bytesReceived = 0;
                char buff[FILE_TRANSFER_BUFFER_SIZE];
                memset(buff, '0', sizeof(buff));
                while ((bytesReceived = recv(hSocket, buff, 4, 0)) > 0) {
                    fwrite(buff, 1, bytesReceived, fp);
                    if (bytesReceived % 100 == 0) {
                        printf("Bytes received %d\n", bytesReceived);
                    }
                }
                if (bytesReceived < 0) {
                    printf("\n Read Error \n");
                    printf("Errno %d %s\n", errno, strerror(errno));
                }

                //---------------CLOSE FILE---------
                fclose(fp);
                //char log[256];
                //sprintf(log,"Writing to file \"%s\"\n",FullFilePath);
                //WriteToFile(log,"CLIENT-LOG.txt");
                //WriteToFile(STORAGE_BUFFER,FullFilePath);

                //printf("\nReceived \"%s\" from server\n",pBuffer);
                //printf("\nRecieved amount is \"%d\" ", nReadAmount);
                //printf("\nWriting \"%s\" to file....\n",pBuffer);

                printf("\nClosing socket\n");
                /* close socket */
                if (close(hSocket) == SOCKET_ERROR) {
                    printf("\nCould not close socket\n");
                    return 0;
                }

                //---------------FREE POINTERS-----------
                free(FULL_FILE_PATH);
                free(GET_FILE_REQUEST);
                free(GET_FILE_RESPONSE);
                free(GET_FILE_TOTAL_SIZE);

         pthread_join(threads[thread_id], NULL);

         return 0;
     }





//-----------------HELPER METHODS----------------------
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

char *substring(size_t start, size_t stop, const char *src, size_t size)
{
    //char dst[size];
    char* dst = malloc(size * sizeof(char));

    int count = stop - start;
    if ( count >= --size )
    {
        count = size;
    }
    sprintf(dst, "%.*s", count, src + start);
    return dst;
}
size_t explode(const char *delim, const char *str, char **pointers_out, char *bytes_out)
{
    size_t  delim_length        = strlen(delim);
    char   **pointers_out_start = pointers_out;

    assert(delim_length > 0);

    for (;;) {
        /* Find the next occurrence of the item delimiter. */
        const char *delim_pos = strstr(str, delim);

        /*
         * Emit the current output buffer position, since that is where the
         * next item will be written.
         */
        *pointers_out++ = bytes_out;

        if (delim_pos == NULL) {
            /*
             * No more item delimiters left.  Treat the rest of the input
             * string as the last item.
             */
            strcpy(bytes_out, str);
            return pointers_out - pointers_out_start;
        } else {
            /*
             * Item delimiter found.  The bytes leading up to it form the next
             * string.
             */
            while (str < delim_pos)
                *bytes_out++ = *str++;

            /* Don't forget the NUL terminator. */
            *bytes_out++ = '\0';

            /* Skip over the delimiter. */
            str += delim_length;
        }
    }
}
