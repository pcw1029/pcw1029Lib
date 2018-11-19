/*
 *==================================================
 * gThread.h
 * 최초작성자 : 박철우(AWT)
 * 최초작성일 : 2016.03.17
 * 변 경 일 : 2016.03.17
 * 개정 이력 : 2016.03.17 초기작성
 * 수정시 개정 이력을 남겨주세요...
 *==================================================
*/
#ifndef __GTHREAD_H__
#define __GTHREAD_H__
#include<pthread.h>
#define READ_BUFF_SIZE 1024
#define WRITE_BUFF_SIZE 1024

typedef u_int32_t status_t;
/**
* @brief read큐 write큐 생성 및 초기화 함수
* @param _qsockInfo QSOCKINFO* 형의 모든 큐의 정보 저장을 위한 변수
* @return 큐생성 및 초기화 결과 반환
*/
status_t createQueueSockInfo(void* _qsockInfo);

/**
* @brief 제어판에서 보낸 명령을 구분하고 처리하는 함수 
* @param _qsockInfo QSOCKINFO*형의 큐의 정보를 가진 변수
*/
void* cmdProcessingThread(void* _qsockInfo);

/**
* @brief 제어판에서 보낸 명령을 큐에 저장하는 함수
* @param _qsockInfo QSOCKINFO*형의 큐의 정보를 가진 변수
*/
void* recvThread(void* _qsockInfo);

/**
* @brief 제어판으로 명령의 대한 결과를 보내는 함수
* @param _qsockInfo QSOCKINFO*형의 큐의 정보를 가진 변수
*/
void* sendThread(void* _qsockInfo);

void pushSendData(void* qsockInfo, int iWriteSize, unsigned char *uchWriteBuff );

#endif
