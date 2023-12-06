#!/bin/bash
set -e
cd "$(dirname "$0")"

export CC=/usr/bin/gcc
export CXX=/usr/bin/g++
export OPENSSL_ROOT_DIR=/usr/lib/x86_64-linux-gnu/
export PERFTOOLS_VERBOSE=-1000 # disable gperftools dump logs

mkdir -p heapprofiles
mkdir -p build
cd build

rm -rf heapprofile
mkdir -p heapprofile

cmake ..
cmake --build . --target $1

./$@

echo "Analyzing heap profile dumps..."
pprof --text --inuse_space $1 heapprofile/$(ls heapprofile | grep strtree | tail -1) | grep _build_strtree
pprof --text --inuse_space $1 heapprofile/$(ls heapprofile | grep quadtree | tail -1) | grep _build_quadtree

cd ..
cp -r build/heapprofile heapprofiles/$1-$(date +%Y%m%dT%H%M%S)

