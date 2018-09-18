#include <fcntl.h>
#include <termios.h>
int OpenSerial( char *dev_name, int baud, int vtime, int vmin );
void Close_serial( int fd );

struct __attribute__((__packed__)) MsgHead {
	unsigned char uchStart;			// 메시지 시작(1Byte) 0x10
	unsigned char uchSubId;     	// 메시지 구분(1Byte) 0x01 시리얼
	unsigned char uchUnknown;    	// Don't care
	unsigned char uchDataSize; 	    // Data 크기 
}; //total 4Byte


