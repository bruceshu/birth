#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
	const char *ip = "192.168.8.111";
	char *pSubtitleText;
	
	(void)http_connect(ip, &pSubtitleText);
	
//	printf("the pSubtitleText is :\n%s\n", pSubtitleText);
	printf("the length of SubtitleText is :%d\n", strlen(pSubtitleText));

	return 0;
}





