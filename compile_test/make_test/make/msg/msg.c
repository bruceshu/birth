#include "include/msg_def.h"
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

int msg_create(Message **msg)
{
	if (msg == NULL)
	{
		return ERR;
	}

	Message *msg1;
	msg1 = malloc(sizeof(Message));
	if (!msg1)
	{
		return ERR;
	}

	msg1->what = 1;
	msg1->para = 0;
	msg1->next = NULL;

	*msg = msg1;
	
	return OK;
}

int msg_queue_init(MessageQueue **msgQueue)
{
	if (msgQueue == NULL)
	{
		return ERR;
	}

	MessageQueue *msg_queue1;
	msg_queue1 = malloc(sizeof(MessageQueue));
	if (!msg_queue1)
	{
		return ERR;
	}
	
	memset(msg_queue1, 0, sizeof(MessageQueue));
	*msgQueue = msg_queue1;	
	
	return OK;
}

int msg_queue_put(MessageQueue *msgQueue, Message *msg)
{
	Message *msg1;

	if (!msgQueue || !msg)
	{
		return ERR;
	}

	msg1 = malloc(sizeof(Message));
	if (!msg1)
	{
		return ERR;
	}

	printf("结构体赋值之前打印\n");
	*msg1 = *msg;
	printf("结构体赋值之后打印\n");
	
	if (!msgQueue->first_msg)
	{
		msgQueue->first_msg = msg1;
		msgQueue->last_msg = msgQueue->first_msg;
		msgQueue->nb_msgs++;
	}
	else
	{
		msgQueue->first_msg->next = msg1;
		msgQueue->last_msg = msg1;
		msgQueue->nb_msgs++;
	}

	return OK;
}
