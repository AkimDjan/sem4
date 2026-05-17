#include <iostream>

class Unit{
    unsigned int hp_;
public:
    unsigned int GetHP(){return hp_;}
};
class Horseman:public Unit {};
class Archer:public Unit {};
class HorsemanArcher:public Horseman, public Archer {};
Unit* GetNextUnit(Unit *u) {
    std::cout << "First Unit " << u << std::endl;
    Horseman* h = static_cast<Horseman*>(u);
    std::cout << "Horseman " << h << std::endl;
    HorsemanArcher* ha = static_cast<HorsemanArcher*>(h);
    std::cout << "Horseman Archer " << ha << std::endl;
    Archer* A = static_cast<Archer*>(ha);
    return static_cast<Unit *>(A);
}

int main() {
    Unit* u = new Unit();
    std::cout << u << "\n";
    std::cout << GetNextUnit(u) << "\n";
}