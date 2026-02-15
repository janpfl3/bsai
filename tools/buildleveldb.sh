#!/bin/bash
set -exo pipefail

LEVELDB_REPO=https://github.com/google/leveldb.git
LEVELDB_COMMIT=99b3c03b3284f5886f9ef9a4ef703d57373e61be

mkdir -p build && cd build

git clone --quiet --no-checkout $LEVELDB_REPO leveldb-src
(cd leveldb-src && git rev-parse HEAD && git checkout $LEVELDB_COMMIT)

cmake -S leveldb-src -B leveldb-bld \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_SHARED_LIBS=OFF \
  -DLEVELDB_BUILD_TESTS=OFF \
  -DLEVELDB_BUILD_BENCHMARKS=OFF

cmake --build leveldb-bld
cmake --install leveldb-bld --strip --prefix $PREFIX
