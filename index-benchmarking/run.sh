#!/bin/bash
cd "$(dirname "$0")"

mkdir -p build
cd build

export CC=/usr/bin/gcc
export CXX=/usr/bin/g++
export HEAPPROFILE="../heapprofile/$1-$(date +%Y-%m-%d_%H:%M:%S)"

cmake ..
cmake --build . --target $1
./$@
