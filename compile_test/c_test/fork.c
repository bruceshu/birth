#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

int main()
{
    char *p = (char *)malloc(100);
    pid_t pid = fork();
    if(pid== 0) {
        p[0] = 0x12;
        printf("我是爹的儿子\n");
    }

    printf("%x\n", (unsigned int)p);
    printf("%d\n", (int) p[0]);
    return 0;
}
