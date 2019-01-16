#include <stdio.h>

int main()
{
    int a;
    char b;
    char c;
    char d;

    printf("pls input a number\n");
    scanf("%d", &a);
    scanf("%c", &b); //jie shou \n
    scanf("%c", &c);
    scanf("%c", &d);

    // 测试从控制台敲“enter”，在scanf中如何处理
    printf("a = %d\n", a);
    printf("b = 0X%02x\n", b);    
    printf("c = 0X%02x\n", c);
    printf("d = 0X%02x\n", d);
    return 0;
}
