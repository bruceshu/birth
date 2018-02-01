

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>  
#include <unistd.h>  
#include <arpa/inet.h>  

#define INPUT_SIZE 1024
#define TRUE 1
#define FALSE 0

#define OK 0
#define ERROR -1

int isNumIP(const char *url)
{
    
    while(*url && (*url <= '9' && *url >= '0' || *url == '.'))
    {
        url++;
    }

    if(*url)
    {
        return FALSE;
    }

    return TRUE;
}

int parse_url(const char *url, char *domain, int *port)
{
    char *hostname = NULL;
    char *tmp = NULL;
    
    hostname = strdup(url);
    if (!hostname)
    {
        return ERROR;
    }

    tmp = strchr(hostname, ":");
    if (tmp)
    {
        *tmp = '\0';
        tmp += 1;
        *port = atoi(tmp);
    }

    if(isNumIP(hostname))
    {
        strncpy(domain, hostname, strlen(hostname));
        return OK;
    }

    struct in_addr in;
    struct sockaddr_in addr_in;
    struct hostent *host = NULL;
    host = gethostbyname(hostname);
    if (!host)
    {
        return ERROR;
    }

    strcpy(domain, inet_ntoa( * (struct in_addr*) host->h_addr));        

    return OK;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        goto DETAIL;
    }

    int ret;
    char ip[16] = {0};
    unsigned short port = 9999;
    ret = parse_url(argv[1], ip, &port);
    if (ret < 0)
    {
        goto DETAIL;
    }



    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0)
    {
        printf("create socket failed!\n");
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_port = htons(port);

    int result = connect(client_socket, (struct sockaddr *)&addr, sizeof(addr));
    if (result == -1)
    {
        printf("connect failed\n");
        return -1;
    }
    printf("connect successfully!\n");

    char buf[INPUT_SIZE] = {0};
    while(1)
    {
        printf("please input what you want to say：");  
        scanf("%s", buf);  
        write(client_socket, buf, strlen(buf));  
          
        int ret = read(client_socket, buf, strlen(buf));  
        printf("buf = %s\n", buf);  

        //当输入END时客户端退出
        if(strncmp(buf, "END", 3) == 0)
        {  
            break;  
        }  
    }

    close(client_socket);
    
    return 0;
DETAIL:
    printf("==============================\n");
    printf("please input one paremeter,like this:\n");
    //printf("./client 192.168.135.123:1234 or\n");    
    printf("./client 192.168.135.123 or\n");
    //printf("./client www.baidu.com");
    printf("==============================\n");
    
    return 0;
}
