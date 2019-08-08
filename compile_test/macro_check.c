#include <stdio.h>
#include <stdlib.h>
#define __cplusplus 1

#define CONNECT(a, b) a##b
#define STR(a) #a
#define TEST(a) STR(a)

int main(int argc, char *argv[])
{
	printf("%s:%s:%d\n ", __FILE__, __func__, __LINE__);
	printf("__GNUC__=%d\n", __GNUC__);

	printf("getenv FFREPROT addr=%p\n", getenv("FFREPORT"));
	printf("getenv FFREPORT=%s\n", getenv("FFREPORT"));		
	printf("getenv PATH addr=%p\n", getenv("PATH"));
	printf("getenv=%s\n", getenv("PATH"));

	printf("c:%d, %x, %X\n", 'c', 'c', 'c');
	
	printf("STR(CONNECT(con, 2))=%s\n", STR(CONNECT(con, 2)));
	printf("TEST(CONNECT(con, 2))=%s\n", TEST(CONNECT(con, 2)));
	
	return 0;
}
