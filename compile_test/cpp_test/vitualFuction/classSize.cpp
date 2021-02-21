#include <iostream>
using namespace std;

// #pragma pack(4)

class A {
public:
    char a;
    virtual void func1() {
        cout << "A:" << endl;
    }

    virtual void fun2() {

    }

    virtual void fun3(){}
};

class B:public A {
public:
    virtual void func1()  {
        cout << "B:" << endl;
    }
    
private:
    int b;
    static int bb;
};

class C {
    virtual void fun1() {}
};

class D {
    char ad;
    int d;
    static int dd;
};

class E {
public:
    E(){}
    ~E(){}
    void fun1();
    void fun2(){}
    void fun3() {
        cout << "hello" << endl;
    }
};

int main() {
    A a;
    B b;
    C c;
    D d;
    E e;
    
    cout << "a:mutil function " << sizeof(a) << endl;
    cout << "b:extension " << sizeof(b) << endl;
    cout << "c:virtual " << sizeof(c) << endl;
    cout << "d:static " << sizeof(d) << endl;
    cout << "e:null class " << sizeof(e) << endl;
}
