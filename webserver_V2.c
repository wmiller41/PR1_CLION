



#include "helpers.h"



//usage webserver[options]
/*
options:
-p port (default: 8888)
-t number of worker threads (default :1, range 1-1000)
-f path to static files (defailt: .)
-h show help message

*/

/*
The web server must satisfy the following requirements:

-Must be implemented using the Boss-Worker thread pattern
-Must spawn a pool of worker threads which will handle  requests
-Must have a queue mechanism for communication between the boss and worker threads
-Must serve files from a file directory (path) provided as a command line argument
-If no path is specified, then the files should be served from the directory local to the server executable
-The boss thread must satisfy the following requirements:
	-Must create a socket that listens for client connections/requests
	-The worker threads must satisfy the following requirements:
	-Must receive a client GetFile request, and then process it
	-Must form and send a proper GetFile response to client
		-See the GetFile Protocol below for information about forming proper responses
-Must close client connection
*/



void *DoServerWorkThread(void* THREAD_ARGS);


;

struct server_worker_arg_queue GLOBAL_WORKER_QUEUE;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t c_work = PTHREAD_COND_INITIALIZER;
pthread_cond_t c_boss = PTHREAD_COND_INITIALIZER;



struct server_params PARAMETERS;
struct server_params get_parameters(int num_args, char** arguments);






