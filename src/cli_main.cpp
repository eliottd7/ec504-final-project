#include "cli_funcs.h"
#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>

using namespace std;

int main(int argc, char** argv) {
    vector<string> CLI_input;
    for ( int i = 1; i < argc; i++ ) {
        string s(argv[i]);
        CLI_input.push_back(s);
    }
    try {
        CLI_parser(CLI_input);
    } catch ( string error ) {
        cout << error << endl;
    }
    return 0;
}