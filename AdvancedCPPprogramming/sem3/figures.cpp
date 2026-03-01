#include <iostream>
#include <memory>
#include <vector>

class Figure {
public:
  virtual ~Figure() {}
  virtual float GetPerimetr() const = 0;
  virtual std::string GetName() const = 0;
};

class Circle : public Figure {
public:
  explicit Circle(float radius) : radius_(radius) {}
  float GetPerimetr() const override {return 2 * pi * radius_;}
  std::string GetName() const override { return "circle"; }
private:
  float radius_ = 0;
  static constexpr float pi = 3.14159;
};

class Rectangle : public Figure {
public:
  Rectangle(float l, float w) : l_(l), w_(w) {}
  float GetPerimetr() const override { return 2 * (l_ + w_); }
  std::string GetName() const override { return "rectangle"; }
private:
  float l_ = 0, w_ = 0;
};

class Triangle : public Figure {
public:
  Triangle(float a, float b, float c) : a_(a), b_(b), c_(c) {}
  float GetPerimetr() const override { return a_ + b_ + c_; }
  std::string GetName() const override { return "triangle"; }
private:
  float a_ = 0, b_ = 0, c_ = 0;
};

std::unique_ptr<Figure> CreateFigure(const std::vector<float>& params) {
  switch (params.size()) {
  case 1:
    return std::make_unique<Circle>(params[0]);
  case 2:
    return std::make_unique<Rectangle>(params[0], params[1]);
  case 3:
    return std::make_unique<Triangle>(params[0], params[1], params[2]);
  default:
    std::cout << "No Figure for Params" << std::endl;
  }
  return nullptr;
}

void TryAddFigure(std::vector<std::unique_ptr<Figure>>& figures, const std::vector<float>& params) {
    auto fig = CreateFigure(params);
    if (fig) {figures.emplace_back(std::move(fig));} //std::move создает rvalue ссылку
    else{std::cout << "Failed to create figure for params\n";}
}

int main() {
  std::vector<std::unique_ptr<Figure>> figures;
  figures.reserve(4); //увеличивает capacity
  TryAddFigure(figures, { 1 }); //от push_back отличается тем что push_back сначала создает элемент а потом уже присоединяет
  TryAddFigure(figures, { 1,2});
  TryAddFigure(figures, { 2,2,3 });
  TryAddFigure(figures, {}); //<- тут возвращается nullptr и мы не можем найти для него периметр
  for (const auto& fig : figures) {
    std::cout << "Perimetr of " << fig->GetName() << " is " << fig->GetPerimetr() << std::endl;
  }
  return 0;
}
