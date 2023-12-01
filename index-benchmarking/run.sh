#!/bin/bash
cd "$(dirname "$0")"

mkdir -p build
cd build

export CC=/usr/bin/gcc
export CXX=/usr/bin/g++

cmake ..
cmake --build . --target $1
./$@
