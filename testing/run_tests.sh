#!/bin/sh

cd testing/

FAILURES=0
for file in `find . -name '*.run'`; do
    ./$file
    FAILURES=$(($FAILURES + $?))
done
printf "Overall, \e[34m%d\e[0m tests failed." $FAILURES
