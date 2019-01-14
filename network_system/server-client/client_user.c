#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>  
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>  

#define BUFFER_SIZE 1024
#define UDP_LOCAL_PORT 8000
#define TCP_SERVER_PORT 9999

typedef struct user_client_t {
    int udp_local_socket;
    int udp_remotesocket;

    struct sockaddr_in ser_addr;
} user_client_t;

user_client_t userClient;
char ip[16] = {0};

int exit_signal = 0;

int isNumIP(const char *url)
{
    
    while(*url && (*url <= '9' && *url >= '0' || *url == '.'))
    {
        url++;
    }

    //如果url中有非法字符，则返回错误
    if(*url)
    {
        return FALSE;
    }

    return TRUE;
}

static void* recv_msg(void *arg)
{
    char buf[BUFFER_SIZE] = {0};
    struct sockaddr_in addr;
    int addr_len;
    
    while(1)
    {
        addr_len = sizeof(addr);
        recvfrom(userClient.udp_local_socket, buf, sizeof(buf), 0, (struct sockaddr*)&addr, &addr_len);
        
        printf("%s said: %s", inet_ntoa(addr.sin_addr), buf);
        if (exit_signal) {
            return;
        }
    }
}

static void* send_msg(void *arg)
{
    char buf[BUFFER_SIZE] = {0};
    
    while(1) {
        gets(buf);
        sendto(userClient.udp_local_socket, buf, sizeof(buf), 0, &userClient.ser_addr, sizeof(userClient.ser_addr));
        if (strncmp(buf, 4, "exit")) {
            exit_signal = 1;
            return;
        }
        memset(buf, 0, BUFFER_SIZE);
        sleep(1);
    }
}

static void udp_init()
{
    int ret = -1;
    int udp_local_socket = -1;
    struct sockaddr_in ser_addr;
    struct sockaddr_in local_addr;

    udp_local_socket = socket(AF_INET, SOCK_DGRAM, 0);

    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    local_addr.sin_port = htons(LOCAL_PORT);

    ret = bind(udp_local_socket, (struct sockaddr*)&local_addr, sizeof(local_addr));
    if (ret < 0) {
        printf("socket bind fail!\n");
        return -1;
    }

    memset(&ser_addr, 0, sizeof(ser_addr));
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_addr.s_addr = inet_addr(ip);
    ser_addr.sin_port = htons(port);

    userClient.udp_local_socket = udp_local_socket;
    userClient.ser_addr = ser_addr;
}

int main(int argc, char *argv[])
{
    int ret;
    int client_socket = -1;
    struct sockaddr_in addr;
    pthread_t pthread_send_t;
    pthread_t pthread_recv_t;

    if (argc != 2)
    {
        goto DETAIL;
    }
    
    strncpy(ip, 16, argv[1]);
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0)
    {
        printf("create socket failed!\n");
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(TCP_SERVER_PORT);

    int result = connect(client_socket, (struct sockaddr *)&addr, sizeof(addr));
    if (result == -1)
    {
        printf("connect failed\n");
        return -1;
    }
    
    printf("connect successfully!\n");
    udp_init();
    pthread_create(&pthread_recv_t, NULL, recv_msg, NULL);
    pthread_create(&pthread_send_t, NULL, send_msg, NULL);

    pthread_join(pthread_recv_t);
    pthread_join(pthread_send_t);

    close(client_socket);
    return 0;
    
DETAIL:
    printf("==============================\n");
    printf("please input one paremeter,like this:\n");
    //printf("./client www.baidu.com or\n");    
    printf("./client 127.0.0.1\n");
    printf("==============================\n");
    
    return 0;
}

