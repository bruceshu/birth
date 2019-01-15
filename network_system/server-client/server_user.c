#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>  
#include <sys/socket.h>  
#include <unistd.h>  
#include <arpa/inet.h>  

#include <pthread.h>

#include "utils.h"

static user_t userServer;
static int exit_signal = 0;

int creat_tcp_socket()
{
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket == -1)  
    {  
        printf("create socket failed!\n");
        return -1;
    }

    struct sockaddr_in addr;  
    memset(&addr, 0, sizeof(addr));  
      
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(TCP_SERVER_PORT);

    int ret = bind(server_socket, (struct sockaddr *)&addr, sizeof(addr));
    if(ret == -1)  
    {  
        printf("bind socket failed!\n");
        return -1;  
    } 

    ret = listen(server_socket, CLIENT_NUM); 
    if(ret == -1)  
    {  
        printf("listen socket failed!\n");
        return -1;  
    }  

    userServer.tcp_server_socket = server_socket;
    return 0;  
}

int wait_client()  
{  
    struct sockaddr_in cliaddr;  
    int addrlen = sizeof(cliaddr);  
    
    printf("waiting client to connect...\n");  

    //创建一个和客户端交流的套接字 
    int client_socket = accept(userServer.tcp_server_socket, (struct sockaddr *)&cliaddr, &addrlen); 
    if(client_socket == -1)  
    {  
        printf("accept socket failed!\n");
        return -1;  
    }

    memset(&userServer, 0, sizeof(userServer));
    userServer.cli_addr = cliaddr;
    userServer.tcp_client_socket = client_socket;
    
    printf("success to recive a client ：%s\n", inet_ntoa(cliaddr.sin_addr));      
    return 0;  
}  

static void * udp_send_msg(void * arg)
{
    
    char buf[BUFF_SIZE] = {0};  
    
    while(1)
    {
        scanf("%s", buf);
        sendto(userServer.udp_local_socket, buf, BUFF_SIZE, 0, userServer.cli_addr, sizeof(userServer.cli_addr));

        if (strncmp(buf, 4, "exit")) {
            exit_signal = 1;
            return;
        }
        
        memset(buf, 0, BUFF_SIZE);
        sleep(1);
    }
}

void *udp_recv_msg(void * arg)   
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

static void init_server_udp()
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
    userServer.udp_local_socket = udp_local_socket;
}

static void release_server_udp()
{
    close(userServer.udp_local_socket);
    close(userServer.tcp_server_socket);
    memset(&userServer, 0, sizeof(userServer));
}

int main(int argc, char *argv[])  
{  
    int ret = -1;
    pthread_t udp_recv_t;
    pthread_t udp_send_t;
    
    ret = creat_tcp_socket();
    if (ret < 0) {
        return -1;
    }

    int ret = wait_client();
    if (ret == -1) {
        close(userServer.tcp_server_socket);
        return -1;
    }

    init_server_udp();
    pthread_create(&udp_send_t, NULL, udp_send_msg(void * arg), NULL);
    pthread_create(&udp_recv_t, NULL, udp_recv_msg(void * arg), NULL);

    pthread_join(udp_send_t);
    pthread_join(udp_recv_t);

    release_server_udp();
    return 0;  
}  
