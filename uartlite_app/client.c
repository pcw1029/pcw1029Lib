// system types
#include <sys/types.h>    
// system call
#include <unistd.h>       
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include "uart.h"

#define TOTAL_SIZE 54
#define WRITE_BUFF_SIZE 1024
// Main function
int main(int argc,char* argv[])
{
	int iDataLen;
	int iRemainderLen;
	int iWriteSize;
	int iStart;
	char chTmpSendBuff[TOTAL_SIZE];
	char chSendBuff[WRITE_BUFF_SIZE]={0x10,0x01,0x40,0x0C,0x40,0x01,0x42,0xFF,0xFE,0x34,0x0D,0x43,0xFF,0xFA,0x3F,0x43,0xDC,0x10,0x01,0x40,0x0C,0x40,0x01,0x42,0xFF,0xFE,0x31,0xD8,0x43,0xFF,0xFA,0x3D,0x7A,0xD9,0x10,0x01,0x40,0x0C,0x40,0x00,0x42,0x00,0x00,0x00,0x00,0x43,0x00,0x00,0x00,0x00,0x22};
	int i;
	int iUartFd;

//	iUartFd = OpenSerial("/dev/ttyUSB0",1,5,0);
	
	iRemainderLen = 0;
	srand((unsigned)time(NULL)+(unsigned)getpid());
//    while(1)
	for(i=0; i<100; i++)
    {
		if(iRemainderLen <= 1){
			iRemainderLen = TOTAL_SIZE+1;
			iStart=0;
		}
		iDataLen=0;
		while(iDataLen == 0)
			iDataLen = rand()%iRemainderLen;
		fprintf(stderr,"rand size : %d\n",iDataLen);
		memcpy(chTmpSendBuff, chSendBuff+iStart, iDataLen);
		iWriteSize = iDataLen;
//		iWriteSize = write(iUartFd, chSendBuff, iDataLen);
//		tcflush(iUartFd, TCOFLUSH);
		iRemainderLen = iRemainderLen - iWriteSize;
		iStart += iWriteSize;
		fprintf(stderr,"write size : %d, remainder size : %d\n",iWriteSize, iRemainderLen);
		sleep(1);
    }
    return 0;
}

