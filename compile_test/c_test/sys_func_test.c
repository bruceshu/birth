#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void test_fetch_and_add();
void test_strspn();

int main(int argc, char * argv[])
{
	test_fetch_and_add();
	test_strspn();

	return 0;
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


