#include <stdio.h>
#include "ddstore.h"

using namespace std;

int main(int argc, char **argv) {
	if (argc != 2) {
		return 1;
	}
	DDStore st = DDStore(argv[1]);
	return 0;
}
