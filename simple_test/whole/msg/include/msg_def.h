#ifndef MSG_DEF_H
#define MSG_DEF_H

#include "../../include/type_def.h"

typedef struct Message
{
	uint32 what;
	uint32 para;
	struct Message *next;
}Message;

typedef struct MessageQueue
{
	Message *first_msg, *last_msg;
	uint32 nb_msgs;
}MessageQueue;

#endif 
