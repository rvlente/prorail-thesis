# index-benchmarking

## Build

First, install these prerequisites.

```bash
apt-get update && apt-get upgrade
apt-get install libsqlite3-dev libssl-dev libtiff-dev libgtest-dev
```

Pull submodules.

```bash
git submodule update --init --recursive
```

### PROJ

```bash
cd libs/proj
mkdir build && cd build
cmake --DBUILD_TESTING ..
cmake --build .
sudo cmake --build . --target install
```

### gperftools

```bash
cd libs/gperftools
./autogen.sh
./configure
make
sudo make install
make clean
make distclean
```

## Run experiments

```bash
./run.sh exp11
```
