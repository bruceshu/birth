#include <stdlib.h>


#include <stdio.h>

void swap(int *, int *);

int main(int argc, char *argv[])
{
	int a = 10;
	int b = 12;
	printf("before execute swap. a=%d,b=%d\n", a, b);

	swap(&a, &b);
	printf("after execute swap. a=%d,b=%d\n", a, b);
}


void swap(int *a, int *b)
{
        *a = *a + *b;
        *b = *a - *b;
        *a = *a - *b;
}
  
