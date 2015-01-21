#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>


#define SOCKET_ERROR -1
#define BUFFER_SIZE 10000
#define MESSAGE "If you see this, it's working!"
#define QUEUE_SIZE 5




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


void FillAddress(struct sockaddr_in *Address,int nHostPort);
char* ReadFile(char* fileAddress);
int CreateServerSocket(int *hServerSocket);
char *substring(size_t start, size_t stop, const char *src, size_t size);




int main(int argc, char **argv) {
	printf("Hello, I am the web server!!!!!\n");
	
	//---------SOCKETS---------
	int hSocket, hServerSocket;
	//hServerSocket used to listen for connections?????

	//---------MACHINE INFO--------
	//SOCKET ADDRESS 
	struct sockaddr_in Address;
	int nAddressSize = sizeof(struct sockaddr_in);
	//make this smaller?
	char pBuffer[BUFFER_SIZE];
	int nHostPort;

	//--------GET PARAMETERS HERE-----
	if(argc < 2)
	{
		printf("please review proper parameters");
		return 0;
	}
	else
	{
		//------PARSE PARAMS

		//PORT
		//nHostPort=atoi(argv[1]);
		//this is the port we should run on.
		// if param doesn't exist, set to 8888
		nHostPort = 8888;

		//NUM THREADS

		//PATH TO FILES
	}

	printf("\nStarting server");
	printf("\nCreating a socket!!\n");
	//-------MAKE A SERVER SOCKET------------
	int returnCode = CreateServerSocket(&hServerSocket);
	if(returnCode == 0) {
		printf("\nCould not make a socket\n");
		return 0;
	}

	//----------FILL ADDRESS DETAILS
	FillAddress(&Address,nHostPort);
	//----------BIND TO SOCKET TO PORT
	printf("\nBinding to port %d",nHostPort);

	if(bind(hServerSocket,(struct sockaddr*)&Address,sizeof(Address)) == SOCKET_ERROR)
    	{
        printf("\nCould not connect to host\n");
        return 0;
		}
	
	//-----------GET PORT NUMBER
    	getsockname( hServerSocket, (struct sockaddr *) &Address,(socklen_t *)&nAddressSize);
    	printf("\nopened socket as fd (%d) on port (%d) for stream i/o\n",hServerSocket, ntohs	(Address.sin_port) );

          /*printf("Server\n\
              sin_family        = %d\n\
              sin_addr.s_addr   = %d\n\
              sin_port          = %d\n"
              , Address.sin_family
              , Address.sin_addr.s_addr
              , ntohs(Address.sin_port)
            );*/

	printf("\nMaking a listen queue of %d elements",QUEUE_SIZE);
    	/* establish listen queue */
    	if(listen(hServerSocket,QUEUE_SIZE) == SOCKET_ERROR)
    	{
		printf("\nCould not listen\n");
		return 0;
    	}

	for(;;)
	{
        printf("\nWaiting for a connection\n");
        /* get the connected socket */

        hSocket=accept(hServerSocket,(struct sockaddr*)&Address,(socklen_t *)&nAddressSize);

				printf("\nGot a connection");

				//SEE IF CLIENT SENT A MESSAGE?
				char strGetFileRequest[100];
				printf("\n\nI should be receiving a well formed Get File request.\n\n");
				read(hSocket,strGetFileRequest,sizeof(strGetFileRequest));

				if(strGetFileRequest != NULL){
			    printf("\nReceieved \"%s\" from client", strGetFileRequest);
				}
				else{
				printf("No GetFile request from client");
				}

				//PARSE TARGET FROM GET FILE REQUEST...
				//assume that "getFile GET " protocol means that file name will start from
				// 13th character in string

				char* strFileName = substring(13,100,strGetFileRequest,sizeof(strGetFileRequest));
				printf("\nFile name is \"%s\"" ,strFileName);
				//GET FILE AND PUT IN INTO BUFFER...


				//MESSAGE IS CURRENTLY THE DEFAULT FROM SOURCE CODE. WE CAN RUN STRCPY AGAIN AND JUST OVERWRITE IT
				//strcpy(pBuffer,strWorkLoadFileName);
				printf("\nCopy contents of file to buffer and send");
				strcpy(pBuffer,ReadFile(strFileName));
				printf("\nSending contents of file (\"%s\") to client",pBuffer);
				/* number returned by read() and write() is the number of bytes
				** read or written, with -1 being that an error occured
				** write what we received back to the server */
				write(hSocket,pBuffer,strlen(pBuffer)+1);

				/* read from socket into buffer */
				//read(hSocket,pBuffer,BUFFER_SIZE);


				//if(strcmp(pBuffer,MESSAGE) == 0)
					//printf("\nThe messages match");
				//else
					//printf("\nSomething was changed in the message");

				printf("\nClosing the socket");
				/* close socket */
				if(close(hSocket) == SOCKET_ERROR)
				{
				 printf("\nCould not close socket\n");
				return 0;
        		}
    }
}

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

