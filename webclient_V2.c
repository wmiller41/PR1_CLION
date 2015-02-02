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

//struct worker_args_struct {
    //char* file_name;
    //int thread_id;
    //char* download_directory;
    //int file_paths_size;
    //char** file_paths;
//};

struct worker_args_struct2 {
    //char* file_name;
    int thread_id;
    char* download_directory;
    int file_paths_size;
    char* file_paths[1024];
};


struct metrics{
    float bytes_sent;
    int total_msec;
    int finished_files;
    int t_start;
    int t_end;

};


struct metrics global_metrics;

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
    RETURN_PARAMS.PARAM_NUM_WORKER_THREADS = 4;
    RETURN_PARAMS.PARAM_SERVER_ADDRESS = "0.0.0.0";
    RETURN_PARAMS.PARAM_PATH_TO_WORKLOAD_FILE = "workload-1.txt";
    RETURN_PARAMS.PARAM_NUM_OF_REQUESTS = 12;
    RETURN_PARAMS.PARAM_PATH_TO_DOWNLOAD_DIRECTORY = "PAYLOAD";


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

char strHostName[HOST_NAME_SIZE];
int nHostPort;
int NUM_REQUESTS;
char *WORKLOAD_FILE_NAME;
char *FILE_NAME;
char *DOWNLOAD_DIRECTORY;
int NUM_WORKER_THREADS;

