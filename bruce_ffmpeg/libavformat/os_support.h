/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-10-09
 * Description:
 
*********************************/


#ifndef OS_SUPPORT_C
#define OS_SUPPORT_C

#include <unistd.h>
#include "config.h"

/* events & revents */
#define POLLIN     0x0001  /* any readable data available */
#define POLLOUT    0x0002  /* file descriptor is writeable */
#define POLLRDNORM POLLIN
#define POLLWRNORM POLLOUT
#define POLLRDBAND 0x0008  /* priority readable data */
#define POLLWRBAND 0x0010  /* priority data can be written */
#define POLLPRI    0x0020  /* high priority readable data */

#if defined(__OS2__)
#define SHUT_RD 0
#define SHUT_WR 1
#define SHUT_RDWR 2
#endif

#if defined(_WIN32)
#define SHUT_RD SD_RECEIVE
#define SHUT_WR SD_SEND
#define SHUT_RDWR SD_BOTH
#ifndef S_IRUSR
#define S_IRUSR S_IREAD
#endif
#ifndef S_IWUSR
#define S_IWUSR S_IWRITE
#endif
#endif

#if !HAVE_CLOSESOCKET
#define closesocket close
#endif

#if 0 //后续需要时放开
static inline int is_dos_path(const char *path)
{
#if HAVE_DOS_PATHS
    if (path[0] && path[1] == ':')
        return 1;
#endif
    return 0;
}
#endif

int ff_socket_nonblock(int socket, int enable);

#endif
