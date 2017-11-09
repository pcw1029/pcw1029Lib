#ifndef __TCPSTRUCT_H__
#define __TCPSTRUCT_H__

struct __attribute__((__packed__)) OPCode {
	unsigned char uchSrc;           	// 송신장비
	unsigned char uchDest;      	// 수신장비
	unsigned char uchIcdCode;          	// 수행명령
};

struct __attribute__((__packed__)) TcpMsgHead {
	struct OPCode opCode;  			// 수행 대상 명령
	unsigned int uiSizeofData; 	// 데이터 영역을 압축 및 암호 해제시 크기(최대:64KB)
};


typedef struct __attribute__((__packed__)) _TrackerSW{
	unsigned char uchSw;
} TRACKER_SW;

typedef struct __attribute__((__packed__)) _RespTrackerSW{
	unsigned char uchResult;
} RESP_TRACKER_SW;
#endif
