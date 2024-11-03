#include <stdio.h>
#include "ddstore.cpp"

using namespace std;

int main(int argc, char **argv) {
	if (argc != 2) {
		return 1;
	}
	ddstore st = ddstore(argv[1]);
	return 0;
}
