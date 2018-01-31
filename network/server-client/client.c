#include <stdio.h>
#include <sys/socket.h>


#define INPUT_SIZE 1024
#define PORT 9999

#if 0
void parse_url(const char *url, char *domain, int *port)
{
    int j = 0;
    int start = 0;
    *port = 80;
    char *patterns[] = {"http://", "https://", NULL};

    for (int i = 0; patterns[i]; i++)
        if (strncmp(url, patterns[i], strlen(patterns[i])) == 0)
        {
            start = strlen(patterns[i]);
            break;
        }

    for (int i = start; url[i] != '/' && url[i] != '\0'; i++, j++)
        domain[j] = url[i];
    domain[j] = '\0';

    char *pos = strstr(domain, ":");
    if (pos)
        sscanf(pos, ":%d", port);

    for (int i = 0; i < (int)strlen(domain); i++)
    {
        if (domain[i] == ':')
        {
            domain[i] = '\0';
            break;
        }
    }

    j = 0;
    for (int i = start; url[i] != '\0'; i++)
    {
        if (url[i] == '/')
        {
            if (i !=  strlen(url) - 1)
                j = 0;
            continue;
        }
        else
            file_name[j++] = url[i];
    }
    file_name[j] = '\0';
}
#endif


int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        goto DETAIL;
    }
    
    struct sockaddr_in addr;
    int socket;

    //char ip[16] = {0};
    //unsigned short port = 80;
    //parse_url(argv[1], ip, port);

    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0)
    {
        printf("create socket failed!\n");
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_port = htons(PORT);

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

    close(listen_socket);
    
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
