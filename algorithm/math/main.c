


#include <stdio.h>

#include "fac.h"

void menu()
{
	printf("========================\n");
	printf("1: fac(n) = A! = ?\n");
	printf("========================\n");
}

int main(int argc, char *argv[])
{
	int select;
	int num;
	int result;

	menu();
	printf("pls input your select:");
	scanf("%d", &select);
	printf("your select is %d\n", select);

	switch(select)
	{
		case 1:
			printf("pls input your num:");
			scanf("%d", &num);
			printf("your input num is %d\n", num);
			result = fac(num);
			printf("fac(%d)=%d\n", num, result);
			break;
		default:
			printf("your select is illegal\n");
			break;

	}

	return 0;
}

