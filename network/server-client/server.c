

#include <sys/types.h>  
#include <sys/socket.h>  
#include <stdio.h>  
#include <unistd.h>  
#include <string.h>  
#include <arpa/inet.h>  

#define PORT 9999
#define BUFF_SIZE 1024

int creat_socket()
{
    int listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(listen_socket == -1)  
    {  
        printf("create socket failed!\n");
        return -1;  
    }

    struct sockaddr_in addr;  
    memset(&addr, 0, sizeof(addr));  
      
    addr.sin_family = AF_INET;    /* Internet地址族 */  
    addr.sin_port = htons(PORT);  /* 端口号 */  
    addr.sin_addr.s_addr = htonl(INADDR_ANY);   /* IP地址 */  

    int ret = bind(listen_socket, (struct sockaddr *)&addr, sizeof(addr));
    if(ret == -1)  
    {  
        printf("bind socket failed!\n");
        return -1;  
    } 

    ret = listen(listen_socket, 5);        //监听  
    if(ret == -1)  
    {  
        printf("listen socket failed!\n");
        return -1;  
    }  
    
    return listen_socket;  
}

int wait_client(int listen_socket)  
{  
    struct sockaddr_in cliaddr;  
    int addrlen = sizeof(cliaddr);  
    
    printf("waiting client to connect...\n");  

    //创建一个和客户端交流的套接字 
    int client_socket = accept(listen_socket, (struct sockaddr *)&cliaddr, &addrlen); 
    if(client_socket == -1)  
    {  
        printf("accept socket failed!\n");
        return -1;  
    }  
      
    printf("success to recive a client ：%s\n", inet_ntoa(cliaddr.sin_addr));  
      
    return client_socket;  
}  

//信息处理函数,功能是将客户端传过来的小写字母转化为大写字母  
void hanld_client(int listen_socket, int client_socket)   
{  
    char buf[BUFF_SIZE] = {0};  
    
    while(1)  
    {  
        int ret = read(client_socket, buf, BUFF_SIZE-1);  
        if(ret == -1)  
        {  
            printf("read from client socket failed!\n");
            break;  
        }  
        if(ret == 0)  
        {  
            printf("read zero byte!\n");
            break;  
        }  
        
        buf[ret] = '\0';
        
        int i;  
        for(i = 0; i < ret; i++)  
        {  
            buf[i] = buf[i] + 'A' - 'a';  
        }  
        printf("%s\n", buf);
        
        write(client_socket, buf, ret);  
          
        if(strncmp(buf, "end", 3) == 0)  
        {  
            break;  
        }  
    }  
    
    close(client_socket);  
}  

int main()  
{  
    int listen_socket = Creat_socket();  
      
    int client_socket = wait_client(listen_socket);  
      
    hanld_client(listen_socket, client_socket);  
      
    close(listen_socket);  
      
    return 0;  
}  
