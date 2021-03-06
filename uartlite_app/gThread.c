#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include <sys/errno.h>
#include<signal.h>
#include <errno.h>
#include "gQueue.h"
#include "gThread.h"
#include "uart.h"
#include "errexit.h"
#include "commonApi.h"

#define CHECK_SUM_LENTH 1
unsigned int gCnt=0;
//for debugging
extern int errno;

/*
* 목적 : 큐 생성 및 초기화
* 매개변수 : status - 큐 생성 상태 
*      		 own - 생성된 큐의 관리자명
*			 max - 큐의 연결될 Data의 최대 개수
* 반환값 : Queue관리를 위한 구조체
* 개정 이력 : 2016.03.17 초기작성
*/

status_t  
createQueueSockInfo(void* i_SqsockInfo)
{
	QSOCKINFO* SqsockInfo = (QSOCKINFO*)i_SqsockInfo;
	status_t status;
		
	status=EOK;
	SqsockInfo->readQueue =NULL;
	SqsockInfo->writeQueue = NULL;

  	SqsockInfo->readQueue = qCreate(&status, "readQueue", 0xFF);
  	if (SqsockInfo->readQueue == NULL)
		printLog("readQueue Create Fail..\n");
	else{
		SqsockInfo->writeQueue = qCreate(&status, "writeQueue", 0xFF);
    	if (SqsockInfo->writeQueue == NULL)
			printLog("writeQueue Create Fail..\n");	
	}
	SqsockInfo->iUartState = 0;
	return status;
}

int pushSendDataToQueu(void* i_SqsockInfo, int iSize, unsigned char *uchWriteBuff ){
	int iPushCnt;
	int iRetVal=0;
	int iUartState;
	QData* qData;
	QSOCKINFO* SqsockInfo = (QSOCKINFO*)i_SqsockInfo;
	status_t status=0;
	qData = qDataCreate(iSize);
	SqsockInfo->iUartState |= 0x10;//send data 
	if(qData == NULL)
		printLog( "by pcw : %s():%d qDataCreate Fail...",__func__,__LINE__);
	else{
		qData->iLength = iSize;
	
		memcpy((void*)qData->pvData, (void*)uchWriteBuff, iSize);
		for(iPushCnt=0; iPushCnt<10; iPushCnt++){
			status = qPushMutex(SqsockInfo->writeQueue, &qData->link);
			usleep(100);
			if (status == EOK)
				break;
		}
		if(status != EOK){
			printLog( "by pcw : %s():%d qPushMutex Fail...",__func__,__LINE__);
			qDataDestroy(qData);
			qData=NULL;
			iRetVal = -1;
		}
	}
	return iRetVal;
}

