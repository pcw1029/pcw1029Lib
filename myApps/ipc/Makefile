CC=gcc
CFLAGS=-O3 -pipe -Wall -Wextra -pedantic
INCLUDE=-I.
EXAMPLE_DIR=examples
OBJ=Message.o TimerWrapper.o MessageQueueWrapper.o MessageQueueWrapperTest.o SharedMemoryWrapperTest.o SharedMemoryWrapper.o
BIN=messageQueueWrapperTest sharedMemoryWrapperTest server client

all: $(OBJ) $(BIN)

Message.o: Message.c
	$(CC) -o $@ $(CFLAGS) -c $^

TimerWrapper.o: TimerWrapper.c
	$(CC) -o $@ $(CFLAGS) -c $^

MessageQueueWrapper.o: MessageQueueWrapper.c
	$(CC) -o $@ $(CFLAGS) -c $^

MessageQueueWrapperTest.o: MessageQueueWrapperTest.c
	$(CC) -o $@ $(CFLAGS) -c $(INCLUDE) $^

messageQueueWrapperTest: MessageQueueWrapperTest.o MessageQueueWrapper.o TimerWrapper.o Message.o
	$(CC) -o $@ $(CFLAGS) $^ -lrt

client: client.o MessageQueueWrapper.o TimerWrapper.o Message.o
	$(CC) -o $@ $(CFLAGS) $^ -lrt

server: server.o MessageQueueWrapper.o TimerWrapper.o Message.o
	$(CC) -o $@ $(CFLAGS) $^ -lrt

SharedMemoryWrapper.o: SharedMemoryWrapper.c
	$(CC) -o $@ $(CFLAGS) -c $^

SharedMemoryWrapperTest.o: SharedMemoryWrapperTest.c
	$(CC) -o $@ $(CFLAGS) -c $(INCLUDE) $^

sharedMemoryWrapperTest: SharedMemoryWrapperTest.o SharedMemoryWrapper.o
	$(CC) -o $@ $(CFLAGS) $^ -lrt

clean: 
	-rm -f $(OBJ) $(BIN)
