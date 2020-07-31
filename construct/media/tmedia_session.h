



#ifndef TMEDIA_SESSION_H
#define TMEDIA_SESSION_H

#include "tsk_object.h"
#include "tsk_common.h"


#define SDP 0

/** Session manager */
typedef struct tmedia_session_mgr_s
{
    TSK_DECLARE_OBJECT;

    //! whether we are the offerer or not
    tsk_bool_t offerer;
    //! local IP address or FQDN
    char *addr;
    //! public IP address or FQDN
    char *public_addr;
    //! whether the @a addr is IPv6 or not (useful when @addr is a FQDN)
    tsk_bool_t ipv6;

#if SDP
    struct
    {
        uint32_t lo_ver;
        tsdp_message_t *lo;

        int32_t ro_ver;
        tsdp_message_t *ro;
    } sdp;
#endif

    tsk_bool_t started;
    tsk_bool_t ro_provisional;
    //! session type
    tmedia_type_t type;
    //! bandwidth level
    tmedia_bandwidth_level_t bl;

    /* session error callback */
    struct
    {
        tmedia_session_onerror_cb_f fun;
        const void *usrdata;
    } onerror_cb;

	/* rfc5168 callback */
	struct {
		tmedia_session_rfc5168_cb_f fun;
		const void* usrdata;
	} rfc5168_cb;
    //! List of all sessions
    tmedia_sessions_L_t *sessions;

    //! User's parameters used to confugure plugins
    tmedia_params_L_t *params;
    
    SessionStartFailCallback_t session_start_fail_callback;
    
    TSK_DECLARE_SAFEOBJ;
} tmedia_session_mgr_t;







#endif