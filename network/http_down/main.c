#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <pthread.h>

#include "http_download_api.h"

int main(int argc, char const *argv[])
{
    /*
        test url:
        1. https://nodejs.org/dist/v4.2.3/node-v4.2.3-linux-x64.tar.gz
        2. http://img.ivsky.com/img/tupian/pre/201312/04/nelumbo_nucifera-009.jpg
    */
    char url[1024] = "127.0.0.1";
    char domain[64] = {0};
    char ip_addr[16] = {0};
    int port = 80;
    char file_name[256] = {0};

    if (argc == 1)
    {
        printf("please input params\n");
        exit(0);
    }
    else
        strcpy(url, argv[1]);

    puts("1: Parsing url...");
    parse_url(url, domain, &port, file_name);

    if (argc == 3)
        strcpy(file_name, argv[2]);

    puts("2: Get ip address...");
    get_ip_addr(domain, ip_addr);
    if (strlen(ip_addr) == 0)
    {
        printf("can not get ip address\n");
        return 0;
    }

    puts("\n>>>>Detail<<<<");
    printf("URL: %s\n", url);
    printf("DOMAIN: %s\n", domain);
    printf("IP: %s\n", ip_addr);
    printf("PORT: %d\n", port);
    printf("FILENAME: %s\n\n", file_name);

    //����http����ͷ��Ϣ
    char header[1024] = {0};
    sprintf(header, \
            "GET %s HTTP/1.1\r\n"\
            "Accept:text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"\
            "User-Agent:Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537(KHTML, like Gecko) Chrome/47.0.2526Safari/537.36\r\n"\
            "Host:%s\r\n"\
            "Connection:close\r\n"\
            "\r\n"\
        ,url, domain);

    printf("the header of http is:\n%sthe length of header is:%d\n", header, (int) strlen(header));

    //�����׽���
    int client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client_socket < 0)
    {
        printf("invalid socket descriptor: %d\n", client_socket);
        exit(-1);
    }

    //������ַ�ṹ��
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip_addr);
    addr.sin_port = htons(port);

    //���ӷ�����
    puts("3: Connect server...");
    int res = connect(client_socket, (struct sockaddr *) &addr, sizeof(addr));
    if (res == -1)
    {
        printf("connect failed, return: %d\n", res);
        exit(-1);
    }

    puts("4: Send request...");//�������������������
    write(client_socket, header, strlen(header));

    int mem_size = 4096;
    int length = 0;
    int len;
    char *buf = (char *) malloc(mem_size * sizeof(char));
    char *response = (char *) malloc(mem_size * sizeof(char));

    //ÿ�ε����ַ���ȡ��Ӧͷ��Ϣ, ������ȡ������Ӧ���ֵ�ͷ��, ���浥�����߳�����
    while ((len = read(client_socket, buf, 1)) != 0)
    {
        if (length + len > mem_size)
        {
            //��̬�ڴ�����, ��Ϊ�޷�ȷ����Ӧͷ���ݳ���
            mem_size *= 2;
            char * temp = (char *) realloc(response, sizeof(char) * mem_size);
            if (temp == NULL)
            {
                printf("realloc failed\n");
                exit(-1);
            }
            response = temp;
        }

        buf[len] = '\0';
        strcat(response, buf);

        //�ҵ���Ӧͷ��ͷ����Ϣ, ����"\n\r"Ϊ�ָ��
        int flag = 0;
        for (int i = strlen(response) - 1; response[i] == '\n' || response[i] == '\r'; i--, flag++);
        if (flag == 4)
            break;

        length += len;
    }

    printf("\n>>>>Response header:<<<<\n%s", response);

    resp = get_resp_header(response);
    strcpy(resp.file_name, file_name);

    printf("5: Start thread to download...\n");
    /*���µ��߳������ļ�*/
    pthread_t download_thread;
    pthread_create(&download_thread, NULL, download, (void *) &client_socket);
    pthread_join(download_thread, NULL);
    
    return 0;
}
