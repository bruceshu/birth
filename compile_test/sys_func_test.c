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

	int result = 0;

	result = strspn(str1, str3);
	printf("the result of strspn is %d\n", result);

	result = strcspn(str1, str3);
	printf("the result of strcspn is %d\n", result);
}
