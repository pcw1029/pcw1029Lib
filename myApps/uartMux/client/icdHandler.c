#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<fcntl.h>
#include "tcpStruct.h"
#include "errexit.h"

int ICDHandler(unsigned char* puchWriteData, unsigned char uchIcdCode)
{
	int iRetSize=0;
	switch(uchIcdCode){
		case 0x01://tracker switch change 
			iRetSize = changeTrackerSW(puchWriteData);
			break;

		default :
			fprintf(stderr,"### %s():%d ###\n",__func__,__LINE__);
			break;
	}
	return iRetSize;
}


int changeTrackerSW(unsigned char* puchWriteData){
	int menu;
	TRACKER_SW *trackerSw;
	trackerSw = (TRACKER_SW*)puchWriteData;

	fprintf(stderr,"0. nothing..\n");
	fprintf(stderr,"1. joystick\n");
	fprintf(stderr,"2. Video\n");
	fprintf(stderr,"3. Mono pulse\n");
	scanf("%d",&menu);

	if(menu == 1){
		trackerSw->uchSw=0x01;
	}else if(menu == 2){
		trackerSw->uchSw=0x02;
	}else if(menu == 3){
		trackerSw->uchSw=0x03;
	}else{
		trackerSw->uchSw=0x00;
	}
	return sizeof(TRACKER_SW);
}
