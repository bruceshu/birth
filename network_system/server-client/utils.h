#ifndef UTILS_H
#define UTILS_H

#include <sys/socket.h>

#define BUFF_SIZE 1024
#define UDP_LOCAL_PORT 8000
#define TCP_SERVER_PORT 9999

#define CLIENT_NUM 1
#define IP_LEN 16

typedef struct user_t {
    int udp_local_socket;

    int tcp_client_socket;
    int tcp_server_socket;

    struct sockaddr_in cli_addr;
    struct sockaddr_in ser_addr;
} user_t;

#endif
