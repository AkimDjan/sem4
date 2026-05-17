#include <iostream>

enum NPCUnits{
    Warior, //0
    Mag, //1, можно переопределить Mag = 10
    Rogue,  //2
};

enum class NPCClass{
    Warior, //0
    Mag, //1, можно переопределить Mag = 10
    Rogue,  //2
};

int main() {
    int a = Warior;
    std::cout<<a<<"\n";
}


