/**
一个优秀的 hash 算法，将能实现：
正向快速：给定明文和 hash 算法，在有限时间和有限资源内能计算出 hash 值。
逆向困难：给定（若干） hash 值，在有限时间内很难（基本不可能）逆推出明文。
输入敏感：原始输入信息修改一点信息，产生的 hash 值看起来应该都有很大不同。
冲突避免：很难找到两段内容不同的明文，使得它们的 hash 值一致（发生冲突）。即对于任意两个不同的数据块，
        其hash值相同的可能性极小；对于一个给定的数据块，找到和它hash值相同的数据块极为困难。

当前应用最多的hash算法有MD5、SHA-1、SHA-2。
SHA-1基本上已经没有安全性了，2017年2 月 23 日谷歌宣布实现了 SHA-1 碰撞。
详细可查看https://www.zhihu.com/question/56234281/answer/148349930

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

// 原始的散列函数构造
int hash_one(int n)
{
    return n % 8;
}

// 对原始的散列函数增加简单的运算
int hash_two(int n)
{
    return (n % 8) ^ 5;
}

// 对原始的散列函数增加复杂一点的运算
int hash_three(int n)
{
    return ((n + 2 + (n << 1)) % 8) ^ 5;
}
