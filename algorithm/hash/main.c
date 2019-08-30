

#include <stdio.h>

#include "hash_code.h"

void debug_hash_string_test(char *str1, char *str2, char *str3)
{
    unsigned int hash_value;
    if (!str1 || !str2 || !str3)
    {
        return;
    }

    printf("===========begin %s============\n", __func__);
    hash_value = debug_hash_string(str1);
    printf("the hash value of [%s] is %d\n", str1, hash_value);
    
    hash_value = debug_hash_string(str2);    
    printf("the hash value of [%s] is %d\n", str2, hash_value);
    
    hash_value = debug_hash_string(str3);    
    printf("the hash value of [%s] is %d\n", str3, hash_value);
    
    printf("===========end %s============\n\n", __func__);
}

void jdk_hash_code_test(char *str1, char *str2, char *str3)
{
    unsigned int hash_value;
    if (!str1 || !str2 || !str3)
    {
        return;
    }

    printf("===========begin %s============\n", __func__);
    hash_value = jdk_hash_code(str1);
    printf("the hash value of [%s] is %d\n", str1, hash_value);
    
    hash_value = jdk_hash_code(str2);    
    printf("the hash value of [%s] is %d\n", str2, hash_value);
    
    hash_value = jdk_hash_code(str3);    
    printf("the hash value of [%s] is %d\n", str3, hash_value);
    
    printf("===========end %s============\n\n", __func__);
}

void hash_one_test()
{
    int hash_value;
    int j = 0;
    
    printf("===========begin %s============\n", __func__);

    for (int i=100; i<300; i++)
    {
        hash_value = hash_one(i);
        printf("%2d", hash_value);        
        j++;

        if (j % 32 == 0)
        {
            printf("\n");
        }
    }

    if (j % 32 != 0)
        printf("\n");
    
    printf("===========end %s============\n\n", __func__);
}

void hash_two_test()
{
    int hash_value;
    int j = 0;
    
    printf("===========begin %s============\n", __func__);

    for (int i=100; i<300; i++)
    {
        hash_value = hash_two(i);
        printf("%2d", hash_value);        
        j++;

        if (j % 32 == 0)
        {
            printf("\n");
        }
    }

    if (j % 32 != 0)
        printf("\n");
    
    printf("===========end %s============\n\n", __func__);
}

void hash_three_test()
{
    int hash_value;
    int j = 0;
    
    printf("===========begin %s============\n", __func__);

    for (int i=100; i<300; i++)
    {
        hash_value = hash_three(i);
        printf("%2d", hash_value);        
        j++;

        if (j % 32 == 0)
        {
            printf("\n");
        }
    }

    if (j % 32 != 0)
        printf("\n");
    
    printf("===========end %s============\n\n", __func__);
}


void simple_hash_test()
{
    hash_one_test();
    hash_two_test();
    hash_three_test();
}

int main(int argc, char *argv[])
{
	char *company = "coship";
	char *zh_name = "shuhuan";
    char *en_name = "bruce";

    debug_hash_string_test(company, zh_name, en_name);
    jdk_hash_code_test(company, zh_name, en_name);
    simple_hash_test();

	return 0;
}

