#!/bin/bash

mkdir validdir
echo "hello world" > validdir/validfile.txt
mkdir validemptydir
g++ --std=c++20 -Og -g -o test_cli_lib.test -I ../src test_cli_lib.cpp ../src/ddstore.cpp
gdb test_cli_lib.test

# run "validdir/validfile.txt" "validdir" "tmplocker" "validemptydir"
