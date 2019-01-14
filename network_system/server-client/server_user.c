#include <sys/types.h>  
#include <sys/socket.h>  
#include <stdio.h>  
#include <unistd.h>  
#include <string.h>  
#include <arpa/inet.h>  
#include <pthread.h>

#define TCP_SERVER_PORT 9999
#define BUFF_SIZE 1024
#define CLIENT_NUM 1

pthread_t udp_recv_t;
pthread_t udp_send_t;

int creat_socket()
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
    
    return server_socket;  
}

int wait_client(int server_socket)  
{  
    struct sockaddr_in cliaddr;  
    int addrlen = sizeof(cliaddr);  
    
    printf("waiting client to connect...\n");  

    //创建一个和客户端交流的套接字 
    int client_socket = accept(server_socket, (struct sockaddr *)&cliaddr, &addrlen); 
    if(client_socket == -1)  
    {  
        printf("accept socket failed!\n");
        return -1;  
    }

    printf("success to recive a client ：%s\n", inet_ntoa(cliaddr.sin_addr));      
    return client_socket;  
}  

static void * send_msg(void * arg)
{
    
}
//将客户端信息原样输出
void *hanld_client(void * arg)   
{  
    char buf[BUFF_SIZE] = {0};  
    int client_socket = *(int *)arg;
    int ret;

    while(1)  
    {  
        ret = recv(client_socket, buf, BUFF_SIZE-1, 0);
        if(ret == -1) {  
            printf("read from client socket failed!\n");
            break;  
        } else if(ret == 0) {  
            printf("client %d said nothing!\n", client_socket);
            break;
        }
        
        buf[ret] = '\0';
        printf("client %d said:%s\n", client_socket, buf);
        
        if(strncmp(buf, "end", 3) == 0)
        {  
            break;  
        }
    }  

    pthread_mutex_lock(&mutex);
    count_client--;
    pthread_mutex_unlock(&mutex);
    printf("current client num is %d\n", count_client);

    close(client_socket);  
}  


int main()  
{  
    int server_socket = creat_socket();
    if (server_socket == -1) {
        return -1;
    }

    int client_socket = wait_client(server_socket);
    if (client_socket == -1) {
        close(server_socket);
        return -1;
    }

    pthread_create(&udp_send_t, NULL, udp_send_msg(void * arg), (void *)&client_socket);
    pthread_create(&udp_recv_t, NULL, udp_recv_msg(void * arg), (void *)&client_socket);

    pthread_join(udp_send_t);
    pthread_join(udp_recv_t);

    close(server_socket);  
      
    return 0;  
}  

