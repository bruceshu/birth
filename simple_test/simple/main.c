#include <stdio.h>
#include <stdbool.h>
#include "include/test.h"


int main(int argc, char *argv[])
{
	printf("Hello bruce, To be your best today!\n");
/* test NULL of ptr  */
	char acPara[10] = {'a','b'};
	char *pTmp = NULL;
	printf("the addr of acPara is:%d\n", acPara);
	printf("the addr of pTmp is:%d\n", pTmp );
	

/* test the value of bool type   */
	bool bPara1;
	bool bPara2 = true;
	printf("the default value of bool type is:%d\n", bPara1);
	printf("the value of true is:%d\n", bPara2);


/*	if (TEST_GOTO)
	{
		printf("the TEST_GOTO is false\n");
		goto test;
	}
*/

/* test goto syntax */
test:
	printf("test goto syntax:test\n");
	return -1;
}

