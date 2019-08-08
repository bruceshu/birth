#include <stdio.h>

//close()
#include <unistd.h>

//open()
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//exit()
#include <stdlib.h>

//strlen()
#include <string.h>

#include <pthread.h>

#include "./../log_system/log.h"

#define LOG_FILE_MODULE 1
#define FILENAME "test"

void *write_buf(void *para);
void *get_file_size(void *para);

char *buf = "bruceshu";

void main(int argc, char *argv[])
{
	pthread_t write_t;
	pthread_t get_size_t;

	pthread_create(&write_t, NULL, write_buf, NULL);	
	sleep(3);
	pthread_create(&get_size_t, NULL, get_file_size, NULL);	

	int fd = open(FILENAME, O_RDONLY, 0666);
	if (fd < 0) {
		debug_log(LOG_FILE_MODULE, LOG_INFO, "open file failed!\n");
		exit(1);
	}	
	debug_log(LOG_FILE_MODULE, LOG_INFO, "fd is %d\n", fd);
	
	pthread_join(write_t, NULL);
	pthread_join(get_size_t, NULL);
}

void *write_buf(void *para)
{
	int fd1 = open(FILENAME, O_WRONLY|O_CREAT|O_TRUNC, 0666);
        if (fd1 < 0) {
                debug_log(LOG_FILE_MODULE, LOG_INFO,"open test file failed!\n");
                exit(1);
        }
        debug_log(LOG_FILE_MODULE, LOG_INFO, "fd1 is %d\n", fd1);

	while(1) {
		write(fd1, buf, strlen(buf));
		sync();
		sleep(1);
	}
}

void *get_file_size(void *para)
{
	int ret;
	int fd2 = open(FILENAME, O_RDONLY, 0666);
        if (fd2 < 0 ) {
                debug_log(LOG_FILE_MODULE, LOG_INFO, "open test file failed!\n");
                exit(1);
        }
        debug_log(LOG_FILE_MODULE, LOG_INFO, "fd2 is %d\n", fd2);

	while(1) {
		struct stat st;
		ret = fstat(fd2, &st);	
		if (ret < 0) {
			debug_log(LOG_FILE_MODULE, LOG_INFO, "excute fstat failed!\n");
		}
		debug_log(LOG_FILE_MODULE, LOG_INFO, "st.st_size is %d\n", st.st_size);
        debug_log(LOG_FILE_MODULE, LOG_INFO, "st.st_mode is 0x%04x\n", st.st_mode);
		sleep(2);
	}
}

