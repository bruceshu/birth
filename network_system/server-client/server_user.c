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
        printf("bind tcp socket failed!\n");
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
    struct sockaddr_in tcp_cli_addr;  
    struct sockaddr_in udp_cli_addr;
    int addrlen = sizeof(tcp_cli_addr);  
    
    printf("waiting client to connect...\n");  

    //创建一个和客户端交流的套接字 
    int client_socket = accept(userServer.tcp_server_socket, (struct sockaddr *)&tcp_cli_addr, &addrlen); 
    if(client_socket == -1)  
    {  
        printf("accept socket failed!\n");
        return -1;  
    }

    userServer.tcp_cli_addr = tcp_cli_addr;
    userServer.tcp_client_socket = client_socket;

    udp_cli_addr.sin_family = AF_INET;
    udp_cli_addr.sin_addr.s_addr = inet_addr(inet_ntoa(tcp_cli_addr.sin_addr));
    udp_cli_addr.sin_port = htons(UDP_LOCAL_PORT);
    userServer.udp_cli_addr = udp_cli_addr;

    printf("success to recive a client ：%s\n", inet_ntoa(tcp_cli_addr.sin_addr));      
    return 0;  
}  

static void * udp_send_msg(void * arg)
{
    
    char buf[BUFF_SIZE] = {0};
    
    while(1)
    {
        scanf("%[^]", buf);
        sendto(userServer.udp_local_socket, buf, BUFF_SIZE, 0, (struct sockaddr*)&userServer.udp_cli_addr, sizeof(userServer.udp_cli_addr));

        if (!strncmp(buf, "exit", 4)) {
            printf("%s over\n", __func__);
            return NULL;
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
        if (!strncmp(buf, "exit", 4)) {
            printf("%s over\n", __func__);
            return NULL;
        }

        printf("[小默] said:%s\n", buf);
        //printf("[client %s] said:%s\n", inet_ntoa(addr.sin_addr), buf);
        memset(buf, 0, sizeof(buf));
    }
}  

static void init_server_udp()
{
    int ret = -1;
    int udp_local_socket = -1;
    struct sockaddr_in localAddr;

    udp_local_socket = socket(AF_INET, SOCK_DGRAM, 0);

    memset(&localAddr, 0, sizeof(localAddr));
    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    localAddr.sin_port = htons(UDP_LOCAL_PORT);

    ret = bind(udp_local_socket, (struct sockaddr*)&localAddr, sizeof(localAddr));
    if (ret < 0) {
        printf("bind udp socket failed!\n");
        return;
    }

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

    ret = wait_client();
    if (ret < 0) {
        close(userServer.tcp_server_socket);
        return -1;
    }

    init_server_udp();
    pthread_create(&udp_send_t, NULL, udp_send_msg, NULL);
    pthread_create(&udp_recv_t, NULL, udp_recv_msg, NULL);

    pthread_join(udp_send_t, NULL);
    pthread_join(udp_recv_t, NULL);

    release_server_udp();
    return 0;  
}  

