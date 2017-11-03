// system types
#include <sys/types.h>    
// socket API
#include <sys/socket.h>   
// struct sockaddr_in
#include <netinet/in.h>   
// inet_addr()
#include <arpa/inet.h>    
// system call
#include <unistd.h>       
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include "gQueue.h"
#include "gThread.h"
#include "errexit.h"

extern int errno;

#define BACK_LOG_QLEN 10
#define SOCK_BUFSIZE 1024

int passiveSock(struct sockaddr_in* i_SsockaddrServer, int i_iPort)
{
    int iListenFd;
    if ((iListenFd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
        errexit("can't create socket: %s \n", strerror(errno));

    // Allow socket reuse local addr and port
    int opt = SO_REUSEADDR;   
    setsockopt(iListenFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Initialize socket addr
    bzero(i_SsockaddrServer, sizeof(*i_SsockaddrServer));
    i_SsockaddrServer->sin_family = AF_INET;
    i_SsockaddrServer->sin_port = htons((unsigned short)i_iPort);
    // here htonl() is optional
    i_SsockaddrServer->sin_addr.s_addr = htonl(INADDR_ANY);  

    // Bind socket to server address
    if (bind(iListenFd, (struct sockaddr*)i_SsockaddrServer, sizeof(*i_SsockaddrServer)) < 0)
        errexit("can't bind to %s port: %s \n", i_iPort, strerror(errno));
    // For TCP socket, convert it to passive mode
    if (listen(iListenFd, BACK_LOG_QLEN) < 0)
        errexit("can't listen on %s port: %s \n", i_iPort, strerror(errno));

    return iListenFd;
}

// Main function
int main(int argc,char* argv[])
{
    int iListenFd, iClientFd;        
	int iPort;

	pthread_t recvThreadId;
	pthread_t sendThreadId;
	pthread_t commandThreadId;

	QSOCKINFO *SqsockInfo;
    // Server socket addr
    struct sockaddr_in SsockaddrServer;        
    // Client socket addr
	struct sockaddr_in SsockaddrClient;        
    socklen_t socklenSize;

	if(argc != 2)
        errexit("Usage : %s [PORT]:\n", argv[0]);

	iPort = atoi(argv[1]);
	if(iPort<0 || iPort >65536)
        errexit("PORT RANGE (1 ~ 65535)\n");

    socklenSize = sizeof(struct sockaddr_in);

    printLog("[SERVER] creating passive socket..\n");
    iListenFd = passiveSock(&SsockaddrServer, iPort);
    printLog("[SERVER] passive socket %d created.\n", iListenFd);

    while(1)
    {
        // connect to first client, create temporary socket 
        printLog("[SERVER] waiting for client..\n");
        if ((iClientFd = accept(iListenFd, (struct sockaddr*)&SsockaddrClient, &socklenSize)) < 0)
            errexit("accept failed: %s\n", strerror(errno));
        
		printLog("[SERVER] servant socket %d created.\n", iClientFd);

		// Initial client information struct
		SqsockInfo = (QSOCKINFO*)malloc(sizeof(QSOCKINFO));
		SqsockInfo->iSock = iClientFd;

		//소켓정보와 큐 정보생성 및 관리 변수에 할당
		if(createQueueSockInfo(SqsockInfo) != EOK){
			close(iClientFd);
    		close(iListenFd);
        	errexit("create QueueSockInfo fail.\n");
		}

		//NOTE: threads sharing the memory, client variable cannot reuse
        memcpy(&SqsockInfo->SsockaddrClient, &SsockaddrClient, sizeof(SsockaddrClient));  


		// commandGroup 쓰레드 생성
		if(pthread_create(&recvThreadId, NULL, recvCommand, (void*)SqsockInfo))
        	errexit("create recv thread failed: %s\n", strerror(errno));
		//쓰레드와 메인 분리
		pthread_detach(recvThreadId);

		// commandGroup 쓰레드 생성
		if(pthread_create(&commandThreadId, NULL, commandGroup, (void*)SqsockInfo))
        	errexit("create command thread failed: %s\n", strerror(errno));
		//쓰레드와 메인 분리
		pthread_detach(commandThreadId);

		// commandGroup 쓰레드 생성
		if(pthread_create(&sendThreadId, NULL, sendCommandResult, (void*)SqsockInfo))
        	errexit("create send thread failed: %s\n", strerror(errno));
		//쓰레드와 메인 분리
		pthread_detach(sendThreadId);

    }
    close(iListenFd);
    return 0;
}

