all: server serialThread client_
CC := /home/pcw1029/projects/myir_2017.08/output/host/usr/bin/arm-buildroot-linux-gnueabi-gcc
	  
INCS = -I./

server: server.o errexit.o gThread.o gQueue.o icdHandler.o MessageQueueWrapper.o Message.o TimerWrapper.o
	$(CC) -o $@ $^ -lpthread -lrt

serialThread: errexit.o gQueue.o serial.o serialThread.o MessageQueueWrapper.o Message.o TimerWrapper.o
	$(CC) -o $@ $^ -lpthread -lrt

client_: errexit.o gQueue.o serial.o client_.o MessageQueueWrapper.o Message.o TimerWrapper.o
	$(CC) -o $@ $^ -lpthread -lrt

%.o: %.c
	$(CC) -c -g -o $@ $<

clean:
	rm -f *.o server 