//status_t status;
void* cmdProcessingThread(void* i_SqsockInfo)
{
	pid_t myPid;
	QSOCKINFO* SqsockInfo = (QSOCKINFO*)i_SqsockInfo;
	QData* qData;
	status_t status;
	int iSize=0;
	unsigned int uiCount;
	char chReadBuff[READ_BUFF_SIZE]={0x0};
	struct MsgHead* pstMsgHead;
	char chIcdCode=0x0;
	int iAz, iEl;
	char chChangeValue[4];
	float fAz, fEl;
	
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
			bzero((void*)chReadBuff, READ_BUFF_SIZE);
			iSize = qData->iLength;
			memcpy((void*)chReadBuff, qData->pvData, iSize);
			fprintf(stderr,"### %s():%d count : %d [iSize:%d]###\n",__func__,__LINE__,gCnt++,iSize);
			printRawData(chReadBuff, iSize, RECV);	
			if(iSize > 5){
				chIcdCode = chReadBuff[6];
				fprintf(stderr,"### %s():%d icd:%02x ###\n",__func__,__LINE__, chIcdCode);
				if(chIcdCode == 0x42){
					chChangeValue[3] = chReadBuff[7];
					chChangeValue[2] = chReadBuff[8];
					chChangeValue[1] = chReadBuff[9];
					chChangeValue[0] = chReadBuff[10];
					memcpy(&iAz, chChangeValue, sizeof(int));
					fAz = iAz / 4096.0;
				}
				chIcdCode = chReadBuff[11];
				fprintf(stderr,"### %s():%d icd:%02x ###\n",__func__,__LINE__, chIcdCode);
				if(chIcdCode == 0x43){
					chChangeValue[3] = chReadBuff[12];
					chChangeValue[2] = chReadBuff[13];
					chChangeValue[1] = chReadBuff[14];
					chChangeValue[0] = chReadBuff[15];
					memcpy(&iEl, chChangeValue, sizeof(int));
					fEl = iEl / 4096.0;
				}
				fprintf(stderr,"Az : %f, El : %f\n",fAz, fEl);
			}

			// delete qData	
			qDataDestroy(qData);
			qData=NULL;
		}else{
			usleep(1000);
		}
	}
	//sender thread end
	pthread_cancel(SqsockInfo->sendThreadId);
	status = qDestroyMutex (SqsockInfo->writeQueue);
	SqsockInfo->writeQueue=NULL;
	if(status != EOK)
		printLog( "by pcw : error qDestroyMutex-writeQueu\n");
	//recv thread end
	pthread_cancel(SqsockInfo->recvThreadId);
	status = qDestroyMutex (SqsockInfo->readQueue);
	SqsockInfo->readQueue = NULL;
	if(status != EOK)
		printLog( "by pcw : error qDestroyMutex-readQueu\n");
	return NULL;
}


int check_integrity(unsigned char *uchReadBuff, int iDataLen)
{
	int i;
	int tmp;
	int iCheckSum=0;
	char chCheckSum;
	fprintf(stderr,"check data size : %d\n",iDataLen);
	for(i=0; i<iDataLen-1; i++){
		if((uchReadBuff[i]&0xF0) == 0xF0)
			tmp = (uchReadBuff[i]&0x0F)+240;
		else
			tmp = uchReadBuff[i];
		iCheckSum += tmp;
//		fprintf(stderr,"%d. %02x, %02X\n",i, uchReadBuff[i], iCheckSum);
	}
	memcpy(&chCheckSum, &iCheckSum, 1);
	//fprintf(stderr,"len:%d, sum:%d(%02X) %02X, %02X\n",iDataLen, iCheckSum, iCheckSum, chCheckSum, uchReadBuff[16]);
	if((chCheckSum&0x000000FF) == uchReadBuff[iDataLen-1])
		return 1;
	else{
		fprintf(stderr,"len:%d, sum:%d(%02X) %02X, %02X\n",iDataLen, iCheckSum, iCheckSum, chCheckSum&0x000000FF, uchReadBuff[16]);
		return 0;
	}
}

int checkHeader(unsigned char* uchReadBuff, int iTotalSize)
{
	int i;
	short nTmp;
	int iDataLen;
	int iNext;
	for(i=0; i<iTotalSize-3; i++){
		if(uchReadBuff[i] == 0x10){
			if(uchReadBuff[i+1] == 0x01){
				iDataLen = uchReadBuff[i+3];
				if(iDataLen <= 0x0C){//받고자 하는 데이터의 최대 길이가 12바이트가 넘지 않는다.
					break;
/*
					iNext = iDataLen + 4 + 1;
					if(iNext + 4 > iTotalSize){
						if(uchReadBuff[iNext] == 0x10){
							if(uchReadBuff[iNext+1] == 0x01)
								break;
						}
					}
*/
				}
			}
		}
	}
	return i;
}

