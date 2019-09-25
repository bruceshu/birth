#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>  //sleep
#include <signal.h>  //signal


void help();
void test_fetch_and_add();
void test_strspn();
void test_setvbuf();
void test_signal();

int main(int argc, char * argv[])
{
    int select;

    help();
    printf("which one do you want to test:");
    scanf("%d", &select);

    switch (select)
    {
        case 1:
            test_fetch_and_add();
            break;
        case 2:
            test_strspn();
            break;
        case 3:
            test_setvbuf();
            break;
        case 4:
            test_signal();
            break;
        case 100:
            test_fetch_and_add();
            test_strspn();
            test_setvbuf();
            test_signal();
            break;
        default:
            printf("your select is error!\n");
            break;
            
    }
    
    return 0;
}

void help()
{
    printf("======== 1 test_fetch_and_add =========\n");
    printf("======== 2 test_strspn =========\n");
    printf("======== 3 test_setvbuf =========\n");
    printf("======== 4 test_signal =========\n");
    printf("======== 100 test all function =========\n");
}

/*
* 测试系统原子函数 __sync_fetch_and_add 系列函数
*/
void test_fetch_and_add()
{
    int operand = 10;
    int result = 0;

    printf("before execute __sync_fetch_and_add. operand=%d\n", operand);
    result = __sync_fetch_and_add(&operand, 1);
    printf("the result of execute __sync_fetch_and_add is:%d\n", result);
    printf("after execute __sync_fetch_and_add. operand = %d\n", operand);
    printf("\n");
}

/*
* 测试系统字符串操作函数 strspn 系列函数
*/
void test_strspn()
{
    char *str1 = "abcdefghijklmnopqrstuvwxyz";
    char *str2 = "shuhuan";
    char *str3 = "123456";
    char *str4 = "shu123huan";

    printf("str1=%s\n", str1);
    printf("str2=%s\n", str2);
    printf("str3=%s\n", str3);
    printf("str4=%s\n", str4);
 
    int result = 0;
    result = strspn(str2, str1);
        printf("the result of strspn(str2,str1) is %d\n", result);

        result = strspn(str3, str1);
        printf("the result of strspn(str3,str1) is %d\n", result);

        result = strspn(str4, str1);
        printf("the result of strspn(str4,str1) is %d\n", result);

        result = strcspn(str1, str3);
        printf("the result of strcspn(str1,str3) is %d\n", result);
    printf("\n");
}

/*
* 将文件流和缓冲关联在一起，从而可以操作缓冲区的内容和大小。
* 关联函数：setbuf(FILE *stream, char *buf); fopen("filename", "r+"); open("filename", int flag, int mode); fflush(FILE *stream);
*/
#define LOOP_NUM 10
void test_setvbuf()
{
    char buff[1024];
    int i=0;

    memset(buff, 0, sizeof(buff));

    printf("before setvbuf\n");
    while (i++ < LOOP_NUM) {
        printf("hello world!");
        sleep(1);
    }

    printf("\nafter setvbuf\n");
    fflush(stdout);

    setvbuf(stdout, buff, _IONBF, sizeof(buff));
    i = 0;
    while (i++ < LOOP_NUM) {
        printf("hello world!");
        sleep(1);
    }

    printf("buff is:%s\n", buff);


    memset(buff, 0, sizeof(buff));
    fprintf(stdout, "start line buffer\n");
    setvbuf(stdout, buff, _IOLBF, sizeof(buff));

    fprintf(stdout, "hello,my name is shuhuan.");
    fprintf(stdout, "output into the buff.");
    fflush(stdout);
    fprintf(stdout, "the fflush is called\n");
    fprintf(stdout, "it will sleep 5 seconds next\n");

    memset(buff, 0, sizeof(buff));
    fprintf(stdout, "start full buffer\n");
    setvbuf(stdout, buff, _IOFBF, sizeof(buff));

    fprintf(stdout, "hello,my name is shuhuan\n");
    fprintf(stdout, "output into the buff\n\n\n");
    fflush(stdout);
    fprintf(stdout, "the fflush is called\n");
    fprintf(stdout, "it will sleep 5 seconds next\n");
    sleep(5);
}

static void handler(int s)
{
    if (s == SIGBUS)
        printf(" now got a bus error signal\n");

    if (s == SIGSEGV)
        printf(" now got a segmentation violation signal\n");

    if (s == SIGILL)
        printf(" now got an illegal instruction signal\n");

    exit(1);
}

/*
* 测试软中断原理，signal 系列函数
*/
void test_signal()
{
    int *p=NULL;
    signal(SIGBUS, handler);
    signal(SIGSEGV, handler);
    signal(SIGILL, handler);
    *p=0;
}

