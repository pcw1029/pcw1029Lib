all: server ctag client

INCS = -I./

server: server.o errexit.o gThread.o gQueue.o commonApi.o uart.o
	gcc -DSW_DBUG_LOG -o $@ $^ -lpthread

client: client.o uart.o
	gcc -DSW_DBUG_LOG -o $@ $^ -lpthread

ctag:
	ctags -R

%.o: %.c
	gcc -c -g -o $@ $<

clean:
	rm -f *.o server client tags
