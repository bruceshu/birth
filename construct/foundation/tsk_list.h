


#ifndef TSK_LIST_H
#define TSK_LIST_H

#include "tsk_object.h"
#include "tsk_mutex.h"


typedef struct tsk_list_item_s
{
	TSK_DECLARE_OBJECT;
	void* data; /**< Opaque data. <b>Must</b> be a @ref _Page_TinySAK_AnsiC_Object_Programming "well-defined" object. */
	struct tsk_list_item_s* next; /**< Next item. */
} tsk_list_item_t;

typedef struct tsk_list_s
{
	TSK_DECLARE_OBJECT;
	
	tsk_list_item_t* head; /**< The head of the linked list. */
	tsk_list_item_t* tail; /**< The tail of the linked list. */
	tsk_mutex_handle_t* mutex; /**< on-demand mutex. */
} tsk_list_t;





#endif