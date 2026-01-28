#!/bin/bash
set -exo pipefail

FILENAME=kdsingleapplication-1.2.0
ARCHIVE=$FILENAME.tar.gz
DIRNAME=KDSingleApplication-1.2.0

mkdir -p build && cd build

if [ ! -d $DIRNAME ]; then
    curl -s -L -o $ARCHIVE https://github.com/KDAB/KDSingleApplication/releases/download/v1.2.0/$ARCHIVE
    tar zxf $ARCHIVE
fi

cd $DIRNAME

qt-cmake -S . -B build \
    -DCMAKE_INSTALL_PREFIX=$PREFIX \
    -DKDSingleApplication_STATIC=true \
    -DKDSingleApplication_TESTS=false \
    -DKDSingleApplication_EXAMPLES=false \
    -DKDSingleApplication_DOCS=false

cmake --build build

cmake --install build --prefix $PREFIX
