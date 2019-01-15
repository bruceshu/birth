#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>  
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>  

static user_t userClient;
static char server_ip[IP_LEN] = {0};
static int exit_signal = 0;

static void* udp_recv_msg(void *arg)
{
    int ret;
    char buf[BUFF_SIZE] = {0};  
    struct sockaddr_in addr;
    int addr_len = sizeof(addr);
        
    while(1)  
    {
        recvfrom(userServer.udp_local_socket, buf, sizeof(buf) - 1, 0, (struct sockaddr*)&addr, &addr_len);
        
        memset(buf, 0, sizeof(buf));
        printf("[client %s] said:%s\n", inet_ntoa(addr.sin_addr), buf);

        if (exit_signal) {
            return;
        }
    }
}

static void* udp_send_msg(void *arg)
{
    char buf[BUFF_SIZE] = {0};
    
    while(1) {
        gets(buf);
        sendto(userClient.udp_local_socket, buf, sizeof(buf), 0, &userClient.ser_addr, sizeof(userClient.ser_addr));
        if (strncmp(buf, 4, "exit")) {
            exit_signal = 1;
            return;
        }
        
        memset(buf, 0, BUFF_SIZE);
        sleep(1);
    }
}

static void init_client_udp()
{
    int ret = -1;
    int udp_local_socket = -1;
    struct sockaddr_in localAddr;

    udp_local_socket = socket(AF_INET, SOCK_DGRAM, 0);

    memset(&localAddr, 0, sizeof(remoteAddr));
    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    localAddr.sin_port = htons(UDP_LOCAL_PORT);

    ret = bind(udp_local_socket, (struct sockaddr*)&localAddr, sizeof(localAddr));
    if (ret < 0) {
        printf("socket bind fail!\n");
        return;
    }

    // 记录服务器udp socket
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
    struct sockaddr_in addr;
    pthread_t pthread_send_t;
    pthread_t pthread_recv_t;

    if (argc != 2)
    {
        goto DETAIL;
    }
    
    strncpy(server_ip, 16, argv[1]);
    tcp_client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_client_socket < 0)
    {
        printf("create socket failed!\n");
        return -1;
    }

    userClient.tcp_client_socket = tcp_client_socket;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(server_ip);
    addr.sin_port = htons(TCP_SERVER_PORT);

    int result = connect(client_socket, (struct sockaddr *)&addr, sizeof(addr));
    if (result == -1)
    {
        printf("connect failed\n");
        close(userClient.tcp_client_socket);
        return -1;
    }
    
    userClient.ser_addr = addr;
    printf("connect successfully!\n");
    init_client_udp();
    pthread_create(&pthread_recv_t, NULL, udp_recv_msg, NULL);
    pthread_create(&pthread_send_t, NULL, udp_send_msg, NULL);

    pthread_join(pthread_recv_t);
    pthread_join(pthread_send_t);

    release_client_udp();
    return 0;
    
DETAIL:
    printf("==============================\n");
    printf("please input one paremeter,like this:\n");
    printf("./client 127.0.0.1\n");
    printf("==============================\n");
    
    return 0;
}
