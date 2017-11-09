#include "MessageQueueWrapper.h"
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

int main()
{
	int i;
	int key = 10;
	int qid;
	Message msg;
	char buffer[MESSAGE_SIZE];
	int ret;
	int mtype=1;

	/* Fill buffers with data: */
	memset(buffer, 0, MESSAGE_SIZE);
	/* Create message queues: */
	printf("making msq queue with key %d\n", key);
	qid = messageQueueCreate(key);
	printf("message queue returned qid %d\n", qid);
	if(qid == -1){ return -1; }

	clearMessage(&msg);
	printf("clearing message\n%s\n", messageToString(&msg));

//	strcpy(buffer,"abcdefg");
	setMessage(&msg, buffer, MESSAGE_SIZE, mtype);
	printf("Making message with buffer:\n%s\n\n", buffer);

	/* Send message: */
//	printf("Sending message from qid: %d to qid: %d\n", qid );
//	ret = messageQueueSend(qid, &msg);
//	printf("##### Sending message returned %d\n", ret);
//	printf("Sent message: %s\n", messageToString(&msg));

	/* Receive message: */
	printf("###### Receiving message from qid: %d\n", qid);
	ret = messageQueueReceive(qid, &msg, mtype);
	printf("Receiving message returned %d\n", ret);
	printf("Received message: %s\n", messageToString(&msg));

	/* Delete queues: */
	sleep(10);
	printf("Deleting qid: %d returned %d\n", qid , messageQueueDelete(qid));
	
	return 0;
}
