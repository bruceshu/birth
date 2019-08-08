#include <cutils/properties.h>

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	char buf[20] = {0};
	int len;
	len = property_get("ro.sen5.customer.id", buf, NULL);
	if (len > 0) {
		
		printf("get ro.sen5.customer.id successed! %s\n", buf);	
	}
	printf("get ro.sen5.customer.id failed!\n");
	
	len = property_get("ro.nes.customer.id", buf, NULL);
	if (len > 0) {
		printf("get ro.sen5.customer.id successed! %s\n", buf);
	}
	printf("get ro.sen5.customer.id failed!\n");

}
