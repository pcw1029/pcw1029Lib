#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <termios.h> 

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include <sys/syslog.h>
#include<syslog.h>

#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>

#include<signal.h>
#include<errno.h>
#include "gThread.h"


//#define PID_FILE "/var/run/.pid"
extern int errno;
int g_iDebug=0;

static void getSignalUser2(int sig);

static void getSignalUser2(int sig){ 
	if(access("/tmp/.debug",0)==0)
		g_iDebug=1;
	else
		g_iDebug=0;
	syslog(LOG_INFO, "by pcw : ########## Debug:%s ##########",g_iDebug==1?"ON":"OFF");
}

int main(int argc, const char * argv[])
{
	char chSystemCmd[128];
	char chClientAddr[32];
	char chRaidCheckVal=SUCCEED;
	int iRetValue;
	int iRetVal;
	int iStart=1;
	int i;
	int iServerSock, iClientSock;
	int errnum;
	int iSockAddrReuse=1;
	int	iSockKeepAlive=1, iSockKeepAliveIdle=30; 
	int	iSockKeepAliveCnt=3, iSockKeepAliveInterval=10;
	socklen_t addrLen;
	struct sockaddr_in serverAddr, clientAddr;
	socklen_t clientAddrLen;
	pthread_t recvThreadId, sendThreadId, commandThreadId;
	pid_t fDspPid;

	/* test code */
	char chIpAddr[16];
	int iPort;
	errno=0;
	//fprintf(stderr,"GNUC:%x, GNUC_PRE:%x, STDC:%x, LEVEL:%x\n",__GNUC__,__GNUC_PREREQ,__STDC_VERSION__,__USE_FORTIFY_LEVEL);

	syslog(LOG_INFO, "by pcw : DSP Process start(v.1.2.4)");
#ifdef __DT10__
	signal(SIGUSR1, getSignalUser1);
#endif
	signal(SIGUSR2, getSignalUser2);

	memset(chIpAddr, 0x0, sizeof(chIpAddr));
	iPort = 7000;
	if(argc == 2)
		strcpy(chIpAddr, argv[1]);
	else if(argc == 3){
		strcpy(chIpAddr, argv[1]);
		iPort = atoi(argv[2]);
	}else
		syslog(LOG_INFO,"dsp_f Execution Fail...");
	syslog(LOG_INFO,"== DSP START ==");

	//시그널 처리를 위해 파일에 pid를 기록해 둔다
	fDspPid = getpid();
	sprintf(chSystemCmd,"echo %d > %s", fDspPid, PID_FILE);
	system(chSystemCmd);
	
	//TCP 설정
	iServerSock = socket(AF_INET, SOCK_STREAM, 0);
	if(iServerSock < 0){
		syslog(LOG_INFO,"by pcw : Server Socket Create Fail...\n");
		close(iServerSock);
		//exit(EXIT_FAILURE);
	}
	iRetVal = setsockopt(iServerSock, SOL_SOCKET, SO_REUSEADDR, 
			&iSockAddrReuse, sizeof(iSockAddrReuse));

	memset((void*)&serverAddr, 0x00, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr(chIpAddr);
	serverAddr.sin_port = htons(iPort);

	if(bind(iServerSock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0){
		syslog(LOG_INFO,"by pcw : Bind Error..\n");
		close(iServerSock);
		iStart=0;
	}

	if(listen(iServerSock, 5) < 0){
		syslog(LOG_INFO,"by pcw : Listen Error..\n");
		close(iServerSock);
		iStart=0;
	}
	
	while(iStart==1){
		QSOCKINFO *qsockInfo;
		qsockInfo = (QSOCKINFO*)malloc(sizeof(QSOCKINFO));
		clientAddrLen = sizeof(clientAddr);
		iClientSock = accept(iServerSock, (struct sockaddr *)&clientAddr, &clientAddrLen);
		if(iClientSock == -1){
			errnum = errno;
			if(errnum != EINTR){
				strcpy(chClientAddr, inet_ntoa(clientAddr.sin_addr));
				syslog(LOG_INFO, "by pcw : Accept Error(%s:%d)..\n",chClientAddr,ntohs(clientAddr.sin_port));
				syslog(LOG_INFO, "by pcw : Accept Error(%s)..\n",strerror(errnum));
			}
			close(iClientSock);
		}else{
			strcpy(chClientAddr, inet_ntoa(clientAddr.sin_addr));
			syslog(LOG_INFO,"by pcw : Accept IP : %s\n",chClientAddr);
			fprintf(stderr,"Accept IP : %s\n",chClientAddr);
			//소켓 옵션 설정
			iRetVal = setsockopt(iClientSock, SOL_SOCKET, SO_KEEPALIVE, 
					&iSockKeepAlive, sizeof(int));
			iRetVal = setsockopt(iClientSock, SOL_TCP, TCP_KEEPIDLE, 
					&iSockKeepAliveIdle, sizeof(int));
			iRetVal = setsockopt(iClientSock, SOL_TCP, TCP_KEEPCNT, 
					&iSockKeepAliveCnt, sizeof(int));
			iRetVal = setsockopt(iClientSock, SOL_TCP, TCP_KEEPINTVL, 
					&iSockKeepAliveInterval, sizeof(int));
			
			//소켓정보와 큐 정보생성 및 관리 변수에 할당
			qsockInfo->iSock = iClientSock;
			if(createQueueSockInfo(qsockInfo) != EOK){
				close(iClientSock);
				close(iServerSock);
				//exit(EXIT_FAILURE);
				syslog(LOG_INFO,"by pcw : dsp.c : %d ", __LINE__);
				break;
			}


			if(access("/tmp/.fis1Dsp1",0)==0)
				qsockInfo->chDSPName = (char)F1DSP1;
			else
				qsockInfo->chDSPName = (char)F2DSP1;
			//쓰레드 생성
			pthread_create(&commandThreadId, NULL, commandGroup, (void*)qsockInfo);
			usleep(1000);
			pthread_create(&sendThreadId, NULL, sendCommandResult, (void*)qsockInfo);
			usleep(1000);
			pthread_create(&recvThreadId, NULL, recvCommand, (void*)qsockInfo);
			usleep(1000);

			//각 쓰레드와 메인 분리
			pthread_detach(commandThreadId);
			pthread_detach(sendThreadId);
			pthread_detach(recvThreadId);
		}
	}
	syslog(LOG_INFO, "by pcw : DSP Process end..");
	return 0;
}
