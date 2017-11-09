/*
 *==================================================
 * queue.h
 * 최초작성자 : 박철우(AWT)
 * 최초작성일 : 2016.03.17
 * 변 경 일 : 2016.03.17
 * 목 적 : Queue를 이용하여 데이터 입출력을 여러 프로
 * 세스에서 사용하기 위해
 * 개정 이력 : 2016.03.17 초기작성
 * 수정시 개정 이력을 남겨주세요...
 *==================================================
*/
#ifndef __LQUEUE_H__
#define __LQUEUE_H__

#ifndef EOK
#define EOK 0
#endif

#include<netinet/in.h>

typedef enum {FALSE,TRUE} Boolean;
typedef u_int32_t status_t;

/*
* Queue structure
*/

/*
* 목적 : 큐를 관리하기 위한 Handler구조체
* 멤버변수 : head/tail - 실제 데이터의 처음과 끝
*      		 cnt/max - 데이터 개수 / 최대 저장 개수
*			 mutex - 임계 영역을 보호하기 위한 Mutex key
* 			 own - 큐 Handler의 담당자명
* 개정 이력 : 2016.03.17 초기작성
*/
typedef struct _queue {
    void            *head;
    void            *tail;
    u_int32_t       cnt;
    u_int32_t       max;
    pthread_mutex_t mutex;
    const char      *own;
}Queue;

/*
* 목적 : Queue데이터의 이전 과 다음 순서를 관리하는 구조체
* 멤버변수 : qNext/qPrev - 다음 큐데이터/이전 큐데이터
* 			 ownQueue - 큐 Linker의 담당자명
* 개정 이력 : 2016.03.17 초기작성
*/
typedef struct _qLink {
    void     *qNext;
    void     *qPrev;
    Queue *ownQueue;
}QLink;

/*
* 다음 혹은 이전 Queue에 접근할수 있다.
*/
/*
* 목적 : Queue에 저장될 Data 구조체
* 멤버변수 : link - 이전 혹은 다음 Data구조체 연결 변수
* 			 puchData - Data 
* 개정 이력 : 2016.03.17 초기작성
*/
typedef struct _qData{
    QLink     		link;
	unsigned char 	*puchData;
	int 			iLength;
}QData;


typedef struct _QSOCKINFO{
	char chName;
	int iSock;
	struct sockaddr_in server;        
	Queue *writeQueue;
	Queue *readQueue;
	pthread_t recvThreadId; 
	pthread_t sendThreadId; 
	pthread_t commandThreadId;
}QSOCKINFO;

/**
* @brief read큐 write큐 생성 및 초기화 함수
* @param _qsockInfo QSOCKINFO* 형의 모든 큐의 정보 저장을 위한 변수
* @return 큐생성 및 초기화 결과 반환
*/
status_t createQueue(void* _qsockInfo, char* i_chQueueName);



/*
Queue function list
*/
/**
* @brief 큐를 생성하기 위한 함수
* @param status status_t* 형의 상태값을 저장하는 변수
* @param own const char* 형의 큐의 종류를 구분하기 위한 변수
* @param max size_t 형의 큐의 List개수를 설정하기 위한 변수
* @return 생성된 큐의 주소를 반환
*/
Queue* qCreate (status_t *status, const char *own, size_t max);

/**
* @brief 큐에 들어갈 데이터를 생성하기 위한 함수
* @param iSize int형의 데이터 길이를 나타내는 변수
* @return 큐에 들어갈 데이터의 주소를 반환
*/
QData* qDataCreate(int iSize);

/**
* @brief 큐에 들어있는 데이터를 삭제하기 위한 함수
* @param qData QData*형의 데이터의 주소를 지닌 변수
*/
void qDataDestroy(QData* qData);

/**
* @brief 큐 삭제동안 동시 접근을 막기 위해 Mutex를 사용한 함수
* @param queue Queue* 형의 큐의 주소를 지닌 변수
* @return 큐 삭제 결과를 반환
*/
status_t qDestroyMutex (Queue *queue);

/**
* @brief 큐에 데이터를 삽입하기 위한 함수
* @param queue Queue* 형의 큐의 주소를 지닌 변수
* @param link QLink* 형의 큐의 데이터의 주소를 지닌 변수
* @return 큐에 삽입결과를 반환
*/
status_t qPush (Queue *queue, QLink *link);

/**
* @brief 큐 삽입 동안 동시 접근을 막기 위해 Mutex를 사용한 함수
* @param queue Queue* 형의 큐의 주소를 지닌 변수
* @param link QLink* 형의 큐의 데이터의 주소를 지닌 변수
* @return 큐 삭제 결과를 반환
*/
status_t qPushMutex(Queue *queue, QLink *link);

/**
* @brief 큐에서 데이트를 추출하기 위한 함수
* @param queue Queue* 형의 큐의 주소를 지닌 변수
* @param status status_t*형의 큐의 데이터 추출결과 저장을 위한 변수
*/
void* qPop (Queue *queue, status_t *status);

/**
* @brief 큐에 데이터 추출동안 동시 접근을 막기 위해 Mutex를 사용한 함수
* @param queue Queue* 형의 큐의 주소를 지닌 변수
* @param status status_t*형의 큐의 데이터 추출결과 저장을 위한 변수
*/
void* qPopMutex (Queue *queue, status_t *status);

/**
* @brief 큐의 존재하는 데이터의 개수를 확인 하는 함수
* @param queue Queue* 형의 큐의 주소를 지닌 변수
* @return 큐의 남아있는 데이터의 개수 반환
*/
u_int32_t qGetCount (Queue *queue);

/**
* @brief 큐에 존재하는 데이터 개수를 확인하는 동안 동시 접근을 막기 위해 Mutex를 사용한 함수
* @param queue Queue* 형의 큐의 주소를 지닌 변수
* @return 큐의 남아있는 데이터의 개수 반환
*/
u_int32_t qGetCountMutex (Queue *queue);
#endif 
