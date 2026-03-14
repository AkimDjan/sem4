#include "some.h"

void fun1(const Some& a) {
    std::cout<<a.Do(6)<<"\n"; //вызовет const
}

void fun2(Some a) {
    std::cout<<a.Do(6)<<"\n"; //вызовет не const
}

int main() {return 0;}
