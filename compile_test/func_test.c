#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stddef.h>

int generat_random()
{
	//srand((unsigned)time(NULL));
	for (int i = 0; i < 5; i++) {
		printf("bruce >>> the %d time rand ---> %d\n", i, rand());
	}
	return rand();
}

int main(int argc, char *argv[])
{
	int random_num;

	random_num = generat_random();
	printf("bruce >>> random_num = %d\n", random_num);

	random_num = generat_random();
	printf("bruce >>> random_num = %d\n", random_num);
}
