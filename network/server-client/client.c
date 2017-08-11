#include <stdio.h>
#include <sys/socket.h>

int main(int argc, char *argv[])
{
    struct sockaddr_in addr;
    int socket;

    char ip[16] = {0};
    unsigned short port = 80;

    if (argc != 2)
    {
        goto DETAIL;
    }

    parse_url(argv[1], ip, port);

    int client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client_socket < 0)
    {
        printf("invalid socket descriptor:%d\n", client_socket);
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);

    int result = connect(client_socket, (struct sockaddr *)&addr, sizeof(addr));
    if (result == -1)
    {
        printf("connect failed\n");
        return -1;
    }

DETAIL:
    printf("==============================\n");
    printf("please input one paremeter,like this:\n");
    printf("./client 192.168.135.123:1234\n");    
    printf("./client 192.168.135.123\n");
    printf("./client www.baidu.com");
    return 0;
}
