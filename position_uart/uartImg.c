#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<signal.h>
#include<errno.h>
#include<unistd.h>
#include <sys/mman.h>
#include <sys/time.h>
#include "uartForMux.h"
#include "socket.h"
#include "errexit.h"
#include "gQueue.h"
#include "gThread.h"

int main(int argc, char* argv[] )
{
	char chData[33];
	char chMode[16];
	int iDataFd;
	int iMode;
	int iMemFd;
	int iReadLen;
	int iState;
	FILE* pFd;
	unsigned long *unlGpioAddr;
	unsigned char uchGpioSelectValue=0x00;
	char uchSendBuff[1024];

	pthread_t recvThreadId;
	pthread_t sendThreadId;
	pthread_t sendCmdThreadId;
	pthread_t commandThreadId;

	QSOCKINFO *SqsockInfo;

	if(argc != 2){
		fprintf(stderr,"잘못된 실행입니다.\n");
		exit(1);
	}
	
	//SOCKET 연결
	
	iDataFd = connect_server(REQUEST_DATALINK);
	if(iDataFd == -1){
		printDebugLog("connect server fail....\n");
		exit(1);
	}
	
	
	// Initial client information struct
	SqsockInfo = (QSOCKINFO*)malloc(sizeof(QSOCKINFO));
	//115200, timeout 600ms
	SqsockInfo->iUartFd = OpenSerial(argv[1],1,6,0);
	if(SqsockInfo->iUartFd == -1){
		exit(1);
	}
	//SqsockInfo->iUartFd = OpenSerial("/dev/ttyUSB1",1,6,0);

	//소켓정보와 큐 정보생성 및 관리 변수에 할당
	if(createQueueSockInfo(SqsockInfo) != EOK){
		//errexit("create QueueSockInfo fail.\n");
		fprintf(stderr,"create QueueSockInfo fail.\n");
	}

	//쓰레드 생성
	if(pthread_create(&recvThreadId, NULL, recvThread, (void*)SqsockInfo)){
		//errexit("create recv thread failed: %s\n", strerror(errno));
		fprintf(stderr,"create recv thread failed: %s\n", strerror(errno));
	}
	pthread_detach(recvThreadId);

	if(pthread_create(&commandThreadId, NULL, cmdProcessingThread, (void*)SqsockInfo)){
		//errexit("create command thread failed: %s\n", strerror(errno));
		fprintf(stderr,"create command thread failed: %s\n", strerror(errno));
	}
	pthread_detach(commandThreadId);

	if(pthread_create(&sendThreadId, NULL, sendThread, (void*)SqsockInfo)){
		//errexit("create send thread failed: %s\n", strerror(errno));
		fprintf(stderr,"create send thread failed: %s\n", strerror(errno));
	}
	pthread_detach(sendThreadId);

	sleep(1);
	/*
	//10프레임 마다 데이터 전송
	fprintf(stderr,"SEND DATA 1\n");
	memset(uchSendBuff,0x0,WRITE_BUFF_SIZE);
	uchSendBuff[0]=0x10;
	uchSendBuff[1]=0x01;
	uchSendBuff[2]=0x00;
	uchSendBuff[3]=0x02;
	uchSendBuff[4]=0x70;
	uchSendBuff[5]=0x0f;
	uchSendBuff[6]=0x8d;
	pushSendDataToQueu(SqsockInfo, 7, uchSendBuff);
	sleep(1);
	fprintf(stderr,"SEND DATA 2\n");
	memset(uchSendBuff,0x0,WRITE_BUFF_SIZE);
	uchSendBuff[0]=0x10;
	uchSendBuff[1]=0x01;
	uchSendBuff[2]=0x00;
	uchSendBuff[3]=0x11;
	uchSendBuff[4]=0x71;
	uchSendBuff[13]=0x0d;
	uchSendBuff[21]=0xA0;
	pushSendDataToQueu(SqsockInfo, 22, uchSendBuff);
	*/
	while(1){
		memset(chData, 0x0, sizeof(chData));
		iReadLen = read(iDataFd, chData, sizeof(chData));
		fprintf(stderr,"##### %d #####\n",iReadLen);
		pushSendDataToQueu(SqsockInfo, iReadLen, chData);
	}
	munmap(unlGpioAddr, 4096);
	close(iMemFd);
	return NULL;
}
