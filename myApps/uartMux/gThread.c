#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <sys/errno.h>
#include<signal.h>
#include <errno.h>
#include "gQueue.h"
#include "gThread.h"
#include "tcpStruct.h"
#include "icdHandler.h"
#include "errexit.h"
#include "MessageQueueWrapper.h"

//for debugging
extern int errno;

void rawDataPrint(unsigned char *uchData, int iSize, char* funcName){
	int iCnt;
	int iRemainder=0;
	fprintf(stderr,"========= HEX(Data Size : %03d - %s) ===========\n",iSize,funcName);
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

void pushSendData(void* i_SqsockInfo, int iWriteSize, unsigned char *uchWriteBuff ){
	int iPushCnt;
	QData* qData;
	QSOCKINFO* SqsockInfo = (QSOCKINFO*)i_SqsockInfo;
	status_t status=0;
	qData = qDataCreate(iWriteSize);
	if(qData == NULL)
		printLog( "by pcw : %s():%d qDataCreate Fail...",__func__,__LINE__);
	else{
		qData->iLength = iWriteSize;
	
		memcpy((void*)qData->puchData, (void*)uchWriteBuff, iWriteSize);
		for(iPushCnt=0; iPushCnt<10; iPushCnt++){
			status = qPushMutex(SqsockInfo->writeQueue, &qData->link);
			usleep(100);
			if (status == EOK)
				break;
		}
		if(status != EOK){
			printLog( "by pcw : %s():%d qPushMutex Fail...",__func__,__LINE__);
			qDataDestroy(qData);
		}
	}
	return;
}

//status_t status;
void* commandGroup(void* i_SqsockInfo)
{
	pid_t myPid;
	QSOCKINFO* SqsockInfo = (QSOCKINFO*)i_SqsockInfo;
	QData* qData;
	QData* writeQData;
	status_t status;
	int iWriteSize=0;
	unsigned int uiCount;
	unsigned char uchReadBuff[READ_BUFF_SIZE]={0x0};
	unsigned char uchWriteBuff[WRITE_BUFF_SIZE]={0x0};
	struct TcpMsgHead* readMsgHead;
	struct TcpMsgHead* writeMsgHead;
	unsigned char* puchReadData;
	unsigned char* puchWriteData;
	unsigned char uchICDCode=0x0;	
	
 	// 쓰레드 종료를 하기 위해서 필요한 쓰레드ID를 저장.
	myPid = pthread_self();
	SqsockInfo->commandThreadId = myPid;
	status=EOK;

	while(1){
		uiCount = qGetCountMutex(SqsockInfo->readQueue);
		if(uiCount > 0){
			qData = qPopMutex(SqsockInfo->readQueue, &status);
			if(qData == NULL){
				printLog( "by pcw : readQueue pop Mutex error(no data)\n");
				usleep(1000);
				break;
			}
			bzero((void*)uchReadBuff, READ_BUFF_SIZE);
			memset((void*)uchWriteBuff,0x0, WRITE_BUFF_SIZE);
			memcpy((void*)uchReadBuff, (void*)qData->puchData, qData->iLength);
			iWriteSize = qData->iLength;
			// delete qData	
			qDataDestroy(qData);

			readMsgHead = (struct TcpMsgHead*)uchReadBuff;
			writeMsgHead = (struct TcpMsgHead*)uchWriteBuff;
			writeMsgHead->opCode.uchDest = readMsgHead->opCode.uchSrc;
			writeMsgHead->opCode.uchSrc = readMsgHead->opCode.uchDest;
			
			/*
			 * 제어판으로 응답을 보내기 위해 제어판으로 부터 받은 메시지
			 * 형식을 복사한다. dest <-> source 교체
			 */

			//memcpy(writeMsgHead, readMsgHead, sizeof(struct TcpMsgHead));
			/*
			 * 제어판에서 보낸 명령중 메시지 헤더를 제외한 데이터가 있는경우
			 * puchReadData 사용, puchWriteData는 결과저장을 위한 변수
			 */
			puchReadData = uchReadBuff+sizeof(struct TcpMsgHead);
			puchWriteData = uchWriteBuff+sizeof(struct TcpMsgHead);
			iWriteSize = ICDHandler(puchWriteData, puchReadData, readMsgHead->opCode.uchIcdCode);
			iWriteSize = iWriteSize+sizeof(struct TcpMsgHead);
			if(iWriteSize >= 0){
				pushSendData(SqsockInfo, iWriteSize, uchWriteBuff);
			}
		}else{
			usleep(1000);
		}
	}
	//sender thread end
	pthread_cancel(SqsockInfo->sendThreadId);
	status = qDestroyMutex (SqsockInfo->writeQueue);
	if(status != EOK)
		printLog( "by pcw : error qDestroyMutex-writeQueu\n");
	//recv thread end
	pthread_cancel(SqsockInfo->recvThreadId);
	status = qDestroyMutex (SqsockInfo->readQueue);
	if(status != EOK)
		printLog( "by pcw : error qDestroyMutex-readQueu\n");
	return NULL;
}

void* recvCommand(void* i_SqsockInfo)
{
	pid_t myPid;
	QSOCKINFO* SqsockInfo = (QSOCKINFO*)i_SqsockInfo;
	int iClientSock = SqsockInfo->iSock;
	int iRecvSize=0;
	unsigned char uchReadBuff[READ_BUFF_SIZE]={0x0};
	status_t status;
	static QData* qData;
	unsigned int uiCount;
	int errnum;

	myPid = pthread_self();
	SqsockInfo->recvThreadId= myPid;
	status=EOK;

	while(1){
		bzero((void*)uchReadBuff, READ_BUFF_SIZE);
		usleep(1000);
		iRecvSize = read(iClientSock, (void*)uchReadBuff, READ_BUFF_SIZE);
		
		if(iRecvSize <= 0){
			errnum = errno;
			printLog( "by pcw : recv from Controller Error(read retuan value : %s)... \n",strerror(errnum));
			break;
		}else{		
			
			rawDataPrint(uchReadBuff, iRecvSize, __func__);
			qData = qDataCreate(iRecvSize);
	    	if (qData == NULL)
				printLog( "by pcw : %s():%d error create queue data",__func__,__LINE__);
			else{
				memcpy((void*)qData->puchData, (void*)uchReadBuff, iRecvSize);
				qData->iLength = iRecvSize;
				status = qPushMutex(SqsockInfo->readQueue, &qData->link);
	      		if (status != EOK)
					printLog( "by pcw : %s():%d qPushMutex Error(status : %d)",__func__,__LINE__,status);
			}
		}
	}
	//sender thread end
	pthread_cancel(SqsockInfo->sendThreadId);
	status = qDestroyMutex (SqsockInfo->writeQueue);
	if(status != EOK)
		printLog( "by pcw : error qDestroyMutex-writeQueu\n");
	//command thread end
	pthread_cancel(SqsockInfo->commandThreadId);
	status = qDestroyMutex (SqsockInfo->readQueue);
	if(status != EOK)
		printLog( "by pcw : error qDestroyMutex-readQueu\n");
	return NULL;
}

void* sendCommandResult(void* i_SqsockInfo)
{
	pid_t myPid;
	QSOCKINFO* SqsockInfo = (QSOCKINFO*)i_SqsockInfo;
	QData* qData;
	int iClientSock = SqsockInfo->iSock;
	status_t status;
	unsigned int uiCount;
	unsigned char uchWriteBuff[WRITE_BUFF_SIZE]={0x0};
	int iWriteSize;
	struct TcpMsgHead* readMsgHead;
	myPid = pthread_self();
	SqsockInfo->sendThreadId= myPid;
	status=EOK;

	while(1){
		uiCount = qGetCountMutex(SqsockInfo->writeQueue);
		if(uiCount > 0){
			qData = qPopMutex(SqsockInfo->writeQueue, &status);
			if(qData != NULL){	
				bzero((void*)uchWriteBuff, WRITE_BUFF_SIZE);
				memcpy((void*)uchWriteBuff, (void*)qData->puchData, qData->iLength);
				printLog( "by pcw : send to Client msg : %s\n",uchWriteBuff);
				iWriteSize = write(iClientSock, uchWriteBuff, qData->iLength);
				readMsgHead = (struct TcpMsgHead*)uchWriteBuff;
				qDataDestroy(qData);
			}else{
				printLog( "by pcw : gThread.c : %d... \n",__LINE__);
				break;
			}
		}else
			usleep(100);
	}
	//command thread end
	pthread_cancel(SqsockInfo->commandThreadId);
	//recv thread end
	pthread_cancel(SqsockInfo->recvThreadId);
	status = qDestroyMutex (SqsockInfo->readQueue);
	if(status != EOK)
		printLog( "by pcw : error qDestroyMutex-readQueu\n");
	
	status = qDestroyMutex (SqsockInfo->writeQueue);
	if(status != EOK)
		printLog( "by pcw : error qDestroyMutex-writeQueu\n");
	return NULL;
}

void* ipcHandlerFunction(void* i_SqsockInfo)
{
	pid_t myPid;
	QSOCKINFO* SqsockInfo = (QSOCKINFO*)i_SqsockInfo;
	QData* qData;
	
	//memssage queue variable
	int iRet, iMsgKey;
	int iQueueID;
	Message msg;
	char chMsgBuff[MESSAGE_SIZE];
	int mtype=1;
 	// 쓰레드 종료를 하기 위해서 필요한 쓰레드ID를 저장.
	myPid = pthread_self();
	SqsockInfo->ipcHandlerId = myPid;
	
	iMsgKey = MSG_KEY;

	memset(chMsgBuff, 0x0, sizeof(chMsgBuff));

	iQueueID = messageQueueCreate(iMsgKey);
	printf("message queue returned qid %d\n", iQueueID);
	if(iQueueID == -1)
		return NULL; 

	clearMessage(&msg);

	while(1){
		//setMessage(&msg, chMsgBuff, MESSAGE_SIZE, mtype);
		//printf("creating message\n%s\n", messageToString(&msg));
		iRet = messageQueueReceive(iQueueID, &msg, mtype);
		if(iRet == -1){
			sleep(5);
			continue;
		}
		printf("Receiving message returned %d\n", iRet);
		printf("Received message: %s\n", messageToString(&msg));
		memset(chMsgBuff, 0x0, sizeof(chMsgBuff));
		memcpy(chMsgBuff, msg.buffer, MESSAGE_SIZE);
		pushSendData(SqsockInfo, MESSAGE_SIZE, chMsgBuff);
		clearMessage(&msg);
	}
	//sender thread end
	pthread_cancel(SqsockInfo->sendThreadId);
	//recv thread end
	pthread_cancel(SqsockInfo->recvThreadId);
	return NULL;
}


