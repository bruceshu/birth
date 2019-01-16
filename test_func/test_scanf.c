#include <stdio.h>

int main()
{
    int a;
    char b;
    char c;

    printf("pls input a number\n");
    scanf("%d", &a);
    scanf("%c", &b);
    scanf("%c", &c);

    // 测试从控制台敲“enter”，在scanf中如何处理
    printf("a = %d\n", a);
    printf("b = %c\n", b);    
    printf("b = %c\n", c);
    return 0;
}
