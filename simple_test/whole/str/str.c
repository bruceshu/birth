
#include "include/typedef.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int str_get(char **ppcPara)
{
    int ret = OK;
    char *str;

    str = (char *)malloc(CHAR_MAX * sizeof(char));
    if (str == NULL)
    {
	printf("malloc memory failed\n");
	return ERR;
    }

    printf("please input string:");
    scanf("%s",str);
    if (strlen(str) == 0)
    {
        printf("your input string is null\n");
	return ERR;
    }

    *ppcPara = str;
    return ret;
}
