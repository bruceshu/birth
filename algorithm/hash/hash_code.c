/**
һ������� hash �㷨������ʵ�֣�
������٣��������ĺ� hash �㷨��������ʱ���������Դ���ܼ���� hash ֵ��
�������ѣ����������ɣ� hash ֵ��������ʱ���ں��ѣ����������ܣ����Ƴ����ġ�
�������У�ԭʼ������Ϣ�޸�һ����Ϣ�������� hash ֵ������Ӧ�ö��кܴ�ͬ��
��ͻ���⣺�����ҵ��������ݲ�ͬ�����ģ�ʹ�����ǵ� hash ֵһ�£�������ͻ��������������������ͬ�����ݿ飬
        ��hashֵ��ͬ�Ŀ����Լ�С������һ�����������ݿ飬�ҵ�����hashֵ��ͬ�����ݿ鼫Ϊ���ѡ�

��ǰӦ������hash�㷨��MD5��SHA-1��SHA-2��
SHA-1�������Ѿ�û�а�ȫ���ˣ�2017��2 �� 23 �չȸ�����ʵ���� SHA-1 ��ײ��
��ϸ�ɲ鿴https://www.zhihu.com/question/56234281/answer/148349930

*/


#include <stdio.h>
#include <string.h>

#define MOD 4096

unsigned int debug_hash_string(char *pStr)
{
	unsigned int len;
    unsigned int step;
    unsigned int hash;

	if(!pStr)
	{
		return 0;
	}

	len = strlen(pStr);
    if (len >= 32)
    {
        printf("string is over 32s char\n");
        return 0;
    }

    step = (len >> 5) + 1;
    hash = len;
    for (int i=len; i>=step; i-=step)
    {
        hash = hash ^ ((hash << 5) + (hash >> 2) + (unsigned int)pStr[i-1]);
    }

    return hash % MOD;
}

unsigned int jdk_hash_code(char *pStr)
{
    unsigned int result = 0;
    int len;

    if (!pStr)
    {
        return 0;
    }

    len = strlen(pStr);
    for (int i=0; i<len; i++)
    {
        result = result*31 + (unsigned int)pStr[i];
    }

    return result % MOD;
}

// ԭʼ��ɢ�к�������
int hash_one(int n)
{
    return n % 8;
}

// ��ԭʼ��ɢ�к������Ӽ򵥵�����
int hash_two(int n)
{
    return (n % 8) ^ 5;
}

// ��ԭʼ��ɢ�к������Ӹ���һ�������
int hash_three(int n)
{
    return ((n + 2 + (n << 1)) % 8) ^ 5;
}
