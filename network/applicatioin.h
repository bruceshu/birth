




#ifndef APPLICATION_H
#define APPLICATION_H

#include "../log_system/log.h"


#define APP_EVENT_WILL_HTTP_OPEN  1 //AVAppHttpEvent
#define APP_EVENT_DID_HTTP_OPEN   2 //AVAppHttpEvent
#define APP_EVENT_WILL_HTTP_SEEK  3 //AVAppHttpEvent
#define APP_EVENT_DID_HTTP_SEEK   4 //AVAppHttpEvent

#define APP_EVENT_ASYNC_STATISTIC     0x11000 //AVAppAsyncStatistic
#define APP_EVENT_ASYNC_READ_SPEED    0x11001 //AVAppAsyncReadSpeed
#define APP_EVENT_IO_TRAFFIC          0x12204 //AVAppIOTraffic

#define APP_CTRL_WILL_TCP_OPEN   0x20001 //AVAppTcpIOControl
#define APP_CTRL_DID_TCP_OPEN    0x20002 //AVAppTcpIOControl

#define APP_CTRL_WILL_HTTP_OPEN  0x20003 //AVAppIOControl
#define APP_CTRL_WILL_LIVE_OPEN  0x20005 //AVAppIOControl
#define APP_CTRL_WILL_CONCAT_SEGMENT_OPEN 0x20007 //AVAppIOControl


typedef struct ApplicationContext {
    const AVClass *av_class;
    void *opaque;

    int (*func_on_app_event)(ApplicationContext *pstAppCtx, int event_type ,void *obj, size_t size);
}ApplicationContext;

typedef struct AppTcpIOControl {
    int  error;
    int  family;
    char ip[96];
    int  port;
    int  fd;
}AppTcpIOControl;

typedef struct AppIOTraffic
{
    void   *obj;
    int     bytes;
}AppIOTraffic;



#endif
