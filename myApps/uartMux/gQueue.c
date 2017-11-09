/*
 *==================================================
 * queue.c
 * 최초작성자 : 박철우(AWT)
 * 최초작성일 : 2016.03.17
 * 변 경 일 : 2016.03.17
 * 목 적 : Queue를 이용하여 데이터 입출력을 여러 프로
 * 세스에서 사용하기 위함
 * 개정 이력 : 2016.03.17 초기작성
 * 수정시 개정 이력을 남겨주세요...
 *==================================================
*/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <pthread.h>
#include "gQueue.h"
#include "errexit.h"

/*
* 목적 : 큐 생성 및 초기화
* 매개변수 : status - 큐 생성 상태 
*      		 own - 생성된 큐의 관리자명
*			 max - 큐의 연결될 Data의 최대 개수
* 반환값 : Queue관리를 위한 구조체
* 개정 이력 : 2016.03.17 초기작성
*/
/*
status_t  
createQueue(void* i_SqsockInfo, char* i_chQueueName)
{
	QSOCKINFO* SqsockInfo = (QSOCKINFO*)i_SqsockInfo;
	status_t status;
	char chQueueName[16];
		
	Queue *writeQueue;
	Queue *readQueue;
	status=EOK;
	SqsockInfo->writeQueue = NULL;
	SqsockInfo->readQueue = NULL;

	memset(chQueueName, 0x0, sizeof(chQueueName));
	sprintf(chQueueName,"%s_read",i_chQueueName);
  	SqsockInfo->readQueue = qCreate(&status, i_chQueueName, 0xFF);
  	if (SqsockInfo->readQueue == NULL)
		printLog("%s Queue Create Fail..\n", i_chQueueName);
	else{
		memset(chQueueName, 0x0, sizeof(chQueueName));
		sprintf(chQueueName,"%s_write",i_chQueueName);
		SqsockInfo->writeQueue = qCreate(&status, i_chQueueName, 0xFF);
    	if (SqsockInfo->writeQueue == NULL)
			printLog("%s Queue Create Fail..\n", i_chQueueName);	
		else{
			if(strcmp(i_chQueueName,"serial") == 0){
				memset(chQueueName, 0x0, sizeof(chQueueName));
				sprintf(chQueueName,"%s_positioner",i_chQueueName);
				SqsockInfo->writeQueue = qCreate(&status, i_chQueueName, 0xFF);
   				if (SqsockInfo->writeQueue == NULL)
					printLog("%s Queue Create Fail..\n", i_chQueueName);	
			}
		}
	}
	return status;
}
*/
/*
* 목적 : 큐 생성 및 초기화
* 매개변수 : status - 큐 생성 상태 
*      		 own - 생성된 큐의 관리자명
*			 max - 큐의 연결될 Data의 최대 개수
* 반환값 : Queue관리를 위한 구조체
* 개정 이력 : 2016.03.17 초기작성
*/
Queue* qCreate (status_t *status, const char *own, size_t max)
{
    Queue *queueTmp;
	status_t localStatus;
    *status = ENOMEM;
    localStatus = EOK;

    queueTmp = (Queue *)malloc(sizeof(Queue));
    if (queueTmp == NULL)
        printLog( "by pcw : gQueue.c : %d... \n",__LINE__);
    else{
    	queueTmp->head = NULL;
    	queueTmp->tail = NULL;
    	queueTmp->cnt = 0;
    	queueTmp->own = own;
    	queueTmp->max = max;
   		//뮤텍스 초기화
		localStatus = pthread_mutex_init(&queueTmp->mutex, NULL);
		if(localStatus != EOK){
			free(queueTmp);
			queueTmp = NULL;
        	printLog( "by pcw : gQueue.c : %d... \n",__LINE__);
		}
		*status = localStatus;
	}
    return(Queue*)queueTmp;
}

