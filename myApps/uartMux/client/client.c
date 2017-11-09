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
#include <netdb.h>        
#include "gQueue.h"
#include "gThread.h"
#include "errexit.h"

extern int errno;

#define BACK_LOG_QLEN 10
#define SOCK_BUFSIZE 1024

// Main function
int main(int argc,char* argv[])
{
    int iSock;        
	int iPort;

	pthread_t recvThreadId;
	pthread_t sendThreadId;
	pthread_t commandThreadId;
	QSOCKINFO *SqsockInfo;
    // Server socket addr
    struct sockaddr_in server;        
    struct hostent *hent;
	char *host="192.168.1.11";
    socklen_t socklenSize;
	
	if(argc != 2)
        errexit("Usage : %s [PORT]:\n", argv[0]);

    // convert decimal IP to binary IP
    if ((hent = gethostbyname(host)) == NULL)  
        errexit("gethostbyname failed.\n");

	fprintf(stderr,"### %s():%d ###\n",__func__,__LINE__);
	iPort = atoi(argv[1]);
	if(iPort<0 || iPort >65536)
        errexit("PORT RANGE (1 ~ 65535)\n");

	fprintf(stderr,"### %s():%d ###\n",__func__,__LINE__);
    if ((iSock = socket(PF_INET, SOCK_STREAM, 0)) < 0)
        errexit("create socket failed: %s\n", strerror(errno));

	fprintf(stderr,"### %s():%d ###\n",__func__,__LINE__);
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(iPort);
    server.sin_addr = *((struct in_addr*)hent->h_addr);
    printLog("[CLIENT] server addr: %s, port: %u\n", inet_ntoa(server.sin_addr), ntohs(server.sin_port));

    if (connect(iSock, (struct sockaddr*)&server, sizeof(server)) < 0)
        errexit("connect to server failed: %s\n", strerror(errno));

    printLog("[CLIENT] connected to server %s\n", inet_ntoa(server.sin_addr));

	// Initial client information struct
	SqsockInfo = (QSOCKINFO*)malloc(sizeof(QSOCKINFO));
	SqsockInfo->iSock = iSock;

	//소켓정보와 큐 정보생성 및 관리 변수에 할당
	if(createQueue(SqsockInfo, "tcpIp") != EOK){
		close(iSock);
       	errexit("create TCP IP fail.\n");
	}

	//NOTE: threads sharing the memory, client variable cannot reuse
    memcpy(&SqsockInfo->server, &server, sizeof(server));  

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
	while(1);
    close(iSock);
    return 0;
}

