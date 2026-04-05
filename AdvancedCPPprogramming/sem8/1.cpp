#include <iostream>
#include <vector>
#include <algorithm>
#include <memory>
#include <string>
//интерфейс - набор публичных методов класса, которые могут быть вызваны извне
//пример 1 
class PersonList;
class MusicList;

class Person {
  std::string name;
  friend PersonList;
};

class PersonList {
  std::vector<Person> persons;
public:
  void sort() {
    std::sort(persons.begin(), persons.end(),
      [](Person lhs, Person rhs) {return lhs.name < rhs.name; });
  }
};

class Music {
  std::string name;
  friend MusicList;
};

class MusicList {
  std::vector<Music> musics;
public:
  void sort() {
    std::sort(musics.begin(), musics.end(),
      [](Music lhs, Music rhs) {return lhs.name < rhs.name; });
  }
};
// Здесь нарушен DIP, так как классы PersonList и MusicList зависят от конкретных классов Person и Music, которые 
// являются деталями реализации. Если мы захотим изменить структуру данных для хранения Person или Music, нам придется 
// изменять код в PersonList и MusicList, что нарушает принцип инверсии зависимостей.
// избавляемся следующим образом: создаем интерфейс для сортировки, который будет использоваться в обоих классах, и реализуем 
// его в каждом классе отдельно. Таким образом, PersonList и MusicList будут зависеть от абстрактного интерфейса, а не от 
// конкретных классов Person и Music.
// friend убираем 
// single resposibility тоже нарушен, так как классы PersonList и MusicList выполняют две функции: хранение данных и сортировку.
// Лучше разделить эти функции на два отдельных класса, один для хранения данных, а другой для сортировки.
// есть метод find у алгоритмов и контейнеров, который позволяет найти элемент в контейнере, не нарушая принципа инверсии зависимостей.
// Например, мы можем использовать std::find для поиска элемента в векторе, не завися от конкретного типа данных, который хранится в векторе.
// Это позволяет нам сохранять абстрактность и не нарушать DIP.
// Наприме у set, unordered set и тд
//

// пример 2
class Speakers {
public:
  void TurnSound();
  void IncreaseVolume();
  void DecreaseVolume();
};

class Light {
public:
  void TurnLight();
};

class RC {
  std::vector<std::unique_ptr<Light>> lights_;
  std::unique_ptr<Speakers> speakers;
public:
  void TurnLight(unsigned int i) { lights_[i]->TurnLight(); }
  void TurnSound() { speakers->TurnSound(); }
  void IncreaseVolume() { speakers->IncreaseVolume(); }
  void DecreaseVolume() { speakers->DecreaseVolume(); }
};
// здесь нет никаких нарушений принципов SOLID, так как классы Speakers и Light являются абстрактными интерфейсами, которые используются в классе RC, и RC не зависит от конкретных реализаций этих классов. RC зависит от абстрактных интерфейсов, что позволяет нам легко изменять реализацию Speakers и Light без необходимости изменять код в RC. Также RC выполняет только одну функцию - управление звуком и светом, что соответствует принципу единственной ответственности.
// здесь представлен паттерн проектирования - фасад, который предоставляет простой интерфейс для управления сложной системой, скрывая детали реализации от клиента. Это позволяет нам легко использовать систему, не заботясь о том, как она работает внутри, и также позволяет нам изменять реализацию системы без необходимости изменять код клиента.
// 0.0 

//пример 3
class Sorter {
  std::vector<int>* v_;
public:
  void sort() {
    if (v_->size() < 10)
      std::bubble_sort(v_->begin(), v_->end());
    else
      std::sort(v_->begin(), v_->end());
  }
};

enum class DataType {
 Undefined = 0,
// another types
};


template <class T>
class Data {
DataType dt_;
public:
T data ;
// operators to work with Data
void ProcessData() {
  switch (dt_) {
   // processing depends from types
  }
}
};
// здесь нарушен liskov substitution principle, так как если мы заменим Data<int> на Data<float>, то код, 
// который использует Data<int>, может не работать правильно, так как он может ожидать определенные операции
// и поведение, которые не будут работать с Data<float>. Это может привести к тому, что код будет ломаться или 
// работать неправильно при замене одного типа данных на другой. Чтобы исправить эту проблему, мы можем использовать 
// шаблоны и полиморфизм для обеспечения того, чтобы код мог работать с любыми типами данных без нарушения LSP.
// также нарушен open/closed principle, так как если мы захотим добавить новый тип данных, нам придется изменять 
// код в классе Data, что нарушает принцип открытости/закрытости. Чтобы исправить эту проблему, мы можем использовать 
// шаблоны и полиморфизм для обеспечения того, чтобы код мог работать с любыми типами данных без необходимости изменять 
//код в классе Data.