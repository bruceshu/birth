

#include <stdio.h>


int GetProgramBits()
{
	return sizeof(long long int)*8;
}

int main(int argc, char *argv[])
{
	printf("the program bits is %d\n", GetProgramBits());
	return 0;
}
