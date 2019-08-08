#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <pthread.h>

#include <netdb.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define SERVER_PORT 8888
#define SERVER_IP "192.168.140.112"
#define IP_SIZE 16
#define BUFF_LEN 1024

char *eth = NULL;
typedef struct udp_arg_t {
    int fd;
    struct sockaddr *ser_addr;
}udp_arg_t;

int get_local_ip(const char *eth_inf, char *ip)
{
    int sd;
    struct sockaddr_in sin;
    struct ifreq ifr;

    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sd == -1)
    {
        printf("socket error: %s\n", strerror(errno));
        return -1;
    }

    strncpy(ifr.ifr_name, eth_inf, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = 0;

    // if error: No such device
    if (ioctl(sd, SIOCGIFADDR, &ifr) < 0)
    {
        printf("ioctl error: %s\n", strerror(errno));
        close(sd);
        return -1;
    }

    memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
    snprintf(ip, IP_SIZE, "%s", inet_ntoa(sin.sin_addr));

    close(sd);
    return 0;
}


void *send_udp_msg(void *arg)
{
    char ip[IP_SIZE] = {0};
    struct sockaddr_in addr;
    int len = sizeof(struct sockaddr);
    udp_arg_t *udp_arg = (udp_arg_t *)arg;

    while(1) {
        //get_local_ip(eth, ip);
        //sprintf(buf, "[%s] %s", ip, "send test udp msg");
        char buf[BUFF_LEN] = "test msg";

        sendto(udp_arg->fd, buf, BUFF_LEN, 0, udp_arg->ser_addr, len);
        memset(buf, 0, BUFF_LEN);

        recvfrom(udp_arg->fd, buf, BUFF_LEN, 0, (struct sockaddr*)&addr, &len);
        printf("[server %s] said: %s", inet_ntoa(addr.sin_addr), buf);
        sleep(1);
    }
}

int main(int argc, char * argv[])
{
    int client_fd;
    struct sockaddr_in ser_addr;
    udp_arg_t udp_arg = {0};

    pthread_t send_udp_thread;
    pthread_t recv_udp_thread;

#if 0
    if (argc != 2)
    {
        printf("pls add one parameter the name of eth\n");
        return -1;
    }

    eth = strdup(argv[1]);
    if (!eth)
    {
        printf("[%s %d] strdup failed!", __func__, __LINE__);
    }
#endif

    client_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_fd < 0) {
        printf("create socket failed!\n");
        return -1;
    }

    memset(&ser_addr, 0, sizeof(ser_addr));
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    ser_addr.sin_port = htons(SERVER_PORT);

    printf("begin to send udp msg\n");
    udp_arg.fd = client_fd;
    udp_arg.ser_addr = (struct sockaddr*)&ser_addr;
    if (pthread_create(&send_udp_thread, NULL, send_udp_msg, (void *)&udp_arg) < 0)
    {
        printf("pthread_create failed!\n");
        close(client_fd);
        return -1;
    }

    pthread_join(send_udp_thread, NULL);

    close(client_fd);
    return 0;
}