/*
* 목적 : 큐 데이터 생성및 초기화
* 매개변수 : size - 큐데이터 구조체 중 puchData의 size 
* 반환값 : QData 구조체
* 개정 이력 : 2016.03.17 초기작성
*/
QData* qDataCreate(int iSize)
{
	QData* qData;
	qData = malloc(sizeof(QData));
	if(qData == NULL)
		printLog( "by pcw : gQueue.c : %d... \n",__LINE__);
	else{
    	bzero((void*)qData, sizeof(QData));
		qData->puchData = (unsigned char*)malloc(iSize+1);
		if(qData->puchData == NULL){
			free(qData);
			qData = NULL;
		}else
    		bzero((void*)qData->puchData, iSize);
  	}
	return (QData*)qData;
}

/*
* 목적 : 큐 데이터 메모리 삭제 
* 매개변수 : qData - 큐데이터 구조체 point 
* 반환값 : -
* 개정 이력 : 2016.03.17 초기작성
*/
void qDataDestroy(QData* qData)
{
	if(qData != NULL){
		free(qData);
		qData=NULL;
	}
}

/*
* 목적 : 큐 삭제
* 매개변수 : queue - 큐 Pointor  
* 반환값 : status 큐 메모리 삭제 상태 값
* 개정 이력 : 2016.03.17 초기작성
*/
status_t
qDestroyMutex (Queue *queue)
{
    status_t status;
	status_t retState;
	QData* qData;
	int i;
  	if (queue == NULL)
    	retState = ENOENT;
	else{
  		status = pthread_mutex_lock(&queue->mutex);  
    	if ((queue->head != NULL) || (queue->tail!= NULL) || (queue->cnt > 0)) {
      		pthread_mutex_unlock(&queue->mutex);
			for(i=0; i<0xff; i++){
				if(!((queue->head!=NULL) || (queue->tail!=NULL) || (queue->cnt>0)))
					break;
				qData = qPopMutex(queue, &status);
				qDataDestroy(qData);
			}
			free(queue);
			if(i>0xff)
				retState = EBUSY;
			
      		retState = EOK;
    	} else {
        	pthread_mutex_unlock(&queue->mutex);
        	free(queue);
        	retState = EOK;
    	}
  	}
	if(retState == ENOENT)
		printLog( "by pcw : gQueue.c : %d(ENOENT)... \n",__LINE__);
	else if(retState == EBUSY)
		printLog( "by pcw : gQueue.c : %d(EBUSY)... \n",__LINE__);
	else
		printLog( "by pcw : gQueue.c : %d(EOK)... \n",__LINE__);
  	return retState;
}
/*
* 목적 : 큐에 데이큐 데이터 삽입
*      		 queue - queue handler
*			 link - 데이터 및 Next or Previous 큐 포인터
* 반환값 : 큐 데이터 삽입 결과
* 개정 이력 : 2016.03.17 초기작성
*/
status_t qPush (Queue *queue, QLink *link)
{
    QLink *qlinkCurrentHead;
    status_t state;
    state = EINVAL;
    if (queue == NULL)
    	printLog( "by pcw : gQueue.c : %d... \n",__LINE__);
    else{
    	if (link == NULL)
    		printLog( "by pcw : gQueue.c : %d... \n",__LINE__);
      	else{
			if (queue->max > 0) {
        		if (queue->cnt >= queue->max)
           			state = EBUSY;
    		}
    		assert(link->qNext == NULL);
    		assert(link->qPrev == NULL);
			link->ownQueue = queue;
    		if (queue->head == NULL) {
        		queue->head = link;
        		queue->tail = link;
        		link->qNext = NULL; 
        		link->qPrev = NULL;
    		} else {
       			qlinkCurrentHead = (QLink *)queue->head;
        		qlinkCurrentHead->qPrev = link;
        		link->qNext = qlinkCurrentHead;
        		link->qPrev = NULL;
        		queue->head = link;
    		}
			queue->cnt++;
			state = EOK;
		}
	}
    return state;
}