void* recvThread(void* i_SqsockInfo)
{
	pid_t myPid;
	QSOCKINFO* SqsockInfo = (QSOCKINFO*)i_SqsockInfo;
	int iUartFd = SqsockInfo->iUartFd;
	int iRecvSize=0;
	int iTotalSize=0;
	int iRemainderSize=0;
	int iCopySize=0;
	int iDeleteSize;
	unsigned char uchReadBuff[READ_BUFF_SIZE]={0x0};
	unsigned char uchTmpBuff[READ_BUFF_SIZE]={0x0};
	//struct TcpMsgHead* readMsgHead;
	status_t status;
	static QData* qData;
	unsigned int uiCount;
	int errnum;
	int iDataLen;

	struct MsgHead* pstMsgHead;
	myPid = pthread_self();
	SqsockInfo->recvThreadId= myPid;
	status=EOK;

	tcflush(iUartFd, TCIFLUSH);
	while(1){
		SqsockInfo->iUartState |= 0x01;
		bzero((void*)uchReadBuff, READ_BUFF_SIZE);
		//fprintf(stderr,"### %s():%d tot:%d ###\n",__func__, __LINE__, iTotalSize);
		iRecvSize = read(iUartFd, (void*)uchReadBuff+iTotalSize, READ_BUFF_SIZE-iTotalSize);
		if(iRecvSize <= 0){
			//0.5초 이내 데이터가 안들어오는 경우 
			SqsockInfo->iUartState &= 0xF0;
			while((SqsockInfo->iUartState & 0xF0) == 0x10)
				usleep(500);
		}else{		
			iTotalSize += iRecvSize;
			//fprintf(stderr,"### %s():%d tot:%d ###\n",__func__, __LINE__, iTotalSize);
			//iTotalSize = 0;
			
			while(iTotalSize > 4){
				//헤더 검사
				//fprintf(stderr,"### %s():%d tot:%d ###\n",__func__, __LINE__, iTotalSize);
				iDeleteSize = checkHeader(uchReadBuff, iTotalSize);
				if(iDeleteSize > 0){
					//전송받은 데이터의 처음이 헤더가 아닌 경우 다음 헤더를 찾아 헤더 이전데이터를 삭제한다.
					//fprintf(stderr,"### %s():%d tot:%d del:%d###\n",__func__, __LINE__, iTotalSize, iDeleteSize);
					iTotalSize = iTotalSize - iDeleteSize;
					memset(uchTmpBuff, 0x0, sizeof(uchTmpBuff));
					memcpy(uchTmpBuff, uchReadBuff+iDeleteSize, iTotalSize);
					memset(uchReadBuff, 0x0, sizeof(uchReadBuff));
					memcpy(uchReadBuff, uchTmpBuff, iTotalSize);
				}else{
					//fprintf(stderr,"### %s():%d tot:%d del:%d###\n",__func__, __LINE__, iTotalSize, iDeleteSize);
					pstMsgHead = (struct MsgHead*)uchReadBuff;
					iDataLen = pstMsgHead->uchDataSize; 	    // Data 크기 
					iCopySize = iDataLen + sizeof(struct MsgHead)+CHECK_SUM_LENTH;
					if(iCopySize > iTotalSize){
						//헤더는 정상이지만 아직 모든 데이터를 못 받은 경우
						//fprintf(stderr,"### %s():%d tot:%d cpy:%d###\n",__func__, __LINE__, iTotalSize, iCopySize);
						break;
					}
					//데이터 무결성 확인
					if(check_integrity(uchReadBuff, iCopySize)){
						qData = qDataCreate(iCopySize);
						if (qData == NULL)
							printLog( "by pcw : %s():%d error create queue data",__func__,__LINE__);
						else{
							iRemainderSize = iTotalSize - iCopySize; 
							iTotalSize = iTotalSize - iRemainderSize;
							memcpy(qData->pvData, (void*)uchReadBuff, iCopySize);
							qData->iLength = iCopySize;
							memset(uchTmpBuff, 0x0, sizeof(uchTmpBuff));
							memcpy(uchTmpBuff, uchReadBuff+iCopySize, iRemainderSize);
							memset(uchReadBuff, 0x0, sizeof(uchReadBuff));
							memcpy(uchReadBuff, uchTmpBuff+iCopySize, iRemainderSize);
							status = qPushMutex(SqsockInfo->readQueue, &qData->link);
							if (status != EOK)
								printLog( "by pcw : %s():%d qPushMutex Error(status : %d)",__func__,__LINE__,status);
						}
					}else{
						//무결성이 깨진 경우 무결성 검사한 데이터를 모두 삭제한다.
						fprintf(stderr,"### %s():%d tot:%d cpy:%d###\n",__func__, __LINE__, iTotalSize, iCopySize);
						printRawData(uchReadBuff, iCopySize, RECV);
						iTotalSize = iTotalSize - iCopySize;
						memset(uchTmpBuff, 0x0, sizeof(uchTmpBuff));
						memcpy(uchTmpBuff, uchReadBuff+iCopySize, iTotalSize);
						memset(uchReadBuff, 0x0, sizeof(uchReadBuff));
						memcpy(uchReadBuff, uchTmpBuff, iTotalSize);
						fprintf(stderr,"### %s():%d tot:%d cpy:%d###\n",__func__, __LINE__, iTotalSize, iCopySize);
					}
				}
			}	
			
		}
	}
	//command thread end
	pthread_cancel(SqsockInfo->commandThreadId);
	status = qDestroyMutex (SqsockInfo->readQueue);
	SqsockInfo->readQueue=NULL;
	if(status != EOK)
		printLog( "by pcw : error qDestroyMutex-readQueu\n");
	//sender thread end
	pthread_cancel(SqsockInfo->sendThreadId);
	pthread_exit(SqsockInfo->sendThreadId);
	status = qDestroyMutex (SqsockInfo->writeQueue);
	SqsockInfo->writeQueue = NULL;
	if(status != EOK)
		printLog( "by pcw : error qDestroyMutex-writeQueu\n");
	return NULL;
}

