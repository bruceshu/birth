#include <stdio.h>
//#include <atomic.h>

int main(int argc, char * argv[])
{
	int operand = 10;
	int result = 0;

	result = __sync_fetch_and_add(&operand, 1);

	printf("result = %d\n", result);
	printf("operand = %d\n", operand);

	return 0;
}
