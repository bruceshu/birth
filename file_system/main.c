#include <stdio.h>







void download_thread(char *url, char *fileName)
{
	pthread_t download;
	int ret;
	int fd;
	
	char ip[16] = {0};
	short port = 80
	char fileNameTmp[256] = {0};

	ret = parse_url(url, &domain, &port, fileNameTmp);
	if (ret != OK) {
		printf("parse url failed!\n");		
		return;
	}

	ret = get_ip_addr(domain, ip);
	if (ret != OK) {
		printf("get ip addr failed!\n");
		return;	
	}

	network_connect(ip, port, :);
	fd = open(fileName, O_WRONLY|O_CREAT_OTRUNC, 0666);
	if (fd < 0) {
		printf("open file failed!\n");
		return;	
	}

	ret = pthread_create(&download, );
}


void main (int argc, char *argv[])
{
	if (argc != 2) {
		printf("the num of para is wrong,you should input two para!");
	}

	char *url = strdup(argv[1]);
	char fileName[256] = {0};

	download_thread(url, fileName);

	sleep(2);//sleep 单位是秒

	int fd = open(fileName, "rb");
	if (fd < 0) {
		stop_download();
		return;
	}

	int i = 0;
	while(i < 10) {
		get_file_size(fd);
		i++;
		sleep(1);
	}
}
