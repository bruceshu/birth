#include <iostream>
using namespace std;

class A
{
public:
	void printA()
	{
		cout << "hello bruce\n";
	};
};

int main(int argc, char *argv[])
{
	A *a = new A;
	a->printA();

	free(a);//free 释放编译出错
	// delete a; // delete 释放正常

	return 0;
}
