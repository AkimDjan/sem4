// у unique ptr нет конструктора копирования, но есть перемещающий
// shared ptr есть конструктор копирования - он не удаляется пока есть хотя бы одна ссылка на объект (указатель н)
// weak - смысл - наблюдени
// Вновь поговорим про наследование и полиморфизм
// пусть у нас будет класс Unit
#include <iostream>

class Unit{
    unsigned int hp_;
public:
    unsigned int GetHP(){return hp_;}
};

class Horseman:public Unit {

};

class Archer:public Unit {

};

//Наследуемся от unit потому что у нас будут какие-то виртуальные методы, мы их получаем (в том числе и поля получаем)
// можем единообразно все обрабатывать 
// наследование сэкономим время на написание дублирующегося кода

class HorsemanArcher:public Horseman, Archer {

};
//в памяти будет horseman|archer|horsemanarhcer
// а если ближе unit|horseman|unit|archer|horsemanarcher
// происходит дублирование данных - больше памяти тратим!!!
// Может быть такое что где-то захардкожено Archer::hp_, Horseman::hp_, и чел может умереть как лучник но не как всадник
// Можно решить проблему путем Виртуального наследования
// когда писали Archer'а надо было написать :

class Archer: virtual public Unit {

};
class HorsemanArcher:public Horseman, public Archer {

};

//когда мы пытаемся от множественного наследования унаследовать (ромбовидное наследование)

// static cast позволяет брать определенные части укзаателя (делает безопасное преобразование в пределах иерархии)
HorsemanArcher* ha = new HorsemanArcher;
Horseman* pH = static_cast<Horseman*>(ha);
// приходит указатель на первого юнита, надо вернуть указатель на второго unita

//как обрабатывает C++ пустые классы? он добавляет пустой байт

class Unit{
    unsigned int hp_;
public:
    unsigned int GetHP(){return hp_;}
};
class Horseman:public Unit {};
class Archer:public Unit {};
class HorsemanArcher:public Horseman, public Archer {};
Unit* GetNextUnit(Unit *u) {
    std::cout << "First Unit" << u << std::endl;
    Horseman* h = static_cast<Horseman*>(u);
    std::cout << "Horseman" << h << std::endl;
    HorsemanArcher* ha = static_cast<HorsemanArcher*>(h);
    std::cout << "Horseman Archer" << ha << std::endl;
    Archer* A = static_cast<Archer*>(ha);
    return static_cast<Unit *>(A);
}

int main() {
    Unit* u = new Unit();
    std::cout << u << "\n";
    std::cout << GetNextUnit(u) << "\n";
}

// как работает dynamic cast?
// есть таблица виртуальных функций и классы
// если не сможем привести ссылки - получим исключение 
// работает это через RTTI - real time type information. Мы на каждый класс получаем не только таблицу виртуальных функций но и RTTI
// в играх популярна ECS - Entity Component System
