#include <iostream>
using namespace std;

class A {
public:
//    char a;
    virtual void func1() {
        cout << "A:" << endl;
    }
};

class B:public A {
public:
    virtual void func1()  {
        cout << "B:" << endl;
    }
    
private:
    int b;
    int c;
    static int d;
};

int main() {
    A a;
    B b;
//    cout << (int)(a.fun()) << endl;
//    cout << b.fun() << endl;
//    cout << c.fun() << endl;
    
    cout << sizeof(a) << endl;
    cout << sizeof(b) << endl;

}
