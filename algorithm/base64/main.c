
#include <stdio.h>
#include <string.h>

#include "base64.h"

#define LEN 1024

int main(int argc, char *argv[])
{
    char *str = "coship.shuhuan.bruce";
    unsigned int len = strlen(str);
    char out[LEN] = {0};
    int out_len = LEN;
    int ret;
    
    ret = Base64Encode(str, len, out, &out_len);
    if (ret == 0)
    {
        printf("out_len=%d\n", out_len);
        printf("[%s] encoded with base64 is [%s]\n", str, out);
    }
    else
    {
        printf("input param is error\n");
    }

    char *str2 = NULL;
    str2 = malloc(len+1);
    if (!str2)
    {
        return -1;
    }

    memset(str2, 0, len+1);
    ret = Base64Decode(out, out_len, str2, len+1);
    if (ret == 0)
    {
        printf("[%s] decoded with base64 is [%s]\n", out, str2);
    }
    else
    {
        printf("decode base64 failed!\n");
    }
    
	return 0;
}
