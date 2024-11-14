#include <iostream>

int main(int argc, char* argv[]) {

	for (int ct = 0; ct < argc; ct++) {
		if (ct == 0) continue;
		std::cout << "\e[37m" << argv[ct] << "\e[0m";
	}
	std::cout << std::endl;
}
