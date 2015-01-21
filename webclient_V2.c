#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>




#define SOCKET_ERROR        -1
#define BUFFER_SIZE         100000
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
void WriteToFile(char buffer[BUFFER_SIZE]);
char *substring(size_t start, size_t stop, const char *src, size_t size);
size_t explode(const char *delim, const char *str,char **pointers_out, char *bytes_out);


int main(int argc, char **argv) {
    printf("Hello, I am the web client!!!!!!\n");

    int hSocket;                 /* handle to socket */
    struct hostent* pHostInfo;   /* holds info about a machine */
    struct sockaddr_in Address;  /* Internet socket address stuct */
    long nHostAddress;
    char pBuffer[BUFFER_SIZE];
    unsigned nReadAmount;
    char strHostName[HOST_NAME_SIZE];
    int nHostPort;

    if(argc < 3)
      {
	printf("\nUsage: client host-name host-port\n");
	return 0;
      }
    else
      {
	strcpy(strHostName,argv[1]);
	nHostPort=atoi(argv[2]);
      }


    /*---TO DO---*/

    //1.) read workload file in
    char* WORKLOAD_FILE_NAME = "workload.txt";
    char* WORKLOAD_FILE_CONTENTS = ReadFile(WORKLOAD_FILE_NAME);


    //2.) USE "GET FILE" protocol to request files
    // CLIENT SHOULD GENERATE A NUMBER OF "GETFILE" REQUESTS (eventually from different threads)

    //we should have a for loop...for each item in the array of files to retrieve...
    //create and send a proper "get file" request for each one and save it in the directory


        printf("\nWORKLOAD FILE CONTENTS \"%s\"",WORKLOAD_FILE_CONTENTS);

        char** TARGET_FILE_POINTERS = malloc(1000000);
        char* BYTES_OUT = malloc(1000000);
        int TARGET_FILES_SIZE = explode("\n",WORKLOAD_FILE_CONTENTS,TARGET_FILE_POINTERS,BYTES_OUT);
        printf("\nTARGET FILE BYTES \"%s\",TARGET FILE SIZE \"%d\"",TARGET_FILE_POINTERS[3],TARGET_FILES_SIZE);

        // for loop begin
        //for each file to get
        int i = 0;
        for(i = 0; i < TARGET_FILES_SIZE ; i++)
        {

            printf("\nMaking a socket");
            /* make a socket */
            hSocket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

            if(hSocket == SOCKET_ERROR)
            {
                printf("\nCould not make a socket\n");
                return 0;
            }

            /* get IP address from name */
            pHostInfo=gethostbyname(strHostName);
            /* copy address into long */
            memcpy(&nHostAddress,pHostInfo->h_addr,pHostInfo->h_length);

            /* fill address struct */
            Address.sin_addr.s_addr=nHostAddress;
            Address.sin_port=htons(nHostPort);
            Address.sin_family=AF_INET;

            printf("\nConnecting to %s on port %d",strHostName,nHostPort);
            //sleep(10);
            /* connect to host */
            if(connect(hSocket,(struct sockaddr*)&Address,sizeof(Address))
                    == SOCKET_ERROR)
            {
                printf("\nCould not connect to host\n");
                return 0;
            }

            //printf("\n FILE \"%d\" is called \"%s\"",i,TARGET_FILE_POINTERS[i]);
            //char* GET_FILE_REQUEST = strncat("GetFile GET",TARGET_FILE_POINTERS[i],1000);

            char GET_FILE_REQUEST[256]; //= "GetFile GET workload.txt";
            strcpy(GET_FILE_REQUEST,"GetFile GET ");
            strcat(GET_FILE_REQUEST,TARGET_FILE_POINTERS[i]);

            printf("FULL GET FILE REQUEST TO SEND: \"%s\"",GET_FILE_REQUEST);

            //SEND GET FILE REQUEST
            write(hSocket,GET_FILE_REQUEST,sizeof(GET_FILE_REQUEST)+100);

            /* read from socket into buffer
           ** number returned by read() and write() is the number of bytes
           ** read or written, with -1 being that an error occured */
            nReadAmount=read(hSocket,pBuffer,BUFFER_SIZE);

            //NEED TO PARSE ACTUAL DATA FROM THE REQUEST RESPONSE...

            printf("\nReceived \"%s\" from server\n",pBuffer);
            printf("\nRecieved amount is \"%d\" ", nReadAmount);
            printf("\nWriting \"%s\" to file....\n",pBuffer);

            //3.) write to directory from parameter
            //write the file we recieved to buffer
            WriteToFile(pBuffer);
            //end the for loop
            printf("\nClosing socket\n");
            /* close socket */
            if(close(hSocket) == SOCKET_ERROR)
            {
                printf("\nCould not close socket\n");
                return 0;
            }

        }

    free(BYTES_OUT);
    free(TARGET_FILE_POINTERS);


    /* write what we received back to the server */
    //write(hSocket,pBuffer,nReadAmount);
    //printf("\nWriting \"%s\" to server",pBuffer);


    return 0;
}


void WriteToFile(char buffer[BUFFER_SIZE])
{
    FILE *fptr;
    char* filePath = "payload.txt";
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
