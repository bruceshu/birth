

void application_on_io_traffic(ApplicationContext *pstAppCtx, AppIOTraffic *event)
{
    if (pstAppCtx && pstAppCtx->func_on_app_event)
        pstAppCtx->func_on_app_event(pstAppCtx, APP_EVENT_IO_TRAFFIC, (void *)event, sizeof(AppIOTraffic));
}

int application_on_tcp_will_open(ApplicationContext *pstAppCtx)
{
    if (pstAppCtx && pstAppCtx->func_on_app_event) {
        AppTcpIOControl control = {0};
        return pstAppCtx->func_on_app_event(pstAppCtx, APP_CTRL_WILL_TCP_OPEN, (void *)&control, sizeof(AppTcpIOControl));
    }
    return 0;
}

int application_on_tcp_did_open(ApplicationContext *pstAppCtx, int error, int fd, AppTcpIOControl *pstAppTcpIOCtl)
{
    struct sockaddr_storage so_stg;
    socklen_t so_len = sizeof(so_stg);
    
    int       ret = 0;
    int       so_family;
    char      *so_ip_name = pstAppTcpIOCtl->ip;

    if (!pstAppCtx || !pstAppCtx->func_on_app_event || fd <= 0)
        return 0;

    ret = getpeername(fd, (struct sockaddr *)&so_stg, &so_len);
    if (ret)
        return 0;
    
    pstAppTcpIOCtl->error = error;
    pstAppTcpIOCtl->fd = fd;

    so_family = ((struct sockaddr*)&so_stg)->sa_family;
    switch (so_family) {
        case AF_INET: {
            struct sockaddr_in* in4 = (struct sockaddr_in*)&so_stg;
            if (inet_ntop(AF_INET, &(in4->sin_addr), so_ip_name, sizeof(pstAppTcpIOCtl->ip))) {
                pstAppTcpIOCtl->family = AF_INET;
                pstAppTcpIOCtl->port = in4->sin_port;
            }
            break;
        }
        case AF_INET6: {
            struct sockaddr_in6* in6 = (struct sockaddr_in6*)&so_stg;
            if (inet_ntop(AF_INET6, &(in6->sin6_addr), so_ip_name, sizeof(pstAppTcpIOCtl->ip))) {
                pstAppTcpIOCtl->family = AF_INET6;
                pstAppTcpIOCtl->port = in6->sin6_port;
            }
            break;
        }
    }

    return pstAppCtx->func_on_app_event(pstAppCtx, APP_CTRL_DID_TCP_OPEN, (void *)pstAppTcpIOCtl, sizeof(AppTcpIOControl));
}

void application_did_io_tcp_read(ApplicationContext *pstAppCtx, void *obj, int bytes)
{
    AppIOTraffic event = {0};
    if (!pstAppCtx || !obj || bytes <= 0)
        return;

    event.obj        = obj;
    event.bytes      = bytes;

    application_on_io_traffic(pstAppCtx, &event);
}


