#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_multi_table(int n)
{
	if (n < 1 || n > 9)
	{
		return;
	}

	for (int i=1; i<=n; i++)
	{
		for (int j=1; j<=i; j++)
		{
			printf("%d*%d=%-2d ", j, i, i*j);
		}
		printf("\n");
	}
}

