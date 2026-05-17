# SOLID: понятное и практическое руководство

**SOLID** — это набор из 5 принципов объектно-ориентированного проектирования, который помогает писать код:

* проще для понимания;
* легче для изменения;
* удобнее для тестирования;
* менее хрупкий;
* лучше расширяемый.

Название складывается из первых букв пяти принципов:

* **S** — Single Responsibility Principle
* **O** — Open/Closed Principle
* **L** — Liskov Substitution Principle
* **I** — Interface Segregation Principle
* **D** — Dependency Inversion Principle

---

# 1. Single Responsibility Principle (SRP)

## Принцип единственной ответственности

### Формулировка

**У класса должна быть только одна причина для изменения.**

### Что это значит на практике

Класс не должен одновременно:

* считать данные;
* сохранять их;
* работать с сетью;
* рисовать интерфейс.



## Пример нарушения SRP

```python
class Report:
    def __init__(self, data):
        self.data = data

    def generate_text(self):
        return f"Report: {self.data}"

    def save_to_file(self, path):
        with open(path, "w") as f:
            f.write(self.generate_text())
```

### Проблема

Класс делает две вещи:

* формирует отчет;
* сохраняет его.



## Пример соблюдения SRP

```python
class Report:
    def __init__(self, data):
        self.data = data

    def generate_text(self):
        return f"Report: {self.data}"


class ReportSaver:
    def save_to_file(self, report_text, path):
        with open(path, "w") as f:
            f.write(report_text)
```



## Как определить нарушение SRP

Признаки:

* класс делает слишком много;
* есть методы из разных областей;
* сложно описать класс одной функцией.

Проверка:

> Можно ли описать класс без слова "и"?



# 2. Open/Closed Principle (OCP)

## Принцип открытости/закрытости

### Формулировка

**Открыт для расширения, закрыт для изменения.**



## Пример нарушения OCP

```python
class DiscountCalculator:
    def calculate(self, price, customer_type):
        if customer_type == "regular":
            return price
        elif customer_type == "vip":
            return price * 0.9
        elif customer_type == "premium":
            return price * 0.8
```

### Проблема

Добавление нового типа требует изменения кода.


## Пример соблюдения OCP

```python
class DiscountStrategy:
    def calculate(self, price):
        return price


class VipDiscount(DiscountStrategy):
    def calculate(self, price):
        return price * 0.9


class PremiumDiscount(DiscountStrategy):
    def calculate(self, price):
        return price * 0.8
```

```python
def checkout(price, strategy):
    return strategy.calculate(price)
```



## Как определить нарушение OCP

Признаки:

* много `if/elif`;
* нужно менять старый код при добавлении нового поведения.

Проверка:

> Можно ли добавить новый вариант без изменения старого кода?



# 3. Liskov Substitution Principle (LSP)

## Принцип подстановки Лисков

### Формулировка

**Подкласс должен полностью заменять базовый класс.**



## Пример нарушения LSP

```python
class Bird:
    def fly(self):
        print("Flying")


class Penguin(Bird):
    def fly(self):
        raise Exception("Can't fly")
```

### Проблема

Пингвин ломает поведение базового класса.



## Исправление

```python
class Bird:
    pass


class FlyingBird(Bird):
    def fly(self):
        print("Flying")


class Penguin(Bird):
    def swim(self):
        print("Swimming")
```



## Как определить нарушение LSP

Признаки:

* подкласс меняет смысл методов;
* код ломается при подстановке.

Проверка:

> Можно ли заменить базовый класс наследником без ошибок?



# 4. Interface Segregation Principle (ISP)

## Принцип разделения интерфейсов

### Формулировка

**Не заставляй использовать лишние методы.**



## Пример нарушения ISP

```python
class Worker:
    def work(self): pass
    def eat(self): pass
    def sleep(self): pass


class Robot(Worker):
    def work(self): pass
    def eat(self): raise NotImplementedError
    def sleep(self): raise NotImplementedError
```

---

## Пример соблюдения ISP

```python
class Workable:
    def work(self): pass


class Eatable:
    def eat(self): pass


class Human(Workable, Eatable):
    pass


class Robot(Workable):
    pass
```



## Как определить нарушение ISP

Признаки:

* интерфейс слишком большой;
* есть ненужные методы;
* методы выбрасывают `NotImplementedError`.

Проверка:

> Все ли методы реально нужны всем реализациям?



# 5. Dependency Inversion Principle (DIP)

## Принцип инверсии зависимостей

### Формулировка

**Зависим от абстракций, а не от конкретных реализаций.**



## Пример нарушения DIP

```python
class MySQLDatabase:
    def save(self, data):
        pass


class UserService:
    def __init__(self):
        self.db = MySQLDatabase()
```



## Пример соблюдения DIP

```python
class Database:
    def save(self, data):
        pass


class MySQLDatabase(Database):
    def save(self, data):
        pass


class UserService:
    def __init__(self, db: Database):
        self.db = db
```



## Как определить нарушение DIP

Признаки:

* жесткие зависимости (`new`, прямые импорты);
* сложно заменить реализацию.

Проверка:

> Можно ли подменить реализацию без изменения кода?



# Краткое резюме

* **SRP** — одна ответственность
* **OCP** — расширяем без изменений
* **LSP** — наследник = замена
* **ISP** — маленькие интерфейсы
* **DIP** — зависимость от абстракций

---

# Быстрый чек-лист

## SRP

* один класс = одна задача?

## OCP

* можно добавить без изменения?

## LSP

* наследник не ломает поведение?

## ISP

* нет лишних методов?

## DIP

* нет жестких зависимостей?

---

# Итог

SOLID делает код:

* понятнее;
* гибче;
* легче в поддержке;
* удобнее для тестирования.

Главная идея: **слабая связанность + высокая модульность**.
