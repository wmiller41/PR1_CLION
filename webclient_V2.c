
#include "helpers.h"

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


struct thread_metrics *global_thread_metrics;
struct boss_metrics global_boss_metrics;
struct client_params PARAMETERS;

struct client_params get_parameters(int num_args, char** arguments);
struct client_params get_parameters(int num_args, char** arguments){


    struct client_params RETURN_PARAMS;
    RETURN_PARAMS.PARAM_PORT = 8888;
    RETURN_PARAMS.PARAM_NUM_WORKER_THREADS = 1;
    RETURN_PARAMS.PARAM_SERVER_ADDRESS = "0.0.0.0";
    RETURN_PARAMS.PARAM_PATH_TO_WORKLOAD_FILE = "workload.txt";
    RETURN_PARAMS.PARAM_NUM_OF_REQUESTS = 10;
    RETURN_PARAMS.PARAM_PATH_TO_DOWNLOAD_DIRECTORY = "PAYLOAD";

    int c;

    opterr = 0;
    while ((c = getopt (num_args, arguments, "p:t:w:s:d:r:h")) != -1)
        switch (c)
        {
            case 'p':
                RETURN_PARAMS.PARAM_PORT = atoi(optarg);
                break;
            case 't':
                RETURN_PARAMS.PARAM_NUM_WORKER_THREADS = atoi(optarg);
                if((RETURN_PARAMS.PARAM_NUM_WORKER_THREADS > 100) || (RETURN_PARAMS.PARAM_NUM_WORKER_THREADS < 1)){
                    printf("-t must be between 1 and 100\n");
                    abort();
                }else {
                    break;
                }
            case 'w':
                RETURN_PARAMS.PARAM_PATH_TO_WORKLOAD_FILE = optarg;
                printf("OPTARG: %s\n",optarg);
                printf("\nW flag hit,workload file is %s \n",RETURN_PARAMS.PARAM_PATH_TO_WORKLOAD_FILE);
                break;
            case 's':
                RETURN_PARAMS.PARAM_SERVER_ADDRESS = optarg;
                break;
            case 'd':
                RETURN_PARAMS.PARAM_PATH_TO_DOWNLOAD_DIRECTORY = optarg;
                break;
            case 'r':
                RETURN_PARAMS.PARAM_NUM_OF_REQUESTS = atoi(optarg);
                if((RETURN_PARAMS.PARAM_NUM_OF_REQUESTS > 1000)||(RETURN_PARAMS.PARAM_NUM_OF_REQUESTS < 1)){
                    printf("-r must be between 1 and 1000\n");
                    abort();
                }
                else {
                    printf("\nr flag hit, sending %d requests\n", RETURN_PARAMS.PARAM_NUM_OF_REQUESTS);
                    break;
                }
            case 'h':
            printf("usage:\nwebclient [options]\noptions:\n-s server address (Default: 0.0.0.0)\n-p server port (Default: 8888)\n-t number of worker threads (Default: 1, Range: 1-100)\n-w path to workload file (Default: workload.txt)\n-d path to downloaded file directory (Default: null)\n-r number of total requests (Default: 10, Range: 1-1000)\n");

                abort();
                break;
            case '?':
                if (optopt == 'c')
                    fprintf (stderr, "Option -%c requires an argument.\n", optopt);
                else if (isprint (optopt))
                    fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf (stderr,
                            "Unknown option character `\\x%x'.\n",
                            optopt);
            //default:
                //abort ();
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

void *DoWorkThread(void* THREAD_ARGS);

int main(int argc, char **argv) {

        printf("------WEB CLIENT STARTED-----\n");

        DOWNLOAD_DIRECTORY = malloc(256 * sizeof(char));
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

        //METRICS
        clock_t start,end;

        global_boss_metrics.total_bytes_sent = 0;
        global_boss_metrics.total_files = 0;
        global_boss_metrics.total_msec = 0;
        global_boss_metrics.t_start = 0;
        global_boss_metrics.t_end = 0;

        //create metrics array
        global_thread_metrics = malloc(sizeof(struct thread_metrics) * NUM_WORKER_THREADS);

        //initialize
        int i = 0;
        for(i = 0; i < NUM_WORKER_THREADS; i++)
        {
            global_thread_metrics[i].bytes_sent = 0;
            global_thread_metrics[i].finished_files = 0;
        }

        /*---TO DO---*/

        //1.) read workload file in

        char *WORKLOAD_FILE_CONTENTS = ReadFile(WORKLOAD_FILE_NAME);

        //2.) USE "GET FILE" protocol to request files
        // CLIENT SHOULD GENERATE A NUMBER OF "GETFILE" REQUESTS (eventually from different threads)
        //we should have a for loop...for each item in the array of files to retrieve...
        //create and send a proper "get file" request for each one and save it in the directory


        //------------------------WORKLOAD FILE DATA--------------------------
        printf("\nWORKLOAD FILE NAME %s\n",WORKLOAD_FILE_NAME);
        printf("\nWORKLOAD FILE CONTENTS \"%s\"\n", WORKLOAD_FILE_CONTENTS);
        char **TARGET_FILE_POINTERS = malloc((256 * (sizeof(char))));
        char *BYTES_OUT = malloc((256 * (sizeof(char))));
        int TARGET_FILES_SIZE = explode("\n", WORKLOAD_FILE_CONTENTS, TARGET_FILE_POINTERS, BYTES_OUT);
        printf("Seperated workload into \"%d\" files\n", TARGET_FILES_SIZE);

        free(WORKLOAD_FILE_CONTENTS);

        //LIMIT THE NUMBER OF THREADS TO REQUESTS...
        // WE WON'T NEED MORE THAN # of THREADS
        if(NUM_WORKER_THREADS > NUM_REQUESTS)
        {
            NUM_WORKER_THREADS = NUM_REQUESTS;
        }

        struct worker_args_struct2 MASTER_ARGS_ARRAY[NUM_WORKER_THREADS];
        //struct worker_args_struct MASTER_ARGS_ARRAY = malloc(sizeof(struct worker_args_struct)*NUM_WORKER_THREADS);

        //-----------------------FILL UP ONE LARGE ARRAY WITH "R" ITEMS TO MAKE SURE THAT ALL FILES ARE INCLUDED----------------
        char* MASTER_FILE_ARRAY[NUM_REQUESTS];
        int y = 0;
        for (y = 0; y < NUM_REQUESTS; y++) {
           MASTER_FILE_ARRAY[y] = TARGET_FILE_POINTERS[y%TARGET_FILES_SIZE];
        }
         //----------------------FILL EACH ITER IN MASTER_ARGS_ARRAY WITH R/T REQUESTS FROM MASTER_FILE_ARRAY-----------------
        //---------ONE ITER PER THREAD-----------
        i = 0;
        int FILE_ARRAY_COUNTER = 0;
        for (i = 0; i < NUM_WORKER_THREADS; i++) {
            printf("INSIDE LOOP FOR THREAD %d\n",i);
            int x = 0;
            MASTER_ARGS_ARRAY[i].file_paths_size = 0;
            for (x = 0; x < (NUM_REQUESTS/NUM_WORKER_THREADS); x++) {
                //printf("x is %d\n",x);
                MASTER_ARGS_ARRAY[i].file_paths[x] = MASTER_FILE_ARRAY[FILE_ARRAY_COUNTER];
                MASTER_ARGS_ARRAY[i].file_paths_size++;
                printf("\nTHREAD %d ASSIGNED MASTER ARG ARRAY ITER %d,MASTER FILE %d,FILE PATH NUM %d, FILE PATH %s, WHICH CURRENTLY HAS %d ITEMS\n",
                        i,i,FILE_ARRAY_COUNTER,x,MASTER_ARGS_ARRAY[i].file_paths[x],MASTER_ARGS_ARRAY[i].file_paths_size);
                FILE_ARRAY_COUNTER++;
            }
        }



        // i guess we will start clock here?



        for (i = 0; i < NUM_WORKER_THREADS; i++) {
            //-----------------THREAD STUFF-----------------


            //struct worker_args_struct current_thread_args = MASTER_ARGS_ARRAY[i];
            int x;
            for (x = 0; x <= MASTER_ARGS_ARRAY[i].file_paths_size; x++) {
                printf("\n INSIDE BOSS: ITER %d FILE NAME IS %s", x, MASTER_ARGS_ARRAY[i].file_paths[x]);
            }
            MASTER_ARGS_ARRAY[i].thread_id = i;
            MASTER_ARGS_ARRAY[i].download_directory = PARAMETERS.PARAM_PATH_TO_DOWNLOAD_DIRECTORY;
        }
        start = clock();
        global_boss_metrics.t_start = start;

        for (i = 0; i < NUM_WORKER_THREADS; i++) {
            int rc;
            printf("THREAD ARG ID FOR THREAD %d is %d \n", i, MASTER_ARGS_ARRAY[i].thread_id);

            printf("\nCreating thread %d\n", i);
            rc = pthread_create(&threads[i], NULL, DoWorkThread, (void *) &MASTER_ARGS_ARRAY[i]);
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

        end = clock();
        global_boss_metrics.t_end = end;



        for(i=0;i < NUM_WORKER_THREADS;i++){
            global_boss_metrics.total_bytes_sent += global_thread_metrics[i].bytes_sent;
            global_boss_metrics.total_files += global_thread_metrics[i].finished_files;
        }

        global_boss_metrics.total_msec = global_boss_metrics.t_end - global_boss_metrics.t_start;




    //_start: time after worker threads created, but before any of them starts sending requests
    //t_end: time when all worker threads receive all responses
    //total time elapsed: t_end - t_start
    //total bytes received: sum of all per_request_bytes_received
    //avg throughput: (total bytes received) / (total_time_elapsed)
            //avg response time: (sum of all pre-request-response-time) / (total number of requests)


    double difference = global_boss_metrics.t_end - global_boss_metrics.t_start;
    double runtime_millis = ((difference / CLOCKS_PER_SEC) * 1000);
    double runtime_sec = (difference /CLOCKS_PER_SEC);


    printf("-----------METRICS-----------");


    printf("DIFFERENCE %f", difference);




    printf("START: %d cycles \n",global_boss_metrics.t_start);
    printf("END: %d cycles \n",global_boss_metrics.t_end);
    printf("TOTAL TIME ELAPSED: %f (ms) %f (s) \n",runtime_millis,runtime_sec);
    printf("TOTAL BYTES RECEIVED: %d (B) \n",global_boss_metrics.total_bytes_sent);
    printf("AVG. RESPONSE TIME : %f (ms/B)\n",
            (runtime_millis/global_boss_metrics.total_bytes_sent));
    printf("AVG. THROUGHPUT: %f (B/ms)\n",
            (global_boss_metrics.total_bytes_sent/runtime_millis));
    printf("FILES FINISHED: %d\n",
            global_boss_metrics.total_files);


    free(global_thread_metrics);

    return 0;

    }
void *DoWorkThread(void* THREAD_ARGS){

         struct worker_args_struct2 *arguments;
         arguments = (struct worker_args_struct2*) THREAD_ARGS;

         //int workload_amt = (NUM_REQUESTS/NUM_WORKER_THREADS);
         //printf("workload amt is %d\n",workload_amt);

         int thread_id = arguments->thread_id;

         int i = 0;
         for(i = 0; i <= arguments->file_paths_size - 1; i++)
         {
             if(arguments->file_paths[i] != '\0') {

                 char *FILE_NAME = arguments->file_paths[i];

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



                 //----#4------------PARSE REQUEST STATUS-----------------
                 //char *RESPONSE_STATUS = substring(8, 2, GET_FILE_RESPONSE, 3);
                 char *RESPONSE_STATUS = substring(GET_FILE_RESPONSE,9,2);

                 printf("\nStatus of response is  \"%s\", read amount \"%ld\"\n", RESPONSE_STATUS, sizeof(RESPONSE_STATUS));
                 if (strcmp(RESPONSE_STATUS, "OK") != 0) {
                     printf("\nThere was an error!");
                     close(hSocket);
                     //return 0;
                 }

                 //----#5------------CREATE DESTINATION PATH-------------
                 //file name already has /

                 char *FULL_FILE_PATH_TO_SAVE = malloc(256 * (sizeof(char)));
                 FULL_FILE_PATH_TO_SAVE[0] = '\0';
                 //if((strlen(arguments->download_directory)) != 0){
                     strcat(FULL_FILE_PATH_TO_SAVE, arguments->download_directory);
                     strcat(FULL_FILE_PATH_TO_SAVE, FILE_NAME);
                 //
                 //else
                // {
                     //FULL_FILE_PATH_TO_SAVE = FILE_NAME;
                // }

                 printf("Dest. final name is \"%s\" \n", FULL_FILE_PATH_TO_SAVE);

                 FILE *fp = fopen(FULL_FILE_PATH_TO_SAVE, "ab");
                 if (NULL == fp) {
                     printf("\nError opening file..%s\n",strerror(errno));
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

                 printf("updating thread %d metrics\n",arguments->thread_id);
                 global_thread_metrics[arguments->thread_id].bytes_sent = atoi(GET_FILE_TOTAL_SIZE);
                 global_thread_metrics[arguments->thread_id].finished_files ++;

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


                 printf("METRICS: FILES CURRENTLY FINISHED BY THREAD: %d\n", global_thread_metrics[arguments->thread_id].finished_files);




             }


             pthread_join(threads[thread_id], NULL);
         }
         return 0;
     }


//-----------------HELPER METHODS----------------------

