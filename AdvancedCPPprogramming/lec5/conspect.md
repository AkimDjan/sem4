## Лекция 5

Особенности унифицированной инициализации

```cpp
class list_t{
public:
    list_t(){} //если добавить int сюда то станет немного лучше
};

class my_class_t {
public:
    int x = 42;
    my_class_t(list_t, list_t) {};
};

int main(int argc, char** argv){ 
    my_class_t m1(list_t(), list_t()); //сишный вариант, стараемся не использовать такой синтаксис
    std::cout<< m1.x <<"\n";
    my_class_t m2{list_t(), list_t()}; //создать экземпляр класса my_class2 и передать туда в конструктор
    std::cout<< m2.x <<"\n";
}
```

Двойная инициализация

Семантика копирования RVO (return value optimisation)

### Наследование
Расширение класса используя существующие классы

