#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#define SERVER_PORT 8888
#define BUFF_LEN 1024
#define SERVER_IP "192.168.140.112"

void udp_msg_sender(int fd, struct sockaddr *dst)
{
    while(1) {
        char buf[BUFF_LEN] = "test udp msg\n";
        int len = sizeof(*dst);

        sendto(fd, buf, BUFF_LEN, 0, dst, len);
        memset(buf, 0, BUFF_LEN);

        sleep(1);
    }
}

int main(int argc, char * argv[])
{
    int client_fd;
    struct sockaddr_in ser_addr;

    client_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_fd < 0) {
        printf("create socket failed!\n");
        return -1;
    }

    memset(&ser_addr, 0, sizeof(ser_addr));
    ser_addr.sin_family = AF_INET;
    //ser_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    ser_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    ser_addr.sin_port = htons(SERVER_PORT);

    udp_msg_sender(client_fd, (struct sockaddr*)&ser_addr);

    close(client_fd);
    return 0;
}
