// Повторение crtp 
// пример 1
#include<iostream> 

template <class Animal>
class AnimalBase {
public:
  void Sound() const {
    static_cast<const Animal*>(this)->Sound();
  }
};

class Cat : public AnimalBase<Cat> {
public:
  void Sound() const {
    std::cout << "Purr" << std::endl;
  }
};

class Dog : public AnimalBase<Dog> {
public:
  void Sound() const {
    std::cout << "Woof" << std::endl;
  }
};

template <class Animal>
void fun(const AnimalBase<Animal>& animal) {
  animal.Sound();
}

template <class T>
class Counter {
protected:
     inline static size_t count = 0;
public:
    static size_t GetCount() { return count;}
    Counter() { count++; }
    ~Counter() { count--; }
};

class A : public Counter<A> {};
class B : public Counter<B> {};

int main() {
    Cat cat;
    Dog dog;
    fun(cat);
    fun(dog);
    A a1, a2;
    B b1;
    std::cout << A::GetCount() << std::endl;
    std::cout << B::GetCount() << std::endl;
    return 0;
}


//пример 2 

template <class Derived>
class A {
public:
    void fun() {
        static_cast<Derived*>(this)->fun();
    }
};

class B: public A<B>{
public:
    void fun() {};
    // полиморфизм - есть клиенттский код который использует все наши наследники
};

class C: public A<C>{
public:
    void fun() {};
};


template<class D>
void DoSmthOnA(A<D>& a){ //нельзя передавать сам объект, может не удастся
    // сам объект не сможет принять вход объекты типа B и C - ошибка будет что не тот параметр хотим передать в функцию
    a.fun(); // здесь вызовется от A<D>
}


// универсальные ссылки
// sfinai
