#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#define SERVER_PORT 8888
#define BUFF_LEN 1024
#define IP_LEN 16

void handle_udp_msg(int fd)
{
    char buf[BUFF_LEN] = {0};
    char ip[IP_LEN] = {0};
    socklen_t len;
    int count = 0;
    struct sockaddr_in client_addr;

    while(1) {
        len = sizeof(client_addr);
        recvfrom(fd, buf, BUFF_LEN, 0, (struct sockaddr*)&client_addr, &len);

        snprintf(ip, IP_LEN - 1, "%s", inet_ntoa(client_addr.sin_addr));
        printf("[receiving msg from client:%s], %s\n",ip, buf);
        memset(buf, 0, BUFF_LEN);

        count++;
        sprintf(buf, "i have received the %d packet", count);
        sendto(fd, buf, strlen(buf), 0, (struct sockaddr*)&client_addr, len);
    }
}

int main(int argc, char * argv[])
{
    int server_fd;
    int ret;
    struct sockaddr_in ser_addr;

    server_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_fd < 0) {
        printf("create socket failed!\n");
        return -1;
    }

    memset(&ser_addr, 0, sizeof(ser_addr));
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    ser_addr.sin_port = htons(SERVER_PORT);

    ret = bind(server_fd, (struct sockaddr*)&ser_addr, sizeof(ser_addr));
    if (ret < 0) {
        printf("socket bind fail!\n");
        return -1;
    }

    printf("socket bind successfully!\n");
    handle_udp_msg(server_fd);

    close(server_fd);
    return 0;
}
