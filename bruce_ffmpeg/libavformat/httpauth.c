/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-09-28
 * Description:
 
*********************************/


#if 0 //后续需要时再放开
char *http_auth_create_response(HTTPAuthState *state, const char *auth, const char *path, const char *method)
{
    char *authstr = NULL;

    /* Clear the stale flag, we assume the auth is ok now. It is reset
     * by the server headers if there's a new issue. */
    state->stale = 0;
    if (!auth || !strchr(auth, ':'))
        return NULL;

    if (state->auth_type == HTTP_AUTH_BASIC) {
        int auth_b64_len, len;
        char *ptr, *decoded_auth = url_decode(auth);

        if (!decoded_auth)
            return NULL;

        auth_b64_len = AV_BASE64_SIZE(strlen(decoded_auth));
        len = auth_b64_len + 30;

        authstr = av_malloc(len);
        if (!authstr) {
            av_free(decoded_auth);
            return NULL;
        }

        snprintf(authstr, len, "Authorization: Basic ");
        ptr = authstr + strlen(authstr);
        av_base64_encode(ptr, auth_b64_len, decoded_auth, strlen(decoded_auth));
        av_strlcat(ptr, "\r\n", len - (ptr - authstr));
        av_free(decoded_auth);
    } else if (state->auth_type == HTTP_AUTH_DIGEST) {
        char *username = url_decode(auth), *password;

        if (!username)
            return NULL;

        if ((password = strchr(username, ':'))) {
            *password++ = 0;
            authstr = make_digest_auth(state, username, password, path, method);
        }
        av_free(username);
    }
    return authstr;
}


char *url_decode(const char *url)
{
    int s = 0, d = 0, url_len = 0;
    char c;
    char *dest = NULL;

    if (!url)
        return NULL;

    url_len = strlen(url) + 1;
    dest = av_malloc(url_len);

    if (!dest)
        return NULL;

    while (s < url_len) {
        c = url[s++];

        if (c == '%' && s + 2 < url_len) {
            char c2 = url[s++];
            char c3 = url[s++];
            if (av_isxdigit(c2) && av_isxdigit(c3)) {
                c2 = av_tolower(c2);
                c3 = av_tolower(c3);

                if (c2 <= '9')
                    c2 = c2 - '0';
                else
                    c2 = c2 - 'a' + 10;

                if (c3 <= '9')
                    c3 = c3 - '0';
                else
                    c3 = c3 - 'a' + 10;

                dest[d++] = 16 * c2 + c3;

            } else { /* %zz or something other invalid */
                dest[d++] = c;
                dest[d++] = c2;
                dest[d++] = c3;
            }
        } else if (c == '+') {
            dest[d++] = ' ';
        } else {
            dest[d++] = c;
        }

    }

    return dest;
}


static char *make_digest_auth(HTTPAuthState *state, const char *username, const char *password, 
                                const char *uri,  const char *method)
{
    DigestParams *digest = &state->digest_params;
    int len;
    uint32_t cnonce_buf[2];
    char cnonce[17];
    char nc[9];
    int i;
    char A1hash[33], A2hash[33], response[33];
    AVMD5 *md5ctx;
    uint8_t hash[16];
    char *authstr;

    digest->nc++;
    snprintf(nc, sizeof(nc), "%08x", digest->nc);

    /* Generate a client nonce. */
    for (i = 0; i < 2; i++)
        cnonce_buf[i] = get_random_seed();
    data_to_hex(cnonce, (const uint8_t*) cnonce_buf, sizeof(cnonce_buf), 1);
    cnonce[2*sizeof(cnonce_buf)] = 0;

    md5ctx = av_md5_alloc();
    if (!md5ctx)
        return NULL;

    av_md5_init(md5ctx);
    update_md5_strings(md5ctx, username, ":", state->realm, ":", password, NULL);
    av_md5_final(md5ctx, hash);
    ff_data_to_hex(A1hash, hash, 16, 1);
    A1hash[32] = 0;

    if (!strcmp(digest->algorithm, "") || !strcmp(digest->algorithm, "MD5")) {
    } else if (!strcmp(digest->algorithm, "MD5-sess")) {
        av_md5_init(md5ctx);
        update_md5_strings(md5ctx, A1hash, ":", digest->nonce, ":", cnonce, NULL);
        av_md5_final(md5ctx, hash);
        ff_data_to_hex(A1hash, hash, 16, 1);
        A1hash[32] = 0;
    } else {
        /* Unsupported algorithm */
        av_free(md5ctx);
        return NULL;
    }

    av_md5_init(md5ctx);
    update_md5_strings(md5ctx, method, ":", uri, NULL);
    av_md5_final(md5ctx, hash);
    ff_data_to_hex(A2hash, hash, 16, 1);
    A2hash[32] = 0;

    av_md5_init(md5ctx);
    update_md5_strings(md5ctx, A1hash, ":", digest->nonce, NULL);
    if (!strcmp(digest->qop, "auth") || !strcmp(digest->qop, "auth-int")) {
        update_md5_strings(md5ctx, ":", nc, ":", cnonce, ":", digest->qop, NULL);
    }
    update_md5_strings(md5ctx, ":", A2hash, NULL);
    av_md5_final(md5ctx, hash);
    ff_data_to_hex(response, hash, 16, 1);
    response[32] = 0;

    av_free(md5ctx);

    if (!strcmp(digest->qop, "") || !strcmp(digest->qop, "auth")) {
    } else if (!strcmp(digest->qop, "auth-int")) {
        /* qop=auth-int not supported */
        return NULL;
    } else {
        /* Unsupported qop value. */
        return NULL;
    }

    len = strlen(username) + strlen(state->realm) + strlen(digest->nonce) +
              strlen(uri) + strlen(response) + strlen(digest->algorithm) +
              strlen(digest->opaque) + strlen(digest->qop) + strlen(cnonce) +
              strlen(nc) + 150;

    authstr = av_malloc(len);
    if (!authstr)
        return NULL;
    snprintf(authstr, len, "Authorization: Digest ");

    /* TODO: Escape the quoted strings properly. */
    av_strlcatf(authstr, len, "username=\"%s\"",   username);
    av_strlcatf(authstr, len, ", realm=\"%s\"",     state->realm);
    av_strlcatf(authstr, len, ", nonce=\"%s\"",     digest->nonce);
    av_strlcatf(authstr, len, ", uri=\"%s\"",       uri);
    av_strlcatf(authstr, len, ", response=\"%s\"",  response);

    // we are violating the RFC and use "" because all others seem to do that too.
    if (digest->algorithm[0])
        av_strlcatf(authstr, len, ", algorithm=\"%s\"",  digest->algorithm);

    if (digest->opaque[0])
        av_strlcatf(authstr, len, ", opaque=\"%s\"", digest->opaque);
    if (digest->qop[0]) {
        av_strlcatf(authstr, len, ", qop=\"%s\"",    digest->qop);
        av_strlcatf(authstr, len, ", cnonce=\"%s\"", cnonce);
        av_strlcatf(authstr, len, ", nc=%s",         nc);
    }

    av_strlcatf(authstr, len, "\r\n");

    return authstr;
}


