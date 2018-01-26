#include <stdio.h>

#include "fun1.h"

void fun3()
{
	printf("this is fun3\n");
	printf("fun3 called fun1 begin\n");
	fun1();
	printf("fun3 called fun1 end\n");
}
