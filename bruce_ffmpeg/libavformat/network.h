/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-09-28
 * Description:
 
*********************************/


#ifndef NETWORK_H
#define NETWORK_H

#include "url.h"

#ifndef INET_ADDRSTRLEN
#define INET_ADDRSTRLEN 16
#endif

#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN INET_ADDRSTRLEN
#endif


/* getaddrinfo constants */
#ifndef EAI_AGAIN
#define EAI_AGAIN 2
#endif
#ifndef EAI_BADFLAGS
#define EAI_BADFLAGS 3
#endif
#ifndef EAI_FAIL
#define EAI_FAIL 4
#endif
#ifndef EAI_FAMILY
#define EAI_FAMILY 5
#endif
#ifndef EAI_MEMORY
#define EAI_MEMORY 6
#endif
#ifndef EAI_NODATA
#define EAI_NODATA 7
#endif
#ifndef EAI_NONAME
#define EAI_NONAME 8
#endif
#ifndef EAI_SERVICE
#define EAI_SERVICE 9
#endif
#ifndef EAI_SOCKTYPE
#define EAI_SOCKTYPE 10
#endif
#ifndef AI_PASSIVE
#define AI_PASSIVE 1
#endif
#ifndef AI_CANONNAME
#define AI_CANONNAME 2
#endif
#ifndef AI_NUMERICHOST
#define AI_NUMERICHOST 4
#endif
#ifndef NI_NOFQDN
#define NI_NOFQDN 1
#endif
#ifndef NI_NUMERICHOST
#define NI_NUMERICHOST 2
#endif
#ifndef NI_NAMERQD
#define NI_NAMERQD 4
#endif
#ifndef NI_NUMERICSERV
#define NI_NUMERICSERV 8
#endif
#ifndef NI_DGRAM
#define NI_DGRAM 16
#endif

#if HAVE_WINSOCK2_H
#include <winsock2.h>
#include <ws2tcpip.h>

#ifndef EPROTONOSUPPORT
#define EPROTONOSUPPORT WSAEPROTONOSUPPORT
#endif
#ifndef ETIMEDOUT
#define ETIMEDOUT       WSAETIMEDOUT
#endif
#ifndef ECONNREFUSED
#define ECONNREFUSED    WSAECONNREFUSED
#endif
#ifndef EINPROGRESS
#define EINPROGRESS     WSAEINPROGRESS
#endif

#define getsockopt(a, b, c, d, e) getsockopt(a, b, c, (char*) d, e)
#define setsockopt(a, b, c, d, e) setsockopt(a, b, c, (const char*) d, e)

int ff_neterrno(void);

#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>

#define ff_neterrno() AVERROR(errno)
#endif /* HAVE_WINSOCK2_H */

#define POLLING_TIME 100 /// Time in milliseconds between interrupt check

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

int ff_network_init(void);
int ff_tls_init(void);
void ff_tls_deinit(void);
void ff_network_close(void);

void ff_log_net_error(void *ctx, int level, const char* prefix);
int ff_socket(int af, int type, int proto);
int ff_listen(int fd, const struct sockaddr *addr, socklen_t addrlen);
int ff_accept(int fd, int timeout, URLContext *h);
int ff_listen_bind(int fd, const struct sockaddr *addr, socklen_t addrlen, int timeout, URLContext *h);
int ff_network_wait_fd_timeout(int fd, int write, int64_t timeout, AVIOInterruptCB *int_cb);
int ff_network_wait_fd(int fd, int write);





#endif
