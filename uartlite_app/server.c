// system types
#include <sys/types.h>    
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
#include "uart.h"

extern int errno;

// Main function
int main(int argc,char* argv[])
{
	pthread_t recvThreadId;
	pthread_t sendThreadId;
	pthread_t commandThreadId;

	QSOCKINFO *SqsockInfo;

	unsigned char uchSendBuff[WRITE_BUFF_SIZE];

    while(1)
    {
		// Initial client information struct
		SqsockInfo = (QSOCKINFO*)malloc(sizeof(QSOCKINFO));
		SqsockInfo->iUartFd = OpenSerial("/dev/ttyUSB0",1,5,0);

		//소켓정보와 큐 정보생성 및 관리 변수에 할당
		if(createQueueSockInfo(SqsockInfo) != EOK){
        	errexit("create QueueSockInfo fail.\n");
		}

		//쓰레드 생성
		if(pthread_create(&recvThreadId, NULL, recvThread, (void*)SqsockInfo))
        	errexit("create recv thread failed: %s\n", strerror(errno));
		pthread_detach(recvThreadId);

		if(pthread_create(&commandThreadId, NULL, cmdProcessingThread, (void*)SqsockInfo))
        	errexit("create command thread failed: %s\n", strerror(errno));
		pthread_detach(commandThreadId);

		if(pthread_create(&sendThreadId, NULL, sendThread, (void*)SqsockInfo))
        	errexit("create send thread failed: %s\n", strerror(errno));
		pthread_detach(sendThreadId);
		sleep(1);
		//10프레임 마다 데이터 전송
		memset(uchSendBuff,0x0,WRITE_BUFF_SIZE);
		uchSendBuff[0]=0x10;
		uchSendBuff[1]=0x01;
		uchSendBuff[2]=0x00;
		uchSendBuff[3]=0x02;
		uchSendBuff[4]=0x70;
		uchSendBuff[5]=0x0a;
		uchSendBuff[6]=0x8d;
		pushSendDataToQueu(SqsockInfo, 7, uchSendBuff);
		sleep(2);

	//데이터 종류 선택
		memset(uchSendBuff,0x0,WRITE_BUFF_SIZE);
		uchSendBuff[0]=0x10;
		uchSendBuff[1]=0x01;
		uchSendBuff[2]=0x00;
		uchSendBuff[3]=0x11;
		uchSendBuff[4]=0x71;
		uchSendBuff[13]=0x0d;
		uchSendBuff[21]=0xA0;
		pushSendDataToQueu(SqsockInfo, 22, uchSendBuff);
	
		while(1);
    }
    return 0;
}

