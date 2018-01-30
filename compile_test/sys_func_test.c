#include <stdio.h>
#include <string.h>

void test1();
void test2();

int main(int argc, char * argv[])
{
//	test1();

	test2();

	return 0;
}

/* 测试原子函数 */
void test1()
{
	int operand = 10;
	int result = 0;

	result = __sync_fetch_and_add(&operand, 1);
	printf("result = %d\n", result);
	printf("operand = %d\n", operand);
}

void test2()
{
	char *str1 = "abcdefghijklmnopqrstuvwxyz";
	char *str2 = "shuhuan";
	char *str3 = "123456";
	char *str4 = "shu123huan";

	int result = 0;

	result = strspn(str2, str1);
        printf("the result of strspn is %d\n", result);

        result = strspn(str3, str1);
        printf("the result of strspn is %d\n", result);

        result = strspn(str4, str1);
        printf("the result of strspn is %d\n", result);

        result = strcspn(str1, str3);
        printf("the result of strcspn is %d\n", result);
}