#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <stdlib.h>
#include <fcntl.h>

int OpenSerial( char *dev_name, int baud, int vtime, int vmin ) 
{ 
	int fd; 
	struct termios newtio; 

	// 시리얼포트를 연다. 
	fd = open( dev_name, O_RDWR | O_NOCTTY ); 
	if ( fd < 0 ) 
	{ 
		// 파일 열기 실패
		fprintf(stderr,"Device OPEN FAIL %s\n", dev_name ); 
		return -1; 
	} 
	// 시리얼 포트 환경을 설정한다. 
	memset(&newtio, 0, sizeof(newtio)); 
	newtio.c_iflag = IGNPAR; // non-parity 
	newtio.c_oflag = 0; 
	newtio.c_cflag = CS8 | CLOCAL | CREAD; // NO-rts/cts Serial 프로그램 예제
	newtio.c_lflag = ~(ICANON); 
	switch( baud ) 
	{ 
		case 115200 : newtio.c_cflag |= B115200; break; 
		case 57600 : newtio.c_cflag |= B57600; break; 
		case 38400 : newtio.c_cflag |= B38400; break; 
		case 19200 : newtio.c_cflag |= B19200; break; 
		case 9600 : newtio.c_cflag |= B9600; break;
		default : newtio.c_cflag |= B115200; break; 
	} 
	//set input mode (non-canonical, no echo,.....) 
	newtio.c_lflag = 0; 
	newtio.c_cc[VTIME] = vtime; // timeout 0.1초 단위
	newtio.c_cc[VMIN] = vmin; // 최소 n 문자 받을 때까진 대기
	tcflush ( fd, TCIOFLUSH); 
	tcsetattr( fd, TCSANOW, &newtio ); 
	return fd; 
} 

void Close_serial( int fd ) 
{ 
	close( fd ); 
}
