#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>


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


void WriteToFile(char buffer[BUFFER_SIZE]);

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

    /* connect to host */
    if(connect(hSocket,(struct sockaddr*)&Address,sizeof(Address)) 
       == SOCKET_ERROR)
    {
        printf("\nCould not connect to host\n");
        return 0;
    }


    /*---TO DO---*/

    //1.) read workload file in



    /* read from socket into buffer
    ** number returned by read() and write() is the number of bytes
    ** read or written, with -1 being that an error occured */




    //2.) USE "GET FILE" protocol to request files
    // CLIENT SHOULD GENERATE A NUMBER OF "GETFILE" REQUESTS (eventually from different threads)

    //we should have a for loop...for each item in the array of files to retrieve...
    //create and send a proper "get file" request for each one and save it in the directory


    // for loop begin
    //for each file to get
        //spoof request now to fill out server logic
        char* GET_FILE_REQUEST = "GetFile GET workload.txt";

        write(hSocket,GET_FILE_REQUEST,sizeof(GET_FILE_REQUEST)+100);

        nReadAmount=read(hSocket,pBuffer,BUFFER_SIZE);

        printf("\nReceived \"%s\" from server\n",pBuffer);
        printf("\nRecieved amount is \"%d\" ", nReadAmount);
        printf("\nWriting \"%s\" to file....\n",pBuffer);

        //3.) write to directory from parameter
        //write the file we recieved to buffer
        WriteToFile(pBuffer);
    //end the for loop


    /* write what we received back to the server */
    //write(hSocket,pBuffer,nReadAmount);
    //printf("\nWriting \"%s\" to server",pBuffer);

    printf("\nClosing socket\n");
    /* close socket */                       
    if(close(hSocket) == SOCKET_ERROR)
    {
        printf("\nCould not close socket\n");
        return 0;
    }

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





