#include "include/msg_def.h"
#include <stdlib.h>

int msg_model_test()
{
	int ret;
	Message *msg;
	MessageQueue *msgQueue;

	msg_queue_init(&msgQueue);
	msg_create(&msg);
	
	msg_queue_put(msgQueue, msg);
	free(msg);

	return ret;
}
