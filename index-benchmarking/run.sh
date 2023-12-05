#!/bin/bash
cd "$(dirname "$0")"

export CC=/usr/bin/gcc
export CXX=/usr/bin/g++

HEAPPROFILE_DIR="heapprofile/$1-$(date +%Y%m%dT%H%M%S)"
mkdir -p $HEAPPROFILE_DIR

mkdir -p build
cd build

cmake ..
cmake --build . --target $1

# gperftools settings
export HEAPPROFILE="../$HEAPPROFILE_DIR/$1"
export PERFTOOLS_VERBOSE=-1000 # disable dump logs
./$@

cd ..

echo "Analyzing heap profile... (will print relevant entry)"
pprof --text --inuse_space build/$1 $HEAPPROFILE_DIR/$(ls $HEAPPROFILE_DIR | tail -1) | grep _build_
