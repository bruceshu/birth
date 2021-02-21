
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
    
    ~B(){
        cout << "delete B" <<endl;
    };
};

int main()
{
    cout << "子类对象赋值给父类指针" << endl;
    A *ab = new B();
    ab->out1();
    ab->out2();
    ab->out3();
    cout << "************************" << endl;
    cout << "子类对象赋值给子类指针" << endl;
    B *b = new B;
    b->out1();
    b->out2();
    b->out3();

    delete ab;
    delete b;
    //free(ab)  //free释放的是内存空间，与malloc对应。delete首先调用对象的析构函数，然后再释放内存空间，与new对应。
    return 0;
}
