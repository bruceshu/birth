#include "3des.h"
#include "http.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	int ret = OK;
	const char *url = "http://192.168.8.111/test.srt";
	char *pSubtitleText;
	
	ret = http_connect(url, &pSubtitleText);
	if (ret != OK)
	{
		return ERROR;
	}
	
	printf("the length of SubtitleText is :%d\n", strlen(pSubtitleText));

	const char *key = "123456123456123456123456";
	char *in = pSubtitleText;
	//char *in = "qwertyuiopasdfghjklzxcvbnm[];',./qwertyuiop";
	//char data[100] = {0};
	//char out[100] = {0};
	//char back[100] = {0};

	char *data = malloc(strlen(in)+8);
	if (data == NULL)
	{
		return ERROR;
	}

	char *out = malloc(strlen(in)+8);
	if (out == NULL)
	{
		return ERROR;
	}

	char *back = malloc(strlen(in)+8);
	if (back == NULL)
	{
		return ERROR;
	}

	int outSize = 0;
	char iv[8] = {0};

	//printf("bruce >>>>>> before encrypt string is:%s\n", in);
	printf("bruce >>>>>> length of in is:%d\n", strlen(in));
	
	RunPad(PAD_PKCS_7, in, strlen(in), data, &outSize);
	//printf("bruce >>>>>> after pad string is:%s\nstrlen(data)=%d\n", data, outSize);
	
	ret = Run3Des(ENCRYPT, ECB, data, outSize, key, strlen(key), out, outSize, iv);
	if (ret != 1)
	{
		printf("bruce >>>>>> encrypt failed\n");
	}
	printf("bruce >>>>>> after encrypt string is:%s\n", out);

	ret = Run3Des(DECRYPT, ECB, out, outSize, key, strlen(key), back, outSize, iv);
	if (ret != 1)
	{
		printf("bruce >>>>>> decrypt failed\n");
	}
	//printf("bruce >>>>>> after decrypt string is:%s\n", back);

	int size = RunRsm(back);
	//printf("bruce >>>>>> size = %d\n", size);
	printf("bruce >>>>>> after rsm string is:%s\n", back);
	
	free(pSubtitleText);
	free(data);
	free(out);
	free(back);
	pSubtitleText = NULL;
	in = NULL;
	data = NULL;
	out = NULL;
	back = NULL;

	return ret;
}