int main(int argc, char **argv) {
    printf("-----SERVER STARTED------\n");

    //----------GET PARAMETERS-------------------
    PARAMETERS = get_parameters(argc,argv);

    //----------CREATE QUEUE FOR WORKERS--------
    // queue is just int socket descriptors.
    struct server_worker_arg_queue wq;
    InitializeQueue(&wq);

    int NUM_THREADS = PARAMETERS.PARAM_NUM_WORKER_THREADS;
    int t;

    pthread_t threads[NUM_THREADS];
    for(t=0;t < NUM_THREADS ;t++) {
        int rc;

        //--------CREATE POOL OF THREADS-----------
        //no params for threads because they will get work from the queue
        rc = pthread_create(&threads[t], NULL, DoServerWorkThread,NULL);
        if (rc) {
            printf("\nERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
        else
        {
            printf("THREAD %d CREATED\n",t);
        }
    }


    //---------SOCKETS---------
        int hServerSocket;

        //hServerSocket used to listen for connections?????

        //---------MACHINE INFO--------
        //SOCKET ADDRESS
        struct sockaddr_in Address;
        int nAddressSize = sizeof(struct sockaddr_in);
        //make this smaller?

        int nHostPort = PARAMETERS.PARAM_PORT;

        //--------GET PARAMETERS HERE-----TO DO!!!!!!!!!!!!!!-----
        //if (argc < 2) {
            //rintf("please review proper parameters");
            //return 0;
        //}
        //else {
            //------PARSE PARAMS
            //PORT
            //nHostPort=atoi(argv[1]);
            //this is the port we should run on.
            // if param doesn't exist, set to 8888
            //nHostPort = 8888;
            //NUM THREADS
            //PATH TO FILES
        //}

        printf("\nStarting server");

        printf("\nCreating a socket!!\n");
        //-------MAKE A SERVER SOCKET------------
        int hSocket;
        int returnCode = CreateServerSocket(&hServerSocket);
        if (returnCode == 0) {
            printf("\nCould not make a socket\n");
            return 0;
        }
        //----------FILL ADDRESS DETAILS------------------
        FillAddress(&Address, nHostPort);
        //----------BIND TO SOCKET TO PORT
        printf("\nBinding to port %d", nHostPort);

        if (bind(hServerSocket, (struct sockaddr *) &Address, sizeof(Address)) == SOCKET_ERROR) {
            printf("\nCould not connect to host\n");
            return 0;
        }
        //-----------GET PORT NUMBER-------------------
        getsockname(hServerSocket, (struct sockaddr *) &Address, (socklen_t *) &nAddressSize);
        printf("\nOpened socket as fd (%d) on port (%d) for stream i/o\n", hServerSocket, ntohs(Address.sin_port));

        /*printf("Server\n\
                  sin_family        = %d\n\
                  sin_addr.s_addr   = %d\n\
                  sin_port          = %d\n"
                  , Address.sin_family
                  , Address.sin_addr.s_addr
                  , ntohs(Address.sin_port)
                );*/

        printf("\nMaking a listen queue of %d elements", QUEUE_SIZE);
        /* establish listen queue */


    while(1){
        if (listen(hServerSocket, QUEUE_SIZE) == SOCKET_ERROR) {
            printf("\nCould not listen\n");
            return 0;
        }

        printf("\nWaiting for a connection");
        /* get the connected socket */

        hSocket = accept(hServerSocket, (struct sockaddr *) &Address, (socklen_t *) &nAddressSize);

        printf("\nGot a connection");


    //---------------FOREVER------------

        //---------GET READY TO PLACE IN QUEUE--------
        struct server_worker_args TEMP_ARGS_FOR_QUEUE;
        TEMP_ARGS_FOR_QUEUE.hsocket = hSocket;


        //--------CRITICAL SECTION---------/
        pthread_mutex_lock(&m);
            printf("\nINSIDE BOSS' CRITICAL SECTION\n");
            int cur = GLOBAL_WORKER_QUEUE.NEXT_TO_ADD;
            if (cur > WORKER_QUEUE_SIZE)
            {
                //avoid overflow
                exit(1);
            }

            while (cur == (GLOBAL_WORKER_QUEUE.NEXT_TO_REMOVE + 1)) {  /* block if buffer is full */
                printf("BOSS THREAD IS WAITING - NEXT TO ADD IS SAME AS NEXT TO REMOVE\n");
                pthread_cond_wait (&c_boss, &m);
            }
            printf("\nADDING SOCKED DESC %d TO QUEUE",TEMP_ARGS_FOR_QUEUE.hsocket);
            //ADD ON NEW ITEM
            AddToEnd(&GLOBAL_WORKER_QUEUE,TEMP_ARGS_FOR_QUEUE);

        pthread_mutex_unlock(&m);
        printf("\nEXITED CRITICAL SECTION\n");
        //--------END CRITICAL SECTION-----/

        //------------------AWAKE SLEEPING WORKER(S)
        printf("\nSIGNALING C_WORKER\n");
        pthread_cond_signal(&c_work);

    }
}

void *DoServerWorkThread(void* THREAD_ARGS) {


    while(1) {

        //-----------------SLEEP UNTIL AWOKEN?----------------

        struct server_worker_args arguments;
        //CRITICAL SECTION?
        printf("WORKER THREAD IS LOCKING\n");
        pthread_mutex_lock (&m);

            while(GLOBAL_WORKER_QUEUE.NEXT_TO_REMOVE >= (GLOBAL_WORKER_QUEUE.NEXT_TO_ADD))
            //((GLOBAL_WORKER_QUEUE.NEXT_TO_ADD - 1) == GLOBAL_WORKER_QUEUE.NEXT_TO_REMOVE))
            {//block as no new data is around
                printf("\nWORKER THREAD IS WAITING\n");
                pthread_cond_wait (&c_work, &m);
            }

            arguments = RemoveFromFront(&GLOBAL_WORKER_QUEUE);
            printf("WORKER THREAD IS UNLOCKING\n");
            pthread_mutex_unlock (&m);

        printf("WORKER THREAD IS SIGNALING BOSS\n");
        pthread_cond_signal(&c_boss);
        //END CRITICAL SECTION

        //------GET SOCKET FROM ARGS------
        int hSocket = arguments.hsocket;


        //SEE IF CLIENT SENT A MESSAGE?
        char *strGetFileRequest = malloc(256 * sizeof(char));
        printf("\nI should be receiving a well formed Get File request.\n");

        //----#1------------RECEIVE GETFILE REQUEST--------------

        read(hSocket, strGetFileRequest, 256 * sizeof(char));

        printf("request %s",strGetFileRequest);

        //if (strGetFileRequest != NULL) {
            //printf("\nReceieved \"%s\" from client", strGetFileRequest);
       // }
       // else {
            //printf("No GetFile request from client");
       // }

        //GetFile STATUS filesize FILE
        //assume that "getFile GET " protocol means that file name will start from
        // 13th character in string

        //---#2-----------PARSE GET FILE REQUEST, GET NAME-------
        char *GET_FILE_RETURN = malloc(256 * sizeof(char));

        char *strFileName = malloc(256 * sizeof(char));
        strFileName[0] = '\0';
        strcat(strFileName,PARAMETERS.PARAM_PATH);
        //strcat(strFileName,substring(13, 1000, strGetFileRequest, (256 * sizeof(char))));
        strcat(strFileName,substring(strGetFileRequest,14,1000));

        printf("\nFile name is \"%s\"", strFileName);

        //---#3-----------CHECK ON FILE--------------------------
        // if strFileName is found, concat to GET_FILE RETURN
        // else, return the file not found status and close up shop
        if (access(strFileName, F_OK) == -1) {
            // file doesn't exists
            strcat(GET_FILE_RETURN, "GetFile FILE_NOT_FOUND 0 0");
            write(hSocket, GET_FILE_RETURN, strlen(GET_FILE_RETURN));
            printf("\nFILE NOT FOUND....Closing the socket");
            /* close socket */
            if (close(hSocket) == SOCKET_ERROR) {
                printf("\nCould not close socket\n");
                return 0;
            }
        }
        else {

            long FILE_SIZE;


            char *STORAGE_BUFFER = malloc(FILE_STORAGE_BUFFER_SIZE);
            //strcpy(STORAGE_BUFFER,strWorkLoadFileName);
            printf("\nReading file and getting file size\n");

            strcpy(STORAGE_BUFFER, ReadFile_wSize(strFileName, &FILE_SIZE));

            //TURN FILE SIZE INTO STRING
            char *CHAR_FILE_SIZE = malloc((256 * (sizeof(char))));
            sprintf(CHAR_FILE_SIZE, "%ld", FILE_SIZE);

            printf("Creating return GetFile request");
            GET_FILE_RETURN[0] = '\0';
            strcat(GET_FILE_RETURN, "GetFile OK");
            strcat(GET_FILE_RETURN, "\0");
            //strcat(GET_FILE_RETURN,STORAGE_BUFFER);

            printf("\nReturning GetFile Response (\"%s\") to client\n", GET_FILE_RETURN);
            write(hSocket, GET_FILE_RETURN, 256 * sizeof(char));

            printf("\nReturning File Size Character(\"%s\")", CHAR_FILE_SIZE);
            write(hSocket, CHAR_FILE_SIZE, 256 * sizeof(char));

            //SEND FILE
            int fd;
            //fd = open(strFileName,O_RDONLY);
            FILE *fp = fopen(strFileName, "r");
            fd = fileno(fp);

            printf("File Descriptor: %d \n", fd);
            if (fd < 0) {
                printf("ERROR OPENING FILE");
                return 0;
            }

            //off_t offset;
            int SIZE_SENT = 0;
            long FILE_REMAINING = FILE_SIZE;

            while (FILE_REMAINING > 0) {
               //printf("Sending file...");
                //SIZE_SENT = sendfile(hSocket, fd, &offset,256);
                SIZE_SENT = sendfile(hSocket, fd, NULL, FILE_SIZE);
                FILE_REMAINING -= SIZE_SENT;
                //char log[300];
                //sprintf(log, "%d sent, %ld remains\n", SIZE_SENT, FILE_REMAINING);
                //WriteToFile(log, "SERVER-LOG.txt");
                //printf("%d sent, %ld remains\n", SIZE_SENT, FILE_REMAINING);

                if (SIZE_SENT == -1) {
                    printf("something went wrong with sendfile()!...Errno %d %s\n", errno, strerror(errno));
                    fprintf(stderr, "error.. %d of %ld bytes sent\n", SIZE_SENT, FILE_SIZE);
                    exit(1);
                }

            }

            if (SIZE_SENT != FILE_SIZE) {
                fprintf(stderr, "incomplete transfer from sendfile: %d of %ld bytes\n", SIZE_SENT, FILE_SIZE);
                exit(1);
            }


            fclose(fp);


            free(STORAGE_BUFFER);
            free(GET_FILE_RETURN);
            free(strGetFileRequest);
            free(strFileName);
            //free(CHAR_FILE_SIZE);
            /* read from socket into buffer */
            //read(hSocket,STORAGE_BUFFER,FILE_STORAGE_BUFFER_SIZE);

            printf("\nClosing the socket\n\n");
            /* close socket */
            if (close(hSocket) == SOCKET_ERROR) {
                printf("\nCould not close socket\n");
                return 0;
            }
        }
    }
        //DO WE NEED TO RETURN ANYTHiNG?
        return NULL;

}

struct server_params get_parameters(int num_args, char** arguments)
{


    /*
options:
-p port (default: 8888)
-t number of worker threads (default :1, range 1-1000)
-f path to static files (defailt: .)
-h show help message
*/
    struct server_params RETURN_PARAMS;

    //set defaults
    RETURN_PARAMS.PARAM_PORT = 8888;
    RETURN_PARAMS.PARAM_NUM_WORKER_THREADS = 5;



    int c;

    opterr = 0;
    while ((c = getopt (num_args, arguments, "p:t:f:h")) != -1)
        switch (c)
        {
            case 'p':
                RETURN_PARAMS.PARAM_PORT = atoi(optarg);
                break;
            case 't':
                RETURN_PARAMS.PARAM_NUM_WORKER_THREADS = atoi(optarg);
                break;
            case 'f':
                RETURN_PARAMS.PARAM_PATH = optarg;
                break;
            case 'h':
                printf("options:\n-p port (default: 8888)\n-t number of worker threads (default :1, range 1-1000)\n-f path to static files (default: .)\n-h show help message\n");


                exit(1);

            default:
                abort ();
        }


    printf("args...port:%d threads:%d path:%s",
            RETURN_PARAMS.PARAM_PORT,
            RETURN_PARAMS.PARAM_NUM_WORKER_THREADS,
            RETURN_PARAMS.PARAM_PATH);

    return RETURN_PARAMS;

}



//----------QUEUE HELPERS----------





