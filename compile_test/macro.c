#include <stdio.h>

int main(int argc, char *argv[])
{
	printf("%s ", __FILE__);
	printf("%s ", __func__);
	rintf("%d\n", __LINE__);
	printf("__GNUC__=%d\n", __GNUC__);
	printf("__cplusplus=%d\n", __cplusplus);
	return 0;
}
