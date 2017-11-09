#include<stdio.h>
#include <pthread.h>
#include <sys/time.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <fcntl.h> 
#include <stdlib.h> 
#include <string.h> 
#include <errno.h>
#include "gQueue.h"
#include "serial.h"
#include "errexit.h"
#include "MessageQueueWrapper.h"

extern int errno;

void rawDataPrint(unsigned char *uchData, int iSize, char* funcName){
	int iCnt;
	int iRemainder=0;
	fprintf(stderr,"========= HEX(Data Size : %03d-%s) ===========\n",iSize,funcName);
	for(iCnt=1; iCnt<=iSize; iCnt++){
		if(iCnt%16==0){
			fprintf(stderr,"%02X\n",uchData[iCnt-1]);
		}else{
			if(iCnt%8 == 0){ 
				fprintf(stderr,"%02X\t",uchData[iCnt-1]);
			}else{
				fprintf(stderr,"%02X ",uchData[iCnt-1]);
			}
		}   
	} 
	fprintf(stderr,"\n===================== END =====================\n");
}

void* positionerFunction(void* i_SqSerialInfo)
{
	QSERIAL_INFO* SqSerialInfo = (QSERIAL_INFO*)i_SqSerialInfo;
	QData* qData;
	pid_t myPid;
	fd_set readFds;
	status_t status=0;
	struct timeval tv;
	unsigned int uiCount;
	int iSerialFd;
	int iState;
	int iReadSize, iWriteSize;
	unsigned char uchReadBuff[128];
	unsigned char uchWriteBuff[128];

	myPid = pthread_self();
	SqSerialInfo->posionerThId= myPid;
	status=EOK;

	iSerialFd = OpenSerial("/dev/ttyUL4", 115200, 1, 52);                                                                                 
	if(iSerialFd <0){
		printf("Serial Open Error\n");
		exit(1);
	}

	while(1){
		tv.tv_sec=0;
		tv.tv_usec=30;
		FD_ZERO(&readFds);
		FD_SET(iSerialFd, &readFds);
		uiCount = qGetCountMutex(SqSerialInfo->positionerQueue);
		if(uiCount > 0){
			qData = qPopMutex(SqSerialInfo->positionerQueue, &status);
			if(qData != NULL){	
				bzero((void*)uchWriteBuff, sizeof(uchWriteBuff));
				memcpy((void*)uchWriteBuff, (void*)qData->puchData, qData->iLength);
				rawDataPrint(uchWriteBuff, qData->iLength,__func__);
				iWriteSize = write(iSerialFd, uchWriteBuff, strlen(uchWriteBuff));
				qDataDestroy(qData);
			}else{
				printLog( "by pcw : gThread.c : %d... \n",__LINE__);
				break;
			}
		}else{
			status=EOK;
			iState = select(iSerialFd+1, &readFds, (fd_set *)0, (fd_set *)0,  &tv);
			if(iState > 0){
				memset(uchReadBuff,0x0,sizeof(uchReadBuff));
				iReadSize = read(iSerialFd, uchReadBuff, sizeof(uchReadBuff));
				fprintf(stderr,"[POSITIONER] recv : %s\n",uchReadBuff);
				qData = qDataCreate(iReadSize);
				qData->iLength = iReadSize;
				memcpy((void*)qData->puchData, (void*)uchReadBuff, iReadSize);
				fprintf(stderr,"[POSITIONER] push readQueue\n");
				status = qPushMutex(SqSerialInfo->readQueue, &qData->link);
				if(status != EOK){
					printLog( "by pcw : %s():%d qPushMutex Fail...",__func__,__LINE__);
					qDataDestroy(qData);
				}
			}
		}
	}
	Close_serial(iSerialFd);
	return 0;
}


