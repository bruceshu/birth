#include <stdio.h>
#include <stdlib.h>
#define __cplusplus 1

int main(int argc, char *argv[])
{
	printf("%s:%s:%d\n ", __FILE__, __func__, __LINE__);
	printf("__GNUC__=%d\n", __GNUC__);
	printf("__cplusplus=%d\n", __cplusplus);
//	printf("%s", __VA_ARGS__);	


	printf("getenv FFREPROT addr=%p\n", getenv("FFREPORT"));
	printf("getenv FFREPORT=%s\n", getenv("FFREPORT"));		
	printf("getenv PATH addr=%p\n", getenv("PATH"));
	printf("getenv=%s\n", getenv("PATH"));
	
	return 0;
}
