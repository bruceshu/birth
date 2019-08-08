/*
* Copyright (c) 2018 bruce
*
* 测试getaddrinfo函数用法，以及涉及到的addrinfo、sockaddr_in 等结构体
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netdb.h>


int main(int argc, char *argv[])
{
    struct addrinfo hints;
    struct addrinfo *res;
    struct addrinfo *cur_ai;
    struct sockaddr_in *addr;
    
    int ret;
    char *hostname = "www.bai.com";
    char portstr[8] = {0};
    int port = 80;

    char ipaddr[16] = {0};
    char *ip_ptr = NULL;

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    snprintf(portstr, sizeof(portstr), "%d", port);

    ret = getaddrinfo(hostname, portstr, &hints, &res);
    if (ret != 0) {
        printf("getaddrinfo error\n");
        exit(1);
    }

    for (cur_ai = res; cur_ai != NULL; cur_ai = cur_ai->ai_next) {
        addr = (struct sockaddr_in *)cur_ai->ai_addr;

	//
	//
	//printf("sizeof(addr->sin_addr) = %d\n", sizeof(addr->sin_addr));
        /*sprintf(ipaddr, "%d.%d.%d.%d", (*addr).sin_addr.s_un.S_un_b.s_b1, 
                                       addr->sin_addr.S_un.S_un_b.s_b2,
                                       addr->sin_addr.S_un.S_un_b.s_b3,
                                       addr->sin_addr.S_un.S_un_b.s_b4);

*/      
	ip_ptr = inet_ntoa(addr->sin_addr);
	printf("ip_ptr = %s\n", ip_ptr);  
	//printf("ipaddr = %s\n", ipaddr);
    }

    freeaddrinfo(res);
}








