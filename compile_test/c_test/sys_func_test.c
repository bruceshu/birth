#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void help();
void test_fetch_and_add();
void test_strspn();
void test_setvbuf();

int main(int argc, char * argv[])
{
    int select;

    help();
    printf("which one do you want to test:");
    scanf("%d", &select);

    switch ()
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
            test_fetch_and_add();
            test_strspn();
            test_setvbuf();
            break;
        default:
            printf("your select is error!\n");
            break;
            
    }
    
    return 0;
}

void help()
{
    printf("======== 1、test_fetch_and_add =========\n");
    printf("======== 2、test_strspn =========\n");
    printf("======== 3、test_setvbuf =========\n");
    printf("======== 4、test all function =========\n");
}

void test_fetch_and_add()
{
    int operand = 10;
    int result = 0;

    printf("before execute __sync_fetch_and_add. operand=%d\n", operand);
    result = __sync_fetch_and_add(&operand, 1);
    printf("result = %d\n", result);
    printf("after execute __sync_fetch_and_add. operand = %d\n", operand);
    printf("\n");
}

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

void test_setvbuf()
{
    char buff[1024];

    memset(buff, 0, sizeof(buff));

    fprintf(stdout, "启用全缓存_IOFBF\n");
    setvbuf(stdout, buff, _IOFBF, sizeof(buff));
    
    fprintf(stdout, "hello,my name is shuhuan\n");
    fprintf(stdout, "output into the buff\n");
    fflush(stdout);

    fprintf(stdout, "the fflush is called\n");
    fprintf(stdout, "it will sleep 5 seconds next\n");

    sleep(5);
}
