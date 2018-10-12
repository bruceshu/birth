/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-09-28
 * Description:
 
*********************************/


#ifndef HTTPAUTH_H
#define HTTPAUTH_H



typedef enum HTTPAuthType {
    HTTP_AUTH_NONE = 0,    /**< No authentication specified */
    HTTP_AUTH_BASIC,       /**< HTTP 1.0 Basic auth from RFC 1945  (also in RFC 2617) */
    HTTP_AUTH_DIGEST,      /**< HTTP 1.1 Digest auth from RFC 2617 */
} HTTPAuthType;

typedef struct DigestParams {
    char nonce[300];       /**< Server specified nonce */
    char algorithm[10];    /**< Server specified digest algorithm */
    char qop[30];          /**< Quality of protection, containing the one
                             *  that we've chosen to use, from the
                             *  alternatives that the server offered. */
    char opaque[300];      /**< A server-specified string that should be
                             *  included in authentication responses, not
                             *  included in the actual digest calculation. */
    char stale[10];        /**< The server indicated that the auth was ok,
                             * but needs to be redone with a new, non-stale
                             * nonce. */
    int nc;                /**< Nonce count, the number of earlier replies
                             *  where this particular nonce has been used. */
} DigestParams;

typedef struct HTTPAuthState {
    /**
     * The currently chosen auth type.
     */
    int auth_type;
    /**
     * Authentication realm
     */
    char realm[200];
    /**
     * The parameters specific to digest authentication.
     */
    DigestParams digest_params;
    /**
     * Auth ok, but needs to be resent with a new nonce.
     */
    int stale;
} HTTPAuthState;

#endif
