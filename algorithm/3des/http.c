#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>

#include "http.h"

static char * ffp_get_path(const char *URL)
{
    char *path = strdup(URL);

    path = strchr(path, ':');
    if (path == NULL)
    {
        return NULL;
    }
    
    path+=3;
    printf("bruce >>>>>>>> path=%s\n", path);    
    return path;
}

static char * ffp_get_hostname(const char *path)
{
    char *pHostName = strdup(path);
    char *tmp = NULL;

    tmp = strchr(pHostName, '/');
    if (tmp == NULL)
    {
        return NULL;
    }
    *tmp = '\0';
    
    printf("bruce >>>>>>>> pHostName=%s\n", pHostName);
    return pHostName;
}

static char * ffp_get_ip_from_host(const char *pHostName)
{
    char *ip = strdup(pHostName);
    char *tmp = NULL;
    
    tmp = strchr(ip, ':');
    if (tmp == NULL)
    {
        return NULL;
    }
    *tmp = '\0';

    printf("bruce >>>>>>>> ip=%s", ip);
    return ip;
}

static char * ffp_get_port_from_host(const char *pHostName)
{
    char *port = strdup(pHostName);
    
    port = strchr(port, ':');
    if (port == NULL)
    {
        return NULL;
    }
    port+=1;

    printf("bruce >>>>>>>> port=%s", port);
    return port;
}

static char * ffp_get_ip_from_hostname(const char *pHostName)
{
    struct hostent *hptr;
    const void *addr = NULL;
    char str[32];
    char *ip = NULL;

    if (pHostName == NULL)
    {
	return NULL;
    }

    hptr = gethostbyname(pHostName);
    if (hptr == NULL)
    {
    	return NULL;
    }
    
    addr = inet_ntop(hptr->h_addrtype, (void *)hptr->h_addr, str, sizeof(str));
    if (addr == NULL)
    {
        return NULL;
    }

    ip = strdup(str);
    printf("bruce >>>>>> ip=%s\n", ip);
    return ip;
}

static char * ffp_get_subtitle_path(const char *path)
{
    char *pSubPath = strdup(path);

    pSubPath = strchr(pSubPath, '/');
    if (pSubPath == NULL)
    {
        return NULL;
    }

    printf("bruce >>>>>> pSubPath=%s\n", pSubPath);
    return pSubPath;
}

static int get_ip_and_port(const char *URL, AccessHttp *ffp)
{
    char *path = NULL;
    char *pHostName = NULL;
    char *ip = NULL;
    char *port = NULL;
    char *pSubPath = NULL;

    path = ffp_get_path(URL);
    if (path == NULL)
    {
        return ERROR;
    }

    pHostName = ffp_get_hostname(path);
    if (pHostName == NULL)
    {
        return ERROR;
    }

    if (strchr(pHostName, ':') != NULL)
    {
        ip = ffp_get_ip_from_host(pHostName);
        if (pHostName == NULL)
        {
            return ERROR;
        }

        port = ffp_get_port_from_host(pHostName);
        if (pHostName == NULL)
        {
            return ERROR;
        }
    }
    else
    {
        ip = ffp_get_ip_from_hostname(pHostName);
        if (ip == NULL)
        {
            return ERROR;
        }
    }

    pSubPath = ffp_get_subtitle_path(path);
    if (pSubPath == NULL)
    {
        return ERROR;
    }

    free(path);
    free(pHostName);
    path = NULL;
    pHostName = NULL;

    ffp->pSubIP = ip;
    ffp->pSubPort = port;
    ffp->pSubPath = pSubPath;
	
    return OK;
}


int http_connect(const char* path, char **ppSubtitleText)
{
    struct sockaddr_in address;
    AccessHttp stHttp;    
    int sockfd;
    int len; 
    int result;
    int realloc_count = 0;
    int port = 80;
    char httpstring[100];
    char *pTmp;
    
    if (!ppSubtitleText)
    {
	return ERROR;
    }

    result = get_ip_and_port(path, &stHttp);
    if (result != OK)
    {
	return ERROR;
    }

    if (stHttp.pSubPort != NULL)
    {
	port = atoi(stHttp.pSubPort);
    }

    sprintf(httpstring,"GET %s HTTP/1.1\r\n"
          "Host: %s:%d\r\n"
          "Connection: Close\r\n\r\n",stHttp.pSubPath, stHttp.pSubIP, port); 
 
    printf("bruce >>>>>> httpstring:%s\n", httpstring);   
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
	return ERROR;
    }
    
    int nNetTimeOut = 1000;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&nNetTimeOut, sizeof(int));
    
    char *pTmpIndex = pTmp;
    while(recv(sockfd, &ch, 1, 0))
    { 
	*pTmpIndex  = ch;
        pTmpIndex++;
	
	if (strlen(pTmp) % (SUBTITLE_TEXT_SIZE - 1) == 0)
	{
	    realloc_count++;
	    pTmp = (char *)realloc(pTmp, SUBTITLE_TEXT_SIZE + (SUBTITLE_TEXT_SIZE - 1) * realloc_count);
	    if (pTmp == NULL)
	    {
		printf("realloc failed\n");
		return ERROR;
	    }

	    pTmpIndex = pTmp + (SUBTITLE_TEXT_SIZE - 1) * realloc_count;
	}
    }

    *ppSubtitleText = pTmp;
    close(sockfd); 

    return OK; 
}

