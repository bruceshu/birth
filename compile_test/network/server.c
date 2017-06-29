/*
 * name            : server.c
 * Author          : bruce
 * Version         : 1.5.1
 * Copyright       : www.bruce.com
 * Description     : 服务器程序
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>


int main(void)
{
	int sfd,ind;
	struct sockaddr_in addr;
	struct sockaddr_in client;
	socklen_t len;	
	char buf[1024];
	char *pMyAddr = "192.168.8.111";
	int pid;
	
	addr.sin_family = AF_INET;
	addr.sin_port = htons(5050);
	addr.sin_addr.s_addr = inet_addr(pMyAddr);

	printf("socket start\n");
	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sfd < 0)
	{
		printf("socket error\n");
		return -1;	
	}

	printf("bind start\n");
	if (bind(sfd, (struct sockaddr *)&addr, sizeof(struct sockaddr)) < 0)
	{
		printf("bind error\n");
		return -1;
	}

	printf("listen start\n");
	if (listen(sfd, 1024) < 0)
	{
		printf("listen error\n");
		return -1;
	}

	for (;;)
	{
		printf("accept start\n");
		memset(&client, 0, sizeof(client));
		len = sizeof(client);
		ind = accept(sfd, (struct sockaddr *)&client, &len);
		if (ind < 0)
		{
			printf("accept error: %d\n", ind);
			return -1;
		}
		printf("in for \n");
		printf("client addr %s port %d\n", inet_ntop(AF_INET, &client.sin_addr, buf, sizeof(buf)), ntohs(client.sin_port));

		pid = fork();
		if (pid == 0)
		{
			close(sfd);
			handle(ind);
		}
		else if (pid < 0)
		{
			close(ind);
		}
		else
		{
			//do father
		}
	}
	
	return EXIT_SUCCESS;
} 

int handle(int point)
{
	int ret;
	char buf[1024];
	
	for (;;)
	{
		ret = read(point, buf, sizeof(buf));
		if (ret < 0)
		{
			printf("read error\n");
			close(point);
			return -1;
		}
		else if (ret == 0)
		{
			printf("client exit\n");
			close(point);
			return 0;
		}
	
		printf("client:%s\n", buf);
		if (strcmp("exit", buf) == 0)
		{
			printf("exit\n");
			close(point);
			return 0;
		}
	}
}


























