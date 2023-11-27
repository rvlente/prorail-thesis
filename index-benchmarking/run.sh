#!/bin/bash
cd "$(dirname "$0")"

if [[ "$1" =~ ^(geos-strtree|geos-quadtree|s2-pointindex|s2-shapeindex|h3-cindex)$ ]]
  then
    echo "Building $1"
  else
    echo "Please specifiy one of [geos-strtree, geos-quadtree, s2-shapeindex, h3-customindex]"
    exit 1
fi

mkdir -p build
cd build

cmake ..
cmake --build . --target $1
./$@
