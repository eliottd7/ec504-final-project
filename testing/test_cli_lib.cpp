/*
	This file will test the library functions in cli.h
	For the actual command line interface testing, see test_cli.run
*/

#include "cli_funcs.h"
#include <iostream>
#include <vector>
#include <string>

#define RED_TEXT ("\e[31m")
#define END_FMT ("\e[0m\n")

int main() {

	int failures = 0;

	// 1. CLI_error()
	bool caught = false;
	try {
		CLI_error();
	}
	catch (string err) {
		caught = true;
	}
	if (caught != true) {
		std::cout << RED_TEXT << "Failed to throw the error when error explicitly invoked" << END_FMT;
	}

	// 2. test_path()


}