void* serialHandler(void* i_SqSerialInfo)
{
	QSERIAL_INFO* SqSerialInfo = (QSERIAL_INFO*)i_SqSerialInfo;
	QData* qData;
	pid_t myPid;
	status_t status=EOK;

	unsigned int uiReadCount;
	unsigned int uiWriteCount=0;
	unsigned char uchReadBuff[128];
	unsigned char uchWriteBuff[128];
//memssage queue variable
	int iRet, iMsgKey;
	int iQueueID;
	Message msg;
	char chMsgBuff[MESSAGE_SIZE];
	int mtype=1;

	myPid = pthread_self();
	SqSerialInfo->serialHandlerThId= myPid;
	
	iMsgKey = MSG_KEY;
	iQueueID = messageQueueGet(iMsgKey);
	clearMessage(&msg);
	printf("creating message\n%s\n", messageToString(&msg));
	fprintf(stderr,"Queue ID : %d\n",iQueueID);

	while(1){
		uiReadCount = qGetCountMutex(SqSerialInfo->readQueue);
		if(uiReadCount > 0){
			status=EOK;
			qData = qPopMutex(SqSerialInfo->readQueue, &status);
			if(qData != NULL){	
				bzero((void*)uchReadBuff, sizeof(uchReadBuff));
				memcpy((void*)uchReadBuff, (void*)qData->puchData, qData->iLength);
				memset(chMsgBuff,0x0,MESSAGE_SIZE);	
				memcpy((void*)chMsgBuff, uchReadBuff, qData->iLength);
				
				fprintf(stderr,"Send MSG_QUEUE\n");
				//rawDataPrint(chMsgBuff, qData->iLength,__func__);
				setMessage(&msg, chMsgBuff,MESSAGE_SIZE ,mtype);
				iRet = messageQueueSend(iQueueID, &msg);
				//printf("Deleting qid: %d returned %d\n", iQueueID, messageQueueDelete(iQueueID));
				printf("creating message\n%s\n", messageToString(&msg));
				clearMessage(&msg);
				qDataDestroy(qData);
		//	}else{
				//printLog( "by pcw : %s() : %d... \n",__func__,__LINE__);
			}
		}
/*
		uiWriteCount = qGetCountMutex(SqSerialInfo->writeQueue);
		if(uiWriteCount > 0){
			status=EOK;
			qData = qPopMutex(SqSerialInfo->writeQueue, &status);
			if(qData != NULL){	
				bzero((void*)uchWriteBuff, sizeof(uchWriteBuff));
				memcpy((void*)uchWriteBuff, (void*)qData->puchData, qData->iLength);
				qDataDestroy(qData);
			}else{
				printLog( "by pcw : %s() : %d... \n",__func__,__LINE__);
			}
		} 
*/
		if(uiReadCount== 0 && uiWriteCount==0)
			usleep(10);

	}
	return 0;
}

void* trackerFunction(void* i_SqSerialInfo)
{
	QSERIAL_INFO* SqSerialInfo = (QSERIAL_INFO*)i_SqSerialInfo;
	QData* qIpcData;
	QData* qPositionData;
	pid_t myPid;
	fd_set readFds;
	status_t status=0;
	struct timeval tv;
	unsigned int uiCount;
	int iSerialFd;
	int iState;
	int iReadSize;
	unsigned char uchReadBuff[128];
	FILE* trackerStatusFd;
	char tStatus[2];
	int tSw, _tSw;
	myPid = pthread_self();
	SqSerialInfo->trackerThId = myPid;
	
	status=EOK;

	iSerialFd = OpenSerial("/dev/ttyUL5", 38400, 1, 52);                                                                                 
	if(iSerialFd <0){
		printf("Serial Open Error\n");
		exit(1);
	}

	trackerStatusFd = popen("cat /tmp/trackerSw","r");
	if(trackerStatusFd !=NULL){
		fgets(tStatus,sizeof(tStatus),trackerStatusFd);
		pclose(trackerStatusFd);
		tSw = atoi(tStatus);
	}
	while(1)
	{
		tv.tv_sec=0;
		tv.tv_usec=30;
		FD_ZERO(&readFds);
		FD_SET(iSerialFd, &readFds);

		trackerStatusFd = popen("cat /tmp/trackerSw","r");
		if(trackerStatusFd !=NULL){
			fgets(tStatus,sizeof(tStatus),trackerStatusFd);
			pclose(trackerStatusFd);
			_tSw = atoi(tStatus);
			if(_tSw != tSw){
				fprintf(stderr,"change serial...\n");
				Close_serial(iSerialFd);
				iSerialFd = OpenSerial("/dev/ttyUL5", 38400, 1, 52);                                                                                 
				tSw=_tSw;
			}
		}

		iState = select(iSerialFd+1, &readFds, (fd_set *)0, (fd_set *)0,  &tv);
		if(iState > 0){
			memset(uchReadBuff,0x0,sizeof(uchReadBuff));
			iReadSize = read(iSerialFd, uchReadBuff, sizeof(uchReadBuff));
			fprintf(stderr,"[TRACKER] recv : %s\n",uchReadBuff);
			qPositionData = qDataCreate(iReadSize);
			qPositionData->iLength = iReadSize;
			memcpy((void*)qPositionData->puchData, (void*)uchReadBuff, iReadSize);
			status = qPushMutex(SqSerialInfo->positionerQueue, &qPositionData->link);
			if(status != EOK){
				printLog( "by pcw : %s():%d qPushMutex Fail...",__func__,__LINE__);
				qDataDestroy(qPositionData);
			}
			qIpcData = qDataCreate(iReadSize);
			qIpcData->iLength = iReadSize;
			memcpy((void*)qIpcData->puchData, (void*)uchReadBuff, iReadSize);
			status = qPushMutex(SqSerialInfo->readQueue, &qIpcData->link);
			if(status != EOK){
				printLog( "by pcw : %s():%d qPushMutex Fail...",__func__,__LINE__);
				qDataDestroy(qIpcData);
			}
		}
	}
	Close_serial(iSerialFd);
	return 0;
}


