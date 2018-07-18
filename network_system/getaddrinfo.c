/*
* Copyright (c) 2018 bruce
*
* 测试getaddrinfo函数用法，以及涉及到的addrinfo、sockaddr_in 等结构体
*
*/

int main(int argc, char *argv[])
{
    struct addrinfo hints;
    struct addrinfo *res;
    struct addrinfo *cur_ai;
    struct sockaddr_in *addr;
    
    char ipaddr[16] = {0};
    char *hostname = "35.197.157.137";
    char portstr[8] = {0};
    int port = 80;

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    snprintf(portstr, sizeof(portstr), "%d", port);

    ret = getaddrinfo(hostname, portstr, &hints, &res);
    if (ret != 0) {
        printf("getaddrinfo error\n");
        exit(1);
    }

    for (cur_ai = res; cur_ai != NULL; cur_ai = cur_ai->next) {
        addr = (struct sockaddr_in *)cur_ai->ai_addr;
        sprintf(ipaddr, "%d.%d.%d.%d", addr->sin_addr.S_un.s_un_b.s_b1, 
                                       addr->sin_addr.S_un.s_un_b.s_b2,
                                       addr->sin_addr.S_un.s_un_b.s_b3,
                                       addr->sin_addr.S_un.s_un_b.s_b4);

        printf("ipaddr = %s\n", ipaddr);
    }

    freeaddrinfo(res);
}








