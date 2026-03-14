#pragma once //добавляет только один раз включение

#include <bits/stdc++.h>
//API = Application Programming Interface - набор сигнатур, которыми мы что-то вызываем
//ABI = Application Binary Interface 
// Соглашение о том, в каком порядке, какие регистры подаются в функцию
// при компилировании программы мы запускаем, все перестает работать/собираться
// h
template<typename T>
class C{
public:
    
    void fun();
private:
    T a;
    T b;
    T c; //<- при добавлении сюда, поменяется ли API, ABI? (API - нет, ABI - да)
};
//cpp
// void C::fun(){
//     ...;
// }
//PIMPL Pimpl — техника программирования в C++, которая позволяет отделить интерфейс класса от его реализации
// Тестовый примерчик концепции PIMPLe
class B;
class A{
    B* b;
};
B b;
class B{
    A a;
};

//pimpl.h
class SomeImpl;
class Some{
public:
    int Do(size_t i) const;
    int& Do(size_t i);
    void Inc();
private:
    std::unique_ptr<SomeImpl> pimpl_=nullptr;
};
//pimpl.cpp
//Если что-то подключени в h

//#include "p.h"

int main() {return 0;}

void fun1(const Some& a) {
    std::cout<<a.Do(6)<<"\n"; //вызовет const
}

void fun2(Some a) {
    std::cout<<a.Do(6)<<"\n"; //вызовет не const
}

//unique ptr используется для гарантии вычисления из памяти
//some.cpp
class SomeImpl{
std::vector<int> v_={3,2,1};
public:
    int Do(size_t i) const {
        i = i % 3;
        return v_[i];
    }
private:
    int Inc(){
        //напишем сами
    }
};

int Some::Do(size_t i) const {
    return pimpl_->Do(i);
}
//деструктор был создан в hнике, поэтому надо в hнике объявить в деструкторе, а описать в cpp