int main(int argc,char* argv[])
{
	status_t status;
	pthread_t positionerThId;
	pthread_t trackerThId;
	pthread_t serialHandlerThId;
	pthread_t gpsThId;
	pthread_t modemThId;
	QSERIAL_INFO *SqSerialInfo;
	

	{
		SqSerialInfo = (QSERIAL_INFO*)malloc(sizeof(QSERIAL_INFO));
		SqSerialInfo->writeQueue=NULL;
		SqSerialInfo->readQueue=NULL;
		SqSerialInfo->positionerQueue=NULL;
		
		//큐 정보생성 및 관리 변수에 할당
  		SqSerialInfo->writeQueue = qCreate(&status, "WSerial", 0xFF);
  		if (SqSerialInfo->writeQueue == NULL)
        	errexit("create serial write queue fail.\n");
		
  		SqSerialInfo->readQueue = qCreate(&status, "RSerial", 0xFF);
  		if (SqSerialInfo->readQueue == NULL)
        	errexit("create serial read queue fail.\n");
		
  		SqSerialInfo->positionerQueue = qCreate(&status, "PSerial", 0xFF);
  		if (SqSerialInfo->positionerQueue == NULL)
        	errexit("create serial positioner queue fail.\n");


		// Posioner 쓰레드 생성
		if(pthread_create(&positionerThId, NULL, positionerFunction, (void*)SqSerialInfo))
        	errexit("create send thread failed: %s\n", strerror(errno));
		//쓰레드와 메인 분리
		pthread_detach(positionerThId);


		// Tracker 쓰레드 생성
		if(pthread_create(&trackerThId, NULL, trackerFunction, (void*)SqSerialInfo))
        	errexit("create send thread failed: %s\n", strerror(errno));
		//쓰레드와 메인 분리
		pthread_detach(trackerThId);

		// serial handler 쓰레드 생성
		if(pthread_create(&serialHandlerThId, NULL, serialHandler, (void*)SqSerialInfo))
        	errexit("create send thread failed: %s\n", strerror(errno));
		//쓰레드와 메인 분리
		pthread_detach(serialHandlerThId);
/*
		// gps 쓰레드 생성
		if(pthread_create(&gpsThId, NULL, gpsFunction, (void*)SqSerialInfo))
        	errexit("create send thread failed: %s\n", strerror(errno));
		//쓰레드와 메인 분리
		pthread_detach(gpsThId);

		// commandGroup 쓰레드 생성
		if(pthread_create(&modemThId, NULL, modemFunction, (void*)SqSerialInfo))
        	errexit("create send thread failed: %s\n", strerror(errno));
		//쓰레드와 메인 분리
		pthread_detach(modemThId);
*/
		while(1);
	}
	return 0;
}

