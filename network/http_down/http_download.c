#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>

#include "http_download.h"

struct resp_header resp;//全剧变量以便在多个进程中使用

void parse_url(const char *url, char *domain, int *port, char *file_name)
{
    /*通过url解析出域名, 端口, 以及文件名*/
    int j = 0;
    int start = 0;
    *port = 80;
    char *patterns[] = {"http://", "https://", NULL};

    for (int i = 0; patterns[i]; i++)
        if (strncmp(url, patterns[i], strlen(patterns[i])) == 0)
        {
            start = strlen(patterns[i]);
            break;
        }

    //解析域名, 保留域名后面的端口号
    for (int i = start; url[i] != '/' && url[i] != '\0'; i++, j++)
        domain[j] = url[i];
    domain[j] = '\0';

    //解析端口号, 如果没有, 那么设置端口为80
    char *pos = strstr(domain, ":");
    if (pos)
        sscanf(pos, ":%d", port);

    //删除域名端口号
    for (int i = 0; i < (int)strlen(domain); i++)
    {
        if (domain[i] == ':')
        {
            domain[i] = '\0';
            break;
        }
    }

    //获取下载文件名
    j = 0;
    for (int i = start; url[i] != '\0'; i++)
    {
        if (url[i] == '/')
        {
            if (i !=  strlen(url) - 1)
                j = 0;
            continue;
        }
        else
            file_name[j++] = url[i];
    }
    file_name[j] = '\0';
}

void get_ip_addr(const char *domain, char *ip_addr)
{
    /*通过域名得到相应的ip地址*/
    struct hostent *host = gethostbyname(domain);
    if (!host)
    {
        ip_addr = NULL;
        return;
    }

    for (int i = 0; host->h_addr_list[i]; i++)
    {
        strcpy(ip_addr, inet_ntoa( * (struct in_addr*) host->h_addr_list[i]));
        break;
    }
}

struct resp_header get_resp_header(const char *response)
{
    /*获取响应头的信息*/
    struct resp_header resp;

    char *pos = strstr(response, "HTTP/");
    if (pos)
        sscanf(pos, "%*s %d", &resp.status_code);//返回状态码

    pos = strstr(response, "Content-Type:");//返回内容类型
    if (pos)
        sscanf(pos, "%*s %s", resp.content_type);

    pos = strstr(response, "Content-Length:");//内容的长度(字节)
    if (pos)
        sscanf(pos, "%*s %ld", &resp.content_length);

    return resp;
}

/*用于显示下载进度条*/
static void progressBar(long cur_size, long total_size)
{
    float percent = (float) cur_size / total_size;
    const int numTotal = 50;
    int numShow = (int)(numTotal * percent);

    if (numShow == 0)
        numShow = 1;

    if (numShow > numTotal)
        numShow = numTotal;

    char sign[51] = {0};
    memset(sign, '=', numTotal);

    printf("\r%.2f%%\t[%-*.*s] %.2f/%.2fMB", percent * 100, numTotal, numShow, sign, cur_size / 1024.0 / 1024.0, total_size / 1024.0 / 1024.0);
    fflush(stdout);

    if (numShow == numTotal)
        printf("\n");
}

/*下载文件函数, 放在线程中执行*/
void * download(void * socket_d)
{
    int client_socket = *(int *) socket_d;
    int length = 0;
    int mem_size = 4096;//mem_size might be enlarge, so reset it
    int buf_len = mem_size;//read 4k each time
    int len;

    //创建文件描述符
    int fd = open(resp.file_name, O_CREAT | O_WRONLY, S_IRWXG | S_IRWXO | S_IRWXU);
    if (fd < 0)
    {
        printf("Create file failed\n"); 
        exit(0);
    }

    char *buf = (char *) malloc(mem_size * sizeof(char));

    //从套接字中读取文件流
    while ((len = read(client_socket, buf, buf_len)) != 0 && length < resp.content_length)
    {
        write(fd, buf, len);
        length += len;
        progressBar(length, resp.content_length);
    }

    if (length == resp.content_length)
        printf("Download successful ^_^\n\n");
}
