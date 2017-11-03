#ifndef __TCPSTRUCT_H__
#define __TCPSTRUCT_H__

struct __attribute__((__packed__)) OPCode {
	unsigned char uchSource;           	// 송신장비
	unsigned char uchDestination;      	// 수신장비
	unsigned char uchIcdCode;          	// 수행명령
	unsigned char uchClassCode : 4;    	// 메시지 우선순위
	unsigned char uchOperator: 4;	 	// 운용자 구분
};

struct __attribute__((__packed__)) TcpMsgHead {
	struct OPCode opCode;  			// 수행 대상 명령
	unsigned short unSizeofData; 	// 데이터 영역을 압축 및 암호 해제시 크기(최대:64KB)
	unsigned short unOperatorId;  	// POSN에 로그인한 운용자 ID
						   			// 지상임무연동기가 항공에서 수신한 메시지를 어느 POSN으로 전송할지 구분하는 필드
};

#endif
