#include <iostream>

int main() {
	std::cout << "HelloWorld" << std::endl;
	int a = 10;
	int b = a/3;
	int c = 13;
	c += b;
	std::cout << "a = " << a << std::endl;
	std::cout << "c = " << c << std::endl;
	return 0;
}