#include "class_test.h"

int main(int argc, char *argv[])
{
	A *a = new A;
	a->printA();
	
	//free(a);//free 释放编译出错
	delete a; // delete 释放正常

	return 0;
}
