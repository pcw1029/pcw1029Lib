all: ctag uart_test userInterface

INCS = -I./

uart_test:  uartImg.o uartForMux.o socket.o errexit.o gThread.o gQueue.o commonApi.o
	gcc -DSW_DBUG_LOG -o $@ $^ -lpthread

userInterface: userInterface.c socket.o errexit.o
	gcc -DSW_DBUG_LOG -o $@ $^ 

ctag:
	ctags -R

%.o: %.c
	gcc -c -g -o $@ $<

clean:
	rm -f *.o uart_test tags userInterface
