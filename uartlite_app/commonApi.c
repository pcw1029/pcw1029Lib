#include<stdio.h>
#include<string.h>
#include "commonApi.h"
#include "uart.h"

void printHeaderInfo(char* buff, int iTcpHeadFlag)
{
	int i;
	short nTmp;
	struct MsgHead* pstMsgHead;
	char chIcdName[32];
	pstMsgHead = (struct MsgHead*)buff;
	fprintf(stderr,"*************** TCP HEADER (%s) ***************\n",(iTcpHeadFlag == RECV) ? "RECV" : "SEND");
	fprintf(stderr,"PACKET DATA : ");
	nTmp = buff[0] & 0x0000ff;
	fprintf(stderr,"%02X",nTmp);
	for(i=1; i<sizeof(struct MsgHead); i++){
		nTmp = buff[i] & 0x0000ff;
		fprintf(stderr,"|%02X",nTmp);
	}
	fprintf(stderr,"\n");

	fprintf(stderr,"       Msg Start : 0x%02X\n",pstMsgHead->uchStart);
	fprintf(stderr," 메시지 일련번호 : 0x%02X\n",pstMsgHead->uchSubId);
	fprintf(stderr,"     데이터 크기 : 0x%02X\n",pstMsgHead->uchDataSize); 	    // Data 크기 
	//fprintf(stderr,"*************************************************\n");
}

void printRawData(char* pchBuff, int iSize, int iTcpHeadFlag)
{
	int i;
	short nTmp;
	char *chTmp;

	chTmp = pchBuff;
	fprintf(stderr,"\n############# PRINT DATA(size:%03d) #############\n", iSize);
	fprintf(stderr,"******************** RAW DATA *******************\n");
	for(i=1; i<=iSize; i++){
		nTmp = chTmp[i-1] & 0x0000ff;
		if(i%16 == 0){
			if(i==iSize)
				fprintf(stderr,"%02X", nTmp);
			else
				fprintf(stderr,"%02X\n", nTmp);
		}else if(i%8 == 0){
			if(i==iSize)
				fprintf(stderr,"%02X", nTmp);
			else
				fprintf(stderr,"%02X - ", nTmp);
		}else{
			if(i==iSize)
				fprintf(stderr,"%02X",nTmp);
			else
				fprintf(stderr,"%02X|",nTmp);
		}
	}
	fprintf(stderr,"\n*************************************************\n");
	fprintf(stderr,"##################################################\n");
}


void printRawDataSeq(char* pchBuff, int iStart, int iEnd)
{
	int i;
	short nTmp;
	char *chTmp;
	chTmp = (char*)(pchBuff+(iStart-1));
	fprintf(stderr,"\n############# PRINT DATA(size:%03d) #############\n", iEnd-iStart);
	fprintf(stderr,"******************** RAW DATA *******************\n");
	for(i=iStart; i<=iEnd; i++){
		fprintf(stderr, "%02d - [%02X]\n",i-1, chTmp[i-1]);
	}
	fprintf(stderr,"*************************************************\n");
	fprintf(stderr,"##################################################\n");
}


