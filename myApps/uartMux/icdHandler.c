#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<fcntl.h>
#include "tcpStruct.h"
#include "errexit.h"

int ICDHandler(unsigned char* puchWriteData, unsigned char* puchReadData, unsigned char uchIcdCode)
{
	int iRetSize=0;
	switch(uchIcdCode){
		case 0x01 ://tracker switch change 
			iRetSize = changeTrackerSW(puchWriteData, puchReadData);
			break;

		default :
			fprintf(stderr,"### %s():%d ###\n",__func__,__LINE__);
			break;
	}
	return iRetSize;
}


int changeTrackerSW(unsigned char* puchWriteData, unsigned char* puchReadData){
	TRACKER_SW *trackerSw;
	RESP_TRACKER_SW *r_trackerSw;
	int fp;
	unsigned char read_buff;

	trackerSw = (TRACKER_SW*)puchReadData;
	r_trackerSw = (RESP_TRACKER_SW*)puchWriteData;

	fp = open("/dev/memMap", O_RDWR | O_NONBLOCK);
	if(fp < 0){ 
		fprintf(stderr,"open error\n");
		r_trackerSw->uchResult=0x0;
	}   
	if(trackerSw->uchSw == 0x01){
		fprintf(stderr,"### SELECT S/W : %02X ###\n",trackerSw->uchSw);
		read_buff=0x01;
		system("echo 1 > /tmp/trackerSw");
	}else if(trackerSw->uchSw == 0x02){
		fprintf(stderr,"### SELECT S/W : %02X ###\n",trackerSw->uchSw);
		read_buff=0x02;
		system("echo 2 > /tmp/trackerSw");
	}else if(trackerSw->uchSw == 0x03){
		fprintf(stderr,"### SELECT S/W : %02X ###\n",trackerSw->uchSw);
		read_buff=0x03;
		system("echo 3 > /tmp/trackerSw");
	}else{
		fprintf(stderr,"### %s():%d ###\n",__func__,__LINE__);
		read_buff=0x00;
		system("echo 0 > /tmp/trackerSw");
	}
	write(fp, &read_buff, 0); 
	close(fp);
	r_trackerSw->uchResult=0x1;
	return 1;
}
