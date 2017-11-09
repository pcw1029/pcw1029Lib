all: client

INCS = -I./ 


client: client.o errexit.o gThread.o gQueue.o icdHandler.o 
	gcc -o $@ $^ -lpthread -lrt

%.o: %.c
	gcc -c -g -o $@ $<

clean:
	rm -f *.o server client
