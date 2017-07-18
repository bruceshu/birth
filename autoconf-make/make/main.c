
#include "include/type_def.h"
//#include "include/str.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
	int ret = OK;

/*	for(int i = 0; i < argc; i++)
		printf("%s\n",argv[i]);
*/

	
/*	char *pcGetStr = NULL;
	ret = str_get(&pcGetStr);
	if (ret != OK)
	{
		printf("intput error\n");
		return ERR;
	}

	printf("the string is:%s\n",pcGetStr);
*/

/*	printf("%s(%d):%s\n", __FILE__, __LINE__, __FUNCTION__); 
*/
	
	ret = msg_model_test();
	if (ret != OK)
	{	
		printf("failed :msg_model_test\n");
		return ERR;
	}

	return ret;
}
