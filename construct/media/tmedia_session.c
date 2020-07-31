
#include "tmedia_session.h"




#define TSDP_HEADER_O_SESS_ID_DEFAULT		123456
#define TSDP_HEADER_O_SESS_VERSION_DEFAULT	678901

//=================================================================================================
//	Media Session Manager object definition
//
static tsk_object_t *tmedia_session_mgr_ctor (tsk_object_t *self, va_list *app)
{
    tmedia_session_mgr_t *mgr = self;
    if (mgr)
    {
        mgr->sessions = tsk_list_create ();

        mgr->sdp.lo_ver = TSDP_HEADER_O_SESS_VERSION_DEFAULT;
        mgr->sdp.ro_ver = -1;

        mgr->bl = tmedia_defaults_get_bl ();
        tsk_safeobj_init (mgr);
    }
    return self;
}

static tsk_object_t *tmedia_session_mgr_dtor (tsk_object_t *self)
{
    tmedia_session_mgr_t *mgr = self;
    if (mgr)
    {
        TSK_OBJECT_SAFE_FREE (mgr->sessions);

        TSK_OBJECT_SAFE_FREE (mgr->sdp.lo);
        TSK_OBJECT_SAFE_FREE (mgr->sdp.ro);

        TSK_OBJECT_SAFE_FREE (mgr->params);

        TSK_FREE (mgr->public_addr);


        TSK_FREE (mgr->addr);

        tsk_safeobj_deinit (mgr);
    }

    return self;
}

static const tsk_object_def_t tmedia_session_mgr_def_s = {
    sizeof (tmedia_session_mgr_t),
    tmedia_session_mgr_ctor,
    tmedia_session_mgr_dtor,
    tsk_null,
};