char *http_auth_create_response(HTTPAuthState *state, const char *auth, const char *path, const char *method)
{
    char *authstr = NULL;

    /* Clear the stale flag, we assume the auth is ok now. It is reset by the server headers if there's a new issue. */
    state->stale = 0;
    if (!auth || !strchr(auth, ':'))
        return NULL;

    if (state->auth_type == HTTP_AUTH_BASIC) {
        int auth_b64_len, len;
        char *ptr, *decoded_auth = url_decode(auth);

        if (!decoded_auth)
            return NULL;

        auth_b64_len = AV_BASE64_SIZE(strlen(decoded_auth));
        len = auth_b64_len + 30;

        authstr = av_malloc(len);
        if (!authstr) {
            av_free(decoded_auth);
            return NULL;
        }

        snprintf(authstr, len, "Authorization: Basic ");
        ptr = authstr + strlen(authstr);
        av_base64_encode(ptr, auth_b64_len, decoded_auth, strlen(decoded_auth));
        av_strlcat(ptr, "\r\n", len - (ptr - authstr));
        av_free(decoded_auth);
    } 
    else if (state->auth_type == HTTP_AUTH_DIGEST) {
        char *username = url_decode(auth), *password;

        if (!username)
            return NULL;

        if ((password = strchr(username, ':'))) {
            *password++ = 0;
            authstr = make_digest_auth(state, username, password, path, method);
        }
        av_free(username);
    }
    return authstr;
}

void ff_http_auth_handle_header(HTTPAuthState *state, const char *key, const char *value)
{
    if (!av_strcasecmp(key, "WWW-Authenticate") || !av_strcasecmp(key, "Proxy-Authenticate")) {
        const char *p;
        if (av_stristart(value, "Basic ", &p) &&
            state->auth_type <= HTTP_AUTH_BASIC) {
            state->auth_type = HTTP_AUTH_BASIC;
            state->realm[0] = 0;
            state->stale = 0;
            ff_parse_key_value(p, (ff_parse_key_val_cb) handle_basic_params,
                               state);
        } else if (av_stristart(value, "Digest ", &p) &&
                   state->auth_type <= HTTP_AUTH_DIGEST) {
            state->auth_type = HTTP_AUTH_DIGEST;
            memset(&state->digest_params, 0, sizeof(DigestParams));
            state->realm[0] = 0;
            state->stale = 0;
            ff_parse_key_value(p, (ff_parse_key_val_cb) handle_digest_params,
                               state);
            choose_qop(state->digest_params.qop,
                       sizeof(state->digest_params.qop));
            if (!av_strcasecmp(state->digest_params.stale, "true"))
                state->stale = 1;
        }
    } else if (!av_strcasecmp(key, "Authentication-Info")) {
        ff_parse_key_value(value, (ff_parse_key_val_cb) handle_digest_update,
                           state);
    }
}
#endif
