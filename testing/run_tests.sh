#!/bin/sh

cd testing/

total_fails=0
for file in `find . -name '*.run'`; do
	echo "Running $file"
    ./"$file"
    total_fails=$((total_fails + $?))
done
echo "Overall, $total_fails tests failed."
