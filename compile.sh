#!/bin/bash

if [[ $# -lt 1 ]] then
    echo "Usage: $0 <fileName>"
    exit 1
fi

g++ -I../src -L. -std=c++20 ../qs_cpp/$1.cpp libnon_qt_lib.a -o ../qs_cpp/$1.qse &&
../qs_cpp/$1.qse $1