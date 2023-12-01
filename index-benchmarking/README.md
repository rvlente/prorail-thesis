# index-benchmarking

```bash
./run.sh s2-shapeindex path/to/geopackage
```

## Build

First, install these prerequisites.

```bash
apt-get update && apt-get upgrade
apt-get install libsqlite3 libssl-dev
```

### PROJ

```bash
cd libs/proj
mkdir build && cd build
cmake --DBUILD_TESTING ..
cmake --build .
sudo cmake --build . --target install
```

### GDAL

```bash
cd libs/gdal
mkdir build && cd build
cmake ..
cmake --build .
sudo cmake --build . --target install
```
