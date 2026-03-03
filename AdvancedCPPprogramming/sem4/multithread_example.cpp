#include <iostream>
#include <thread>
#include <mutex>

class SomeWork {
public:
  void Add(int count) {
    {
                                                // Можно раскоментить и посмотреть id потоков. И что будет если не лочить мьютекс?
      //std::lock_guard<std::mutex> lg(m_);
      //std::cout << std::this_thread::get_id() << std::endl; 
    }
    for (int i = 0; i < count; ++i)
      shared_ += 1;
  }

  void AddWithMutex(int count) {
    {
      //std::lock_guard<std::mutex> lg(m_);
      //std::cout << std::this_thread::get_id() << std::endl;
    }
    for (int i = 0; i < count; ++i) {
      std::lock_guard<std::mutex> lg(m_);
      shared_ += 1;
    }
  }

  int GetShared() const {
    std::lock_guard<std::mutex> lg(m_);  // Что будет если мьютекс будет не mutable
    return shared_;
  }

  void Null() {
    shared_ = 0;
  }
protected:
  int shared_ = 0;
  mutable std::mutex m_;
};

int main() {
  SomeWork sw;
  std::cout << "Without mutex" << std::endl;
  for (int i = 0; i < 50000; ++i) {
    std::thread th1(&SomeWork::Add, &sw, 100);  // Запускаем в 2 потоках функцию Add
    std::thread th2(&SomeWork::Add, &sw, 100);
    th1.join();
    th2.join();  // Ждём пока оба потока закончат выполнение функции. Основной поток ничего это время не делает, а зря
    auto shared = sw.GetShared();
    if (shared != 200)
      std::cout << sw.GetShared() << std::endl;
    sw.Null();
  }
  std::cout << "and now with mutex" << std::endl;
  for (int i = 0; i < 50000; ++i) {
    std::thread th1(&SomeWork::AddWithMutex, &sw, 100);
    std::thread th2(&SomeWork::AddWithMutex, &sw, 100);
    th1.join();
    th2.join();
    auto shared = sw.GetShared();
    if (shared != 200)
      std::cout << sw.GetShared() << std::endl;
    sw.Null();
  }
  return 0;
}
