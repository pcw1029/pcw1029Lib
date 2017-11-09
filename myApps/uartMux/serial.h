#ifndef __SERIAL_H__
#define __SERIAL_H__
int OpenSerial( char *dev_name, int baud, int vtime, int vmin );
void Close_serial( int fd );
#endif
