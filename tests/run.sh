#!/bin/bash

cd `dirname $0`
tests=`ls -d * | grep -v run.sh`
for test in $tests; do
    echo "=================================================== $test"
    cd $test
    g++ -o main main.cpp || exit 1
    diff <(./main) expected-output.txt || exit 2
    cd ..
    echo "PASSED"
done

