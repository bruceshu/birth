#include <iostream>
using namespace std;


class A {
private:
    int a1;
protected:
    int a2;
public:
    int a3;
    
    A() {
        a1 = 1;
        a2 = 2;
        a3 = 3;
    }
    
    void fun() {
        cout << "a1:" << a1 << endl;
        cout << "a2:" << a2 << endl;
        cout << "a3:" << a3 << endl;
    }
};

class B:public A {
public:
    int b1;
    
    B(int n){
        A();
        b1 = n;
    }
    
    void fun() {
        cout<< "B b1:" << b1 << endl;
//        cout<< "a1:" << a1 << endl;  //错误，a1是父类私有成员，子类不能访问
        cout<< "B a2:" << a2 << endl;
        cout<< "B a3:" << a3 << endl;
    }
};

class C:protected A {
public:
    C() {
        A();
    }
    
    void fun() {
//        cout<< "C a1:" << a1 << endl;  //错误，a1是父类私有成员，子类不能访问
        cout<< "C a2:" << a2 << endl;
        cout<< "C a3:" << a3 << endl;
    }
};

class D:private A {
public:
    D() {
        A();
    }
    
    void fun() {
//        cout<< "D a1:" << a1 << endl;  //错误，a1是父类私有成员，子类不能访问
        cout<< "D a2:" << a2 << endl;
        cout<< "D a3:" << a3 << endl;
    }
};

int main() {
    B b(10);
    C *c = new C;
    D *d = new D();
    
    b.fun();
    c->fun();
    d->fun();
    
    cout << "B:" << b.b1 << endl;
//    cout << b.a1 << endl;  //错误，a1是私有成员，外部不能访问
//    cout << b.a2 << endl;  //错误，a2是保护成员，外部不能访问
    cout << "B:" << b.a3 << endl;
    
//    cout << "C:" << c->a1 << endl;  //错误，a1是私有成员，外部不能访问
//    cout << "C:" << c->a2 << endl;  //错误，a2是保护成员，外部不能访问
//    cout << "C:" << c->a3 << endl;  //错误，a3是保护成员，外部不能访问
}
