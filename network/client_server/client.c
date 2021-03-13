
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "utils.h"

static user_t userClient;

// ********************
// tcp连接发送心跳包
// ********************
void* send_tcp_msg(void *arg)
{
    char buf[BUFF_SIZE] = {"heart timing"};
    int ret;
    
    while(1) {
        ret = send(userClient.tcp_client_socket, buf, strlen(buf), 0);
        if(ret == -1) {
            printf("Error no.%d: %s\n", errno, strerror(errno));
            break;
        }

        // 每隔10s发送一次心跳包
        sleep(10);
    }

    return NULL;
}

void* recv_tcp_msg(void *arg)
{
    char buf[BUFF_SIZE] = {0};
    int len = 0;
    
    while(1)
    {
        len = recv(userClient.tcp_client_socket, buf, sizeof(buf), 0);
        if(len == -1) {
            printf("Error no.%d: %s\n", errno, strerror(errno));
            break;
        } else if(len == 0) {
            printf("server is down!\n");
            break;
        }

        printf("recv server(%s) answers:%s\n", inet_ntoa(userClient.tcp_ser_addr.sin_addr), buf);
        memset(buf, 0, BUFF_SIZE);
    }

    close(userClient.tcp_client_socket);
    return NULL;
}

int connect_to_server() {
    int client_socket = -1;
    struct sockaddr_in addr;

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1)
    {
        printf("create socket failed!\n");
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    addr.sin_port = htons(SERVER_TCP_PORT);

    int result = connect(client_socket, (struct sockaddr *)&addr, sizeof(addr));
    if (result == -1)
    {
        printf("connect failed\n");
        return -1;
    }

    userClient.tcp_client_socket = client_socket;
    userClient.tcp_ser_addr = addr;
    printf("connect successfully!\n");
    return 0;
}

int start_tcp() {
    int ret;
    pthread_t pthread_send_t;
    pthread_t pthread_recv_t;
    
    ret = connect_to_server();
    if (ret != 0)
    {
        return -1;
    }
    
    pthread_create(&pthread_recv_t, NULL, recv_tcp_msg, NULL);
    pthread_create(&pthread_send_t, NULL, send_tcp_msg, NULL);
    // pthread_join(pthread_recv_t, NULL);
    // pthread_join(pthread_send_t, NULL);
    return 0;
}

// ************
// udp报文传输
// ************
void *send_udp_msg(void *arg)
{
    printf("%s\n", __func__);
    char buf[BUFF_SIZE] = {0};
    int ret = 0;

    while(1) {
        printf("你对***说：\n");
        scanf("%[^\n]", buf);
        setbuf(stdin, NULL);
        if (strncmp(buf, "exit", 4) == 0) {
            printf("停止对***说!\n");
            break;
        }

        ret = sendto(userClient.udp_client_socket, buf, strlen(buf), 0, (struct sockaddr*)&userClient.udp_ser_addr, sizeof(userClient.udp_ser_addr));
        if(ret == -1) {
            printf("[%s:%d]Error no.%d: %s\n", __func__, __LINE__, errno, strerror(errno));
            break;
        }
        memset(buf, 0, BUFF_SIZE);
        sleep(1);
    }

    if(userClient.udp_client_socket != -1) {
        close(userClient.udp_client_socket);
        userClient.udp_client_socket = -1;
    }
    return NULL;
}

void * recv_udp_msg(void *arg) {
    printf("%s\n", __func__);
    char buf[BUFF_SIZE] = {0};
    struct sockaddr_in addr;
    int addr_len;
    int ret = 0;
    
    while(1)
    {
        ret = recvfrom(userClient.udp_client_socket, buf, BUFF_SIZE, 0, (struct sockaddr*)&addr, (socklen_t*)&addr_len);
        if(ret == -1) {
            printf("Error no.%d: %s\n", errno, strerror(errno));
            break;
        } else if(ret == 0) {
            printf("server is down!\n");
            break;
        }

        printf("server(%s) said: %s\n", inet_ntoa(addr.sin_addr), buf);
        memset(buf, 0, BUFF_SIZE);
    }

    if(userClient.udp_client_socket != -1) {
        close(userClient.udp_client_socket);
        userClient.udp_client_socket = -1;
    }

    return NULL;
}

int start_udp() {
    printf("%s\n", __func__);
    int udp_socket;
    struct sockaddr_in ser_addr;

    pthread_t send_udp_thread;
    pthread_t recv_udp_thread;

    udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_socket < 0) {
        printf("create socket failed!\n");
        return -1;
    }

    memset(&ser_addr, 0, sizeof(ser_addr));
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    ser_addr.sin_port = htons(SERVER_UDP_PORT);

    userClient.udp_client_socket = udp_socket;
    userClient.udp_ser_addr = ser_addr;
    pthread_create(&send_udp_thread, NULL, send_udp_msg, NULL);
    //暂时用不到udp接收
    // pthread_create(&recv_udp_thread, NULL, recv_udp_msg, NULL);
    pthread_join(send_udp_thread, NULL);
    pthread_join(recv_udp_thread, NULL);
    return 0;
}

int main() {
    int ret = 0;
    
    ret = start_tcp();
    if(ret == 0) {
        start_udp();
    }

    return 0;
}