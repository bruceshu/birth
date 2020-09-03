
#include <iostream>
using namespace std;

class A
{
public:
    A() {
        cout << "create A" << endl;
    }
    virtual void out1() = 0; ///由子类实现
    virtual void out2() ///默认实现
    {
        cout << "A(out2)" << endl;
    }
    void out3() ///强制实现
    {
        cout << "A(out3)" << endl;
    }
    virtual ~A(){
        cout << "delete A" << endl;
    };
};

class B : public A
{
public:
    B() {
        cout << "create B" << endl;
    }
    void out1()
    {
        cout << "B(out1)" << endl;
    }
    void out2()
    {
        cout << "B(out2)" << endl;
    }
    void out3()
    {
        cout << "B(out3)" << endl;
    }
    virtual ~B(){
        cout << "delete B" <<endl;
    };
};

int main()
{
    A *ab = new B();
    ab->out1();
    ab->out2();
    ab->out3();
    cout << "************************" << endl;
    B *bb = new B;
    bb->out1();
    bb->out2();
    bb->out3();

    delete ab;
    delete bb;
    return 0;
}
