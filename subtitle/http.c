#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include "typedef.h"

int http_connect(const char* path, char **ppSubtitleText)
{ 
    int sockfd;
    int len; 
    struct sockaddr_in address; 
    int result; 
    char httpstring[100]; 
    char *pTmp;
    char *pSubtitlePath = "/test.srt";
    int realloc_count = 0;

    if (!ppSubtitleText)
    {
	return ERR_PTR_NULL;
    }
	

    sprintf(httpstring,"GET %s HTTP/1.1\r\n"
          "Host: %s\r\n"
          "Connection: Close\r\n\r\n",pSubtitlePath, path); 
    char ch; 
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = inet_addr(path); 
    address.sin_port = htons(80); 
    len = sizeof(address);

    result = connect(sockfd,(struct sockaddr *)&address,len); 
    if(result == -1)
    { 
       perror("oops: client"); 
       return -1; 
    }
    printf("connect successful\n");
    
    result = send(sockfd,httpstring,strlen(httpstring),0); 
    if (result < 0)
    {
	printf("send failed!\n");
	return -1;
    }    
    printf("write successful\n");	    

    pTmp = (char *)malloc(SUBTITLE_TEXT_SIZE);
    if (pTmp == NULL)
    {
	printf("malloc failed\n");
	return ERR_MALLOC_FAILED;
    }
    
    int nNetTimeOut = NET_TIMEOUT;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&nNetTimeOut, sizeof(int));
    
    char *pTmpIndex = pTmp;
    while(recv(sockfd, &ch, 1, 0))
    { 
        printf("%c", ch); 
	*pTmpIndex  = ch;
        pTmpIndex++;
	
	if (strlen(pTmp) % (SUBTITLE_TEXT_SIZE - 1) == 0)
	{        
	    realloc_count++;
	    pTmp = (char *)realloc(pTmp, SUBTITLE_TEXT_SIZE + (SUBTITLE_TEXT_SIZE - 1) * realloc_count);
	    if (pTmp == NULL)
	    {
		printf("realloc failed\n");
		return ERR_MALLOC_FAILED;
	    }

	    pTmpIndex = pTmp + (SUBTITLE_TEXT_SIZE - 1) * realloc_count;
	}
    }

    *ppSubtitleText = pTmp;
    close(sockfd); 

    return OK; 
}

