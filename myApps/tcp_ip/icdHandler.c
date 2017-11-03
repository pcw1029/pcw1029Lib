#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<fcntl.h>
#include "tcpStruct.h"
#include "errexit.h"

int ICDHandler(unsigned char* puchWriteData, unsigned char* puchReadData, int size)
{
	int iReturnSize=0;
	memcpy(puchWriteData, puchReadData, size);
	return size;
}

