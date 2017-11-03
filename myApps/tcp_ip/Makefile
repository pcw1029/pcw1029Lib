all: server client

INCS = -I./

server: server.o errexit.o gThread.o gQueue.o icdHandler.o
	gcc -o $@ $^ -lpthread

client: client.o errexit.o
	gcc -o $@ $^

%.o: %.c
	gcc -c -g -o $@ $<

clean:
	rm -f *.o server client
