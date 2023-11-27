# index-benchmarking

```bash
./run.sh s2-shapeindex path/to/geopackage
```

## Build

First, install these prerequisites.

```bash
apt-get update && apt-get upgrade
apt-get install libsqlite3 libssl-dev libproj-dev
```

### GDAL

```bash
cd libs/gdal
mkdir build && cd build
cmake ..
cmake --build .
cmake --build . --target install
```
