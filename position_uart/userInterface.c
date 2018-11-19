#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include "socket.h"
#include "errexit.h"

#define STRING_LENGTH 64

int strsplit(char* str, char sep, char** part, int maxparts){
	int parts=0;
	char* p1 = str; 
	char* p2;

	part[parts++] = p1;

	while((p2=strchr(p1,sep)) && (parts<maxparts)){
		*p2=0;
		p1=p2+1;
		part[parts++]=p1;
	}    
	return parts;
}

int changeInt(unsigned char ch)
{
	if(ch == 'A' || ch == 'a'){
		return 10;
	}else if(ch == 'B' || ch == 'b'){
		return 11;
	}else if(ch == 'C' || ch == 'c'){
		return 12;
	}else if(ch == 'D' || ch == 'd'){
		return 13;
	}else if(ch == 'E' || ch == 'e'){
		return 14;
	}else if(ch == 'F' || ch == 'f'){
		return 15;
	}else{
		return ch-'0';
	}
}

unsigned char atox(char* str, int size)
{
	unsigned char uchRetVal=0;
	int val;
	if(size != 2){
		fprintf(stderr,"error string size : %d\n",size);
		exit(1);
	}
	val = (changeInt(str[0]) * 16) + changeInt(str[1]);
	uchRetVal = val;
	return uchRetVal;
}

int main(int argc, char* argv[] )
{
	unsigned char uchData[STRING_LENGTH];
	unsigned char uchStrData[STRING_LENGTH];
	char* strParts[STRING_LENGTH/2];
	int iSplitCnt;
	int i;
	int iWriteSize;
	int iDataLinkListenFd;
	int iDataLinkAcceptFd;
	unsigned char chEndStr[]="FF:FF:FF:FF";

	iDataLinkListenFd = init_listen_server(REQUEST_DATALINK);
   	iDataLinkAcceptFd = accept_client(iDataLinkListenFd);
	//다른 프로세스들과 파일의 시리얼 모드를 보고 동작한다.
	while(1){
		memset(uchStrData, 0x0, sizeof(uchStrData));
		fprintf(stderr,"INPUT DATA (ex 01:1A:3d) end=FF:FF:FF:FF > ");
		scanf("%s", uchStrData);
		if(strlen(uchStrData)==11 && strcmp(uchStrData, chEndStr)==0){
			fprintf(stderr,"프로그램 종료\n");
			break;
		}
		iSplitCnt = strsplit(uchStrData, ':', strParts, 32);
		for(i=0; i<iSplitCnt; i++){
			if((i+1) % 16 == 0)
				fprintf(stderr,"%02X\n", atox(strParts[i], strlen(strParts[i])));
			else if((i+1) % 8 == 0)
				fprintf(stderr,"%02X\t", atox(strParts[i], strlen(strParts[i])));
			else 
				fprintf(stderr,"%02X:", atox(strParts[i], strlen(strParts[i])));
			uchData[i] = atox(strParts[i], strlen(strParts[i]));
		}
		iWriteSize = write(iDataLinkAcceptFd, uchData, iSplitCnt);
		fprintf(stderr,"\nwrite size : %d\n",iWriteSize);
		
	}
	close(iDataLinkListenFd);
	return 0;
}
