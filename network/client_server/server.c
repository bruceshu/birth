
#include <arpa/inet.h>
#include <sys/types.h>  
#include <sys/socket.h>  
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h> // close 和 sleep 头文件
#include <pthread.h>
#include <errno.h>

#include "utils.h"

int client_count = 0;
pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;

int create_tcp_socket() {
    int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(tcp_socket == -1)  
    {  
        printf("create socket failed!\n");
        return -1;
    }

    struct sockaddr_in addr;  
    memset(&addr, 0, sizeof(addr));  
    addr.sin_family = AF_INET;    /* Internet地址族 */  
    addr.sin_addr.s_addr = htonl(INADDR_ANY);   /* IP地址 */  
    addr.sin_port = htons(SERVER_TCP_PORT);

    int ret = bind(tcp_socket, (struct sockaddr *)&addr, sizeof(addr));
    if(ret == -1)  
    {  
        printf("bind socket failed!\n");
        return -1;  
    } 

    ret = listen(tcp_socket, CLIENT_NUM); 
    if(ret == -1)  
    {  
        printf("listen socket failed!\n");
        return -1;  
    }  

    return tcp_socket;
}

user_t * wait_client(int server_socket) {
    struct sockaddr_in cli_addr;  
    int addr_len = sizeof(cli_addr);  
    
    printf("waiting client to be connected...\n");  

    //创建一个和客户端交流的套接字 
    int client_socket = accept(server_socket, (struct sockaddr *)&cli_addr, (socklen_t *)&addr_len); 
    if(client_socket == -1)  
    {  
        printf("accept socket failed!\n");
        return NULL;  
    }  
          
    pthread_mutex_lock(&mutex);
    ++client_count;
    printf("a client has been connected ：%s\n", inet_ntoa(cli_addr.sin_addr));  
    printf("current client num is %d\n", client_count);
    pthread_mutex_unlock(&mutex);

    user_t *user = (user_t *)malloc(sizeof(user_t));
    if(user) {
        user->tcp_client_socket = client_socket;
        user->tcp_cli_addr = cli_addr;
    }
    
    return user;
}

void *handle_tcp_msg(void * arg)   
{  
    pthread_t self = pthread_self();
    char buf[BUFF_SIZE] = {0};  
    user_t *user = (user_t *)arg;
    int ret;

    while(1)  
    {  
        ret = recv(user->tcp_client_socket, buf, BUFF_SIZE-1, 0);
        if(ret == -1) {  
            printf("Error no.%d: %s\n", errno, strerror(errno));
            break;  
        }else if(ret == 0) {
            printf("client is down! thread id is %d\n", (int)self);
            break;
        }
        
        printf("client(%s) said:%s\n", inet_ntoa(user->tcp_cli_addr.sin_addr), buf);
        memset(buf, 0, BUFF_SIZE);
        send(user->tcp_client_socket, TCP_ACK, strlen((TCP_ACK)), 0);
    }  

    pthread_mutex_lock(&mutex);
    --client_count;
    printf("current client num is %d\n", client_count);
    pthread_mutex_unlock(&mutex);

    // remove_user_frome_clients(client_socket);
    close(user->tcp_client_socket);
    free(user);
    return NULL;
}  

void * connect_user(void *arg) {
    int server_socket = *(int *)arg;
    user_t *user = NULL;
    pthread_t thread_id;

    while(1) {
        if (client_count < CLIENT_NUM)
        {
            user = wait_client(server_socket);  
            if(user) {
                pthread_create(&thread_id, NULL, handle_tcp_msg, (void *)user);
                pthread_detach(thread_id);
            }
        }
        else
        {
            //当前连接数满了，则每隔5s扫描一次用户数
            sleep(5); //单位是秒
        }
    }

    return NULL;
}

// *******************
// udp 处理模块
// *******************
int create_udp_socket() {
    int udp_socket;
    int ret;
    struct sockaddr_in ser_addr;

    udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_socket < 0) {
        printf("create udp socket failed!\n");
        return -1;
    }

    memset(&ser_addr, 0, sizeof(ser_addr));
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    ser_addr.sin_port = htons(SERVER_UDP_PORT);

    ret = bind(udp_socket, (struct sockaddr*)&ser_addr, sizeof(ser_addr));
    if (ret < 0) {
        printf("udp socket bind fail!\n");
        return -1;
    }

    printf("create udp socket successfully!\n");
    return udp_socket;
}

void * handle_udp_msg(void *arg)
{
    char buf[BUFF_SIZE] = {0};
    int ret;
    socklen_t addr_len;
    struct sockaddr_in client_addr;
    int udp_socket = *(int *)arg;
    pthread_t self = pthread_self();

    while(1) {
        ret = recvfrom(udp_socket, buf, BUFF_SIZE, 0, (struct sockaddr*)&client_addr, &addr_len);
        if(ret == -1) {
            printf("Error no.%d: %s\n", errno, strerror(errno));
            break;
        }else if(ret == 0) {
            printf("client stop to speak to me! thread id is %d\n", (int)self);
            break;
        }

        printf("client(%s) said to me:%s\n", inet_ntoa(client_addr.sin_addr), buf);
        memset(buf, 0, BUFF_SIZE);

        // 局域网内，收到的udp包是从0.0.0.0地址传过来的。
        // sendto(udp_socket, UDP_ACK, strlen((UDP_ACK)), 0, (struct sockaddr*)&client_addr, sizeof(client_addr));
    }

    return NULL;
}

int main() {
    int tcp_socket = create_tcp_socket();
    int udp_socket = create_udp_socket();
    pthread_t tcp_connect;
    pthread_t handle_udp;

    pthread_create(&tcp_connect, NULL, connect_user, (void*)&tcp_socket);
    pthread_create(&handle_udp, NULL, handle_udp_msg, (void *)&udp_socket);
    pthread_join(tcp_connect, NULL);
    pthread_join(handle_udp, NULL);
    return 0;
}