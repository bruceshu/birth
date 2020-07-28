/*
 bruce 2020-7-28
 */

#include "tsk_object.h"

#if defined(DEBUG)
    #define TSK_DEBUG_OBJECTS	1
    static int tsk_objects_count = 0;
#else 
    #define TSK_DEBUG_OBJECTS	0
#endif


tsk_object_t *tsk_object_new(const tsk_object_def_t *obj_def, ...)
{
    tsk_object_t *new_obj = tsk_calloc(1, obj_def->size);
    if (new_obj)
    {
        memset(new_obj, 0, obj_def->size);
        *((const tsk_object_def_t **) new_obj) = obj_def;
        ((tsk_object_header_t*)new_obj)->ref_count = 1;
        ((tsk_object_header_t*)new_obj)->lock = 0;
        if (obj_def->constructor) {
            va_list ap;
            tsk_object_t * new_obj_ = new_obj;
            va_start(ap, obj_def);
            new_obj = obj_def->constructor(new_obj, &ap);
            va_end(ap);

            if (!new_obj) {
                if (obj_def->destructor) {
                    obj_def->destructor(new_obj_);
                }
                tsk_free(&new_obj_);
            }
#if TSK_DEBUG_OBJECTS
		printf("N∞ objects:%d\n", ++tsk_objects_count);
#endif
        } else {
            printf("No constructor found.\n");
        }
    } else {
        printf("Failed to create new tsk_object.\n");
    }
}

tsk_size_t tsk_object_sizeof(const tsk_object_t *self)
{
	const tsk_object_def_t **objdef = (const tsk_object_def_t **)self;
	if (objdef && *objdef) {
		return (*objdef)->size;
	}
	else {
		TSK_DEBUG_ERROR("NULL object definition.");
		return 0;
	}
}