void* sendThread(void* i_SqsockInfo)
{
	pid_t myPid;
	QSOCKINFO* SqsockInfo = (QSOCKINFO*)i_SqsockInfo;
	QData* qData;
	int iUartFd = SqsockInfo->iUartFd;
	status_t status;
	unsigned int uiCount;
	unsigned char uchWriteBuff[WRITE_BUFF_SIZE]={0x0};
	int iWriteSize;
	myPid = pthread_self();
	SqsockInfo->sendThreadId= myPid;
	status=EOK;

	while(1){
		uiCount = qGetCountMutex(SqsockInfo->writeQueue);
		if(uiCount > 0){
			qData = qPopMutex(SqsockInfo->writeQueue, &status);
			if(qData != NULL){	
				bzero((void*)uchWriteBuff, WRITE_BUFF_SIZE);
				memcpy((void*)uchWriteBuff, (void*)qData->pvData, qData->iLength);
				printRawData(uchWriteBuff, qData->iLength, SEND);
					
				//fprintf(stderr,"### %s():%d ###\n",__func__, __LINE__);
				while((SqsockInfo->iUartState & 0x0F) == 0x01)
					usleep(500);
				//fprintf(stderr,"### %s():%d ###\n",__func__, __LINE__);
				SqsockInfo->iUartState &= 0x0F;

				iWriteSize = write(iUartFd, uchWriteBuff, qData->iLength);
				tcflush(iUartFd, TCOFLUSH);
				qDataDestroy(qData);
				qData=NULL;
			}else{
				printLog( "by pcw : gThread.c : %d... \n",__LINE__);
				break;
			}
		}else
			usleep(100);
	}
	//command thread end
	//recv thread end
	pthread_cancel(SqsockInfo->recvThreadId);
	status = qDestroyMutex (SqsockInfo->readQueue);
	SqsockInfo->readQueue=NULL;
	if(status != EOK)
		printLog( "by pcw : error qDestroyMutex-readQueu\n");
	
	status = qDestroyMutex (SqsockInfo->writeQueue);
	SqsockInfo->writeQueue=NULL;
	if(status != EOK)
		printLog( "by pcw : error qDestroyMutex-writeQueu\n");
	return NULL;
}