/*
* 목적 : 큐에 데이터 삽입을 위한 뮤텍스 추가
*      		 queue - queue handler
*			 link - 데이터 및 Next or Previous 큐 포인터
* 반환값 : 큐 데이터 삽입 결과
* 개정 이력 : 2016.03.17 초기작성
*/
status_t qPushMutex(Queue *queue, QLink *link )
{
  	status_t     status;
  	status = pthread_mutex_lock(&queue->mutex);
  	if (status != EOK) 
   		printLog( "by pcw : gQueue.c : %d... \n",__LINE__);
  	else{    
  		status = qPush(queue, link);
	}
   	pthread_mutex_unlock(&queue->mutex);
  	return(status);
}

/*
* 목적 : 큐에 데이큐 데이터 추출
*      		 queue - queue handler
*			 status - 데이터 추출 결과 상태 저장
* 반환값 : 추추출된 데이터의 포인터
* 개정 이력 : 2016.03.17 초기작성
*/
void* qPop (Queue *queue, status_t *status)
{
    QLink *qlinkCurrentTail, *qlinkNewTail;
    
    if (queue == NULL) {
    	printLog( "by pcw : gQueue.c : %d... \n",__LINE__);
      	*status = EINVAL;
      	qlinkCurrentTail = NULL;
    }else{
    	if (queue->tail == NULL) {
    		printLog( "by pcw : gQueue.c : %d... \n",__LINE__);
			*status = ENODATA;
			qlinkCurrentTail = NULL;
    	}else{
    		qlinkCurrentTail = queue->tail;
        	if (queue->head == queue->tail) {
        		queue->head = NULL;
        		queue->tail = NULL;
    		} else {
        		queue->tail = qlinkCurrentTail->qPrev;
        		qlinkNewTail = qlinkCurrentTail->qPrev;
        		qlinkNewTail->qNext = NULL;
    		}
    		qlinkCurrentTail->qNext = NULL;
    		qlinkCurrentTail->qPrev = NULL;
    		qlinkCurrentTail->ownQueue = NULL;
    		queue->cnt--;
    		*status = EOK;
   		}
	}
    return((void *)qlinkCurrentTail);
}

/*
* 목적 : 큐에 데이큐 데이터 추출을 위한 뮤텍스 추가
*      		 queue - queue handler
*			 status - 데이터 추출 결과 상태 저장
* 반환값 : 추추출된 데이터의 포인터
* 개정 이력 : 2016.03.17 초기작성
*/
void* qPopMutex (Queue *queue, status_t *status)
{
    status_t _status;
    QLink *qlinkCurrentTail;

    _status = pthread_mutex_lock(&queue->mutex);
    *status = _status;
    if (_status != EOK){
  		printLog( "by pcw : gQueue.c : %d... \n",__LINE__);
        qlinkCurrentTail = NULL;
    }else{
    	qlinkCurrentTail = qPop(queue, status);
	}
   	pthread_mutex_unlock(&queue->mutex);
    return((void *)qlinkCurrentTail);
}

/*
* 목적 : 큐에 저장된 데이터의 개수 확인
*      		 queue - queue handler
* 반환값 : 데이터 개수
* 개정 이력 : 2016.03.17 초기작성
*/
u_int32_t qGetCount (Queue *queue)
{
    return(queue->cnt);
}

/*
* 목적 : 큐에 저장된 데이터의 개수 확인을 위한 뮤텍스 추가
*      		 queue - queue handler
* 반환값 : 데이터 개수
* 개정 이력 : 2016.03.17 초기작성
*/
u_int32_t qGetCountMutex (Queue *queue)
{
    size_t size;
    status_t status;
    status = pthread_mutex_lock(&queue->mutex);
    size = 0;
    if (status != EOK)
    	printLog( "by pcw : gQueue.c : %d... \n",__LINE__);
    else{
    	size = qGetCount(queue);
	}
   	pthread_mutex_unlock(&queue->mutex);
    return(size);
}
