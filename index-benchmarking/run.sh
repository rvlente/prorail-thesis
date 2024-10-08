#!/bin/bash
set -e
cd "$(dirname "$0")"

export CC=/usr/bin/gcc
export CXX=/usr/bin/g++
export OPENSSL_ROOT_DIR=/usr/lib/x86_64-linux-gnu/
export PERFTOOLS_VERBOSE=-1000 # disable gperftools dump logs
export HEAP_PROFILE_INUSE_INTERVAL=0 # disable dump intervals
export HEAP_PROFILE_ALLOCATION_INTERVAL=0

mkdir -p results

rm -rf tmp
mkdir -p tmp
mkdir -p build
cd build

cmake ..
cmake --build . --target $1

cd ..

./build/$@

rm -rf tmp
