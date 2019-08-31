
#include <stdio.h>

#include "base64.h"

#define LEN 1024

int main(int argc, char *argv[])
{
    char *str = "coship.shuhuan.bruce";
    int len = strlen(str);
    char des[LEN];

    encodeBase64(str, len, des, LEN);
    printf("%s encoded with base64 is %s\n", str, des);
    
	return 0;
}
