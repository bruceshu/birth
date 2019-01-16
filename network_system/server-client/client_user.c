#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <pthread.h>

#include <sys/socket.h>
#include <sys/types.h>  
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>  

#include "utils.h"


static user_t userClient;
static char server_ip[IP_LEN] = {0};

static void* udp_recv_msg(void *arg)
{
    char buf[BUFF_SIZE] = {0};  
    struct sockaddr_in addr;
    int addr_len = sizeof(addr);
        
    while(1)  
    {
        recvfrom(userClient.udp_local_socket, buf, sizeof(buf) - 1, 0, (struct sockaddr*)&addr, &addr_len);        
        if (!strncmp(buf, "exit", 4)) {
            printf("%s over\n", __func__);
            return NULL;
        }

        printf("[小焕] said:%s\n", buf);
        //printf("[server %s] said:%s\n", inet_ntoa(addr.sin_addr), buf);
        memset(buf, 0, sizeof(buf));
    }
}

static void* udp_send_msg(void *arg)
{
    char buf[BUFF_SIZE] = {0};
    
    while(1) {
        scanf("%[^\n]", buf);
        sendto(userClient.udp_local_socket, buf, sizeof(buf), 0, (struct sockaddr*)&userClient.udp_ser_addr, sizeof(userClient.udp_ser_addr));
        if (!strncmp(buf, "exit", 4)) {
            printf("%s over\n", __func__);
            return NULL;
        }
        
        memset(buf, 0, BUFF_SIZE);
        sleep(1);
    }
}

static void init_client_udp()
{
    int ret = -1;
    int udp_local_socket = -1;
    struct sockaddr_in udpSerAddr;
    struct sockaddr_in udpCliAddr;

    udp_local_socket = socket(AF_INET, SOCK_DGRAM, 0);

    memset(&udpSerAddr, 0, sizeof(udpSerAddr));
    udpSerAddr.sin_family = AF_INET;
    udpSerAddr.sin_addr.s_addr = inet_addr(server_ip);
    udpSerAddr.sin_port = htons(UDP_LOCAL_PORT);
    userClient.udp_ser_addr = udpSerAddr;

    memset(&udpCliAddr, 0, sizeof(udpCliAddr));
    udpCliAddr.sin_family = AF_INET;
    udpCliAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    udpCliAddr.sin_port = htons(UDP_LOCAL_PORT);

    ret = bind(udp_local_socket, (struct sockaddr*)&udpCliAddr, sizeof(udpCliAddr));
    if (ret < 0) {
        printf("socket bind fail!\n");
        return;
    }

    userClient.udp_local_socket = udp_local_socket;
}

static void release_client_udp()
{
    close(userClient.udp_local_socket);
    close(userClient.tcp_client_socket);
    memset(&userClient, 0, sizeof(userClient));
}

int main(int argc, char *argv[])
{
    int ret;
    int tcp_client_socket = -1;
    struct sockaddr_in tcp_ser_addr;
    struct sockaddr_in udp_ser_addr;
    pthread_t pthread_send_t;
    pthread_t pthread_recv_t;

    if (argc != 2)
    {
        goto DETAIL;
    }
    
    strncpy(server_ip, argv[1], MIN(strlen(argv[1]), IP_LEN));
    tcp_client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_client_socket < 0)
    {
        printf("create socket failed!\n");
        return -1;
    }

    userClient.tcp_client_socket = tcp_client_socket;
    memset(&tcp_ser_addr, 0, sizeof(tcp_ser_addr));
    tcp_ser_addr.sin_family = AF_INET;
    tcp_ser_addr.sin_addr.s_addr = inet_addr(server_ip);
    tcp_ser_addr.sin_port = htons(TCP_SERVER_PORT);

    int result = connect(tcp_client_socket, (struct sockaddr *)&tcp_ser_addr, sizeof(tcp_ser_addr));
    if (result == -1)
    {
        printf("connect failed\n");
        close(userClient.tcp_client_socket);
        return -1;
    }
    
    printf("connect successfully!\n");
    init_client_udp();
    pthread_create(&pthread_recv_t, NULL, udp_recv_msg, NULL);
    pthread_create(&pthread_send_t, NULL, udp_send_msg, NULL);

    pthread_join(pthread_recv_t, NULL);
    pthread_join(pthread_send_t, NULL);

    release_client_udp();
    return 0;
    
DETAIL:
    printf("==============================\n");
    printf("please input one paremeter,like this:\n");
    printf("./client 127.0.0.1\n");
    printf("==============================\n");
    return 0;
}
