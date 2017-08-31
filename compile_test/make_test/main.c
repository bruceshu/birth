#include <stdio.h>
#include "print.h"

int main(int argc, char *argv[])
{
	int num;

	printf("please input the number of multiplication table:");
	scanf("%d", &num);

	print_multi_table(num);
	
	return 0;
}
