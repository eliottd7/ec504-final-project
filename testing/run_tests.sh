#!/bin/sh

cd testing/

for file in `find . -name '*.run'`; do
	echo "Running $file"
    ./"$file"
done
