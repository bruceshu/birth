
#include <stdio.h>
#include <string.h>

#include "base64.h"

#define LEN 1024

int main(int argc, char *argv[])
{
    char *str = "coship.shuhuan.bruce";
    unsigned int len = strlen(str);
    char des[LEN] = {0};
    int out_len = LEN;
    int ret;
    
    ret = Base64Encode(str, len, des, &out_len);
    if (ret == 0)
    {
        printf("out_len=%d\n", out_len);
        printf("%s encoded with base64 is %s\n", str, des);
    }
    else
    {
        printf("input param is error\n");
    }
    
	return 0;
}
