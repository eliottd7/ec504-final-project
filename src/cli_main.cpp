#include "cli_funcs.h"
#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>
//#include <chrono>

using namespace std;
using namespace chrono;

int main(int argc, char** argv) {
    //auto start = high_resolution_clock::now();
    int val = call_dd(argc, argv);
    //auto stop = high_resolution_clock::now();
    //cout << "Execution time, microseconds: " << duration_cast<microseconds>(stop - start).count() << endl;
    return val;
}