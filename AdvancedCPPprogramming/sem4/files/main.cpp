#include "Some.h"

#include <iostream>

void fun(const Some& a) {
	std::cout << a.Do(6) << std::endl;
}

void fun2(Some a) {
	auto& i = a.Do(0);
	std::cout << a.Do(0) << std::endl;
}

int main() {
	Some a;
	fun(a);
	fun2(a);
	return 0;
}