int main(int argc, char **argv) {

        clock_t start,end;


        global_metrics.bytes_sent = 0;
        global_metrics.finished_files = 0;
        global_metrics.total_msec = 0;
        global_metrics.t_start = 0;
        global_metrics.t_end = 0;




        DOWNLOAD_DIRECTORY = malloc(256 * sizeof(char));

        printf("------WEB CLIENT STARTED-----\n");

        DOWNLOAD_DIRECTORY[0] = '\0';
        //place in folder

        //if (argc < 3) {
            //printf("\nUsage: client host-name host-port\n");
            //return 0;
        //}
        //else {
            // get parameters
            PARAMETERS = get_parameters(argc,argv);
            // assign parameters
            strcpy(strHostName,PARAMETERS.PARAM_SERVER_ADDRESS);
            nHostPort = PARAMETERS.PARAM_PORT;
            WORKLOAD_FILE_NAME = PARAMETERS.PARAM_PATH_TO_WORKLOAD_FILE;
            NUM_REQUESTS = PARAMETERS.PARAM_NUM_OF_REQUESTS;
            DOWNLOAD_DIRECTORY = PARAMETERS.PARAM_PATH_TO_DOWNLOAD_DIRECTORY;
            NUM_WORKER_THREADS = PARAMETERS.PARAM_NUM_WORKER_THREADS;
            printf("NUM WORKER THREADS %d \n",NUM_WORKER_THREADS);
       // }


        /*---TO DO---*/

        //1.) read workload file in

        char *WORKLOAD_FILE_CONTENTS = ReadFile(WORKLOAD_FILE_NAME);

        //2.) USE "GET FILE" protocol to request files
        // CLIENT SHOULD GENERATE A NUMBER OF "GETFILE" REQUESTS (eventually from different threads)
        //we should have a for loop...for each item in the array of files to retrieve...
        //create and send a proper "get file" request for each one and save it in the directory


        //------------------------WORKLOAD FILE DATA--------------------------
        printf("\nWORKLOAD FILE CONTENTS \"%s\"\n", WORKLOAD_FILE_CONTENTS);
        char **TARGET_FILE_POINTERS = malloc((256 * (sizeof(char))));
        char *BYTES_OUT = malloc((256 * (sizeof(char))));
        int TARGET_FILES_SIZE = explode("\n", WORKLOAD_FILE_CONTENTS, TARGET_FILE_POINTERS, BYTES_OUT);
        printf("Seperated workload into \"%d\" files\n", TARGET_FILES_SIZE);

        free(WORKLOAD_FILE_CONTENTS);
        //------------1 per worker--------

        struct worker_args_struct2 MASTER_ARGS_ARRAY[NUM_WORKER_THREADS];
        //struct worker_args_struct MASTER_ARGS_ARRAY = malloc(sizeof(struct worker_args_struct)*NUM_WORKER_THREADS);
        //------------ALLOCATE MEMORY FOR WORKER_ARGS PARAMETER STRUCTURE-----



        int i = 0;
        //int y = 0;
        //for (i = 0; i < NUM_WORKER_THREADS; i++) {
            //MASTER_ARGS_ARRAY->file_paths = malloc(sizeof(char*) * (NUM_REQUESTS / NUM_WORKER_THREADS));
           //for (y = 0; y <= NUM_REQUESTS / NUM_WORKER_THREADS; y++) {
            //MASTER_ARGS_ARRAY[i].file_paths[y] = malloc((256 * (sizeof(char))));
        //}
        //}

        //------------fill the arrays of file names before assigning
        for (i = 0; i < NUM_WORKER_THREADS; i++) {
            printf("INSIDE LOOP FOR THREAD %d",i);
            int x = 0;
            MASTER_ARGS_ARRAY[i].file_paths_size = 0;
            for (x = 0; x < (NUM_REQUESTS/NUM_WORKER_THREADS); x++) {
                printf("x is %d\n",x);
                MASTER_ARGS_ARRAY[i].file_paths[x] = TARGET_FILE_POINTERS[x%TARGET_FILES_SIZE];
                MASTER_ARGS_ARRAY[i].file_paths_size++;
                printf("\nTHREAD %d ASSIGNED MASTER ARG ARRAY ITER %d,FILE PATH NUM %d, FILE PATH %s, WHICH CURRENTLY HAS %d ITEMS\n",
                        i,i,x,MASTER_ARGS_ARRAY[i].file_paths[x],MASTER_ARGS_ARRAY[i].file_paths_size);
            }
        }

        // i guess we will start clock here?

        start = clock();
        global_metrics.t_start = start;


        int turn_off = 0;
        if(turn_off != 1) {
            //-------1 SOCKET PER THREAD---------
            for (i = 0; i < NUM_WORKER_THREADS; i++) {
                //-----------------THREAD STUFF-----------------
                int rc;
                long t = i;
                //-----FINISH PARAMS FOR THREAD-----
                //struct worker_args_struct current_thread_args = MASTER_ARGS_ARRAY[i];
                int x;
                for(x = 0; x <= MASTER_ARGS_ARRAY[i].file_paths_size; x++) {
                    printf("\n INSIDE BOSS: ITER %d FILE NEAME IS %s",x,MASTER_ARGS_ARRAY[i].file_paths[x]);
                }

                MASTER_ARGS_ARRAY[i].thread_id = t;
                MASTER_ARGS_ARRAY[i].download_directory = PARAMETERS.PARAM_PATH_TO_DOWNLOAD_DIRECTORY;

                printf("THREAD ARG ID FOR THREAD %ld is %d \n", t, MASTER_ARGS_ARRAY[i].thread_id);

                printf("\nIn file loop: creating thread %ld\n", t);
                rc = pthread_create(&threads[t], NULL, DoWorkThread, (void *) &MASTER_ARGS_ARRAY[i]);
                if (rc) {
                    printf("\nERROR; return code from pthread_create() is %d\n", rc);
                    exit(-1);
                }

            }
            // clean up and join
            int iter;
            for (iter = 0; iter < NUM_WORKER_THREADS; iter++) {
                //try to rejoin main thread in worker function
                //free(MASTER_ARGS_ARRAY->file_paths[iter]);
                pthread_join(threads[iter], NULL);
            }


        }

        end = clock();
        global_metrics.t_end = end;
        global_metrics.total_msec = global_metrics.t_end - global_metrics.t_start;
    //_start: time after worker threads created, but before any of them starts sending requests
    //t_end: time when all worker threads receive all responses
    //total time elapsed: t_end - t_start
    //total bytes received: sum of all per_request_bytes_received
    //avg throughput: (total bytes received) / (total_time_elapsed)
            //avg response time: (sum of all pre-request-response-time) / (total number of requests)

    printf("START: %d\n",global_metrics.t_start);
    printf("END: %d\n",global_metrics.t_end);
    printf("TOTAL TIME ELAPSED: %d\n",global_metrics.total_msec);
    printf("TOTAL BYTES RECEIVED: %f\n",global_metrics.bytes_sent);
    printf("AVG. RESPONSE TIME : %f\n",(global_metrics.total_msec/global_metrics.bytes_sent));
    printf("AVG. THROUGHPUT: %f\n",(global_metrics.bytes_sent/global_metrics.total_msec));
    printf("FILES FINISHED: %d\n",global_metrics.finished_files);

    return 0;

    }


     //int DoWork(char *FILE_NAME, int hSocket) {
     void *DoWorkThread(void* THREAD_ARGS){

         struct worker_args_struct2 *arguments;
         arguments = (struct worker_args_struct2*) THREAD_ARGS;

         int workload_amt = (NUM_REQUESTS/NUM_WORKER_THREADS);
         printf("workload amt is %d\n",workload_amt);

         int thread_id = arguments->thread_id;

         int i = 0;
         for(i = 0; i <= arguments->file_paths_size - 1; i++)
         {
             if(arguments->file_paths[i] != '\0') {

                 char *FILE_NAME = arguments->file_paths[i];


                 printf("dat file name tho (in thread:) %s \n", arguments->file_paths[i]);
                 //printf("download dir (in thread:) %s \n\n",arguments->download_directory);
                 //printf("dile path size tho (in thread:) %d \n\n",arguments->file_paths_size);
                 //printf("thread id tho (in thread:) %d \n\n",arguments->thread_id);

                 int hSocket;                 /* handle to socket */
                 struct hostent *pHostInfo;   /* holds info about a machine */
                 struct sockaddr_in Address;  /* Internet socket address stuct */
                 long nHostAddress;
                 //char pBuffer[FILE_STORAGE_BUFFER_SIZE];
                 //unsigned nReadAmount;

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
                 printf("\nConnecting to %s on port %d\n", strHostName, nHostPort);
                 /* connect to host */
                 if (connect(hSocket, (struct sockaddr *) &Address, sizeof(Address)) == SOCKET_ERROR) {
                     printf("\nCould not connect to host\n");
                     return 0;
                 }

                 //-----------------END SOCKET STUFF-------------------------


                 printf("\nTHREAD #%d SPINNING UP\n", thread_id);

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
                 printf("\nTHREAD %d FULL GET FILE REQUEST TO SEND: \"%s\"", thread_id, GET_FILE_REQUEST);

                 //----#2------------SEND GET FILE REQUEST----------------

                 write(hSocket, GET_FILE_REQUEST, 256 * sizeof(char));

                 //----#3------------READ GET FILE RESPONSE/SIZE TO EXPECT---------------
                 //GetFile STATUS filesize FILE
                 //get response
                 read(hSocket, GET_FILE_RESPONSE, 256 * (sizeof(char)));
                 // get file size
                 read(hSocket, GET_FILE_TOTAL_SIZE, 256 * (sizeof(char)));
                 printf("\nReceived \"%s\" from server\n", GET_FILE_RESPONSE);
                 printf("\n File to expect is size \"%s\"\n", GET_FILE_TOTAL_SIZE);

                 global_metrics.bytes_sent += atoi(GET_FILE_TOTAL_SIZE);

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

                 char *FULL_FILE_PATH_TO_SAVE = malloc(256 * (sizeof(char)));
                 strcat(FULL_FILE_PATH_TO_SAVE, arguments->download_directory);
                 strcat(FULL_FILE_PATH_TO_SAVE, FILE_NAME);


                 printf("Dest. final name is \"%s\" \n", FULL_FILE_PATH_TO_SAVE);

                 FILE *fp = fopen(FULL_FILE_PATH_TO_SAVE, "ab");
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
                 free(FULL_FILE_PATH_TO_SAVE);
                 free(GET_FILE_REQUEST);
                 free(GET_FILE_RESPONSE);
                 free(GET_FILE_TOTAL_SIZE);
                 //free(arguments->file_paths[arguments->thread_id]);
                 global_metrics.finished_files ++;

                 printf("METRICS: FILES CURRENTLY FINISHED: %d\n",global_metrics.finished_files);

             }


             pthread_join(threads[thread_id], NULL);
         }
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
