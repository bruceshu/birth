/*
 * name : client.c
 * Author : bruce
 * version : 1.5.1
 * Copyright : www.bruce.com bruce company
 * Description : 连接网络客户端程序
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>



int handle(int fd);

int main(void)
{
	int nsd;
	char buf[1024];

	char *pMyAddr = "192.168.8.111";
	struct sockaddr_in addr;

	printf("welcome to echo client\n");
	nsd = socket(AF_INET, SOCK_STREAM, 0);
	printf("connect start\n");

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(5050);
	addr.sin_addr.s_addr = inet_addr(pMyAddr);
	
	printf("segmant1:inet_addr()\n");
	
	if (connect(nsd, (struct sockaddr *)&addr, sizeof(struct sockaddr)) < 0)
	{
		printf("connect error\n");
		return -1;	
	}

	sleep(5);
	printf("handle start\n");
	handle(nsd);
	
	return EXIT_SUCCESS;
}

int handle(int fd)
{
	char send[1024], rev[1024];

	for (;;)
	{
		memset(send, 0, sizeof(send));
		memset(rev, 0, sizeof(rev));
		
		if (fgets(send, 1024, stdin) == NULL)
		{
			break;
		}

		printf("write start\n");
		write(fd, send, strlen(send));
		read(fd, rev, strlen(rev));
	}

	return 0;
}





















