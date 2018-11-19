#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include "uartDataLink.h"
#include "socket.h"
#include "errexit.h"

int iChangeMode = NONE;

void changeUartMode(int iSigNo)
{
	printDebugLog("#### %s():%d ####\n",__func__,__LINE__);
	iChangeMode = WAITING;
	while(iChangeMode != NONE){
		usleep(30);
	}
	printDebugLog("#### %s():%d ####\n",__func__,__LINE__);
	iChangeMode = CHANGE;
}

int main(int argc, char* argv[] )
{
	struct uart *pstUart;
	char chData[33];
	char chMode[16];
	int iDataFd;
	int iMode;
	FILE* pFd;
	struct sigaction int_handler = {.sa_handler=changeUartMode};
	fd_set readFds;
	struct timeval tv;
	int iState;

	if(sigaction(SIGUSR1, &int_handler, NULL) < 0){
		exit(1);
	}
	// POSITION RAW SOCKET 연결
	iDataFd = connect_server(REQUEST_DATALINK);
	if(iDataFd == -1){
		printDebugLog("connect server fail....\n");
		exit(1);
	}


	pstUart = (struct uart*)malloc(sizeof(struct uart));

	pstUart->baud_rate = 9600;
	memset(pstUart->uart_path, 0x0, sizeof(pstUart->uart_path));
	strcpy(pstUart->uart_path, "/dev/ttyUL1");
	pstUart->blocking_mode = 1;
	pstUart->parity_check = 0;
	pstUart->two_stop_bit = 0;

	uart_open(pstUart);
	iMode = DATALINK;

	while(1){
		if(iMode == DATALINK){
			printDebugLog("#### %s():%d ####\n",__func__,__LINE__);
			memset(chData, 0x0, sizeof(chData));
			uart_recv(pstUart, chData, 32);
			printDebugLog("DATA LINK recv data : %s\n",chData);
			//todo 데이터 가공
	
			write(iDataFd, chData, sizeof(chData));
		}else
			printDebugLog("#### %s():%d ####\n",__func__,__LINE__);

		if(iChangeMode == WAITING){
			iChangeMode = NONE;
			usleep(100);

			if(iChangeMode == CHANGE){
				pFd = popen("cat /tmp/.changeMode","r");
				if(pFd != NULL){
					memset(chMode, 0x0, sizeof(chMode));
					fgets(chMode, sizeof(chMode), pFd);
					pclose(pFd);
					if(strcmp(chMode, "DATALINK")==0)
						iMode = DATALINK;
					else if(strcmp(chMode, "MONO")==0)
						iMode = MONO;
					else if(strcmp(chMode, "IMG1")==0)
						iMode = IMG1;
					else if(strcmp(chMode, "IMG2")==0)
						iMode = IMG2;
					else if(strcmp(chMode, "MANUAL")==0)
						iMode = MANUAL;
					else
						printDebugLog("%s():%d....\n",__func__,__LINE__);
				}else{
					printDebugLog("/tmp/.changeMode not found...\n");
				}
			}
			iChangeMode = WORKING;
		}
	}
	uart_close(pstUart);
	free(pstUart);
	return NULL;
}
