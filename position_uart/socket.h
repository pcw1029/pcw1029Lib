#ifndef __SOCKET_H__
#define __SOCKET_H__


#define REQUEST_DATALINK "/tmp/dataLink.domain"
#define REQUEST_UART "/tmp/uart.domain"

enum {NONE, WORKING, WAITING, CHANGE};
enum {DATALINK, MONO, IMG1, IMG2, MANUAL };
			
int init_listen_server(const char *domain_name);
int accept_client(int lsn_fd);
int connect_server(const char *domain_name);

#endif

