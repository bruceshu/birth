#ifndef UTILS_H
#define UTILS_H


#define BUFF_SIZE 1024

#define UDP_LOCAL_PORT 8000

#define SERVER_IP "192.168.1.104"
#define SERVER_TCP_PORT 10001
#define SERVER_UDP_PORT 10002
#define CLIENT_NUM 1000
#define TCP_ACK "tcp ack"
#define UDP_ACK "udp ack"


#define IP_LEN 16

#if 1
typedef struct user_s {
    int udp_client_socket;
    int tcp_client_socket;

    struct sockaddr_in udp_ser_addr;
    struct sockaddr_in tcp_ser_addr;
    
    struct sockaddr_in tcp_cli_addr;
} user_t;
#endif


#define MIN(a, b) (a) > (b) ? (b) : (a) 

#endif
